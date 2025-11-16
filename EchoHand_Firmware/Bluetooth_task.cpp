#include "Bluetooth_task.h"
#include "PersistentState.h"
#include <cstring>
#include "config.h"

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

  //swapping over to Serial port for now
  // Serial.println("Starting Bluetooth Serial...");
  // Serial1.begin(38400, SERIAL_8N1, 18, 17);
  
  EchoStateSnapshot s;
  InputsPayload in{};
  OutputsPayload out{};
  OutputsPayload currentOutData{};

  
  // bluetooth task loop
  uint32_t lastRevision = 0;
  for(;;){

    uint8_t currentByte;

    //if there's data available, read in the payload for outputs
    if(!DEBUG_PRINT)
    {
      if(Serial.available()){
        Serial.read((uint8_t*)&currentByte, sizeof(currentByte));
        if(currentByte == START_BYTE){
          Serial.read((uint8_t*)&currentOutData, sizeof(OutputsPayload));
          Serial.read((uint8_t*)&currentByte, sizeof(currentByte));
          if(currentByte == END_BYTE){
            out = currentOutData;
          }
            
        }
      }
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
      if(!DEBUG_PRINT)
      {
        Serial.write((uint8_t*)&in, sizeof(InputsPayload));
      }
      lastRevision = s.revision;
    }
    
    vTaskDelay(1);
     
  }
 
}