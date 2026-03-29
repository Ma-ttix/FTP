#include "csapp.h"
#include "ftp.h"

long getfileSize(const char *filename){
    FILE *file = fopen(filename, "rb");
    if(file == NULL){
        perror("fopen");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fclose(file);
    return size;
}

int requestGETs(int connfd, request_t req){
    response_t response;
    char tmp[MAXLINE + 7]; // + 7 pour la taille de "server/"
    snprintf(tmp, sizeof(tmp), "server/%s", req.nomfic);
    strcpy(req.nomfic, tmp);
    FILE* fd = fopen(req.nomfic, "rb");
    if(fd){
        fprintf(stdout, "File %s open\n", req.nomfic);
    }
    else{
        perror("fopen");
        response.code = htonl(1);
        response.fileSize = htobe64(0);
        Rio_writen(connfd, &response, sizeof(response_t));
        fprintf(stdout, "Invalid file name %s\n", req.nomfic);
        return 0;
    }

    size_t fileSize = getfileSize(req.nomfic);
    char* file = malloc(PACKET_SIZE);
    if(file == NULL){
        perror("malloc");
        response.code = htonl(2);
        response.fileSize = htobe64(0);
        Rio_writen(connfd, &response, sizeof(response_t));
        fprintf(stdout, "Unsuccessful write on socket\n");
        fclose(fd);
        return 0;
    }

    if(fseek(fd, req.offset, SEEK_SET)!=0){
        perror("fseek");
        response.code = htonl(2);
        response.fileSize = htobe64(0);
        Rio_writen(connfd, &response, sizeof(response_t));
        fprintf(stdout, "Unsuccessful write on socket\n");
        fclose(fd);
        return 0;
    }

    response.code = htonl(0);
    response.fileSize = htobe64(fileSize - req.offset);
    response.nbPackets = htonl((fileSize - req.offset)/PACKET_SIZE); // nombre de paquets entiers
    response.lastPacketSize = htobe64((fileSize - req.offset)%PACKET_SIZE); // taille du dernier paquet
    Rio_writen(connfd, &response, sizeof(response_t));

    size_t sizePacket;
    int i = 0;
    while((sizePacket = fread(file, 1, PACKET_SIZE, fd)) > 0){ // dans cet ordre car fread renvoie le nombre de paquets lus (pas le nombre d'octets) donc problème pour le dernier paquet si dans l'autre sens
        if(rio_writen(connfd, file, sizePacket) < 0){ // utilisation de rio_writen (non Rio_writen) afin de gérer l'erreur et de ne pas tuer le processus
            if(errno == EPIPE || errno == ECONNRESET){
                fprintf(stdout, "Client disconnected during transfer\n");
                free(file);
                fclose(fd);
                return 1;
            }
            perror("rio_writen");
            free(file);
            fclose(fd);
            return 1;
            }
        #ifdef TALK
        fprintf(stdout, "writing packet %d with size %ld\n", i, sizePacket);
        #endif
        i++;
    }
    fprintf(stdout, "Successful write on socket\n");
    free(file);
    fclose(fd);
    return 0;
}

int ftp(int connfd){
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    request_t req;
    Rio_readnb(&rio, &req, sizeof(request_t));
    req.typereq = ntohl(req.typereq);
    req.offset = be64toh(req.offset);

    if(req.typereq == GET){
        int ret = requestGETs(connfd, req);
        return ret;
    }
    else if(req.typereq == BYE){
        response_t response;
        response.code = htonl(0);
        response.fileSize = htobe64(0);
        Rio_writen(connfd, &response, sizeof(response_t));
        return 1;
    }
    else{
        response_t response;
        response.code = htonl(3);
        response.fileSize = htobe64(0);
        Rio_writen(connfd, &response, sizeof(response_t));
        return 0;
    }
}