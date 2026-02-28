#ifndef CONFIG_H_YDHSECZA
#define CONFIG_H_YDHSECZA

#include <stddef.h>
#include <stdint.h>
#include <x86intrin.h>

static inline uint64_t rdtsc_begin(void) {
        _mm_lfence();
        return __rdtsc();
}

static inline uint64_t rdtsc_end(void) {
        uint64_t t = __rdtsc();
        _mm_lfence();
        return t;
}

static uint64_t interaction_count = 0;

struct {
        int         window_width, window_height;
        const char *window_title;
        const int   target_fps;

        const float dt;
        const float half_life;
        const float max_radius;
        const float damping;
        const float force_factor;
        const float cell_size;

        size_t       particles_count;
        const int    particle_size;
        const size_t particles_type_count;
} appConfig = {
    .window_width  = 800,
    .window_height = 600,
    .window_title  = "Particles",
    .target_fps    = 60,

    .max_radius   = 0.04,
    .dt           = 0.016,
    .half_life    = 0.040,
    .damping      = 0.98,
    .force_factor = 3,

    .particles_count      = 5000,
    .particle_size        = 1,
    .particles_type_count = 5,
};

#endif /* end of include guard: CONFIG_H_YDHSECZA */
