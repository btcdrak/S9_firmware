SUMMARY = "VLAN utlities"
DESCRIPTION = "VLAN utilities"
HOMEPAGE = "http://scry.wanfear.com/~greear/vlan.html"
#TODO: BUGTRACKER = "http://sourceforge.net/tracker/?group_id=13478&atid=113478"
#TODO: SECTION = "libs"
LICENSE = "GPLv2+ & LGPLv2+"
LIC_FILES_CHKSUM = "file://vconfig.c;md5=293ef69c4e88434d37a5ee577a5288cb \
                    file://macvlan_config.c;md5=d4249f372a09269b36c4fab961e20ccc"

DEPENDS = "virtual/kernel"

PR = "r0"

SRC_URI= "http://ftp.de.debian.org/debian/pool/main/v/vlan/vlan_${PV}.orig.tar.gz"

SRC_URI[md5sum] = "5f0c6060b33956fb16e11a15467dd394"
SRC_URI[sha256sum] = "3b8f0a1bf0d3642764e5f646e1f3bbc8b1eeec474a77392d9aeb4868842b4cca"

S = "${WORKDIR}/vlan"

do_compile() {

    cd ${S}
    make -B vconfig CC=${TARGET_PREFIX}gcc STRIP=${TARGET_PREFIX}strip HOME=${STAGING_KERNEL_DIR}/include
}

do_install() {
    install -d ${D}/usr/bin
    install -m 0755 vconfig ${D}/usr/bin/vconfig
}

FILES_${PN} = "/usr/bin/vconfig"
