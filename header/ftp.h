#ifndef FTP_H
#define FTP_H

typedef enum {GET}typereq_t;

typedef struct{
    typereq_t typereq;
    char nomfic[MAXLINE];
} request_t;

typedef struct{
    int code;
    int fileSize;
} response_t;

#endif