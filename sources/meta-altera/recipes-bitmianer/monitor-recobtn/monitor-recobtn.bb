DESCRIPTION = "Daemon to monitor recovery button"
LICENSE = "GPL"
LIC_FILES_CHKSUM = "file://COPYING;md5=d41d8cd98f00b204e9800998ecf8427e"

SRC_URI = "file://monitor-recobtn.c \
	file://init \
	file://factory_config_reset.sh \
	file://COPYING \
"

S = "${WORKDIR}"

do_compile() {
	make monitor-recobtn
	#make monitor-pwbtn
}

do_install() {
    install -d ${D}${bindir}
	install -m 0755 ${WORKDIR}/monitor-recobtn ${D}${bindir}/monitor-recobtn
	install -m 0755 ${WORKDIR}/factory_config_reset.sh ${D}${bindir}

    install -d ${D}${sysconfdir}/init.d
	install -m 0755 ${WORKDIR}/init ${D}${sysconfdir}/init.d/monitor-recobtn
	update-rc.d -r ${D} monitor-recobtn start 70 S .
}
