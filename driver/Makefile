
KERN_DIR = /home/book/work/100ask_stm32mp157_pro-sdk/Linux-5.4/

all:
	make -C $(KERN_DIR) M=`pwd` modules 
	@cp *.ko out/

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	@rm -rf modules.order
	@rm -rf out/*

# 参考内核源码drivers/char/ipmi/Makefile
# 要想把a.c, b.c编译成ab.ko, 可以这样指定:
# ab-y := a.o b.o
# obj-m += ab.o

tes_all-y := tes_bus.o tes.o
obj-m	+= tes_all.o
