/*
 * ftpserverpool.c
 */

#include "csapp.h"

#define MAX_NAME_LEN 256
#ifndef NPROC
#define NPROC 10
#endif

int ftp(int connfd);

int pidchild[NPROC];

void handlerSIGCHLD(int sig){
    while(waitpid(-1, NULL, WNOHANG)>0);
}

void handlerSIGINT(int sig){
    int i = 0;
    while(i!=NPROC){
        if(pidchild[i]!=0) kill(pidchild[i], SIGINT);
        i++;
    }
    exit(0);
}

/*
 * Note that this code only works with IPv4 addresses
 * (IPv6 is not supported)
 */
int main(int argc, char **argv)
{
    int listenfd, connfd, port;
    struct sockaddr_in clientaddr;
    socklen_t clientlen = (socklen_t)sizeof(clientaddr);
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];

    /*if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }*/

    Signal(SIGCHLD, handlerSIGCHLD);
    Signal(SIGINT, handlerSIGINT);

    port = atoi(argv[1]);

    listenfd = Open_listenfd(port);
    int pid = -1;
    int i = 0;
    while(pid!=0 && i!=NPROC){
        pid = Fork();
        pidchild[i] = pid;
        i++;
    }
    if(pid == 0){
        #ifdef TALK
        printf("Executant process %d of pid %d from server %d created\n", i, getpid(), getppid());
        #endif
        while(1){

            connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
            if(connfd < 0) continue;

            /* determine the name of the client */
            Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAX_NAME_LEN, 0, 0, 0);

            /* determine the textual representation of the client's IP address */
            Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string, INET_ADDRSTRLEN);

            printf("Server %d connected to %s (%s)\n", getppid(), client_hostname, client_ip_string);
            #ifdef TALK
            printf("Connected using child %d of pid %d\n", i, getpid());
            #endif

            while(ftp(connfd) == 0);
            Close(connfd);
            printf("Server %d disconnected to %s (%s)\n", getppid(), client_hostname, client_ip_string);
            #ifdef TALK
            printf("Disonnected from child %d of pid %d\n", i, getpid());
            #endif
        }
    }
    else{
        while(waitpid(-1, NULL, 0)>0);
        exit(0);
    }
}