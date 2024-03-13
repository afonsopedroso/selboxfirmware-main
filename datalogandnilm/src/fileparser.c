#include "../include/fileparser.h"

// to enable the removal of the oldest file
int removeOldFile = 0;

/******************************************************************************
function:  Writes the raw data file
parameter:
    structure array with ADC1 and ADC2 values
Info:
******************************************************************************/
void *writeFile1(void *datapoint)
{
    char FILENAME[60];
    FILE *fpt;
    char bufStamp[26]; //, usec_buf[6]; //, bufVoltage[seconds2save * wSize * (8) + 1], bufCurrent[seconds2save * wSize * (8) + 1], buffer[6];
    char buff[70];
    // struct tm *tm;
    // struct timeval tv;
    int i;
    setUsedCore(writerCPU);
    /******************************************************************************
    Info: gets timestamp before writting (~1min delay from readings) and writes a
    .txt file with 450000 data points with the format timestamp/voltage points
    separated by ","/current points separated by ",".
    ******************************************************************************/
    while (1)
    {
        pthread_mutex_lock(&lock1);
        strcpy(bufStamp, timeStamp1);
        i = 0;
        while (bufStamp[i] != '\0')
        {
            if (bufStamp[i] == ' ')
                bufStamp[i] = ',';
            i++;
        }
        struct data_point *arr_datapoint = (struct data_point *)datapoint;
        // gettimeofday(&tv, NULL);
        // tm = localtime(&tv.tv_sec);
        // strftime(bufStamp, 22, "%Y-%m-%d,%H:%M:%S", tm);
        // strcat(bufStamp, ".");
        // sprintf(usec_buf, "%d", (int)tv.tv_usec);
        // strcat(bufStamp, usec_buf);
        sprintf(FILENAME, "/home/sel/data/%s.txt", bufStamp);
        printf("Filename %s\n", FILENAME);
        fpt = fopen(FILENAME, "w");
        fprintf(fpt, "%s/", bufStamp);
        for (i = 0; i < seconds2save * wSize; i++)
        {
            fprintf(fpt, "%d,", arr_datapoint[i].voltage);
        }
        fprintf(fpt, "/");
        for (i = 0; i < seconds2save * wSize; i++)
        {
            fprintf(fpt, "%d,", arr_datapoint[i].current);
        }
        fclose(fpt);
        bufStamp[0] = '\0';
        sprintf(buff, "gzip -f %s", FILENAME);
        // printf("compress file %s\n", buff);
        system(buff);
        if (removeOldFile == 1)
        {
            if (pthread_create(&ti[0], NULL, removeFile, NULL) != 0)
            {
                perror("Error removeFile pthread create: ");
            }
            if (pthread_detach(ti[0]) != 0)
            {
                perror("Error removeFile pthread detach: ");
            }
        }
    }
}

void *writeFile2(void *datapoint)
{
    char FILENAME[60];
    FILE *fpt;
    char bufStamp[26]; //, usec_buf[6]; //, bufVoltage[seconds2save * wSize * (8) + 1], bufCurrent[seconds2save * wSize * (8) + 1], buffer[6];
    char buff[70];
    // struct tm *tm;
    // struct timeval tv;
    int i;
    setUsedCore(writerCPU);
    /******************************************************************************
    Info: gets timestamp before writting (~1min delay from readings) and writes a
    .txt file with 450000 data points with the format timestamp/voltage points
    separated by ","/current points separated by ",".
    ******************************************************************************/
    while (1)
    {
        pthread_mutex_lock(&lock2);
        strcpy(bufStamp, timeStamp2);
        i = 0;
        while (bufStamp[i] != '\0')
        {
            if (bufStamp[i] == ' ')
                bufStamp[i] = ',';
            i++;
        }
        struct data_point *arr_datapoint = (struct data_point *)datapoint;
        // gettimeofday(&tv, NULL);
        // tm = localtime(&tv.tv_sec);
        // strftime(bufStamp, 22, "%Y-%m-%d,%H:%M:%S", tm);
        // strcat(bufStamp, ".");
        // sprintf(usec_buf, "%d", (int)tv.tv_usec);
        // strcat(bufStamp, usec_buf);
        sprintf(FILENAME, "/home/sel/data/%s.txt", bufStamp);
        printf("Filename %s\n", FILENAME);
        fpt = fopen(FILENAME, "w");
        fprintf(fpt, "%s/", bufStamp);
        for (i = 0; i < seconds2save * wSize; i++)
        {
            fprintf(fpt, "%d,", arr_datapoint[i].voltage);
        }
        fprintf(fpt, "/");
        for (i = 0; i < seconds2save * wSize; i++)
        {
            fprintf(fpt, "%d,", arr_datapoint[i].current);
        }
        fclose(fpt);
        bufStamp[0] = '\0';
        sprintf(buff, "gzip -f %s", FILENAME);
        // printf("compress file %s\n", buff);
        system(buff);
        if (removeOldFile == 1)
        {
            if (pthread_create(&ti[1], NULL, removeFile, NULL) != 0)
            {
                perror("Error removeFile pthread create: ");
            }
            if (pthread_detach(ti[1]) != 0)
            {
                perror("Error removeFile pthread detach: ");
            }
        }
    }
}

/******************************************************************************
function:  Removes oldest file when logged data exceeds the window defined in
logFileN in the vars.h file
parameter:
Info:
******************************************************************************/
void *removeFile()
{
    DIR *d;
    struct dirent *dir;
    int i = 0;
    char fileNames[logFileN + 50][100];
    char file2delete[100];
    char command[100];
    struct tm *tm;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);
    strftime(file2delete, 22, "%Y-%m-%d %H:%M:%S", tm);
    printf("%s\n", file2delete);
    time_t t1 = mktime(tm); // t is now your desired time_t
    time_t t2;
    printf("t1 %ld, date %s\n", t1, file2delete);
    double seconds;
    setUsedCore(writerCPU);
    printf("Thread to delete\n");
    d = opendir(dataPath);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0)
            {
                strcpy(fileNames[i], dir->d_name);
                fileNames[i][strlen(fileNames[i]) - 7] = '\0';
                fileNames[i][strlen(fileNames[i]) - 9] = ' ';
                printf("%s\n", fileNames[i]);
                strptime(fileNames[i], "%F %T", tm);
                t2 = mktime(tm);
                seconds = difftime(t1, t2);
                printf("t1 %ld, t2 %ld, seconds %f\n", t1, t2, seconds);
                printf("FileNames %s, %d\n", fileNames[i], i);
                if (seconds > 0)
                {
                    strcpy(file2delete, fileNames[i]);
                    printf("%s\n", file2delete);
                    t1 = t2;
                }
                i++;
            }
        }
        closedir(d);
        if (i > logFileN)
        {
            file2delete[strlen(file2delete) - 9] = ',';
            command[0] = '\0';
            strcat(command, dataPath);
            strcat(file2delete, ".txt.gz");
            strcat(command, file2delete);
            printf("Delete File:\t %s\n", command);
            remove(command);
        }
    }
    return 0;
}
/******************************************************************************
function:  Compress a file in thread
parameter:
Info:
******************************************************************************/
// void *compressFile(void *arg)
// {
//     char buffer[50];
//     char *file;
//     while (1)
//     {
//         pthread_mutex_lock(&compress);
//         sprintf(buffer, "gzip -f %s\n", FILE2COMPRESS);
//         printf("compress file %s\n", buffer);
//         system(buffer);
//     }
// }
