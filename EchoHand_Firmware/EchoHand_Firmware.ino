#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Bluetooth_task.h"
#include "PersistentState.h"
// After mechincal structure is placed, we will calculate with the constant is
#define resistor_to_angle_constant 1

// Declare Functions
void TaskAnalogRead(void *pvParameters);

void setup() 
{
  // Set Fast BaudRate
  Serial.begin(115200); 

  // Wait for serial device to finish initalizing
  while(!Serial);

  //Debug Print
  Serial.println("Starting FreeRTOS task...");

  // Create the task
  // xTaskCreate(
  //   TaskAnalogRead,   // Fucntion name of Task
  //   "AnalogRead",     // Name of Task
  //   2048,             // Stack size (bytes) for task
  //   NULL,             // Parameters(none)
  //   1,                // Priority level(1->highest)
  //   NULL              // Task handle(for RTOS API maniuplation)
  // );

  xTaskCreate(
    TaskBluetoothSerial,   // Fucntion name of Task
    "BluetoothSerial",     // Name of Task
    8192,             // Stack size (bytes) for task
    NULL,             // Parameters(none)
    1,                // Priority level(1->highest)
    NULL              // Task handle(for RTOS API maniuplation)
  );
}

void loop() 
{
  // Empty — work is done in the FreeRTOS task
}

//Description: Reads all Analog Data from sensors
//Parameters: pvParameters which is a place holder for any pointer to any type
//Return: none, it will simply pass the information on to the next core for processing
void TaskAnalogRead(void *pvParameters) 
{
  //Pin locations for fingers
  const int thumbPin = 13; 
  const int indexPin = 12; 
  const int middlePin = 11;  
  const int ringPin = 10; 
  const int pinkiePin = 9; 

  // To not get compiler unused variable error
  (void) pvParameters;

  // Fetch analog data from sensors forever
  for (;;) 
  {
    int thumbAngle = map(analogRead(thumbPin), 0, 4000, 0, 90);
    int indexAngle = map(analogRead(indexPin), 0, 4000, 0, 90);
    int middleAngle = map(analogRead(middlePin), 0, 4000, 0, 90);
    int ringAngle = map(analogRead(ringPin), 0, 4000, 0, 90);
    int pinkeAngle = map(analogRead(pinkeAngle),0,4000,0,90);
    
    //Send Data to Persistant State
    PersistentState::instance().setFingerAngle(0, thumbAngle);
    PersistentState::instance().setFingerAngle(1, indexAngle);
    PersistentState::instance().setFingerAngle(2, middleAngle);
    PersistentState::instance().setFingerAngle(3, ringAngle);   
    PersistentState::instance().setFingerAngle(4, pinkeAngle);

    // Serial.println("Thumb Angle: " + String(thumbAngle) + 
    // " Index Angle: " + String(indexAngle) +  
    // " Middle Finger Angle: " + String(middleAngle) +
    // " Ring Finger Angle: " + String(ringAngle) +
    // " Pinkie Finger Angle: " + String(pinkeAngle));
    vTaskDelay(1); 
  }
}
