#remember to replace indentation with tabs if copied
obj-m += core.o
rootkit-objs := client_interface.o hooks.o hooked_syscalls.o networking.o syscall_table.o
ccflags += -ggdb -DDEBUG

all:
	make -C  /lib/modules/$(shell uname -r)/build M=$(PWD) modules #replace indentation here

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean #and here
