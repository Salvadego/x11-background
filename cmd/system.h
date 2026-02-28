#ifndef SYSTEM_H_URBBF5FI
#define SYSTEM_H_URBBF5FI

#include "config.h"
#include "math.h"
#include "types.h"
#include "utils.h"
#include <limits.h>
#include <math.h>
#include <raylib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
        IntArray   colors;
        FloatArray x, y, z, vx, vy, vz;
} Particles;

#define px(s, i)     (s)->particles->x[(i)]
#define py(s, i)     (s)->particles->y[(i)]
#define pvx(s, i)    (s)->particles->vx[(i)]
#define pvy(s, i)    (s)->particles->vy[(i)]
#define pz(s, i)     (s)->particles->z[(i)]
#define pvz(s, i)    (s)->particles->vz[(i)]
#define pcolor(s, i) (s)->particles->colors[(i)]

typedef struct {
        size_t cols, rows;
        float  cell_size;

        IntArray cell_start;
        IntArray cell_count;
        IntArray indices;
} Grid;

typedef struct {
        size_t total_particles;
        size_t total_cells;
        size_t non_empty_cells;
        size_t max_particles_in_cell;
        float  avg_particles_per_cell;
        float  avg_particles_per_nonempty_cell;
        size_t estimated_interactions_per_frame;
} GridStats;

typedef struct {
        Particles *particles;
        FloatArray matrix;
        size_t     count;
        size_t     types_count;
        Grid      *grid;
} System;

GridStats ComputeGridStats(System *s) {
        GridStats stats = {0};

        Grid  *g           = s->grid;
        size_t total_cells = g->cols * g->rows;

        stats.total_particles = s->count;
        stats.total_cells     = total_cells;

        size_t total_particles_in_cells = 0;

        for (size_t c = 0; c < total_cells; ++c) {

                int count = g->cell_count[c];

                if (count > 0)
                        stats.non_empty_cells++;

                if ((size_t)count > stats.max_particles_in_cell)
                        stats.max_particles_in_cell = count;

                total_particles_in_cells += count;
        }

        stats.avg_particles_per_cell =
            (float)total_particles_in_cells / (float)total_cells;

        if (stats.non_empty_cells > 0)
                stats.avg_particles_per_nonempty_cell =
                    (float)total_particles_in_cells /
                    (float)stats.non_empty_cells;

        float avg_neighbors = stats.avg_particles_per_cell * 9.0f;

        stats.estimated_interactions_per_frame =
            (size_t)(avg_neighbors * (float)s->count);

        return stats;
}

void PrintGridStats(GridStats s) {
        printf("Particles: %zu\n", s.total_particles);
        printf("Total cells: %zu\n", s.total_cells);
        printf("Non-empty cells: %zu\n", s.non_empty_cells);
        printf("Max per cell: %zu\n", s.max_particles_in_cell);
        printf("Avg per cell: %.2f\n", s.avg_particles_per_cell);
        printf("Avg per non-empty cell: %.2f\n",
               s.avg_particles_per_nonempty_cell);
        printf("Estimated interactions/frame: %zu\n",
               s.estimated_interactions_per_frame);
        printf("-------------------------------------------------\n");
}

float GetRandomFloatValue(void) {
        return (float)GetRandomValue(0, INT_MAX) / (float)INT_MAX;
}

FloatArray makeRandomMatrix(Arena *a, size_t types_count) {
        size_t     size = types_count * types_count;
        FloatArray rows =
            arena_alloc(a, size * sizeof(float), AlignOfType(float));

        for (size_t i = 0; i < size; ++i) {
                float rv = GetRandomFloatValue() * 2 - 1;
                rows[i]  = rv;
        }

        return rows;
}

void InitGrid(Arena *arena, Grid *g, size_t particle_count, float cell_size) {
        g->cell_size = cell_size;

        g->cols = (size_t)(1.0f / cell_size) + 1;
        g->rows = (size_t)(1.0f / cell_size) + 1;

        size_t total_cells = g->cols * g->rows;

        g->cell_start =
            arena_alloc(arena, total_cells * sizeof(int), AlignOfType(int));

        g->cell_count =
            arena_alloc(arena, total_cells * sizeof(int), AlignOfType(int));

        g->indices =
            arena_alloc(arena, particle_count * sizeof(int), AlignOfType(int));
}

