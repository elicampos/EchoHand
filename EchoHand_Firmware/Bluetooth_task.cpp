#include "Bluetooth_task.h"

void TaskBluetoothSerial(void *pvParameters){
  // //bluetooth init
  
  //uses parameter to avoid compiler error
  (void) pvParameters;

  // //bluetooth task loop
  for(;;){
     //if there is a bluetooth character to read, it reads it and writes it out to serial/uart

    vTaskDelay(1);
  }
}