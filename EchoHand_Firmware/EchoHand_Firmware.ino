#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Bluetooth_task.h"

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
  xTaskCreate(
    TaskAnalogRead,   // Fucntion name of Task
    "AnalogRead",     // Name of Task
    2048,             // Stack size (bytes) for task
    NULL,             // Parameters(none)
    1,                // Priority level(1->highest)
    NULL              // Task handle(for RTOS API maniuplation)
  );

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
  const int indexPin = 12;  // Example ADC pin (adjust for your board)
  const int middlePin = 11;  // Example ADC pin (adjust for your board)
  const int ringPin = 10;  // Example ADC pin (adjust for your board)
  const int pinkePin = 9;  // Example ADC pin (adjust for your board)

  // To not get compiler unused variable error
  (void) pvParameters;

  int thumbAngle = analogRead(thumbPin)*resistor_to_angle_constant;
    int indexAngle = analogRead(indexPin)*resistor_to_angle_constant;
    int middleAngle = analogRead(middlePin)*resistor_to_angle_constant;
    int ringAngle = analogRead(ringPin)*resistor_to_angle_constant;
    int pinkeAngle = analogRead(pinkePin)*resistor_to_angle_constant;
  // Fetch analog data from sensors forever
  for (;;) 
  {
    thumbAngle = analogRead(thumbPin)*resistor_to_angle_constant;
    indexAngle = analogRead(indexPin)*resistor_to_angle_constant;
    middleAngle = analogRead(middlePin)*resistor_to_angle_constant;
    ringAngle = analogRead(ringPin)*resistor_to_angle_constant;
    pinkeAngle = analogRead(pinkePin)*resistor_to_angle_constant;
    
    // Serial.println("Thumb Angle: " + String(thumbAngle) + 
    // " Index Angle: " + String(indexAngle) +  
    // " Middle Finger Angle: " + String(middleAngle) +
    // " Ring Finger Angle: " + String(ringAngle) +
    // " Pinkie Finger Angle: " + String(pinkeAngle));
    vTaskDelay(1); 
  }
}
