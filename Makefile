# SPDX-License-Identifier: GPL-2.0-only

# Kbuild file for out-of-tree build of the XR17V358 serial driver.
#
# Usage (from this directory):
#   make                  – build against the running kernel
#   make KDIR=<path>      – build against a specific kernel source/build tree
#   make clean            – remove build artifacts

obj-m += xr17v358.o

KDIR ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
