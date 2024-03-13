// gcc -Wall -std=c99 -Wextra readDataLog.c -o readDataLog

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_SIZE 205 // size of the json string 1 datapoint per minute
//[{"deviceid" : "b827eb278b02", "time" : "2021-12-04 12:40:06","rmsvoltage" : "226.14", "rmscurrent" : "56.884", "totalpower" :"35.999", "energy" : "0.59998", "activepower" : "35.999", powerfactor"="0.33"}]
// transaction information
#define PORT "443"
//#define HOSTNAME "192.168.2.1"
#define HOSTNAME "datalogger.smartenergylab.pt"
#define RESOURCEPATH "save_new_log"

int main()
{
    char *bufBody = NULL;
    char *request = NULL;
    int loggedPoints = 7;
    int transferedPoints = 5;
    int i = 0;
    FILE *ptr;
    ptr = fopen("datalog.json", "r");
    if (ptr == NULL)
    {
        printf("Error unable to read file: \n");
    }
    else
    {
        bufBody = malloc(MAX_SIZE * sizeof(char));
        request = malloc((MAX_SIZE + strlen(bufBody) + 2) * sizeof(char));
        // while (i < transferedPoints)
        // {
        //  fgets(bufBody, MAX_SIZE * sizeof(char), ptr);
        //  printf("Read %d points and didn't transfered\n", i);
        //     i++;
        // }
        for (i = 0; i < loggedPoints; i++)
        {
            if (i < transferedPoints)
            {
                fgets(bufBody, MAX_SIZE * sizeof(char), ptr);
                printf("Read %d points and didn't transfered\n", i);
                continue;
            }
            printf("Entered the loop for transfering\n");
            if (fgets(bufBody, MAX_SIZE * sizeof(char), ptr) != NULL)
            {
                printf("Read datapoint to transfer %d\n", i);
                bufBody[strlen(bufBody) - 1] = '\0';
                sprintf(request, "POST /%s HTTP/1.1\r\nHOST: %s\r\nContent-Type: application/json\r\nContent-Length: %u\r\n\r\n",
                        RESOURCEPATH, HOSTNAME, strlen(bufBody) + 2); // "+2" to cope with the [ and ]
                strcat(request, "[");
                strcat(request, bufBody);
                strcat(request, "]");
                printf("\nLogged Request\n%s\n", request);
                // if (SSL_write(ssl, request, strlen(request)) <= 0)
                // { /* encrypt & send message */
                //     perror("Error SSL_write: ");
                // }
                // else
                // {
                //     bytes = SSL_read(ssl, buf, sizeof(buf)); /* get reply & decrypt */
                //     buf[bytes] = 0;
                //     printf("\nReceived:\n%s\n", buf);
                //     loggedData = FALSE;
                //     loggedPoints = 0;
                //     if (remove("datalog.json") != 0)
                //     {
                //         perror("Error remove datalog.json: ");
                //         // TODO try again next time
                //     }
                // }
            }
        }
        printf("logged point transfered %d transferedPoints %d\n", loggedPoints, transferedPoints);
        if (i == loggedPoints)
        {
            printf("Reset counters and remove file\n");
            loggedPoints = 0;
            transferedPoints = 0;
        }
        printf("logged point transfered %d transferedPoints %d\n", loggedPoints, transferedPoints);
        free(request);
        free(bufBody);
    }
    return 0;
}