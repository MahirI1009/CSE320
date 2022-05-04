#include <stdlib.h>
#include <unistd.h>

#include "pbx.h"
#include "server.h"
#include "debug.h"
#include "csapp.h"
#include "csapp.c"
#include "netdb.h"
#include "netinet/in.h"

static void sighup_handler(int sig);
static void *thread(void *vargp);
static void terminate(int status);

/*
 * "PBX" telephone exchange simulation.
 *
 * Usage: pbx <port>
 */
int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.

    // Perform required initialization of the PBX module.
    debug("Initializing PBX...");
    pbx = pbx_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function pbx_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.

    // int option, port;
    // while((option = getopt(argc, argv, ":pn")) != -1) {
    //     switch(option) {
    //         case 'p':
    //         case 'n':
    //             port = option;
    //             break;
    //         case ':':
    //             fprintf(stderr, "usage: %s <port>\n", argv[0]);
    //             exit(0);
    //         case '?':
    //             fprintf(stderr, "usage: %s <port>\n", argv[0]);
    //             exit(0);
    //     } 
    // }

    int port = atoi(argv[2]);
    
    int listenfd, *connfd;
    struct sigaction act;
    act.sa_handler = &sighup_handler;
    sigaction(SIGHUP, &act, NULL);
    listenfd = open_listenfd(port);
    pthread_t tid;
    while(1) {
        connfd = malloc(sizeof(int));
        *connfd = accept(listenfd, NULL, NULL);
        Pthread_create(&tid, NULL, pbx_client_service, connfd);
    }
}

void sighup_handler(int sig) { terminate(SIGHUP); }

/*
 * Function called to cleanly shut down the server.
 */
static void terminate(int status) {
    debug("Shutting down PBX...");
    pbx_shutdown(pbx);
    debug("PBX server terminating");
    exit(status);
}
