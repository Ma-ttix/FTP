#include "csapp.h"
#ifndef NBSLAVES
#define NBSLAVES 3
#endif
int main() {
    int listenfd = Open_listenfd(2122); // port 2122 car de mon pc le port 2121 ne marche pas ( already use)  à changé s'il remarche
    int connfd;

    int Esclave[NBSLAVES]; // initialisation d'un tableau afin de stocker les différents serveurs esclaves
    int portBase = 2123;
    for(int i= 0; i < NBSLAVES; i++) {
        Esclave[i] = portBase + i; // permet d'ajouter un nouveau numéro de port à chaque serveur esclave
    }

    int esclaveCourant = 0;

    while(1) {
        connfd = accept(listenfd, NULL, NULL);

        int portEsclave = Esclave[esclaveCourant];
        esclaveCourant = (esclaveCourant + 1) % NBSLAVES; // si par exemple 3 nbslaves (de 0 à 2)alors: Esclave[0] -> esclaveCourant = 0 + 1 %3 = 1 ainsi de suite jusqu'a
                                                            // Esclave[2] -> esclaveCourant = 2 + 1 % 3 = 0 (on revient au premier serveur esclave)

        Rio_writen(connfd, &portEsclave, sizeof(int));

        printf("Client redirigé vers le port %d\n", portEsclave);

        Close(connfd);
    }
}