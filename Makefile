.PHONY: all
obj-m := iogen.o

CC              := gcc
KVERSION        := $(shell uname -r)
KDIR            := /lib/modules/$(KVERSION)/build

iogen-objs := iogenmod.o


all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean: 
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install
