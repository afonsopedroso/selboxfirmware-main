#ifndef NILM_H_
#define NILM_H_

#define _GNU_SOURCE

#include <bcm2835.h>
#include "../include/calcmetrics.h"
#include "../include/fileparser.h"
#include "../include/AD-DA-WS-RPI.h"
#include "../include/vars.h"

pthread_t tid[nThreads];

extern char mac[20];        // defined in calcmetric.h
extern float offsetVoltage; // defined in calcmetric.h
extern float offsetCurrent; // define in calcmetric.h
extern int invertedCT;
void getMacAddr();
void sig_handler();

#endif // NILM_H_