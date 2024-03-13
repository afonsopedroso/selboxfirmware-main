#ifndef SSLTRANSACTION_H_
#define SSLTRANSACTION_H_

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
//#include <openssl/applink.c>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <fcntl.h>
#include <netinet/tcp.h>

#include "../include/vars.h"

int logData(char *body);
SSL_CTX *InitCTX(void);
extern void setUsedCore(int CPU);
void ShowCerts(SSL *ssl);
int OpenConnection();
void *sslTransaction(void *msg);
// int socket_connect(char *host, in_port_t port); //uses obsolete functions

#endif