#ifndef FTP_H
#define FTP_H

#define PACKET_SIZE 4096

typedef enum {GET, BYE, FAUX}typereq_t;

typedef struct{
    typereq_t typereq;
    char nomfic[MAXLINE];
    long offset;
} request_t;

typedef struct{
    int code;
    size_t fileSize;
    int nbPackets;
    size_t lastPacketSize;
} response_t;

#endif