/*
  library for getting the syscall table and kallsyms_lookup_name
	- kallsyms_lookup_name is no longer exported, so this is a workaround
		see the links below for more
			-  https://xcellerator.github.io/posts/linux_rootkits_11/
			-  https://gist.github.com/linuxthor/a2b2ef9e39470fd6dd188b25c427322b
		if not available lookup kallsyms_lookup_name not exported workaround
*/

#include <linux/kernel.h> //contains types, macros, functions for the kernel   e.g. KERN_INFO
#include <linux/unistd.h> //contains syscall names to numbers
#include <linux/kprobes.h> //kprobes- used in workaround for kallsyms_lookup_name not exported



unsigned long * __sys_call_table = NULL;  //syscall table adress NULL = (void*)0
typedef unsigned long (*kallsyms_lookup_name_t)(const char *name); //typedef for kallsyms_lookup_name() so kp.addr can be easily cast, and makes formatting nicer
kallsyms_lookup_name_t kallsyms_lookup_name_workaround=(kallsyms_lookup_name_t) 0; //function for kallysms_lookup_name_workaround definition
static void generate_kallsyms_lookup_name_workaround(void); //function to generate kallsyms_lookup_name



static int get_syscall_table(void){ // finds the memory adress of the sycall table and stores it in __sys_call_table
	//checks if kallsyms_lookup_name has been created
	if (kallsyms_lookup_name_workaround==(kallsyms_lookup_name_t) NULL){ //if kallsysms_lookup_name_workaround is NULL
		generate_kallsyms_lookup_name_workaround(); //gets adress of kallsyms_lookup_name
	}
	 __sys_call_table = (unsigned long*) kallsyms_lookup_name_workaround("sys_call_table"); //sets syscall_table to syscall_table adress
	return 0; //returns 0 for no error
}


static void generate_kallsyms_lookup_name_workaround(void){ //uses kprobe to find the memory adress of kallsyms_lookup_name and creates function
	struct kprobe kp = { //kprobe definition
		.symbol_name = "kallsyms_lookup_name"
	};
	register_kprobe(&kp); // register the kallsyms_lookup_name kprobe
	kallsyms_lookup_name_workaround = (kallsyms_lookup_name_t) kp.addr; // assign kallsyms_lookup_name symbol to kp.addr
	unregister_kprobe(&kp); // done with the kallsyms_lookup_name kprobe, so unregister it
  }
