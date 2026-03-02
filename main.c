#define _POSIX_C_SOURCE 199309L
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/shapeconst.h>
#include <time.h>
#define Font RayFont
#include <raylib.h>
#undef Font
#include <X11/extensions/Xfixes.h>
#include <stdio.h>

#include "config.h"
#include "system.h"
#include "types.h"
#include "utils.h"

static Window overlay_win;

void InitOverlay(Display *dpy, int screen, int width, int height) {
        Window  root  = RootWindow(dpy, screen);
        Visual *vis   = DefaultVisual(dpy, screen);
        int     depth = DefaultDepth(dpy, screen);

        XSetWindowAttributes attrs;
        attrs.override_redirect = True;
        attrs.background_pixel  = 0x000000;

        overlay_win = XCreateWindow(dpy,
                                    root,
                                    0,
                                    0,
                                    width,
                                    height,
                                    0,
                                    depth,
                                    InputOutput,
                                    vis,
                                    CWOverrideRedirect | CWBackPixel,
                                    &attrs);

        XFixesSetWindowShapeRegion(dpy, overlay_win, ShapeInput, 0, 0, 0);

        XMapWindow(dpy, overlay_win);
        XLowerWindow(dpy, overlay_win);

        char *data           = malloc(width * height * 4);
        appConfig.global_img = XCreateImage(
            dpy, vis, depth, ZPixmap, 0, data, width, height, 32, 0);
}

static void sleep_frame(void) {
        struct timespec ts;
        ts.tv_sec  = 0;
        ts.tv_nsec = 16000000; // ~16ms = 60 FPS
        nanosleep(&ts, NULL);
}

static inline void update(System *system) {
        ParticlesUpdate(system);
}

int main(void) {
        Arena arena      = arena_create(Megabytes(200));
        Arena temp_arena = arena_create(Megabytes(200));

        SetRandomSeed((unsigned int)time(NULL));
        Particles particles = {0};
        Grid      grid      = {0};
        System    system    = {0};
        InitSystem(&arena,
                   &temp_arena,
                   &system,
                   &particles,
                   &grid,
                   appConfig.particles_count,
                   appConfig.particles_type_count);

        Display *dpy = XOpenDisplay(NULL);
        if (!dpy) {
                fprintf(stderr, "Cannot open display\n");
                return 1;
        }

        int screen              = DefaultScreen(dpy);
        appConfig.window_width  = DisplayWidth(dpy, screen);
        appConfig.window_height = DisplayHeight(dpy, screen);

        InitOverlay(
            dpy, screen, appConfig.window_width, appConfig.window_height);

        XStoreName(dpy, overlay_win, "Particles Background");

        GC gc = XCreateGC(dpy, overlay_win, 0, NULL);

        XLowerWindow(dpy, overlay_win);

        while (true) {
                update(&system);

                ParticlesDraw(dpy, gc, overlay_win, &system);
                // XFlush(dpy);

                sleep_frame();
        }

        arena_destroy(&arena);
        XDestroyImage(appConfig.global_img);
        CloseWindow();
        return 0;
}
