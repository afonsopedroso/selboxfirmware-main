#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <json-c/json.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netdb.h>

#include <string.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#include <sys/time.h>

#define MAX_SIZE 1000
#define PORT 80
#define HOSTNAME "datalogger.smartenergylab.pt"
//#define HOSTNAME "6c46-178-166-100-63.ngrok.io"

//#define DOMAINNAME "datalogger.smartenergylab.pt/save_new_log"
#define RESOURCEPATH "save_new_log"

int socket_connect(char *host, in_port_t port)
{
    struct hostent *hp;
    struct sockaddr_in addr;
    int on = 1, sock;

    if ((hp = gethostbyname(host)) == NULL)
    {
        herror("gethostbyname");
        exit(1);
    }
    bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&on, sizeof(int));

    if (sock == -1)
    {
        perror("setsockopt");
        exit(1);
    }

    if (connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
    {
        perror("connect");
        exit(1);
    }
    return sock;
}

int main()
{
    int fd;
    char msg[MAX_SIZE] = {0};
    char request[MAX_SIZE] = {0};
    char buffer[MAX_SIZE];
    // json_object *jobj = json_object_new_object();
    // json_object *jrmsV = json_object_new_string("123");
    // json_object *jtime = json_object_new_string("2021-10-16 12:43:02");
    // json_object_object_add(jobj, "deviceid", jrmsV);
    // json_object_object_add(jobj, "time", jtime);
    // json_object_object_add(jobj, "rmsvoltage", jrmsV);
    // json_object_object_add(jobj, "rmscurrent", jrmsV);
    // strcat(msg, json_object_to_json_string(jobj));
    sprintf(msg, "{\"deviceid\" : \"b827eb278b02\", \"time\" : \"2021-10-16 12:43:02\", \"rmsvoltage\" : \"215.14\", \"rmscurrent\" : \"1.43\"},{\"deviceid\" : \"b827eb278b02\", \"time\" : \"2021-10-16 12:43:12\", \"rmsvoltage\" : \"25.144\", \"rmscurrent\" : \"0.43\"},{\"deviceid\" : \"b827eb278b02\", \"time\" : \"2021-10-16 12:43:22\", \"rmsvoltage\" : \"22.44\", \"rmscurrent\" : \"10.4\"},{\"deviceid\" : \"b827eb278b02\", \"time\" : \"2021-10-16 12:43:32\", \"rmsvoltage\" : \"100.144\", \"rmscurrent\" : \"100.43\"},{\"deviceid\" : \"b827eb278b02\", \"time\" : \"2021-10-16 12:43:42\", \"rmsvoltage\" : \"245.144\", \"rmscurrent\" : \"10.434\"},{\"deviceid\" : \"b827eb278b02\", \"time\" : \"2021-10-16 12:43:52\", \"rmsvoltage\" : \"265.44\", \"rmscurrent\" : \"11.43\"}");
    sprintf(request, "POST /%s HTTP/1.1\r\nHOST: %s\r\nContent-Type: application/json\r\nContent-Length: %lu\r\n\r\n", RESOURCEPATH, HOSTNAME, strlen(msg) + 2);
    strcat(request, "[");
    strcat(request, msg);
    strcat(request, "]");
    printf("%s\n", request);
    // char pszRequest[100] = {0};
    // char pszResourcePath[] = "";
    // char pszHostAddress[] = HOSTNAME;
    // sprintf(pszRequest, "GET /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\n\r\n", pszResourcePath, pszHostAddress);
    // printf("Created Get Request is below:\n\n\n");
    // printf("%s", pszRequest);
    fd = socket_connect(HOSTNAME, PORT);
    write(fd, request, strlen(request)); // write(fd, char[]*, len);
    bzero(buffer, MAX_SIZE);
    while (read(fd, buffer, MAX_SIZE - 1) != 0)
    {
        fprintf(stderr, "%s", buffer);
        bzero(buffer, MAX_SIZE);
    }
    shutdown(fd, SHUT_RDWR);
    close(fd);

    return 0;
}