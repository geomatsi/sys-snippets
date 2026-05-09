DESCRIPTION = "GLES2 texture test application"
PR = "r0"

SRCREV = "${AUTOREV}"
SRC_URI = "git://github.com/geomatsi/gles2-tests;branch=master;protocol=git"

S="${WORKDIR}/git"

TARGET_CC_ARCH += "-DLINUX -DEGL_API_FB ${LDFLAGS}"

do_compile() {
    oe_runmake test-vivante
}

do_install() {
    install -d ${D}${bindir}
    install -m 777  ${S}/test-vivante ${D}${bindir}/test-vivante
}
