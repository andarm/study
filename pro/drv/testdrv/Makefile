ifneq ($(KERNELRELEASE),)

obj-m := test.o

else

KERNELDIR 	?=  /home/saftop/rt/linux-3.2
PWD			:= $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

endif

clean:
	rm -rf *.o Module.* *.ko *.mod.c *.order .*.cmd .tmp_versions
