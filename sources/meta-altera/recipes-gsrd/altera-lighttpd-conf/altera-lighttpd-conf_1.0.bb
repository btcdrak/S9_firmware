DESCRIPTION = "Altera GSRD customization"
AUTHOR = "Tien Hock Loh <thloh@altera.com>"
SECTION = "gsrd"
LICENSE = "BSD-3-Clause"
LIC_FILES_CHKSUM="file://${COMMON_LICENSE_DIR}/BSD-3-Clause;md5=550794465ba0ec5312d6919e203a55f9"
S="${WORKDIR}"
DEPENDS = "lighttpd"

SRC_URI = "file://lighttpd.conf"

FILES_${PN} = "/etc/lighttpd.conf \
		"
do_install() {
	install -d ${D}/etc
	install -m 0755 lighttpd.conf ${D}/etc/lighttpd.conf
}

