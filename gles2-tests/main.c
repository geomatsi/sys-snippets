#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <math.h>

#include "gles2.h"
#include "plat.h"

/* */

static EGLNativeDisplayType nativeDisplay;
static EGLNativeWindowType  nativeWindow;
static int width;
static int height;

static EGLDisplay eglDisplay = EGL_NO_DISPLAY;
static EGLContext eglContext = EGL_NO_CONTEXT;
static EGLSurface eglWindowSurface = EGL_NO_SURFACE;
static EGLConfig eglConfig;

/* */

static void handle_egl_error(const char *name)
{
    static char * const error_strings[] = {
        "EGL_SUCCESS",
        "EGL_NOT_INITIALIZED",
        "EGL_BAD_ACCESS",
        "EGL_BAD_ALLOC",
        "EGL_BAD_ATTRIBUTE",
        "EGL_BAD_CONFIG",
        "EGL_BAD_CONTEXT",
        "EGL_BAD_CURRENT_SURFACE",
        "EGL_BAD_DISPLAY",
        "EGL_BAD_MATCH",
        "EGL_BAD_NATIVE_PIXMAP",
        "EGL_BAD_NATIVE_WINDOW",
        "EGL_BAD_PARAMETER",
        "EGL_BAD_SURFACE"
    };

    EGLint error_code=eglGetError();

    fprintf(stderr," %s: egl error \"%s\" (0x%x)\n", name,
            error_strings[error_code-EGL_SUCCESS], error_code);

    exit(EXIT_FAILURE);
}

static int eglOpen(void)
{

    EGLint majorVersion;
    EGLint minorVersion;
    EGLint numConfigs;
    EGLBoolean ret;

    const char *ver, *extensions;

    int k = 0, i = 0, fnd = 0;

    EGLint gl_context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

#if 0
    EGLint pi32ConfigAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 5,
        EGL_GREEN_SIZE, 6,
        EGL_BLUE_SIZE, 5,
        EGL_ALPHA_SIZE, EGL_DONT_CARE,
        EGL_NONE
    };

    EGLint pi32ConfigAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };
#else
    EGLint pi32ConfigAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };
#endif

    eglDisplay = eglGetDisplay(nativeDisplay);
    if (!eglDisplay) {
        printf("eglGetDisplay error\n");
        return -1;
    }

    if (!eglInitialize(eglDisplay, &majorVersion, &minorVersion)) {
        printf("eglInitialize failed\n");
        return -1;
    }

    ver = eglQueryString(eglDisplay, EGL_VERSION);
    printf("EGL_VERSION = %s\n", ver);

    extensions = eglQueryString(eglDisplay, EGL_EXTENSIONS);
    printf("EGL_EXTENSIONS: %s\n", extensions);

    eglBindAPI(EGL_OPENGL_ES_API);

    if (!eglChooseConfig(eglDisplay, pi32ConfigAttribs, &eglConfig, 1, &numConfigs) || numConfigs != 1) {
		handle_egl_error("eglChooseConfig");
        return -1;
    }

    eglWindowSurface = eglCreateWindowSurface(eglDisplay, eglConfig, nativeWindow, NULL);
    if (eglWindowSurface == EGL_NO_SURFACE) {
        handle_egl_error("eglCreateWindowSurface");
        return -1;
    }

    eglContext = eglCreateContext(eglDisplay, eglConfig, NULL, gl_context_attribs);
    if (eglContext == EGL_NO_CONTEXT) {
        handle_egl_error("eglCreateContext");
        return -1;
    }

    ret = eglMakeCurrent(eglDisplay, eglWindowSurface, eglWindowSurface, eglContext);
    if (ret == EGL_FALSE) {
        handle_egl_error("eglMakeCurrent");
        return -1;
    }

    return 0;
}

static void eglClose(void)
{
    eglMakeCurrent(eglDisplay, NULL, NULL, NULL);
    eglDestroyContext(eglDisplay, eglContext);
    eglDestroySurface(eglDisplay, eglWindowSurface);
    eglTerminate(eglDisplay);
}

int main(int argc, char *argv[])
{
    int iterations = 100;
    int ret = 0;

    /* */

    if(argc > 1)
        iterations = atoi(argv[1]);

	ret = platform_open();
	if (0 > ret) {
		printf("can't init platform\n");
		return -1;
	}

	nativeDisplay = plat_get_display();
    if (0 > nativeDisplay) {
        printf("can't get platform display\n");
        return -1;
    }

	nativeWindow = plat_get_window();
    if (0 > nativeWindow) {
        printf("can't get platform window\n");
        return -1;
    }

	plat_get_geometry(&width, &height);
	ret = eglOpen();
	if (0 > ret) {
        printf("error in eglOpen\n");
        return -1;
    }

	gles2_init();

    while(iterations-- >= 0) {
		gles2_update_state(iterations);
		gles2_draw(width, height);

		eglSwapBuffers(eglDisplay, eglWindowSurface);
    }

    eglClose();

	platform_close();

    return 0;
}
