#include "files.c"

struct kern_file *log_file;

static int init_logging(void){//initiates logging ability
	log_file = open_hidden_file("messages.log"); //opens file
	if (log_file==NULL){ //if unable to open
		printk("[init_logging] log file not able to be opened\n");
		return -1; //retun -1 for error
	}
	return 0; //return 0 for no error
}

static int close_logging(void){
	file_close(log_file);
	return 0;
}

static int log(char *message){ //logs message to log_file
	printk("logging \"%s\" length = %i\n", message, strlen((char *) message));
	file_write(log_file, (void *) message, strlen((char *) message));
	return 0;

}

static int get_logs(char *buffer){
	file_read(log_file, buffer);
	return 0;
}
