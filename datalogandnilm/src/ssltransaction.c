#include "../include/ssltransaction.h"

// #define FALSE 0
// #define TRUE 1

int loggedData = FALSE; // flags the existence of logged data in file
int loggedPoints = 0;
int transferedPoints = 0; // successfully transfered data points

void *sslTransaction(void *msg)
{
    char *body = msg;
    char *bufBody;
    char *request;
    SSL_CTX *ctx;
    int server;
    SSL *ssl;
    char buf[10 * MAX_SIZE];
    int bytes;
    int i = 0;
    setUsedCore(workerCPU);
    SSL_library_init();
    ctx = InitCTX();
    if (ctx == NULL)
    {
        perror("Error ctx == NULL: ");
        logData(body);
    }
    else
    {
        server = OpenConnection();
        if (server == -1)
        {
            perror("Error server = OpenConnection(): ");
            logData(body);
        }
        else
        {
            ssl = SSL_new(ctx); /* create new SSL connection state */
            if (ssl == NULL)
            {
                perror("Error ssl = SSL_new(ctx): ");
                logData(body);
            }
            else
            {
                SSL_set_fd(ssl, server); /* attach the socket descriptor */
                if (SSL_connect(ssl) == -1)
                { /* perform the connection */
                    perror("Error SSL_connect(ssl): ");
                    logData(body);
                }
                else
                {
                    // sends the actual datapoint
                    ShowCerts(ssl); /* get any certs */
                    request = malloc((MAX_SIZE + strlen(body) + 2) * sizeof(char));
                    sprintf(request, "POST /%s HTTP/1.1\r\nHOST: %s\r\nContent-Type: application/json\r\nContent-Length: %u\r\n\r\n",
                            RESOURCEPATH, HOSTNAME, strlen(body) + 2); // "+2" to cope with the [ and ]
                    strcat(request, "[");
                    strcat(request, body);
                    strcat(request, "]");
                    printf("\n%s\n", request);
                    if (SSL_write(ssl, request, strlen(request)) <= 0)
                    { /* encrypt & send message */
                        perror("Error SSL_write: ");
                        logData(body);
                        free(request);
                    }
                    else // if not collected backend doesn't akcnowledge
                    {
                        bytes = SSL_read(ssl, buf, sizeof(buf)); /* get reply & decrypt */
                        buf[bytes] = 0;
                        printf("\nReceived:\n%s\n", buf);
                        free(request);
                        // if theres logged data it sends till the file is over locks the access to log
                        if (loggedData == TRUE)
                        {
                            loggedData = FALSE;
                            pthread_mutex_lock(&loglock);
                            FILE *ptr;
                            ptr = fopen("datalog.json", "r");
                            if (ptr == NULL)
                            {
                                perror("Error unable to read file: ");
                            }
                            else
                            {
                                bufBody = malloc(MAX_SIZE * sizeof(char));
                                request = malloc((MAX_SIZE + strlen(bufBody) + 2) * sizeof(char));
                                for (i = 0; i > loggedPoints; i++)
                                {
                                    if (i < transferedPoints) // to read datapoints that were already transfered
                                    {
                                        fgets(bufBody, MAX_SIZE * sizeof(char), ptr);
                                        printf("Read %d points and didn't transfered\n", i);
                                        continue;
                                    }
                                    if (fgets(bufBody, MAX_SIZE * sizeof(char), ptr) != NULL) // to read and send datapoints that weren't transfered
                                    {
                                        bufBody[strlen(bufBody) - 1] = '\0';
                                        sprintf(request, "POST /%s HTTP/1.1\r\nHOST: %s\r\nContent-Type: application/json\r\nContent-Length: %u\r\n\r\n",
                                                RESOURCEPATH, HOSTNAME, strlen(bufBody) + 2); // "+2" to cope with the [ and ]
                                        strcat(request, "[");
                                        strcat(request, bufBody);
                                        strcat(request, "]");
                                        printf("\nLogged Request\n%s\n", request);
                                        if (SSL_write(ssl, request, strlen(request)) <= 0)
                                        { /* encrypt & send message */
                                            perror("Error SSL_write: ");
                                            break;
                                        }
                                        else
                                        {
                                            bytes = SSL_read(ssl, buf, sizeof(buf)); /* get reply & decrypt */
                                            buf[bytes] = 0;
                                            printf("\nReceived:\n%s\n", buf);
                                        }
                                    }
                                    else
                                    {
                                        perror("Error fgets(bufBody: ");
                                    }
                                }
                                fclose(ptr);
                                free(request);
                                free(bufBody);
                                if (i == loggedPoints)
                                {
                                    if (remove("datalog.json") != 0)
                                    {
                                        perror("Error remove datalog.json: ");
                                    }
                                    else
                                    {
                                        loggedPoints = 0;
                                        transferedPoints = 0;
                                    }
                                }
                                else
                                {
                                    loggedData = TRUE;
                                    transferedPoints = i;
                                }
                            }
                            pthread_mutex_unlock(&loglock);
                        }
                    }
                }
                SSL_free(ssl); /* release connection state */
            }
            close(server); /* close socket */
        }
        SSL_CTX_free(ctx); /* release context */
    }

    return 0;
}

