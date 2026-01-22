#pragma once
#include <cstring>
#include <cstdio>
#include <Arduino.h>
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <HardwareSerial.h>
#include <string>
#include <climits>
#include <vector>
#include <algorithm>
#include "config.h"
#include "DataBroker.h"
int readSmooth(int pin);
int mapFlex(int raw);
void TaskAnalogRead(void *pvParameters);
