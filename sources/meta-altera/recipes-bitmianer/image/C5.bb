export IMAGE_BASENAME = "C5"
IMAGE_INSTALL = " \
	busybox \
	base-files \
	initscripts \
	sysvinit \
	sysvinit-pidof \
	angstrom-version \
	shadow \
	i2c-tools \
	screen \
	dropbear \
	libcurl \
	lighttpd \
	cgminer \
	dtc \
	stunnel \
	initc-bin \
	openssl \
	mtd-utils \
	curl \
	udev \
	ntp \
	ntpdate \
	netbase \
	avahi \
	monitor-ipsig \
	d-ddos \
	monitor-recobtn \
	minermonitor \
    bitmain-axi \
"
#omap3-mkcard 			   
#cgminer 
#bitmain-spi-s3
#bitmain-lcd
inherit image

do_rootfs_append() {
	echo "do_build_append"
	#cp -rf /home/xxl/AM335x/knc/setup-s3/deploy/eglibc/images/beaglebone/Angstrom-S3-eglibc-ipk-v2013.06-beaglebone.rootfs.cpio.gz.u-boot /tftpboot/initramfs.bin
	#cp -rf /home/xxl/AM335x/knc/setup-s3/deploy/eglibc/images/beaglebone/Angstrom-S3-eglibc-ipk-v2013.06-beaglebone.rootfs.cpio.gz.u-boot /tftpboot/initramfs.bin.SD
}

