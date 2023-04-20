#remember to replace indentation with tabs if copied
obj-m += rootkit.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules #replace indentation here

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean #and here
