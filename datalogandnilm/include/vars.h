#ifndef VARS_H_
#define VARS_H_

#define nThreads 2
#define ioCPU 1
#define workerCPU 2
#define writerCPU 3
// wsize - window size 1 second
#define wSize 7500
// second2save - read seconds in each iteration
#define seconds2save 60
// transaction samples per minute
#define sampPerMin 1 // define samples per minute

// variables for logged raw data files window path and number of logged minutes
#define dataPath "/home/sel/data/"
#define logFileN 3 * 24 * 60 // logged time in minutes

// transaction information
#define PORT "443"
//#define HOSTNAME "192.168.2.1"
#define HOSTNAME "datalogger.smartenergylab.pt"
#define RESOURCEPATH "save_new_log"
#define MAX_SIZE 205 // size of the json string 1 datapoint per minute
//[{"deviceid" : "b827eb278b02", "time" : "2021-12-04 12:40:06","rmsvoltage" : "226.14", "rmscurrent" : "56.884", "totalpower" :"35.999", "energy" : "0.59998", "activepower" : "35.999", powerfactor"="0.33"}]
#define FALSE 0
#define TRUE 1

pthread_mutex_t lock1;
pthread_mutex_t lock2;
pthread_mutex_t lock3;
pthread_mutex_t lock4;
pthread_mutex_t loglock;

int invertedCT;

// shared timestamps between calcmetrics and fileparser to maintain coerence between the read/write values
char timeStamp1[26];
char timeStamp2[26];

struct data_point
{
    int voltage;
    int current;
};

#endif