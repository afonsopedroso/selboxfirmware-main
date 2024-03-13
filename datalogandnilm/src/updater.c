#include "../include/updater.h"

/******************************************************************************
    if connection fails program stops
******************************************************************************/

int main()
{
    SSL_CTX *ctx;
    int server;
    SSL *ssl;
    char buf[10 * MAX_SIZE];
    char msg[2 * MAX_SIZE] = {0};
    char request[10 * MAX_SIZE] = {0};
    int bytes;
    struct json_object *parsed_json;
    struct json_object *installed_firmware_version;
    struct json_object *permitted_firmware_upgrade_version;
    struct json_object *firmware_update_key;
    FILE *fp;
    char buffer[1024];
    struct json_object *file_json;
    struct json_object *jdeviceid;
    struct json_object *jfirmware;
    struct json_object *jhardware;
    float firmwareVer, hardwareVer, newFirmwareVer;
    char download[200];
    char filename[10];
    char unzip[20];

    getMacAddr();
    /******************************************************************************
        reads nilm.conf file
    ******************************************************************************/

    fp = fopen("../device.conf", "r");
    fread(buffer, 1024, 1, fp);
    fclose(fp);

    file_json = json_tokener_parse(buffer);

    json_object_object_get_ex(file_json, "deviceid", &jdeviceid);
    json_object_object_get_ex(file_json, "firmware", &jfirmware);
    json_object_object_get_ex(file_json, "hardware", &jhardware);

    // strcpy(mac, json_object_get_string(jdeviceid));
    firmwareVer = atof(json_object_get_string(jfirmware));
    hardwareVer = atof(json_object_get_string(jhardware));

    SSL_library_init();
    ctx = InitCTX();
    if (ctx == NULL)
    {
        perror("Error ctx == NULL: ");
    }
    else
    {
        server = OpenConnection();
        if (server == -1)
        {
            perror("Error server = OpenConnection(): ");
            return -1;
        }
        else
        {
            ssl = SSL_new(ctx); /* create new SSL connection state */
            if (ssl == NULL)
            {
                perror("Error ssl = SSL_new(ctx): ");
                return -1;
            }
            else
            {
                SSL_set_fd(ssl, server); /* attach the socket descriptor */
                if (SSL_connect(ssl) == -1)
                { /* perform the connection */
                    perror("Error SSL_connect(ssl): ");
                    return -1;
                }
                else
                {
                    sprintf(msg, "{\"version\" : \"%0.2f\", \"deviceid\" : \"%s\"}", firmwareVer, mac);
                    sprintf(request, "POST /%s HTTP/1.1\r\nHOST: %s\r\nContent-Type: application/json\r\nContent-Length: %u\r\n\r\n", UPDATEPATH, UPDATEHOSTNAME, strlen(msg));
                    strcat(request, msg);
                    printf("%s\n", request);
                    if (SSL_write(ssl, request, strlen(request)) <= 0)
                    { /* encrypt & send message */
                        perror("Error SSL_write: ");
                        return -1;
                    }
                    else
                    {
                        bytes = SSL_read(ssl, buf, sizeof(buf)); /* get reply & decrypt */
                        buf[bytes] = 0;
                        printf("\nReceived:\n%s\n", buf);
                    }
                }
                SSL_free(ssl); /* release connection state */
            }

            close(server); /* close socket */
        }
        SSL_CTX_free(ctx); /* release context */
    }
    if (buf == NULL)
    {
        printf("No update permitted\n");
        return 0;
    }

    char *content = strstr(buf, "\r\n\r\n");
    if (content != NULL)
    {
        content += 4; // Offset by 4 bytes to start of content
    }
    else
    {
        content = buf; // Didn't find end of header, write out everything
    }
    // printf("\nParser:\n%s\n\n", content);

    parsed_json = json_tokener_parse(content);
    json_object_object_get_ex(parsed_json, "installed_firmware_version", &installed_firmware_version);
    json_object_object_get_ex(parsed_json, "permitted_firmware_upgrade_version", &permitted_firmware_upgrade_version);
    json_object_object_get_ex(parsed_json, "firmware_update_key", &firmware_update_key);
    printf("installed_firmware_version: %s\n", json_object_get_string(installed_firmware_version));
    printf("permitted_firmware_upgrade_version: %s\n", json_object_get_string(permitted_firmware_upgrade_version));
    printf("firmware_update_key: %s\n\n", json_object_get_string(firmware_update_key));
    if (json_object_get_string(installed_firmware_version) == NULL)
        return 0;
    newFirmwareVer = atof(json_object_get_string(permitted_firmware_upgrade_version));
    if (firmwareVer != newFirmwareVer) // allows both update or downgrade
    {
        system("sudo pkill -15 nilm;");
        system("rm *.zip");
        download[0] = '\0';
        strcat(download, "wget ");
        strcat(download, DOWNLOADURL);
        strcat(download, json_object_get_string(permitted_firmware_upgrade_version));
        strcat(download, ".zip?key=");
        strcat(download, json_object_get_string(firmware_update_key));
        strcat(download, " -O ");
        filename[0] = '\0';
        strcat(filename, json_object_get_string(permitted_firmware_upgrade_version));
        strcat(filename, ".zip");
        strcat(download, filename);
        printf("wget command: %s\n", download);
        system(download);
        system("mkdir tmp");
        unzip[0] = '\0';
        strcat(unzip, "sudo unzip ");
        strcat(unzip, filename);
        strcat(unzip, " -d tmp");
        printf("unzip command: %s\n", unzip);
        system(unzip);
        //  system("mv tmp/updater/update.sh update.sh");
        //  system("sudo rm ../updater/updater");
        //  system("sudo cp sel/updater/updater ../updater/updater;");
        //  system("sudo rm ../firmware/nilm");
        //  system("sudo cp sel/firmware/nilm ../firmware/nilm;");
        //  system("sudo rm ../firmware/startApp.sh;");
        //  system("sudo cp sel/firmware/startApp.sh ../firmware/startApp.sh;");
        //  system("rm *.zip");
        //  system("sudo rm -rf sel;");

        buf[0] = '\0';
        msg[0] = '\0';
        firmwareVer = newFirmwareVer;
        char bufStamp[26];
        struct tm *tm;
        struct timeval tv;
        gettimeofday(&tv, NULL);
        tm = localtime(&tv.tv_sec);
        strftime(bufStamp, 22, "%Y-%m-%d %H:%M:%S", tm);
        fp = fopen("../device.conf", "w");
        sprintf(buf, "{\"deviceid\" : \"%s\", \"firmware\" : \"%0.2f\", \"hardware\" : \"%0.2f\"}", mac, firmwareVer, hardwareVer);
        printf("%s\n", buf);
        fputs(buf, fp);
        fclose(fp);

        SSL_library_init();
        ctx = InitCTX();
        if (ctx == NULL)
        {
            perror("Error ctx == NULL: ");
        }
        else
        {
            server = OpenConnection();
            if (server == -1)
            {
                perror("Error server = updateConnection(): ");
                return -1;
            }
            else
            {
                ssl = SSL_new(ctx); /* create new SSL connection state */
                if (ssl == NULL)
                {
                    perror("Error ssl = SSL_new(ctx): ");
                    return -1;
                }
                else
                {
                    SSL_set_fd(ssl, server); /* attach the socket descriptor */
                    if (SSL_connect(ssl) == -1)
                    { /* perform the connection */
                        perror("Error SSL_connect(ssl): ");
                        return -1;
                    }
                    else
                    {
                        sprintf(msg, "{\"deviceid\" : \"%s\", \"version\" : \"%0.2f\", \"time\" : \"%s\" }", mac, firmwareVer, bufStamp);
                        sprintf(request, "POST /%s HTTP/1.1\r\nHOST: %s\r\nContent-Type: application/json\r\nContent-Length: %u\r\n\r\n", CONFIRMATIONPATH, UPDATEHOSTNAME, strlen(msg));
                        strcat(request, msg);
                        printf("%s\n", request);
                        if (SSL_write(ssl, request, strlen(request)) <= 0)
                        { /* encrypt & send message */
                            perror("Error SSL_write: ");
                            return -1;
                        }
                        else
                        {
                            bytes = SSL_read(ssl, buf, sizeof(buf)); /* get reply & decrypt */
                            buf[bytes] = 0;
                            printf("\nReceived:\n%s\n", buf);
                        }
                    }
                    SSL_free(ssl); /* release connection state */
                }

                close(server); /* close socket */
            }
            SSL_CTX_free(ctx); /* release context */
        }
        system("sudo /home/sel/updater/tmp/updater/update.sh &");
        // system("sudo reboot");
    }
    return 0;
}

void getMacAddr(void)
{
    int fdMAC;
    struct ifreq ifr;
    char *iface = "wlan0";
    fdMAC = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
    ioctl(fdMAC, SIOCGIFHWADDR, &ifr);
    close(fdMAC);
    sprintf(mac, "%.2x%.2x%.2x%.2x%.2x%.2x",
            (unsigned char)ifr.ifr_hwaddr.sa_data[0],
            (unsigned char)ifr.ifr_hwaddr.sa_data[1],
            (unsigned char)ifr.ifr_hwaddr.sa_data[2],
            (unsigned char)ifr.ifr_hwaddr.sa_data[3],
            (unsigned char)ifr.ifr_hwaddr.sa_data[4],
            (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
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

    s = getaddrinfo(UPDATEHOSTNAME, PORT, &hints, &result);
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