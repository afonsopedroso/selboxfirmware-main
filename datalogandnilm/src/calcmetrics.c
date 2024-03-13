#include "../include/calcmetrics.h"

// to enable the logging of raw data
int lograwdata = 0;

/******************************************************************************
 * TODO Create a function to calculate the offset on startup and set these
 * variables, which are declared in calcmetrics.h
 * Method:
 * Voltage average input value should be 0 volts, if not subtract/add the
 * difference to all measurements
 * Current whith the CT open, average value should be 0, if not subtract/add
 * the difference to all measurements
 * One can do this on startup or periodically
 ******************************************************************************/

// v0.0 hardware
// const float voltResComp = 1.17;
// const float ctResComp = 1.07;

// v0.1 hardware
const float voltResComp = 1.255;
//******* for ct of 30A/30mA *******
const float ctResComp = 1.07;
//******* for ct of 120A/40mA ******
// const float ctResComp = 0.9;

/******************************************************************************
function:  Metrics Calculation from array with data points
parameter:
Info:
******************************************************************************/
void *
calcMetrics1(void *datapoint)
{
    struct tm *tm;
    struct timeval tv;
    char *body = malloc(MAX_SIZE * sizeof(char));
    int i, j;
    char bufStamp[26];
    char buf[10];
    double sumV, sumC, sumP, sumE;
    float voltage, current, power, energy, appPower, activePower;
    float rmsVoltage, rmsCurrent;
    float activeEnergy;
    float arrVoltage[sampPerMin], arrCurrent[sampPerMin], arrEnergy[sampPerMin], arrPower[sampPerMin]; // to store interval values
    setUsedCore(workerCPU);

    /******************************************************************************
     * processing and transaction loop
     ******************************************************************************/
    while (1)
    {
        pthread_mutex_lock(&lock3);
        struct data_point *arr_datapoint = (struct data_point *)datapoint;
        body[0] = '\0';
        // request[0] = '\0';
        /******************************************************************************
    Info: object for JSON information
    ******************************************************************************/
        json_object *jobj = json_object_new_object();
        /******************************************************************************
         * ID is the MAC Address
         ******************************************************************************/
        json_object *jID = json_object_new_string(mac);
        /******************************************************************************
         * get timestamp
         ******************************************************************************/
        gettimeofday(&tv, NULL);
        tm = localtime(&tv.tv_sec);
        strftime(bufStamp, 22, "%Y-%m-%d %H:%M:%S", tm);
        if (lograwdata == 1)
        {
            strcpy(timeStamp1, bufStamp);
            pthread_mutex_unlock(&lock1); // unlocks writer1
        }
        /******************************************************************************
         * Calculate 1-min RMS Voltage and Current
         ******************************************************************************/
        json_object_object_add(jobj, "deviceid", jID);
        for (j = 0; j < sampPerMin; j++)
        {
            sumV = 0;
            sumC = 0;
            sumP = 0;
            sumE = 0;
            for (i = j * seconds2save / sampPerMin * wSize; i < (j + 1) * seconds2save / sampPerMin * wSize; i++)
            {
                voltage = (arr_datapoint[i].voltage / pow(2, 23) - 0.5) * 2 * sqrt(2) * voltRating * voltResComp - offsetVoltage; // effective voltage
                sumV += pow(voltage, 2);
                current = (arr_datapoint[i].current / pow(2, 23) - 0.5) * 2 * sqrt(2) * ctRating * ctResComp - offsetCurrent; //  effective current 1.05 compensation for using 56 Ohm instead of 59.1 on the CT burden resistor
                if (invertedCT == 1)
                    current = -current;
                sumC += pow(current, 2);   // square sum for average calculation in the next loop
                power = voltage * current; // not squared here to be used for energy calculation
                sumP += power;             // for average in the next loop
                energy = power / ((seconds2save / sampPerMin) * wSize);
                sumE += energy;
            }
            arrVoltage[j] = sumV;
            arrCurrent[j] = sumC;
            arrPower[j] = sumP;
            arrEnergy[j] = sumE;
        }
        /******************************************************************************
         * Prepares JSON body
         ******************************************************************************/
        for (j = 0; j < sampPerMin; j++)
        {
            /*Creating a json double*/
            rmsVoltage = sqrt(arrVoltage[j] / ((seconds2save / sampPerMin) * wSize));
            gcvt(rmsVoltage, 6, buf); // 6 digit voltage i.e. 230.123 V
            json_object *jrmsV = json_object_new_string(buf);
            /*Creating a json double*/
            rmsCurrent = sqrt(arrCurrent[j] / ((seconds2save / sampPerMin) * wSize));
            gcvt(rmsCurrent, 5, buf); // 5 digit current i.e. 12.421 A
            json_object *jrmsI = json_object_new_string(buf);
            /*Creating a json double*/
            activePower = arrPower[j] / ((seconds2save / sampPerMin) * wSize);
            gcvt(activePower, 5, buf); // 5 digit current i.e. 2300.23W
            json_object *jActP = json_object_new_string(buf);
            /*Creating a json double*/
            activeEnergy = arrEnergy[j] * (seconds2save / sampPerMin) / 3600; //
            gcvt(activeEnergy, 6, buf);                                       // 6 digit current i.e. 230.123WH
            json_object *jAE = json_object_new_string(buf);
            // /*Creating a json double*/
            appPower = rmsVoltage * rmsCurrent;
            gcvt(appPower, 5, buf); // 5 digit current i.e. 2300.23W
            json_object *jAppP = json_object_new_string(buf);
            // /*Creating a json double*/
            gcvt(activePower / appPower, 3, buf); // 5 digit current i.e. 2300.23W
            json_object *jPF = json_object_new_string(buf);

            /*Creating a json string*/
            json_object *jTime = json_object_new_string(bufStamp);
            json_object_object_add(jobj, "time", jTime);
            json_object_object_add(jobj, "rmsvoltage", jrmsV);
            json_object_object_add(jobj, "rmscurrent", jrmsI);
            json_object_object_add(jobj, "totalpower", jAppP);
            json_object_object_add(jobj, "energy", jAE);
            json_object_object_add(jobj, "activepower", jActP);
            json_object_object_add(jobj, "powerfactor", jPF);
            //  json_object_object_add(jobj, "totalenergy", jTE); //cummulative energy
            if (strcat(body, json_object_to_json_string(jobj)) == NULL)
            {
                perror("Error strcpy(body... : ");
            }
            strcat(body, ",");
            tm->tm_sec = tm->tm_sec + seconds2save / sampPerMin; // if more than one data point per minute is set
            mktime(tm);
        }
        body[strlen(body) - 1] = '\0'; // removes last comma
        /******************************************************************************
         * Launches Thread for transaction / log
         ******************************************************************************/
        if (pthread_create(&t[0], NULL, sslTransaction, body) != 0)
        {
            perror("Error sslTransaction pthread create: ");
        }
        if (pthread_detach(t[0]) != 0)
        {
            perror("Error sslTransaction pthread detach: ");
        }
    }
}

