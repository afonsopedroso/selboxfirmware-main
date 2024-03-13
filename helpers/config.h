#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdio.h>
#include <string.h>
#include <json-c/json.h>

int voltRating = 400;
int ctRating = 120;
// resistor value compensation (gave some slack for peak voltages) defined in calcmetrics.c
float voltResComp = 1;
// resistor value compensation (gave some slack for peak currents) defined in calcmetrics.c
float ctResComp = 1;
// compensate for offset from 2.5 V generation defined in calcmetrics.c
float offsetVoltage = 0.0;
// compensate for offset from 2.5 V generation defined in calcmetrics.c
float offsetCurrent = 0.0;
// to store device MAC Address
char mac[13] = "000000000000";
// to store the number logged points (loaded here in case of power outage/forced reboot)
int loggedPoints = 0;
// stores firmware and hardware versions
float firmware = 0.0;
float hardware = 0.1;

int config(void);
void getMacAddr(void);

#endif
