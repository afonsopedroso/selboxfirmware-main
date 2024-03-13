#include <stdio.h>
#include <dirent.h>
#include <time.h>
#include <string.h>

#define dataPath "/Users/diogobrito/Projects/data/testFolder/"
#define logFileN 60 // logged time in minutes
int main(void)
{
    DIR *d;
    struct dirent *dir;
    int i = 0;
    char fileNames[60 * 24 * 5][100];
    char file2delete[100] = "2100-01-01 00:00:00\0";
    char command[100];
    struct tm tm;
    strptime(file2delete, "%Y-%m-%D %H:%M:%S", &tm);
    time_t t1 = mktime(&tm), t2; // t is now your desired time_t
    double seconds;
    d = opendir(dataPath);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            strcpy(fileNames[i], dir->d_name);
            fileNames[i][strlen(fileNames[i]) - 7] = '\0';
            fileNames[i][strlen(fileNames[i]) - 9] = ' ';
            strptime(fileNames[i], "%Y-%m-%d %H:%M:%S", &tm);
            t2 = mktime(&tm);
            seconds = difftime(t1, t2);
            if (seconds > 0)
            {
                strcpy(file2delete, fileNames[i]);
                t1 = t2;
            }
            i++;
        }
        closedir(d);
        if (i > logFileN)
        {
            file2delete[strlen(file2delete) - 9] = ',';
            strcat(command, dataPath);
            strcat(file2delete, ".txt.gz");
            strcat(command, file2delete);
            printf("Delete File:\t %s\n", command);
            remove(command);
        }
    }
    return (0);
}