void *calcMetrics2(void *datapoint)
{
    struct tm *tm;
    struct timeval tv;
    char *body = malloc(MAX_SIZE * sizeof(char));
    int i, j;
    char bufStamp[26];
    char buf[10];
    double sumV, sumC, sumP, sumE;
    float voltage, current, power, energy, appPower, activePower;
    float rmsVoltage, rmsCurrent;
    float activeEnergy;
    float arrVoltage[sampPerMin], arrCurrent[sampPerMin], arrEnergy[sampPerMin], arrPower[sampPerMin]; // to store interval values
    setUsedCore(workerCPU);
    /******************************************************************************
    Info: set affinity to isolated cpu2 (added isolcpu=1,2 to \boot\commandline.txt)
    ******************************************************************************/
    while (1)
    {
        pthread_mutex_lock(&lock4);
        struct data_point *arr_datapoint = (struct data_point *)datapoint;
        body[0] = '\0';
        /******************************************************************************
    Info: object for JSON information
    ******************************************************************************/
        json_object *jobj = json_object_new_object();
        /******************************************************************************
         * ID is the MAC Address
         ******************************************************************************/
        json_object *jID = json_object_new_string(mac);
        /******************************************************************************
         * get timestamp
         ******************************************************************************/
        gettimeofday(&tv, NULL);
        tm = localtime(&tv.tv_sec);
        strftime(bufStamp, 22, "%Y-%m-%d %H:%M:%S", tm);
        if (lograwdata == 1)
        {
            strcpy(timeStamp2, bufStamp);
            pthread_mutex_unlock(&lock2); // unlocks writer2
        }
        /******************************************************************************
         * Calculate 1-min RMS Voltage and Current
         ******************************************************************************/
        json_object_object_add(jobj, "deviceid", jID);
        for (j = 0; j < sampPerMin; j++)
        {
            sumV = 0;
            sumC = 0;
            sumP = 0;
            sumE = 0;
            for (i = j * seconds2save / sampPerMin * wSize; i < (j + 1) * seconds2save / sampPerMin * wSize; i++)
            {
                voltage = (arr_datapoint[i].voltage / pow(2, 23) - 0.5) * 2 * sqrt(2) * voltRating * voltResComp - offsetVoltage; // effective voltage
                sumV += pow(voltage, 2);
                current = (arr_datapoint[i].current / pow(2, 23) - 0.5) * 2 * sqrt(2) * ctRating * ctResComp - offsetCurrent; //  effective current 1.05 compensation for using 56 Ohm instead of 59.1 on the CT burden resistor
                if (invertedCT == 1)
                    current = -current;
                sumC += pow(current, 2);
                // square sum for average calculation in the next loop
                power = voltage * current; // not squared here to be used for energy calculation                                                                                                             // not squared here to be used for energy calculation
                sumP += power;             // for average in the next loop
                energy = power / ((seconds2save / sampPerMin) * wSize);
                sumE += energy;
            }
            arrVoltage[j] = sumV;
            arrCurrent[j] = sumC;
            arrPower[j] = sumP;
            arrEnergy[j] = sumE;
        }
        /******************************************************************************
         * Prepares JSON body
         ******************************************************************************/
        for (j = 0; j < sampPerMin; j++)
        {
            /*Creating a json double*/
            rmsVoltage = sqrt(arrVoltage[j] / ((seconds2save / sampPerMin) * wSize));
            gcvt(rmsVoltage, 6, buf); // 6 digit voltage i.e. 230.123 V
            json_object *jrmsV = json_object_new_string(buf);
            /*Creating a json double*/
            rmsCurrent = sqrt(arrCurrent[j] / ((seconds2save / sampPerMin) * wSize));
            gcvt(rmsCurrent, 5, buf); // 5 digit current i.e. 12.421 A
            json_object *jrmsI = json_object_new_string(buf);
            /*Creating a json double*/
            activePower = arrPower[j] / ((seconds2save / sampPerMin) * wSize);
            gcvt(activePower, 5, buf); // 5 digit current i.e. 2300.23W
            json_object *jActP = json_object_new_string(buf);
            /*Creating a json double*/
            activeEnergy = arrEnergy[j] * (seconds2save / sampPerMin) / 3600; //
            gcvt(activeEnergy, 6, buf);                                       // 6 digit current i.e. 230.123WH
            json_object *jAE = json_object_new_string(buf);
            // /*Creating a json double*/
            appPower = rmsVoltage * rmsCurrent;
            gcvt(appPower, 5, buf); // 5 digit current i.e. 230.23W
            json_object *jAppP = json_object_new_string(buf);
            // /*Creating a json double*/
            gcvt(activePower / appPower, 3, buf); // 5 digit current i.e. 0.23W
            json_object *jPF = json_object_new_string(buf);

            /*Creating a json string*/
            json_object *jTime = json_object_new_string(bufStamp);
            json_object_object_add(jobj, "time", jTime);
            json_object_object_add(jobj, "rmsvoltage", jrmsV);
            json_object_object_add(jobj, "rmscurrent", jrmsI);
            json_object_object_add(jobj, "totalpower", jAppP);
            json_object_object_add(jobj, "energy", jAE);
            json_object_object_add(jobj, "activepower", jActP);
            json_object_object_add(jobj, "powerfactor", jPF);
            //  json_object_object_add(jobj, "totalenergy", jTE); //cummulative energy
            if (strcat(body, json_object_to_json_string(jobj)) == NULL)
            {
                perror("Error strcpy(body... : ");
            }
            strcat(body, ",");
            tm->tm_sec = tm->tm_sec + seconds2save / sampPerMin; // if more than one data point per minute is set
            mktime(tm);
        }
        body[strlen(body) - 1] = '\0'; // removes last comma
        /******************************************************************************
         * Launches Thread for transaction / log
         ******************************************************************************/
        if (pthread_create(&t[1], NULL, sslTransaction, body) != 0)
        {
            perror("Error sslTransaction pthread create: ");
        }
        if (pthread_detach(t[1]) != 0)
        {
            perror("Error sslTransaction pthread detach: ");
        }
    }
}

/******************************************************************************
function: set affinity to isolated cpu2 (added isolcpu=1,2 to \boot\commandline.txt)
******************************************************************************/
void setUsedCore(int CPU)
{
    cpu_set_t set;
    const pthread_t pid = pthread_self();
    CPU_ZERO(&set);
    CPU_SET(CPU, &set);
    pthread_setaffinity_np(pid, sizeof(cpu_set_t), &set);
}
