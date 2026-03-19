/*
 * ftpclient.c
 */
#include "csapp.h"
#include "ftp.h"
#include <time.h>

char** parseString(char* s){
    char** res = malloc(2*sizeof(char*));
    if(!res){
        perror("malloc");
        exit(1);
    }
    res[0] = strtok(s, " ");
    res[1] = strtok(NULL, " ");
    if(!res[1]){
        fprintf(stdout, "Missing argument\nUsage: <commande> <filename>\n");
        exit(1);
    }
    res[1][strcspn(res[1], "\n")] = '\0'; // strcspn retourne l'index de la première occurrence de \n dans buf, par accès au tableau on remplace donc le \n par \0, afin que le nom du fichier soit correct pour Fopen (echo.c\0 est valide mais pas echo.c\n\0)
    return res;
}

void traiterNomCommande(request_t* req, char* mot){
    if(strcmp(mot, "get") == 0) req->typereq = GET;
    //else if(strcmp(mot, "ls") == 0) req->typereq = LS;
    //else if(strcmp(mot, "rm") == 0) req->typereq = RM;
    //else if(strcmp(mot, "PUT") == 0) req->typereq = PUT;
    else req->typereq = FAUX;
}

void traiterErreur(int code){
    switch(code){
        case 1:
            fprintf(stderr, "Error: Invalide file name\n");
            exit(1);
        case 2:
            fprintf(stderr, "Error: Unsuccessful write from server\n");
            exit(2);
        case 3:
            fprintf(stderr, "Error: Invalid command name\n");
            exit(3);
        default:
            return;
    }
}

void requestGETc(rio_t* rio, request_t* req, response_t* response, struct timeval* debut){
    char *buffer = malloc(response->fileSize);
    int sizeRead = Rio_readnb(rio, buffer, response->fileSize);

    char tmp[MAXLINE + 7]; // + 7 pour la taille de "client/"
    snprintf(tmp, sizeof(tmp), "client/%s", req->nomfic);
    strcpy(req->nomfic, tmp);
    FILE* fd = fopen(req->nomfic, "w");
    if(!fd){
        perror("fopen");
        exit(1);
    }

    int bytesWritten = fwrite(buffer, 1, response->fileSize, fd);
    if(bytesWritten != response->fileSize){
        perror("fwrite");
        exit(1);
    }

    fprintf(stdout, "Successful write on file: %s\n", req->nomfic);

    fclose(fd);

    struct timeval fin;
    gettimeofday(&fin, NULL);
    double duree = (double)(fin.tv_sec - debut->tv_sec) + (double)(fin.tv_usec - debut->tv_usec) / 1000000;
    double debitK = sizeRead / (1000 * duree);
    fprintf(stdout, "%d bytes received in %f seconds (%.2f Kbytes/s)\n", sizeRead, duree, debitK);

}

int main(int argc, char **argv)
{
    int clientfd, port;
    char *host;//, buf[MAXLINE];
    rio_t rio;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host> \n", argv[0]);
        exit(0);
    }
    host = argv[1];
    port = 2121;

    /*
     * Note that the 'host' can be a name or an IP address.
     * If necessary, Open_clientfd will perform the name resolution
     * to obtain the IP address.
     */
    clientfd = Open_clientfd(host, port);

    /*
     * At this stage, the connection is established between the client
     * and the server OS ... but it is possible that the server application
     * has not yet called "Accept" for this connection
     */
    printf("client connected to server OS\n");

    Rio_readinitb(&rio, clientfd);

    char ficin[MAXLINE];
    printf("ftp>: ");
    Fgets(ficin, MAXLINE, stdin); // afin de lire la requête de l'utilisateur
    struct timeval debut;
    gettimeofday(&debut, NULL);
    char** reqUser = parseString(ficin);

    request_t req; //= malloc(sizeof(request_t));
    traiterNomCommande(&req, reqUser[0]);
    strcpy(req.nomfic, reqUser[1]);

    Rio_writen(clientfd, &req, sizeof(request_t)); // envoie la requête au serveur

    response_t response;
    Rio_readnb(&rio, &response, sizeof(response_t));
    traiterErreur(response.code); // code exécuté après ça signifie qu'on a pas eu d'erreur

    if(req.typereq == GET){
        requestGETc(&rio, &req, &response, &debut);
    }

    Close(clientfd);
    exit(0);
}