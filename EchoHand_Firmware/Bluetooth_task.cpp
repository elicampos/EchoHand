#include "Bluetooth_task.h"
#include "PersistentState.h"
#include <cstring>

// Defining packed payloads
#pragma pack(push, 1) // pack the structs tp prevent padding between fields. this way the size of the struct is always the same.
struct InputsPayload {
  float fingerAngles[5];
  float joystickXY[2];
  uint32_t buttonsBitmask;
  uint8_t batteryPercent;
};
struct OutputsPayload {
  uint16_t vibrationRPMs[5];
  float servoTargetAngles[5];
};
#pragma pack(pop) 

void TaskBluetoothSerial(void *pvParameters){
  //uses parameter to avoid compiler error
  (void) pvParameters;

  
  Serial.println("Starting Bluetooth Serial...");
  Serial1.begin(38400, SERIAL_8N1, 18, 17);
  
  EchoStateSnapshot s;
  InputsPayload in{};
  OutputsPayload out{};

  // bluetooth task loop
  uint32_t lastRevision = 0;
  for(;;){
    //if there's data available, read in the payload for outputs
    if(Serial1.available()){
      Serial1.read(&out, sizeof(OutputsPayload));
    }

    PersistentState::instance().takeSnapshot(s);
    if (s.revision != lastRevision) {
      // update packed inputs
      
      for (uint8_t i = 0; i < 5; ++i) 
        in.fingerAngles[i] = s.fingerAngles[i];
      in.joystickXY[0] = s.joystickXY[0];
      in.joystickXY[1] = s.joystickXY[1];
      in.buttonsBitmask = s.buttonsBitmask;
      in.batteryPercent = s.batteryPercent;
      Serial1.write((uint8_t*)&in, sizeof(InputsPayload));
      lastRevision = s.revision;
    }
    vTaskDelay(1);
  }
}