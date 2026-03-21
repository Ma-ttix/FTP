/*
 * ftpserverpool.c
 */

#include "csapp.h"

#define MAX_NAME_LEN 256
#ifndef NPROC
#define NPROC 10
#endif

int ftp(int connfd);

void handlerSIGCHLD(int sig){
    while(waitpid(-1, NULL, WNOHANG)>0);
}

void handlerSIGINT(int sig){
    kill(0, SIGINT);
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

    setpgid(0, 0);
    Signal(SIGCHLD, handlerSIGCHLD);
    Signal(SIGINT, handlerSIGINT);

    port = 2121;

    listenfd = Open_listenfd(port);
    int pid = -1;
    int i = 0;
    while(pid!=0 && i!=NPROC){
        pid = Fork();
        i++;
    }
    if(pid == 0){
        //printf("im the child %d of pid %d\n", i, getpid());
        while (1) {

            connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
            if(connfd < 0) continue;

            /* determine the name of the client */
            Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAX_NAME_LEN, 0, 0, 0);

            /* determine the textual representation of the client's IP address */
            Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string, INET_ADDRSTRLEN);

            printf("server connected to %s (%s)\n", client_hostname, client_ip_string);
            printf("connected using child %d of pid %d\n", i, getpid());

            while(ftp(connfd) == 0);
            Close(connfd);
            printf("server disconnected to %s (%s)\n", client_hostname, client_ip_string);
        }
    }
    else{
        while(waitpid(-1, NULL, 0)>0);
        exit(0);
    }
}