#pragma once
#include <cstring>
#include <cstdio>
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <HardwareSerial.h>
#include <string>
#include <WiFi.h>
#include <esp_now.h>
#include "config.h"
#include "DataBroker.h"

#define START_BYTE 0x06
#define END_BYTE 0x07
#define TRIGGER_BUTTON_BITMASK (0x01 << 3)
#define JOYSTICK_BUTTON_BITMASK (0x1 << 2)
#define A_BUTTON_BITMASK (0x1 << 1)
#define B_BUTTON_BITMASK (0x1 << 0)

// Struct for input payloads
struct InputsPayload
{
    int fingerAngles[5];
    int joystickXY[2];
    uint32_t buttonsBitmask;
};

void TaskWifiCommunication(void *pvParameters);