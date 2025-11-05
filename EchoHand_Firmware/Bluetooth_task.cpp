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

// Write callback for outputs
class OutputsCallback : public NimBLECharacteristicCallbacks {
  // callback function for when the client writes to the outputs characteristic
  void onWrite(NimBLECharacteristic* c, NimBLEConnInfo& connInfo) override {
    // read the raw bytes the client wrote to the outputs characteristic
    const std::string& v = c->getValue();

    // DATA VALIDATION/INFORMATION HANDLING (if we need to more here, move elsewhere? I'm not sure if a checksum is needed or if BLE handles that)
    // validate the payload is at least the size of our packed OutputsPayload
    // copy the bytes into a local OutputsPayload
    OutputsPayload p;
    if (v.size() < sizeof(OutputsPayload)) return;
    // DATA VALIDATION END

    
    memcpy(&p, v.data(), sizeof(OutputsPayload));

    // Apply each finger's outputs to the persistent state
    for (uint8_t i = 0; i < 5; ++i) {
      PersistentState::instance().setVibrationRPM(i, p.vibrationRPMs[i]);
      PersistentState::instance().setServoTargetAngle(i, p.servoTargetAngles[i]);
    }
  }
} outputsCb;

 

void TaskBluetoothSerial(void *pvParameters){
  //uses parameter to avoid compiler error
  (void) pvParameters;

  Serial.println("Initializing BLE...");
  // //bluetooth init

  Serial1.begin(38400, SERIAL_8N1, 18, 17);
  
  for(;;){
    Serial1.print("AT\r\n");
    if(Serial1.available())
      Serial.print(Serial1.readString());
    vTaskDelay(1);
  }
  
}