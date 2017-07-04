CROSS=arm-none-linux-gnueabi-

all:rs485

rs485:rs485_test.c 
	$(CROSS)gcc -O2 -static -o $@ $<
	echo $<
clean:
	@rm -vf  rs485  *.o *~
