#
# simple make file
#

LIB_GLES2=-lGLESv2
LIB_EGL=-lEGL
LIB_X11=-lX11
LIB_MISC=-lm

CORE=main.o gles2.o shader_utils.o

all: test-x11

test-x11: ${CORE} plat_x11.o
	${CC} -o $@ $^ ${LIB_GLES2} ${LIB_EGL} ${LIB_X11} ${LIB_MISC} ${LDFLAGS}

test-drm: ${CORE} plat_drm.o
	${CC} -o $@ $^ ${LIB_GLES2} ${LIB_EGL} ${LIB_X11} ${LIB_MISC} ${LDFLAGS}

test-vivante: ${CORE} plat_vivante.o
	${CC} -o $@ $^ ${LIB_GLES2} ${LIB_EGL} ${LIB_MISC} ${CFLAGS}

%.o: %.c
	${CC} -c -o $@ $^

clean:
	rm -rf *.o
	rm -rf test-x11 test-vivante
