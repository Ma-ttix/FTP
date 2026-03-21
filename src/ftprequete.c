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

void requestGETs(int connfd, request_t req){
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
        response.code = 1;
        response.fileSize = 0;
        Rio_writen(connfd, &response, sizeof(response_t));
        fprintf(stdout, "Invalid file name %s\n", req.nomfic);
        fclose(fd);
        return;
    }

    size_t fileSize = getfileSize(req.nomfic);
    char* file = malloc(PACKET_SIZE);
    if(file == NULL){
        perror("malloc");
        response.code = 2;
        response.fileSize = 0;
        Rio_writen(connfd, &response, sizeof(response_t));
        fprintf(stdout, "Unsuccessful write on socket\n");
        fclose(fd);
        return;
    }

    response.code = 0;
    response.fileSize = fileSize;
    response.nbPackets = fileSize/PACKET_SIZE; // nombre de paquets entiers
    response.lastPacketSize = fileSize%PACKET_SIZE; // taille du dernier paquet
    Rio_writen(connfd, &response, sizeof(response_t));

    size_t sizePacket;
    int i = 0;
    while((sizePacket = fread(file, 1, PACKET_SIZE, fd)) > 0){
        Rio_writen(connfd, file, sizePacket);
        fprintf(stderr, "writing packet %d with size %ld\n", i, sizePacket);
        i++;
    }
    fclose(fd);
}

int ftp(int connfd){
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    request_t req;
    Rio_readnb(&rio, &req, sizeof(request_t));

    if(req.typereq == GET){
        requestGETs(connfd, req);
        return 0;
    }
    else if(req.typereq == BYE){
        response_t response;
        response.code = 0;
        response.fileSize = 0;
        Rio_writen(connfd, &response, sizeof(response_t));
        return 1;
    }
    else{
        response_t response;
        response.code = 3;
        response.fileSize = 0;
        Rio_writen(connfd, &response, sizeof(response_t));
        return 0;
    }
}