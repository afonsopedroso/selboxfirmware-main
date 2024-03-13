#ifndef FILEPARSER_H_
#define FILEPARSER_H_

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>

#include "../include/vars.h"

pthread_t ti[nThreads];
extern char mac[20]; // defined in calcmetric.h

void setUsedCore(int CPU);
void *writeFile1(void *datapoint);
void *writeFile2(void *datapoint);
void *removeFile();
// void *compressFile(void *arg)

#endif