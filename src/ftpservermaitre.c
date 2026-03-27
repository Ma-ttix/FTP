#include "csapp.h"

#ifndef NBSLAVES
#define NBSLAVES 3
#endif

void handlerSIGCHLD(int sig){
    while(waitpid(-1, NULL, WNOHANG)>0);
}

void handlerSIGINT(int sig){
    kill(0, SIGINT);
    exit(0);
}

int main(){
    setpgid(0, 0);
    Signal(SIGCHLD, handlerSIGCHLD);
    Signal(SIGINT, handlerSIGINT);

    int port = 2121;
    int listenfd = Open_listenfd(port);
    int connfd;

    int esclave[NBSLAVES]; // initialisation d'un tableau afin de stocker les différents ports des serveurs esclaves

    for(int i= 0; i < NBSLAVES; i++){
        esclave[i] = port + 1 + i; // permet d'ajouter un nouveau numéro de port à chaque serveur esclave
    }

    int pid = -1;
    int i = 0;
    while(pid!=0 && i!=NBSLAVES){
        pid = Fork();
        i++;
    }

    if(pid == 0){
        #ifdef TALK
        printf("Slave server %d of pid %d listening on port %d created\n", i, getpid(), esclave[i-1]);
        #endif
        char slavePort[6];
        snprintf(slavePort, sizeof(slavePort), "%d", esclave[i-1]); // -1 car pas d'esclave 0, l'esclave 1 récupère donc son port dans esclave[0]
        execlp("./ftpserverpool", "./ftpserverpool", slavePort, NULL);
        perror("execlp");
        exit(1);
    }

    // partie uniquement exécutée par le père/serveur maître
    int esclaveCourant = 0;

    while(1){
        connfd = accept(listenfd, NULL, NULL);

        int portEsclave = esclave[esclaveCourant];
        Rio_writen(connfd, &portEsclave, sizeof(int));
        esclaveCourant = (esclaveCourant + 1) % NBSLAVES;

        printf("Client redirected to slave server on port %d\n", portEsclave);

        Close(connfd);
    }
}