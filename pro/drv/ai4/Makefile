ifneq ($(KERNELRELEASE),)

obj-m := ai4.o

else

KERNELDIR 	?=  /home/saftop/rt/linux-3.2
PWD			:= $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

endif

clean:
	rm -rf *.o Module.* *.ko *.mod.c *.order .*.cmd .tmp_versions
test:
	 arm-none-linux-gnueabi-gcc -o test  test_ai.c
cl_test:
	rm test -rf
