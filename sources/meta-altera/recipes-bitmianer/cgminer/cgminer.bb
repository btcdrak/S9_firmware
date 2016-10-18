DESCRIPTION = "Cgminer bitcoin miner SW"
LICENSE = "CLOSED"


DEPENDS = "ncurses curl"

SRCREV = "4.9.2"
PR = "r1"

SRC_URI = "file://cgminer_c5.tar.bz2"

S = "${WORKDIR}/cgminer_c5"
CFLAGS_prepend = "-I ${S}/compat/jansson-2.6/src -I ${S}/compat/libusb-1.0/libusb"


EXTRA_OECONF = "\
	--enable-bitmain-c5 \
	--disable-adl \
	--disable-opencl \
        --disable-libcurl \"
		 
do_configure_prepend() {
	autoreconf -fiv
}

do_compile_append() {
	make api-example
}

do_install_append() {
        install -d ${D}${bindir}
        install -m 0755 ${S}/api-example ${D}${bindir}/cgminer-api
}
 
inherit autotools pkgconfig
