#include "X11/Xlib.h"
#include "X11/Xutil.h"
#include "X11/Xatom.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <math.h>

#include "plat.h"

/* */

#define WIDTH  640
#define HEIGHT 480

/* */

static Window xWindow;
static Display*	xDisplay;
static XVisualInfo* xVisual;
static Colormap xColormap;
static long xScreen;

/* */

static int xwinOpen(void);
static void xwinClose(void);

/* */

static int xwinOpen(void)
{
    XSetWindowAttributes window_attributes;
    Window root_window;

    unsigned int window_mask;
    int height_corrected;
    int width_corrected;
    int color_depth;

    xWindow = 0;
    xScreen = 0;

    xDisplay = NULL;
    xVisual = NULL;

    xDisplay = XOpenDisplay(0);
    if (!xDisplay)
    {
        printf("Error: Unable to open X display\n");
        return -1;
    }

    xScreen = XDefaultScreen(xDisplay);

    /* Get the root window parameters */
    color_depth = DefaultDepth(xDisplay, xScreen);
    root_window = RootWindow(xDisplay, xScreen);

    /* Alloc memory for the visual info */
    xVisual = (XVisualInfo*) calloc(1, sizeof(XVisualInfo));

    /*  Try to find a visual which is matching the needed parameters */
    XMatchVisualInfo(xDisplay, xScreen, color_depth, TrueColor, xVisual);
    if (!xVisual)
    {
        printf("Error: Unable to acquire visual\n");
        xwinClose();
        return -1;
    }

    /* Create the rendercontext color map */
    xColormap = XCreateColormap(xDisplay, root_window, xVisual->visual, AllocNone);
    window_attributes.colormap = xColormap;

    /* Add to these for handling other events */
    window_attributes.event_mask = StructureNotifyMask | ExposureMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask;

    /* Set the window mask attributes */
    window_mask = CWBackPixel | CWBorderPixel | CWEventMask | CWColormap;

    /* Get the corrected window dimensions */
    width_corrected = WIDTH < XDisplayWidth(xDisplay, xScreen) ? WIDTH : XDisplayWidth(xDisplay, xScreen);
    height_corrected = HEIGHT < XDisplayHeight(xDisplay, xScreen) ? HEIGHT : XDisplayHeight(xDisplay, xScreen);

    /* Create the X11 window */
    xWindow = XCreateWindow(xDisplay, RootWindow(xDisplay, xScreen), 0, 0,
            width_corrected, height_corrected, 0, xVisual->depth, InputOutput,
            xVisual->visual, window_mask, &window_attributes);
    if (0 == xWindow)
    {
        printf("Error: Couldn't cread XWindow\n");
        xwinClose();
        return -1;
    }

    /* map the window */
    XMapWindow(xDisplay, xWindow);
    XFlush(xDisplay);

    return 0;
}

static void xwinClose(void)
{
    if (xWindow)
    {
        XDestroyWindow(xDisplay, xWindow);
    }

    if (xColormap)
    {
        XFreeColormap(xDisplay, xColormap);
    }

    if (xDisplay)
    {
        XCloseDisplay(xDisplay);
    }

    if (xVisual)
    {
        free(xVisual);
    }
}

/* */

int platform_open(void)
{
	return xwinOpen();
}

void platform_close(void)
{
	xwinClose();
}

EGLNativeDisplayType plat_get_display(void)
{
	return (EGLNativeDisplayType) xDisplay;
}

EGLNativeWindowType plat_get_window(void)
{
	return (EGLNativeWindowType) xWindow;
}

void plat_get_geometry(int *pw, int *ph)
{
	*pw = WIDTH;
	*ph = HEIGHT;
}
