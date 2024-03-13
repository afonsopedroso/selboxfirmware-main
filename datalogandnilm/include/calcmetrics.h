#ifndef CALCMETRICS_H_
#define CALCMETRICS_H_

#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <sched.h> //for setting processor affinity
#include <sys/time.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <json-c/json.h>
#include "../include/ssltransaction.h"

#include "../include/vars.h"

#define voltRating 230
// for ct of 30A/30mA
#define ctRating 30

// for ct of 120A/40mA
//#define ctRating 120
// resistor value compensation (gave some slack for peak voltages) defined in calcmetrics.c
extern const float voltResComp;
// resistor value compensation (gave some slack for peak currents) defined in calcmetrics.c
extern const float ctResComp;
// compensate for offset from 2.5 V generation defined in calcmetrics.c
float offsetVoltage;
// compensate for offset from 2.5 V generation defined in calcmetrics.c
float offsetCurrent;
// to store device MAC Address
char mac[20];

pthread_t t[nThreads];

void setUsedCore(int CPU);
void *calcMetrics1(void *datapoint);
void *calcMetrics2(void *datapoint);

#endif // CALCMETRICS_H_