static inline size_t GridIndex(Grid *g, float x, float y) {
        size_t cx = (size_t)(x / g->cell_size);
        size_t cy = (size_t)(y / g->cell_size);

        if (cx >= g->cols)
                cx = g->cols - 1;
        if (cy >= g->rows)
                cy = g->rows - 1;

        return cy * g->cols + cx;
}

void GridBuild(System *s) {
        Grid  *g           = s->grid;
        size_t total_cells = g->cols * g->rows;

        memset(g->cell_count, 0, total_cells * sizeof(int));

        // Count
        for (size_t i = 0; i < s->count; ++i) {

                size_t cx = (size_t)(s->particles->x[i] / g->cell_size);
                size_t cy = (size_t)(s->particles->y[i] / g->cell_size);

                if (cx >= g->cols)
                        cx = g->cols - 1;
                if (cy >= g->rows)
                        cy = g->rows - 1;

                size_t cell = cy * g->cols + cx;

                g->cell_count[cell]++;
        }

        // Prefix sum
        int sum = 0;
        for (size_t c = 0; c < total_cells; ++c) {
                g->cell_start[c] = sum;
                sum += g->cell_count[c];
                g->cell_count[c] = 0;
        }

        // Fill
        for (size_t i = 0; i < s->count; ++i) {

                size_t cx = (size_t)(s->particles->x[i] / g->cell_size);
                size_t cy = (size_t)(s->particles->y[i] / g->cell_size);

                if (cx >= g->cols)
                        cx = g->cols - 1;
                if (cy >= g->rows)
                        cy = g->rows - 1;

                size_t cell = cy * g->cols + cx;

                int index         = g->cell_start[cell] + g->cell_count[cell]++;
                g->indices[index] = (int)i;
        }
}

void InitParticles(Arena     *arena,
                   Particles *p,
                   size_t     count,
                   size_t     types_count) {

        p->colors = arena_alloc(arena, count * sizeof(int), AlignOfType(int));
        p->x  = arena_alloc(arena, count * sizeof(float), AlignOfType(float));
        p->y  = arena_alloc(arena, count * sizeof(float), AlignOfType(float));
        p->z  = arena_alloc(arena, count * sizeof(float), AlignOfType(float));
        p->vx = arena_alloc(arena, count * sizeof(float), AlignOfType(float));
        p->vy = arena_alloc(arena, count * sizeof(float), AlignOfType(float));
        p->vz = arena_alloc(arena, count * sizeof(float), AlignOfType(float));

        for (size_t i = 0; i < count; ++i) {
                int color    = GetRandomValue(0, types_count);
                p->colors[i] = color;

                float rx = GetRandomFloatValue();
                float ry = GetRandomFloatValue();
                float rz = GetRandomFloatValue();
                float z  = 0.0f;

                p->x[i]  = rx;
                p->y[i]  = ry;
                p->z[i]  = rz;
                p->vx[i] = z;
                p->vy[i] = z;
                p->vz[i] = z;
        }
}

void InitSystem(Arena     *arena,
                System    *s,
                Particles *p,
                Grid      *g,
                size_t     count,
                size_t     types_count) {
        s->matrix      = makeRandomMatrix(arena, types_count);
        s->particles   = p;
        s->grid        = g;
        s->count       = count;
        s->types_count = types_count;

        InitParticles(arena, p, count, types_count);
        InitGrid(arena, g, count, appConfig.max_radius);
}

static inline float calcForce(float r, float a) {
        const float beta = 0.3;
        if (r < beta) {
                return r / beta - 1;
        } else if (beta < r && r < 1) {
                return a * (1 - fabsf(2 * r - 1 - beta) / (1 - beta));
        } else {
                return 0;
        }
}

