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
  char outputsString[56];
  char* currentByte;
  char command;
  int intValue;
  float floatValue;
  int8_t charCount = 0;

  //set finger splay and leave it
  printf("AB511BB511CB511DB511EB511\n");

  
  // bluetooth task loop
  uint32_t lastRevision = 0;
  for(;;){
    //if there's data available, read in the payload for outputs
    if(!DEBUG_PRINT)
    {
      if(Serial.available()){
        currentByte = outputsString;
        Serial.read(currentByte, sizeof(char));

        while(*currentByte != '\n' && currentByte != outputsString+ 55){
          Serial.read(currentByte, sizeof(char));
          if(*currentByte >= 65 && *currentByte <= 69){
            command = *currentByte;
            currentByte++;
            while(*currentByte != '\n' && *currentByte < 65){
              if(currentByte = outputsString + 55){
                break;
                break;
              }
              intValue *= 10;
              intValue += *currentByte - 48;
              currentByte++;
            }
            PersistentState::instance().setServoTargetAngle(command - 65, (180*intValue)/1000);
          } else if(*currentByte == 'F'){
            charCount = 0;
            command = *currentByte;
            currentByte++;
            while(*currentByte != '\n' && *currentByte < 65){
              if(currentByte = outputsString + 55){
                break;
                break;
              }

              if(charCount > 1){
                floatValue += *currentByte * pow(10, (-1) * charCount);
              }
              charCount++;
              currentByte++;
            }
            for(int i = 0;i < 5;i++){
              PersistentState::instance().setVibrationRPM(i, floatValue*60);
            }
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
        printf("A%dB%dC%dD%dE%d", in.fingerAngles[0], in.fingerAngles[1], in.fingerAngles[2], in.fingerAngles[3], in.fingerAngles[4]);
        printf("F%dG%d", in.joystickXY[0], in.joystickXY[1]);
        if(in.buttonsBitmask & JOYSTICK_BUTTON_BITMASK)
          printf("H");
        if(in.buttonsBitmask & A_BUTTON_BITMASK)
          printf("J");
        if(in.buttonsBitmask & B_BUTTON_BITMASK)
          printf("K");
        printf("\n");

      }
      lastRevision = s.revision;
    }
    
    vTaskDelay(1);
     
  }
 
}