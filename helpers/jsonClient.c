#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <json-c/json.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>

#include <sys/time.h>

#define MAX_SIZE 123
int main()
{
    char *str;
    char temp_buff[MAX_SIZE];
    char bufStamp[26], usec_buf[6];
    struct tm *tm;
    struct timeval tv;
    int fd = 0;
    struct sockaddr_in demoserverAddr;

    fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0)
    {
        printf("Error : Could not create socket\n");
        return 1;
    }
    else
    {
        demoserverAddr.sin_family = AF_INET;
        demoserverAddr.sin_port = htons(8888);
        demoserverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        memset(demoserverAddr.sin_zero, '\0', sizeof(demoserverAddr.sin_zero));
    }

    if (connect(fd, (const struct sockaddr *)&demoserverAddr, sizeof(demoserverAddr)) < 0)
    {
        printf("ERROR connecting to server\n");
        return 1;
    }

    /*Creating a json object*/
    json_object *jobj = json_object_new_object();

    /******************************************************************************
     * get serial number and/or MACaddress
     ******************************************************************************/
    char id[MAX_SIZE] = "00000000a472de57"; // serial number get method for Rpi
    /*Creating a json string*/
    json_object *jID = json_object_new_string(id);
    /******************************************************************************
     * get timestamp
     ******************************************************************************/
    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);
    strftime(bufStamp, 22, "%Y-%m-%d %H:%M:%S", tm);
    /*Creating a json string*/
    json_object *jTime = json_object_new_string(bufStamp);
    /******************************************************************************
     * Calculate 1-min RMS Voltage and Current
     ******************************************************************************/
    double rmsVoltage = 232.197;
    double rmsCurrent = 1.198;
    /*Creating a json double*/
    json_object *jrmsV = json_object_new_double(rmsVoltage);
    json_object *jrmsI = json_object_new_double(rmsCurrent);
    /******************************************************************************
     * Send last log error number
     ******************************************************************************/
    int lastlogentry = 0;
    /*Creating a json double*/
    json_object *jlastlogentry = json_object_new_int(lastlogentry);

    /******************************************************************************
     * Form the json object
     ******************************************************************************/
    json_object_object_add(jobj, "id", jID);
    json_object_object_add(jobj, "time", jTime);
    json_object_object_add(jobj, "rmsvoltage", jrmsV);
    json_object_object_add(jobj, "rmscurrent", jrmsI);
    json_object_object_add(jobj, "lastlogentry", jlastlogentry);

    // printf("Size of JSON object- %lu\n", sizeof(jobj));
    printf("Size of JSON_TO_STRING- %lu,\n %s\n", sizeof(json_object_to_json_string(jobj)), json_object_to_json_string(jobj));

    // printf("Size of string- %lu\n", sizeof(json_object_to_json_string(jobj)));
    if (strcpy(temp_buff, json_object_to_json_string(jobj)) == NULL)
    {
        perror("strcpy");
        return EXIT_FAILURE;
    }
    if (write(fd, temp_buff, strlen(temp_buff)) == -1)
    {
        perror("write");
        return EXIT_FAILURE;
    }

    printf("Written data %lu\n", strlen(temp_buff));
    return EXIT_SUCCESS;
}