static inline void ParticlesUpdate(System *restrict s) {
        GridBuild(s);

        // GridStats stats = ComputeGridStats(s);
        // PrintGridStats(stats);

        Particles *restrict p      = s->particles;
        FloatArray restrict matrix = s->matrix;
        FloatArray restrict px_arr = p->x;
        FloatArray restrict py_arr = p->y;
        FloatArray restrict vx_arr = p->vx;
        FloatArray restrict vy_arr = p->vy;
        IntArray restrict col_arr  = p->colors;

        Grid       *g        = s->grid;
        const float max_r2   = appConfig.max_radius * appConfig.max_radius;
        const float invMaxR2 = 1.0f / max_r2;

        for (size_t i = 0; i < s->count; ++i) {
                float totalForceX = 0.0f;
                float totalForceY = 0.0f;

                size_t      cx = (size_t)(px_arr[i] / g->cell_size);
                size_t      cy = (size_t)(py_arr[i] / g->cell_size);
                const float xi = px_arr[i];
                const float yi = py_arr[i];
                if (cx >= g->cols)
                        cx = g->cols - 1;
                if (cy >= g->rows)
                        cy = g->rows - 1;

                for (int dy = -1; dy <= 1; ++dy) {
                        for (int dx = -1; dx <= 1; ++dx) {

                                int nx = (int)cx + dx;
                                int ny = (int)cy + dy;

                                if (nx < 0)
                                        nx += g->cols;
                                if (nx >= (int)g->cols)
                                        nx -= g->cols;

                                if (ny < 0)
                                        ny += g->rows;
                                if (ny >= (int)g->rows)
                                        ny -= g->rows;

                                const size_t neighbor_cell = ny * g->cols + nx;

                                const int start = g->cell_start[neighbor_cell];
                                const int count = g->cell_count[neighbor_cell];

                                for (int k = 0; k < count; ++k) {

                                        const int j = g->indices[start + k];

                                        if ((size_t)j == i)
                                                continue;

                                        interaction_count++;

                                        float rx = px_arr[j] - xi;
                                        float ry = py_arr[j] - yi;

                                        if (rx > 0.5f)
                                                rx -= 1.0f;
                                        if (rx < -0.5f)
                                                rx += 1.0f;

                                        if (ry > 0.5f)
                                                ry -= 1.0f;
                                        if (ry < -0.5f)
                                                ry += 1.0f;
                                        const float dist2 = rx * rx + ry * ry;

                                        if (dist2 > 0 && dist2 < max_r2) {
                                                const int color_i = col_arr[i];
                                                const float *row =
                                                    &matrix[color_i *
                                                            s->types_count];

                                                const int color_j = col_arr[j];
                                                const float relation =
                                                    row[color_j];

                                                const float r =
                                                    dist2 * invMaxR2;
                                                const float f =
                                                    calcForce(r, relation);

                                                const float invDist =
                                                    1.0f / sqrtf(dist2);

                                                totalForceX += rx * invDist * f;
                                                totalForceY += ry * invDist * f;
                                        }
                                }
                        }
                }

                totalForceX *= appConfig.max_radius * appConfig.force_factor;
                totalForceY *= appConfig.max_radius * appConfig.force_factor;

                vx_arr[i] *= appConfig.damping;
                vy_arr[i] *= appConfig.damping;

                vx_arr[i] += totalForceX * appConfig.dt;
                vy_arr[i] += totalForceY * appConfig.dt;
        }

        // Integrate positions
        for (size_t i = 0; i < s->count; ++i) {
                px_arr[i] += vx_arr[i] * appConfig.dt;
                py_arr[i] += vy_arr[i] * appConfig.dt;

                if (px_arr[i] < 0.0f)
                        px_arr[i] += 1.0f;
                if (px_arr[i] >= 1.0f)
                        px_arr[i] -= 1.0f;

                if (py_arr[i] < 0.0f)
                        py_arr[i] += 1.0f;
                if (py_arr[i] >= 1.0f)
                        py_arr[i] -= 1.0f;
        }
}

static inline void ParticlesDraw(System *s) {
        for (size_t i = 0; i < s->count; ++i) {
                const int screenX =
                    (int)(px(s, i) * (float)appConfig.window_width);
                const int screenY =
                    (int)(py(s, i) * (float)appConfig.window_height);

                float hue = 360 * ((float)pcolor(s, i) / (float)s->types_count);
                Color color = ColorFromHSV(hue, 0.8, 1.0);
                DrawRectangle(screenX,
                              screenY,
                              appConfig.particle_size,
                              appConfig.particle_size,
                              color);
        }
}

#endif /* end of include guard: SYSTEM_H_URBBF5FI */
