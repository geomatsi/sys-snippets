#ifndef PLATFORM_IFACE_H
#define PLATFORM_IFACE_H

#include "EGL/egl.h"

int platform_open(void);
void platform_close(void);

EGLNativeDisplayType plat_get_display(void);
EGLNativeWindowType plat_get_window(void);

void plat_get_geometry(int *w, int *h);

#endif /* PLATFORM_IFACE_H */
