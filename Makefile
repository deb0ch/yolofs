
ifeq ($(KERNELRELEASE),)

# Assume the source tree is where the running kernel was built
# You should set KERNELDIR in the environment if it's elsewhere
KERNELDIR ?= /lib/modules/$(shell uname -r)/build

# The current directory is passed to sub-makes as argument
PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

help:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) help

.PHONY: modules modules_install clean

else

# called from kernel build system: just declare our module
obj-m := yolofs.o
yolo-objs :=

endif
