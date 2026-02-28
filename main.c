#include "config.h"
#include "system.h"
#include "types.h"
#include "utils.h"
#include <raylib.h>
#include <stdio.h>

void update(Arena *arena, System *system) {
        if (IsWindowResized()) {
                appConfig.window_width  = GetScreenWidth();
                appConfig.window_height = GetScreenHeight();

                InitSystem(arena,
                           system,
                           system->particles,
                           system->grid,
                           appConfig.particles_count,
                           appConfig.particles_type_count);
        }

        ParticlesUpdate(system);
}

void draw(System *system) {
        BeginDrawing();
        ClearBackground(BLACK);

        DrawFPS(10, 10);
        ParticlesDraw(system);

        EndDrawing();
}

int main(void) {
        Arena arena = arena_create(Megabytes(1024));

        Particles particles = {0};
        Grid      grid      = {0};
        System    system    = {0};
        InitSystem(&arena,
                   &system,
                   &particles,
                   &grid,
                   appConfig.particles_count,
                   appConfig.particles_type_count);

        InitWindow(appConfig.window_width,
                   appConfig.window_height,
                   appConfig.window_title);
        SetTargetFPS(appConfig.target_fps);

        while (!WindowShouldClose()) {
                // appConfig.dt = GetFrameTime();

                update(&arena, &system);
                draw(&system);
        }

        arena_destroy(&arena);
        CloseWindow();
        return 0;
}