int logData(char *body)
{
    FILE *ptr;
    pthread_mutex_lock(&loglock);
    ptr = fopen("datalog.json", "a");
    fprintf(ptr, "%s\n", body);
    fclose(ptr);
    loggedData = TRUE;
    loggedPoints++;
    pthread_mutex_unlock(&loglock);
    return 0;
}

int OpenConnection(void)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sd, s;
    /* Obtain address(es) matching host/port. */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = IPPROTO_TCP; /* Any protocol */
    s = getaddrinfo(HOSTNAME, PORT, &hints, &result);
    if (s != 0)
    {
        perror("Error getaddrinfo: ");
        return -1;
    }
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        sd = socket(rp->ai_family, rp->ai_socktype,
                    rp->ai_protocol);
        if (sd == -1)
        {
            perror("Error socket: ");
            continue;
        }
        if (connect(sd, rp->ai_addr, rp->ai_addrlen) != -1)
        {
            break; /* Success */
        }
        else
        {
            perror("Error connect: ");
            sd = -1;
        }
    }
    return sd;
}

SSL_CTX *InitCTX(void)
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;
    OpenSSL_add_all_algorithms(); /* Load cryptos, et.al. */
    SSL_load_error_strings();     /* Bring in and register error messages */
    method = TLS_client_method(); /* Create new client-method instance */
    ctx = SSL_CTX_new(method);    /* Create new context */
    return ctx;
}
void ShowCerts(SSL *ssl)
{
    X509 *cert;
    char *line;
    cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */
    if (cert != NULL)
    {
        // printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        // printf("Subject: %s\n", line);
        free(line); /* free the malloc'ed string */
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        // printf("Issuer: %s\n", line);
        free(line);      /* free the malloc'ed string */
        X509_free(cert); /* free the malloc'ed certificate copy */
    }
    else
        printf("Info: No client certificates configured.\n");
}

/* gethostbyname() is obsolete */
// int OpenConnection(void)
//{
// int sd;
// struct hostent *host;
// struct sockaddr_in addr;
// if ((host = gethostbyname(HOSTNAME)) == NULL)
// {
//     perror("Error : hostname");
//     // abort();
// }
// sd = socket(PF_INET, SOCK_STREAM, 0);
// bzero(&addr, sizeof(addr));
// addr.sin_family = AF_INET;
// addr.sin_port = htons(PORT);
// addr.sin_addr.s_addr = *(long *)(host->h_addr);
// if (connect(sd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
// {
//     close(sd);
//     perror("Error : connect");
//     // abort();
// }
// return sd;
//}

/******************************************************************************
function:  unsecure data transaction
parameter:
Info:
******************************************************************************/

// int socket_connect(char *host, in_port_t port)
// {
//     struct hostent *hp;
//     struct sockaddr_in addr;
//     int on = 1, sock;

//     if ((hp = gethostbyname(host)) == NULL)
//     {
//         herror("gethostbyname");
//         exit(1);
//     }
//     bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);
//     addr.sin_port = htons(port);
//     addr.sin_family = AF_INET;
//     sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
//     setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&on, sizeof(int));

//     if (sock == -1)
//     {
//         perror("setsockopt");
//         exit(1);
//     }

//     if (connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1)
//     {
//         perror("connect");
//         exit(1);
//     }
//     return sock;
// }
