
DESCRIPTION = "Kernel Module Bitmain axi Fpga Driver"
HOMEPAGE = "http://www.example.com"
SECTION = "kernel/modules"
PRIORITY = "optional"
LICENSE = "CLOSED"
KERNEL_VERSION="3.10.31"

RRECOMMENDS_${PN} = "kernel (= ${KERNEL_VERSION})"
DEPENDS = "virtual/kernel"
PR = "r0"

SRC_URI = " \
	file://axi_fpga.c \
	file://axi_fpga.h \
	file://Makefile \
	"

S = "${WORKDIR}"

inherit module

do_compile () {
	echo "compile bitmain_axi"
    make
}

do_install () {
	install -d ${D}${base_libdir}/modules
	install -m 0644 ${S}/bitmain_axi${KERNEL_OBJECT_SUFFIX} ${D}${base_libdir}/modules
}
