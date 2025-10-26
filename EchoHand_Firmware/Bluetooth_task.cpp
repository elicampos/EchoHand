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
  NimBLEDevice::init("Echo_Hand");

  //security setup
  NimBLEDevice::setSecurityAuth(true, true, false); //requires bonding, MITM, and BLE secure connections  

  NimBLEServer* BLEServer = NimBLEDevice::createServer();

  // EchoHand custom service + characteristic UUIDs
  static const char* SVC_ECHOHAND = "069a410b-7344-4768-8568-f5a7073a0ab1";
  static const char* CH_INPUTS   = "069a410b-7344-4768-8568-f5a7073a0ab2";
  static const char* CH_OUTPUTS  = "069a410b-7344-4768-8568-f5a7073a0ab3";

  NimBLEService* UserInputService = BLEServer->createService(SVC_ECHOHAND);

  // Packed characteristics
  // Inputs_Packed: glove -> PC (read/notify)
  NimBLECharacteristic* inputsPacked =
        UserInputService->createCharacteristic(CH_INPUTS, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::READ_AUTHEN);
  // Outputs_Packed: PC -> glove (write)
  NimBLECharacteristic* outputsPacked =
        UserInputService->createCharacteristic(CH_OUTPUTS, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_ENC | NIMBLE_PROPERTY::WRITE_AUTHEN);

  outputsPacked->setCallbacks(&outputsCb);
  UserInputService->start();
  
  NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
  advertising->addServiceUUID(SVC_ECHOHAND);
  advertising->start();
  Serial.println("Started BLE advertising");

  

  // bluetooth task loop
  uint32_t lastRevision = 0;
  for(;;){
    EchoStateSnapshot s;
    PersistentState::instance().takeSnapshot(s);
    if (s.revision != lastRevision) {
      // update packed inputs
      InputsPayload in{};
      for (uint8_t i = 0; i < 5; ++i) in.fingerAngles[i] = s.fingerAngles[i];
      in.joystickXY[0] = s.joystickXY[0];
      in.joystickXY[1] = s.joystickXY[1];
      in.buttonsBitmask = s.buttonsBitmask;
      in.batteryPercent = s.batteryPercent;
      inputsPacked->setValue((uint8_t*)&in, sizeof(InputsPayload));
      lastRevision = s.revision;
    }
    vTaskDelay(1);
  }
}