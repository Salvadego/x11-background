#ifndef SYSTEM_H_URBBF5FI
#define SYSTEM_H_URBBF5FI

#include "config.h"
#include "math.h"
#include "types.h"
#include "utils.h"
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <limits.h>
#include <math.h>
#define Font RayFont
#include <raylib.h>
#undef Font
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

        size_t total_cells;

        IntArray cell_start;
        IntArray cell_count;
} Grid;

typedef struct {
        Particles *particles;
        FloatArray matrix;
        size_t     count;
        size_t     types_count;
        Grid      *grid;
        Arena     *temp_arena;
} System;

float GetRandomFloatValue(void) {
        return (float)GetRandomValue(0, 100) / (float)100;
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

void InitGrid(Arena *arena, Grid *g, float cell_size) {
        g->cell_size = cell_size;

        g->cols = (size_t)(1.0f / cell_size) + 1;
        g->rows = (size_t)(1.0f / cell_size) + 1;

        size_t total_cells = g->cols * g->rows;

        g->total_cells = total_cells;
        g->cell_start =
            arena_alloc(arena, total_cells * sizeof(int), AlignOfType(int));
        g->cell_count =
            arena_alloc(arena, total_cells * sizeof(int), AlignOfType(int));
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
        Grid      *g    = s->grid;
        Particles *p    = s->particles;
        Arena     *temp = s->temp_arena;
        size_t     n    = s->count;

        memset(g->cell_count, 0, sizeof(int) * g->total_cells);
        for (size_t i = 0; i < n; ++i) {
                size_t cell = GridIndex(g, p->x[i], p->y[i]);
                g->cell_count[cell]++;
        }

        int sum = 0;
        for (size_t c = 0; c < g->total_cells; ++c) {
                g->cell_start[c] = sum;
                sum += g->cell_count[c];
                g->cell_count[c] = 0;
        }

        float *tmp_x = arena_alloc(temp, sizeof(float) * n, AlignOfType(float));
        float *tmp_y = arena_alloc(temp, sizeof(float) * n, AlignOfType(float));
        float *tmp_vx =
            arena_alloc(temp, sizeof(float) * n, AlignOfType(float));
        float *tmp_vy =
            arena_alloc(temp, sizeof(float) * n, AlignOfType(float));
        int *tmp_c = arena_alloc(temp, sizeof(int) * n, AlignOfType(int));

        if (!tmp_x || !tmp_y || !tmp_vx || !tmp_vy || !tmp_c)
                return;

        for (size_t i = 0; i < n; ++i) {
                size_t cell = GridIndex(g, p->x[i], p->y[i]);
                int    dst  = g->cell_start[cell] + g->cell_count[cell]++;

                tmp_x[dst]  = p->x[i];
                tmp_y[dst]  = p->y[i];
                tmp_vx[dst] = p->vx[i];
                tmp_vy[dst] = p->vy[i];
                tmp_c[dst]  = p->colors[i];
        }

        memcpy(p->x, tmp_x, sizeof(float) * n);
        memcpy(p->y, tmp_y, sizeof(float) * n);
        memcpy(p->vx, tmp_vx, sizeof(float) * n);
        memcpy(p->vy, tmp_vy, sizeof(float) * n);
        memcpy(p->colors, tmp_c, sizeof(int) * n);
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
                Arena     *temp_arena,
                System    *s,
                Particles *p,
                Grid      *g,
                size_t     count,
                size_t     types_count) {

        s->matrix      = makeRandomMatrix(arena, types_count);
        s->particles   = p;
        s->grid        = g;
        s->temp_arena  = temp_arena;
        s->count       = count;
        s->types_count = types_count;

        InitParticles(arena, p, count, types_count);
        InitGrid(arena, g, appConfig.max_radius);
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

static inline void ParticlesUpdate(System *s) {
        arena_reset(s->temp_arena);
        GridBuild(s);

        Grid *g = s->grid;

        // clang-format off
        static const int neighbor_offsets[9][2] = {
            {-1, -1}, { 0, -1}, { 1, -1},
            {-1,  0}, { 0,  0}, { 1,  0},
            {-1,  1}, { 0,  1}, { 1,  1}
        };
        // clang-format on

        const float margin        = appConfig.margin;
        const float wall_strength = appConfig.wall_strength;
        const float max_r         = appConfig.max_radius;
        const float max_r2        = max_r * max_r;
        const float invMaxR2      = 1.0f / max_r2;

        // -------------------------------------------------
        // Iterate by cell
        // -------------------------------------------------
        for (size_t cell = 0; cell < g->total_cells; ++cell) {

                int start_i = g->cell_start[cell];
                int count_i = g->cell_count[cell];
                if (count_i == 0)
                        continue;

                int cx = cell % g->cols;
                int cy = cell / g->cols;

                for (int ii = 0; ii < count_i; ++ii) {

                        int i = start_i + ii;

                        float totalForceX = 0.0f;
                        float totalForceY = 0.0f;

                        float xi = px(s, i);
                        float yi = py(s, i);

                        int    color_i = pcolor(s, i);
                        float *row     = &s->matrix[color_i * s->types_count];

                        for (int n = 0; n < 9; ++n) {

                                int nx = cx + neighbor_offsets[n][0];
                                int ny = cy + neighbor_offsets[n][1];

                                if (nx < 0 || nx >= (int)g->cols || ny < 0 ||
                                    ny >= (int)g->rows)
                                        continue;

                                size_t neighbor_cell = ny * g->cols + nx;

                                int start_j = g->cell_start[neighbor_cell];
                                int count_j = g->cell_count[neighbor_cell];

                                for (int jj = 0; jj < count_j; ++jj) {

                                        int j = start_j + jj;
                                        if (j == i)
                                                continue;

                                        float rx = px(s, j) - xi;
                                        float ry = py(s, j) - yi;

                                        float dist2 = rx * rx + ry * ry;
                                        if (dist2 > 0.0f && dist2 < max_r2) {

                                                float relation =
                                                    row[pcolor(s, j)];
                                                float r = dist2 * invMaxR2;
                                                float f =
                                                    calcForce(r, relation);

                                                float invDist =
                                                    1.0f / sqrtf(dist2);

                                                totalForceX += rx * invDist * f;
                                                totalForceY += ry * invDist * f;
                                        }
                                }
                        }

                        totalForceX *= max_r * appConfig.force_factor;
                        totalForceY *= max_r * appConfig.force_factor;

                        float bx = 0.0f;
                        float by = 0.0f;

                        if (xi < margin)
                                bx += (margin - xi) * wall_strength;

                        if (xi > 1.0f - margin)
                                bx -= (xi - (1.0f - margin)) * wall_strength;

                        if (yi < margin)
                                by += (margin - yi) * wall_strength;

                        if (yi > 1.0f - margin)
                                by -= (yi - (1.0f - margin)) * wall_strength;

                        totalForceX += bx;
                        totalForceY += by;

                        pvx(s, i) *= appConfig.damping;
                        pvy(s, i) *= appConfig.damping;

                        pvx(s, i) += totalForceX * appConfig.dt;
                        pvy(s, i) += totalForceY * appConfig.dt;
                }
        }

        for (size_t i = 0; i < s->count; ++i) {
                px(s, i) += pvx(s, i) * appConfig.dt;
                py(s, i) += pvy(s, i) * appConfig.dt;
        }
}

static inline unsigned long ColorToPixel(Color c) {
        return ((unsigned long)c.r << 16) | ((unsigned long)c.g << 8) |
               ((unsigned long)c.b);
}

static inline void ParticlesDraw(Display *dpy, GC gc, Window win, System *s) {
        int           width  = appConfig.window_width;
        int           height = appConfig.window_height;
        unsigned int *pixels = (unsigned int *)appConfig.global_img->data;

        memset(pixels, 0, width * height * 4);

        for (size_t i = 0; i < s->count; ++i) {
                int x = (int)(px(s, i) * (float)width);
                int y = (int)(py(s, i) * (float)height);

                if (x >= 0 && x < width && y >= 0 && y < height) {
                        float hue             = 300.0f * ((float)pcolor(s, i) /
                                              (float)s->types_count);
                        Color color           = ColorFromHSV(hue, 1.0f, 1.0f);
                        pixels[y * width + x] = ColorToPixel(color);
                }
        }

        XPutImage(
            dpy, win, gc, appConfig.global_img, 0, 0, 0, 0, width, height);
}
#endif
