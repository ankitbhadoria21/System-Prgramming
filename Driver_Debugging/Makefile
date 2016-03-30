obj-m := drv.o
KERNEL_SOURCE := /lib/modules/$(shell uname -r)/build/
PWD := $(shell pwd)
all:
	@${MAKE} -C ${KERNEL_SOURCE} SUBDIRS=${shell pwd} modules
app:
	@gcc test1.c -o test1 -lpthread
	@gcc test2.c -o test2 -lpthread
	@gcc test3.c -o test3 -lpthread
	@gcc test4.c -o test4 -lpthread

clean:
	@${MAKE} -C ${KERNEL_SOURCE} SUBDIRS=${shell pwd} clean
