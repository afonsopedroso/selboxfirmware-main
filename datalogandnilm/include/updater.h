#ifndef UPDATER_H_
#define UPDATER_H_

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
#include <sys/time.h>

#include <json-c/json.h>

#define UPDATEHOSTNAME "datalogger.smartenergylab.pt"
#define UPDATEPATH "firmware_update_check"
#define DOWNLOADURL "https://nilmfirmware.s3.us-east-2.amazonaws.com/"
#define CONFIRMATIONPATH "confirm_firmware_update"
#define PORT "443"

#define MAX_SIZE 128

char mac[20];

void getMacAddr(void);
SSL_CTX *InitCTX(void);
void ShowCerts(SSL *ssl);
int OpenConnection();
void *sslTransaction(void *msg);

#endif