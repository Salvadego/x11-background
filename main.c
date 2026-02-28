#define _POSIX_C_SOURCE 199309L
#include "config.h"
#include "system.h"
#include "types.h"
#include "utils.h"
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <time.h>
#define Font RayFont
#include <raylib.h>
#undef Font
#include <stdio.h>

static void sleep_frame(void) {
        struct timespec ts;
        ts.tv_sec  = 0;
        ts.tv_nsec = 16000000; // ~16ms = 60 FPS
        nanosleep(&ts, NULL);
}

static void set_root_pixmap(Display *dpy, Window root, Pixmap pm) {
        Atom prop_root = XInternAtom(dpy, "_XROOTPMAP_ID", False);
        Atom prop_eset = XInternAtom(dpy, "ESETROOT_PMAP_ID", False);

        XChangeProperty(dpy,
                        root,
                        prop_root,
                        XA_PIXMAP,
                        32,
                        PropModeReplace,
                        (unsigned char *)&pm,
                        1);
        XChangeProperty(dpy,
                        root,
                        prop_eset,
                        XA_PIXMAP,
                        32,
                        PropModeReplace,
                        (unsigned char *)&pm,
                        1);

        XSetWindowBackgroundPixmap(dpy, root, pm);
        XClearWindow(dpy, root);
}

static inline void update(System *system) {
        ParticlesUpdate(system);
}

static inline void
draw(Display *dpy, Window root, GC gc, Pixmap pm, System *system) {
        XSetForeground(dpy, gc, 0x000000);
        XFillRectangle(
            dpy, pm, gc, 0, 0, appConfig.window_width, appConfig.window_height);

        ParticlesDraw(dpy, gc, pm, system);

        set_root_pixmap(dpy, root, pm);
        XFlush(dpy);
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

        int    screen           = DefaultScreen(dpy);
        Window root             = RootWindow(dpy, screen);
        appConfig.window_width  = DisplayWidth(dpy, screen);
        appConfig.window_height = DisplayHeight(dpy, screen);
        int depth               = DefaultDepth(dpy, screen);

        GC gc = XCreateGC(dpy, root, 0, NULL);

        Pixmap pm = XCreatePixmap(
            dpy, root, appConfig.window_width, appConfig.window_height, depth);

        while (true) {
                update(&system);
                draw(dpy, root, gc, pm, &system);
                sleep_frame();
        }

        arena_destroy(&arena);
        XFreePixmap(dpy, pm);
        CloseWindow();
        return 0;
}
