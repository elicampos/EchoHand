#pragma once

#include <cstring>
#include <cstdio>
#include <Arduino.h>
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <HardwareSerial.h>
#include <string>
#include "config.h"
#include "DataBroker.h"
#include <ESP32Servo.h>

void TaskServoControl(void *pvParameters);
