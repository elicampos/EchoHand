#pragma once
//#include <NimBLEDevice.h>

#define START_BYTE 0x06
#define END_BYTE 0x07
#define TRIGGER_BUTTON_BITMASK (0x01 << 3)
#define JOYSTICK_BUTTON_BITMASK (0x1 << 2)
#define A_BUTTON_BITMASK (0x1 << 1)
#define B_BUTTON_BITMASK (0x1 << 0)
void TaskBluetoothSerial(void *pvParameters);