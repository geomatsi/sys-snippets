#include "plat.h"

/* */

static EGLNativeDisplayType native_display;
static EGLNativeWindowType native_window;

static int width;
static int height;

/* */

int platform_open(void)
{
	native_display = fbGetDisplayByIndex(0);
	native_window = fbCreateWindow(native_display, 0, 0, 0, 0);

	fbGetDisplayGeometry(native_display, &width, &height);
}

void platform_close(void)
{

}

EGLNativeDisplayType plat_get_display(void)
{
	return native_display;
}

EGLNativeWindowType plat_get_window(void)
{
	return native_window;
}

void plat_get_geometry(int *pw, int *ph)
{
	*pw = width;
	*ph = height;
}
