/*
 * ftpclient.c
 */
#include "csapp.h"
#include "ftp.h"

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
    printf("Enter file input name: ");
    Fgets(ficin, MAXLINE, stdin); // afin de lire avec \n pour l'envoyer (et surtout le lire facilement)
    ficin[strcspn(ficin, "\n")] = '\0'; // strcspn retourne l'index de la première occurrence de \n dans buf, par accès au tableau on remplace donc le \n par \0, afin que le nom du fichier soit correct pour Fopen (echo.c\0 est valide mais pas echo.c\n\0)
    request_t req; //= malloc(sizeof(request_t));
    strcpy(req.nomfic, ficin);
    req.typereq = GET;
    printf("nomFic: %s\n", req.nomfic);
    Rio_writen(clientfd, req.nomfic, strlen(req.nomfic)); // envoie le nom du fichier au serveur
    //Rio_writen(clientfd, htons(req.typereq), sizeof(req.typereq));

    char ficout[] = "client/";
    strcat(ficout, req.nomfic);
    FILE* fd = fopen(ficout, "w");
    if(!fd) perror("fopen ficout");

    /*while (Rio_readlineb(&rio, buf, MAXLINE) > 0){
        Fputs(buf, fd);
    }*/
    response_t response;
    Rio_readnb(&rio, &response, sizeof(response_t));
    if(response.code == 1) exit(1); // traiter cas erreur
    char *buffer = malloc(response.fileSize);
    Rio_readnb(&rio, buffer, response.fileSize);

    int bytesWritten = fwrite(buffer, 1, response.fileSize, fd);
    if(bytesWritten != response.fileSize){
        perror("Erreur lors de l'écriture dans le fichier");
    }

    fprintf(stderr, "Successful write on file: %s\n", ficout);
    fclose(fd);
    Close(clientfd);
    exit(0);
}