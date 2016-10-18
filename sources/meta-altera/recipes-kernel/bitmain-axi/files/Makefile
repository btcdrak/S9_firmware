
# Makefile for bitmain-axi.ko

TARGET = bitmain_axi

OBJECT = axi_fpga.o

EXTRA_CFLAGS += $(BOARD_TYPE)

ifneq ($(KERNELRELEASE),)

#kbuild syntax.

#obj-$(CONFIG_BITMAIN_ASIC_SPI) += $(TARGET).o
obj-m += $(TARGET).o

$(TARGET)-objs := $(OBJECT)

else

PWD := $(shell pwd)

#KERNEL_DIR := /lib/modules/`uname -r`/build

KDIR := /home/lzq/work/C5/linux-socfpga

all : modules

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) ARCH=$(ARCH)  modules
clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions Module.markers modules.order Module.symvers

endif

