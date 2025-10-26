#include "Bluetooth_task.h"

void BLEInit(){
  Serial.println("Starting BLE initialization");
  NimBLEDevice::init("Echo Hand");
  NimBLEDevice::setPower(3); 

  NimBLEDevice::setSecurityAuth(true, true, true); /** bonding, MITM, don't need BLE secure connections as we are using passkey pairing */
  NimBLEDevice::setSecurityPasskey(123456);
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY); /** Display only passkey */
  NimBLEServer*         pServer                  = NimBLEDevice::createServer();
  NimBLEService*        pService                 = pServer->createService("ABCD");
  NimBLECharacteristic* pNonSecureCharacteristic = pService->createCharacteristic("1234", NIMBLE_PROPERTY::READ);
  NimBLECharacteristic* pSecureCharacteristic =
      pService->createCharacteristic("1235",
                                      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::READ_AUTHEN);

  pService->start();
  pNonSecureCharacteristic->setValue("Hello Non Secure BLE");
  pSecureCharacteristic->setValue("Hello Secure BLE");

  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID("ABCD");
  pAdvertising->start();
  Serial.println("BLE advertising initiated");
}

void TaskBluetoothSerial(void *pvParameters){
  //uses parameter to avoid compiler error
  (void) pvParameters;

  

  

  // // //bluetooth task loop
  // for(;;){
    
    

  //   vTaskDelay(1);
  // }
}