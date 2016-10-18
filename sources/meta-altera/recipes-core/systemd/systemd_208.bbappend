FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://system.conf"

do_install_append() {
	cp ${WORKDIR}/system.conf ${D}${sysconfdir}/systemd
}
