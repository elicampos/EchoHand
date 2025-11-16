#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.h"
#include "Bluetooth_task.h"
#include "PersistentState.h"
#include "ESP32Servo.h"

// Declare Functions
void TaskAnalogRead(void *pvParameters);
void TaskControllerButtons(void* pvParameters);
void TaskServoControl(void* pvParameters);
void TaskPersistentStatePrint(void* pvParameters);

void setup() 
{
  // Set Fast BaudRate
  Serial.begin(115200); 

  //Debug Print
  Serial.println("Starting FreeRTOS task...");

  // Create the task
  xTaskCreatePinnedToCore(
    TaskAnalogRead,   // Fucntion name of Task
    "AnalogRead",     // Name of Task
    2048,             // Stack size (bytes) for task
    NULL,             // Parameters(none)
    2,                // Priority level(1->highest)
    NULL,              // Task handle(for RTOS API maniuplation)
    0                  // Run on core 0
  );

  xTaskCreatePinnedToCore(
    TaskBluetoothSerial,   // Fucntion name of Task
    "BluetoothSerial",     // Name of Task
    8192,             // Stack size (bytes) for task
    NULL,             // Parameters(none)
    2,                // Priority level(1->highest)
    NULL,              // Task handle(for RTOS API maniuplation)
    0                  // Run on core 0
  );

  
  xTaskCreatePinnedToCore(
    TaskControllerButtons,   // Fucntion name of Task
    "ControllerButtons",     // Name of Task
    8192,             // Stack size (bytes) for task
    NULL,             // Parameters(none)
    2,                // Priority level(1->highest)
    NULL,              // Task handle(for RTOS API maniuplation)
    0                  // Run on core 0
  );

  xTaskCreatePinnedToCore(
    TaskServoControl,   // Fucntion name of Task
    "ServoControl",     // Name of Task
    8192,             // Stack size (bytes) for task
    NULL,             // Parameters(none)
    2,                // Priority level(1->highest)
    NULL,              // Task handle(for RTOS API maniuplation)
    0                  // Run on core 0
  );

  xTaskCreatePinnedToCore(
    TaskPersistentStatePrint,   // Fucntion name of Task
    "PersistentStatePrint",     // Name of Task
    8192,             // Stack size (bytes) for task
    NULL,             // Parameters(none)
    1,                // Priority level(1->highest)
    NULL,              // Task handle(for RTOS API maniuplation)
    1                 // Run on core 1
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
    int pinkeAngle = map(analogRead(pinkiePin),0,4000,0,90);
    
    //Send Data to Persistant State
    PersistentState::instance().setFingerAngle(0, thumbAngle);
    PersistentState::instance().setFingerAngle(1, indexAngle);
    PersistentState::instance().setFingerAngle(2, middleAngle);
    PersistentState::instance().setFingerAngle(3, ringAngle);   
    PersistentState::instance().setFingerAngle(4, pinkeAngle);

    vTaskDelay(pdMS_TO_TICKS(10));; 

  }
}


//Description: Reads all Analog and digital data from joysticks
//Parameters: pvParameters which is a place holder for any pointer to any type
//Return: none, it will simply pass the information on to the next core for processing
void TaskControllerButtons(void *pvParameters) 
{
  //Controller button pins
  const int joystick_button_pin = 4; 
  const int joystick_x_pin = 5; 
  const int joystick_y_pin = 6;  
  const int a_button_pin = 7;
  const int b_button_pin = 15;

  // To not get compiler unused variable error
  (void) pvParameters;

  pinMode(joystick_button_pin, INPUT_PULLUP);
  pinMode(a_button_pin, INPUT);
  pinMode(b_button_pin, INPUT);

  // Fetch analog data from sensors forever
  for (;;) 
  {

    // Read controller button values 
    float joystick_x = analogRead(joystick_x_pin);
    float joystick_y = analogRead(joystick_y_pin);
    int joystick_pressed = digitalRead(joystick_button_pin);
    int a_button = digitalRead(a_button_pin);
    int b_button = digitalRead(b_button_pin);
    uint32_t buttonMask = (joystick_pressed << 2) | (a_button << 1) |(b_button);

    //Send Data to Persistant State
    PersistentState::instance().setJoystick(joystick_x, joystick_y);
    PersistentState::instance().setButtonsBitmask(buttonMask);

    vTaskDelay(pdMS_TO_TICKS(10));; 
  }

}
//Description: Commands all haptic spools
//Parameters: pvParameters which is a place holder for any pointer to any type
//Return: none, it will simply pass the information on to the next core for processing
void TaskServoControl(void *pvParameters) 
{
  //Servo finger pins
  static const int thumbPin = 16; 
  static const int indexPin = 17; 
  static const int middlePin = 18;  
  static const int ringPin = 8; 
  static const int pinkiePin = 9; 

  // Setup servo objects
  Servo thumbServo;
  Servo indexServo;
  Servo middleServo;
  Servo ringServo;
  Servo pinkieServo;

  thumbServo.attach(thumbPin);
  indexServo.attach(indexPin);
  middleServo.attach(middlePin);
  ringServo.attach(ringPin);
  pinkieServo.attach(pinkiePin);


  // To not get compiler unused variable error
  (void) pvParameters;

  // Fetch analog data from sensors forever
  for (;;) 
  {

    /* For debugging*/
    PersistentState::instance().setServoTargetAngle(0, 0);
    PersistentState::instance().setServoTargetAngle(1, 0);
    PersistentState::instance().setServoTargetAngle(2, 0);
    PersistentState::instance().setServoTargetAngle(3, 0);
    PersistentState::instance().setServoTargetAngle(4, 0);

    // Read persistant state and command servos
    thumbServo.write(PersistentState::instance().getServoTargetAngle(0));
    indexServo.write(PersistentState::instance().getServoTargetAngle(1));
    middleServo.write(PersistentState::instance().getServoTargetAngle(2));
    ringServo.write(PersistentState::instance().getServoTargetAngle(3));
    pinkieServo.write(PersistentState::instance().getServoTargetAngle(4));
    vTaskDelay(pdMS_TO_TICKS(10));;
  }
}

