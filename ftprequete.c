#include "csapp.h"
#include "ftp.h"

long getfileSize(const char *filename){
    FILE *file = fopen(filename, "rb");
    if(file == NULL){
        perror("fopen");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);
    return size;
}

char* copyFile(const char *filename, long *filesize){
    FILE *file = fopen(filename, "rb");
    if(file == NULL){
        perror("fopen");
        return NULL;
    }

    *filesize = getfileSize(filename);
    if(*filesize == -1){
        fclose(file);
        return NULL;
    }

    char *buffer = malloc(*filesize);
    if (buffer == NULL) {
        perror("malloc");
        fclose(file);
        return NULL;
    }

    int bytesRead = fread(buffer, 1, *filesize, file);
    if (bytesRead != *filesize) {
        perror("fread");
        free(buffer);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return buffer;
}


void ftp(int connfd){
    //char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    //Rio_readlineb(&rio, buf, MAXLINE); // lecture nom de fichier à lire: readlineb car on ne sait pas combien d'octets on va lire
    request_t req;
    Rio_readnb(&rio, req.nomfic, sizeof(req.nomfic));
    //Rio_readnb(&rio, &req.typereq, sizeof(req.typereq));

    char dirname[] = "server/";
    strcat(dirname, req.nomfic);
    strcpy(req.nomfic, dirname);
    FILE* fd = fopen(req.nomfic, "r");
    if(fd){
        fprintf(stderr, "File open\n");
    }
    else{
        fprintf(stderr, "Invalid file name %s\n", req.nomfic);
        return;
    }

    long* fileSize = 0;
    char* file = copyFile(req.nomfic, fileSize);
    response_t response;
    if(!file) response.code = 1; // si erreur
    else response.code = 0;
    response.fileSize = *fileSize;
    Rio_writen(connfd, &response, sizeof(response_t));
    Rio_writen(connfd, file, *fileSize);

    fprintf(stderr, "Successful write on socket\nDisconnecting\n");
}