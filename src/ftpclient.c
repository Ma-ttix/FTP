/*
 * ftpclient.c
 */
#include "csapp.h"
#include "ftp.h"
#include <time.h>

void parseString(char* s, char*** adrRes){
    char** res = *adrRes;
    res[0] = strtok(s, " ");
    res[0][strcspn(res[0], "\n")] = '\0'; // si commande simple (sans 2ème argument) on va enlever le \n afin que traiterNomCommande reconnaise bien la commande
    res[1] = strtok(NULL, " ");
    if(res[1]){ // si on a un 2ème argument
        res[1][strcspn(res[1], "\n")] = '\0'; // strcspn retourne l'index de la première occurrence de \n dans buf, par accès au tableau on remplace donc le \n par \0, afin que le nom du fichier soit correct pour Fopen (echo.c\0 est valide mais pas echo.c\n\0)
    }
    /*if(!res[1]){
        fprintf(stdout, "Missing argument\nUsage: <commande> <filename>\n");
        exit(1);
    }*/
}

void traiterNomCommande(request_t* req, char* mot){
    if(strcmp(mot, "get") == 0) req->typereq = GET;
    //else if(strcmp(mot, "ls") == 0) req->typereq = LS;
    //else if(strcmp(mot, "rm") == 0) req->typereq = RM;
    //else if(strcmp(mot, "put") == 0) req->typereq = PUT;
    else if(strcmp(mot, "bye") == 0) req->typereq = BYE;
    else req->typereq = FAUX;
}

int traiterErreur(int code){
    switch(code){
        case 1:
            fprintf(stderr, "Error: Invalide file name\n");
            return 1;
        case 2:
            fprintf(stderr, "Error: Unsuccessful write from server\n");
            return 1;
        case 3:
            fprintf(stderr, "Error: Invalid command name\n");
            return 1;
        default:
            return 0;
    }
}

void gestionPanne(request_t* req){
    char crash[MAXLINE + 7]; // + 7 pour la taille de "client/"
    snprintf(crash, sizeof(crash), "client/%s", req->nomfic);
    struct stat buffer;
    if(stat(crash, &buffer)!=0) return; // si pas de fichier de crash -> aucun traitement
    if(!S_ISREG(buffer.st_mode)) return;  // existe mais c'est pas un fichier

    FILE* file = fopen(crash, "ab");
    if(!file){
        perror("fopen");
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    req->offset = ftell(file);
    fclose(file);
}

void requestGETc(rio_t* rio, request_t* req, response_t* response, struct timeval* debut){
    char tmp[MAXLINE + 7]; // + 7 pour la taille de "client/"
    snprintf(tmp, sizeof(tmp), "client/%s", req->nomfic);
    strcpy(req->nomfic, tmp);
    char* mode;
    if(req->offset!=0) mode = "ab";
    else mode = "wb";
    FILE* fd = fopen(req->nomfic, mode);
    if(!fd){
        perror("fopen");
        exit(1);
    }

    char* packet = malloc(PACKET_SIZE);
    if(packet == NULL){
        perror("malloc");
        fclose(fd);
        exit(1);
    }

    fseek(fd, req->offset, SEEK_SET); // potentiellement inutile car ab place à la fin et wb écrase le fichier donc fichier vide (peut être source d'erreur)
    size_t fileSize = response->fileSize;
    int i = 0;
    while(i!=response->nbPackets){
        size_t packetRead = Rio_readnb(rio, packet, PACKET_SIZE);
        size_t bytesWritten = fwrite(packet, 1, packetRead, fd);
        fflush(fd);
        if(bytesWritten!=packetRead){
            perror("fwrite");
            free(packet);
            fclose(fd);
            exit(1);
        }
        #ifdef TALK
        fprintf(stdout, "writing packet %d\n", i);
        #endif
        i++;
    }

    if(response->lastPacketSize!=0){
        size_t packetRead = Rio_readnb(rio, packet, response->lastPacketSize);
        size_t bytesWritten = fwrite(packet, 1, packetRead, fd);
        fflush(fd);
        if(bytesWritten!=packetRead){
            perror("fwrite");
            free(packet);
            fclose(fd);
            exit(1);
        }
        #ifdef TALK
        fprintf(stdout, "writing packet %d\n", i);
        #endif
    }
    #ifdef TALK
    fprintf(stdout,"File Size: %ld Packets: %d, Last Packet Size: %ld\n", response->fileSize, response->nbPackets, response->lastPacketSize);
    fprintf(stdout, "Offset: %ld\n", req->offset);
    #endif

    fprintf(stdout, "Successful write on file: %s\n", req->nomfic);

    free(packet);
    fclose(fd);

    struct timeval fin;
    gettimeofday(&fin, NULL);
    double duree = (double)(fin.tv_sec - debut->tv_sec) + (double)(fin.tv_usec - debut->tv_usec) / 1000000;
    double debitK = fileSize / (1000 * duree);
    fprintf(stdout, "%ld bytes received in %f seconds (%.2f Kbytes/s)\n", fileSize, duree, debitK);

}

int main(int argc, char **argv)
{
    int clientfd, port;
    char *host;//, buf[MAXLINE];
    rio_t rio;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <host> \n", argv[0]);
        exit(1);
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

    while(1){
        Rio_readinitb(&rio, clientfd);

        char ficin[MAXLINE];
        printf("ftp>: ");
        Fgets(ficin, MAXLINE, stdin); // afin de lire la requête de l'utilisateur
        struct timeval debut;
        gettimeofday(&debut, NULL);
        char** reqUser = malloc(2*sizeof(char*));
        if(!reqUser){
            perror("malloc");
            exit(1);
        }
        parseString(ficin, &reqUser);

        request_t req; //= malloc(sizeof(request_t));
        strcpy(req.nomfic, ""); // initialisation du nom de fichier à vide
        req.offset = 0; // initialisation du décalage à 0
        traiterNomCommande(&req, reqUser[0]);
        if(reqUser[1]) strcpy(req.nomfic, reqUser[1]); // si on a un 2ème argument (if nécessaire sinon segfault)
        free(reqUser);
        if(req.typereq == GET && (strcmp(req.nomfic, "") == 0)){ // si le nom de fichier est vide (si get avec un seul argument)
            fprintf(stderr, "Missing file name\nUsage: get <filename>\n");
            continue;
        }
        gestionPanne(&req);

        Rio_writen(clientfd, &req, sizeof(request_t)); // envoie la requête au serveur

        response_t response;
        Rio_readnb(&rio, &response, sizeof(response_t));
        if(traiterErreur(response.code) == 1) continue; // code exécuté après ça signifie qu'on a pas eu d'erreur

        if(req.typereq == GET){
            requestGETc(&rio, &req, &response, &debut);
        }
        else if(req.typereq == BYE){
            break;
        }
    }

    Close(clientfd);
    exit(0);
}