//Description: Print out current values from persistant state
//Parameters: pvParameters which is a place holder for any pointer to any type
//Return: none, it will simply pass the information on to the next core for processing
void TaskPersistentStatePrint(void *pvParameters) 
{
  // To not get compiler unused variable error
  (void) pvParameters;

  for(;;)
  {
      if(DEBUG_PRINT)
      {
        Serial.println("\n=== PAYLOAD STATUS ===");

        // Finger angles
        Serial.println("Finger Angles (deg):");
        Serial.printf("  Thumb : %.1f\n",  PersistentState::instance().getFingerAngle(0));
        Serial.printf("  Index : %.1f\n",  PersistentState::instance().getFingerAngle(1));
        Serial.printf("  Middle: %.1f\n",  PersistentState::instance().getFingerAngle(2));
        Serial.printf("  Ring  : %.1f\n",  PersistentState::instance().getFingerAngle(3));
        Serial.printf("  Pinkie: %.1f\n",  PersistentState::instance().getFingerAngle(4));
        Serial.println();

        // Servo targets
        Serial.println("Servo Targets (deg):");
        Serial.printf("  Thumb : %.1f\n",  PersistentState::instance().getServoTargetAngle(0));
        Serial.printf("  Index : %.1f\n",  PersistentState::instance().getServoTargetAngle(1));
        Serial.printf("  Middle: %.1f\n",  PersistentState::instance().getServoTargetAngle(2));
        Serial.printf("  Ring  : %.1f\n",  PersistentState::instance().getServoTargetAngle(3));
        Serial.printf("  Pinkie: %.1f\n",  PersistentState::instance().getServoTargetAngle(4));
        Serial.println();

        // Vibration motors
        Serial.println("Vibration RPM:");
        Serial.printf("  Thumb : %d\n", PersistentState::instance().getVibrationRPM(0));
        Serial.printf("  Index : %d\n", PersistentState::instance().getVibrationRPM(1));
        Serial.printf("  Middle: %d\n", PersistentState::instance().getVibrationRPM(2));
        Serial.printf("  Ring  : %d\n", PersistentState::instance().getVibrationRPM(3));
        Serial.printf("  Pinkie: %d\n", PersistentState::instance().getVibrationRPM(4));
        Serial.println();

        // Joystick
        float joyX, joyY;
        PersistentState::instance().getJoystick(joyX, joyY);
        Serial.println("Joystick:");
        Serial.printf("  X: %.3f\n", joyX);
        Serial.printf("  Y: %.3f\n", joyY);
        Serial.println();

        // Buttons
        uint32_t buttons = PersistentState::instance().getButtonsBitmask();
        Serial.println("Buttons:");
        Serial.printf("  Bitmask      : 0b%03d (0x%02X)\n", buttons, buttons);
        Serial.printf("  Joystick Btn : %s\n", (buttons & 0b100) ? "released" : "PRESSED");
        Serial.printf("  A Button     : %s\n", (buttons & 0b010) ? "PRESSED" : "released");
        Serial.printf("  B Button     : %s\n", (buttons & 0b001) ? "PRESSED" : "released");
        Serial.println();

        // Battery
        Serial.printf("Battery: %d%%\n", PersistentState::instance().getBatteryPercent());
      }
      vTaskDelay(pdMS_TO_TICKS(1000)); 
  }
}


