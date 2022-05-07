/*
 * "PBX" server module.
 * Manages interaction with a client telephone unit (TU).
 */
#include <stdlib.h>

#include "debug.h"
#include "pbx.h"
#include "server.h"
#include "csapp.h"
#include "pthread.h"
#include <stdio.h>

/*
 * Thread function for the thread that handles interaction with a client TU.
 * This is called after a network connection has been made via the main server
 * thread and a new thread has been created to handle the connection.
 */

// arg is a pointer to fd thats used to communicate w client
// after retrieving the fd
// detatch thread
// initialize TU with fd
// register TU w PBX module w an ext #
// service loop that parses msgs sent by client and carries out command

void *pbx_client_service(void *arg) {
    int connfd = *((int*)arg);
    free(arg);
    pthread_detach(pthread_self());

    TU *tu = tu_init(connfd);
    pbx_register(pbx, tu, connfd);
    FILE *file = fdopen(connfd, "r");
    while(1) {
        int i = 0, j = 1;
        char c = fgetc(file), *ogstr = malloc(j);
        char *str1 = ogstr, *str2;
        ogstr[i] = c;
        c  = fgetc(file);
        while (c != '\n') {
            if (c == EOF) break;
            i++; j++;
            ogstr = realloc(ogstr, j);
            ogstr[i] = c;
            c  = fgetc(file);
        }
        ogstr = realloc(ogstr, j+1);
        ogstr[i+1] = '\0';
        str2 = strtok_r(ogstr, " ", &ogstr);
        if (strcmp(str2, "pickup\r") == 0) { debug("%s", "Reached pickup"); tu_pickup(tu); }
        if (strcmp(str2, "hangup\r") == 0) { debug("%s", "Reached hangup"); tu_hangup(tu); }
        if (strcmp(str2, "chat") == 0) { debug("%s", "Reached chat"); tu_chat(tu, ogstr); }
        if (strcmp(str2, "dial") == 0) {
            debug("%s", "Reached dial");
            str2 = strtok_r(ogstr, "\r", &ogstr);
            pbx_dial(pbx, tu, atoi(str2));
        }
        
        free(str1); 
    }
    close(connfd);
}

