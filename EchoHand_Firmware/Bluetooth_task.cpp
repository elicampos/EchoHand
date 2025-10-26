#include "Bluetooth_task.h"

void TaskBluetoothSerial(void *pvParameters){
  //uses parameter to avoid compiler error
  (void) pvParameters;

  // //bluetooth init
  NimBLEDevice::init("Echo_Hand");

  //security setup
  NimBLEDevice::setSecurityAuth(true, true, false); //requires bonding, MITM, and BLE secure connections  

  NimBLEServer* BLEServer = NimBLEDevice::createServer();

  NimBLEService* UserInputService = BLEServer->createService("User_Inputs");
  NimBLECharacteristic* fingerAngle1 =
        UserInputService->createCharacteristic("Finger_Angle_1",
                                       NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::READ_AUTHEN);


  UserInputService->start();
  fingerAngle1->setValue(0.5);
  NimBLEAdvertising* advertisitng = NimBLEDevice::getAdvertising();
  advertising->addServiceUUID("069a410b-7344-4768-8568-f5a7073a0ab1");
  advertising->start();

  

  // // //bluetooth task loop
  // for(;;){
    
    

  //   vTaskDelay(1);
  // }
}