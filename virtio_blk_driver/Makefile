# Makefile for VirtIO Block Driver

KDIR ?= /lib/modules/$(shell uname -r)/build

obj-m := virtio_blk.o

EXTRA_CFLAGS := -Wall -Wextra -DDEBUG

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install

.PHONY: all clean install