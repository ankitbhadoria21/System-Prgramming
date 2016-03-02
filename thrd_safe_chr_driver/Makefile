obj-m := drv.o
KERNEL_SOURCE := /lib/modules/$(shell uname -r)/build/
PWD := $(shell pwd)
all:
	${MAKE} -C ${KERNEL_SOURCE} SUBDIRS=${shell pwd} modules
 
clean:
	${MAKE} -C ${KERNEL_SOURCE} SUBDIRS=${shell pwd} clean
