#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.h"
#include "Bluetooth_task.h"
#include "PersistentState.h"
#include "ESP32Servo.h"
#include <WiFi.h>

// Declare Functions
void TaskAnalogRead(void *pvParameters);
void TaskControllerButtons(void* pvParameters);
void TaskServoControl(void* pvParameters);
void TaskPersistentStatePrint(void* pvParameters);

//Description: Averages value read from an analog pin based off sample rate in config.h
//Parameters: rawanalog pin
//Return: sum value given sample rate
//Note: This will be deprecated if we move to ExpressIf Ide as we can do this via hardware
int readSmooth(int pin) 
{
  long long sum = 0;
  for (int i = 0; i < FLEX_SENSOR_SAMPLE_RATE; i++) 
  {
    sum += abs(analogRead(pin));
  }
  return sum / FLEX_SENSOR_SAMPLE_RATE;
}

//Description: Converts adc range of 400->1250 to 0->4250
//Parameters: rawanalog Value
//Return: value that matches range
int mapFlex(int raw)
{
  // Rought estimates from circuit
  const int RAW_MIN = 400;  
  const int RAW_MAX = 1250;  

  // Clamp range to range expected for opengloves
  if (raw < RAW_MIN) raw = RAW_MIN;
  if (raw > RAW_MAX) raw = RAW_MAX;

  // Map to 0–>4095 
  return (raw - RAW_MIN) * 4095L / (RAW_MAX - RAW_MIN);
}


void setup() 
{
  // Set  BaudRate
  Serial.begin(115200); 

  //Debug Print
  Serial.println("Starting FreeRTOS task...");

  // turn Wi-Fi off to make sure ADC2 doesn't get messed up
  WiFi.mode(WIFI_OFF);     
  
  // Create the task
  xTaskCreatePinnedToCore(
    TaskAnalogRead,   // Fucntion name of Task
    "AnalogRead",     // Name of Task
    2048,             // Stack size (bytes) for task
    NULL,             // Parameters(none)
    2,                // Priority level(1->highest)
    NULL,              // Task handle(for RTOS API maniuplation)
    1                  // Run on core 1
  );

  xTaskCreatePinnedToCore(
    TaskBluetoothSerial,   // Fucntion name of Task
    "BluetoothSerial",     // Name of Task
    8192,             // Stack size (bytes) for task
    NULL,             // Parameters(none)
    1,                // Priority level(1->highest)
    NULL,              // Task handle(for RTOS API maniuplation)
    1                 // Run on core 1
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
    1                 // Run on core 0
  );
}


void loop() 
{
  // Nothing, Freertos runs in task
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

  //Controller button pins
  const int joystick_button_pin = 4; 
  const int joystick_x_pin = 5; 
  const int joystick_y_pin = 6;  
  const int a_button_pin = 7;
  const int b_button_pin = 15;

  pinMode(joystick_button_pin, INPUT_PULLUP);
  pinMode(a_button_pin, INPUT);
  pinMode(b_button_pin, INPUT);

  // To not get compiler unused variable error
  (void) pvParameters;

  // Get range from 0.0V to 3.3V
  analogSetAttenuation(ADC_11db); 
  analogReadResolution(12);

  // Finger Calibration value, no flex position is going to be assumbed when user flashes moule
  /*
  int thumbCalibrationAngle = readSmooth(thumbPin);
  int indexCalibrationAngle = readSmooth(indexPin);
  int middleCalibrationAngle = readSmooth(middlePin);
  int ringCalibrationAngle = readSmooth(ringPin);
  int pinkieCalibrationAngle = readSmooth(pinkiePin);
  */
  
  // Fetch analog data from sensors forever
  for (;;) 
  {
    
    //OpenGloves just wants raw adc val, however we will sample each pin 500 times and average the values, and subtract it by the calibration value, and if somehow negative,
    //due to noise, we will simply round it up to 0
    int rawThumb  = max(0, (readSmooth(thumbPin)));
    int rawIndex  = max(0, (readSmooth(indexPin)));
    int rawMiddle = max(0, (readSmooth(middlePin)));
    int rawRing   = max(0, (readSmooth(ringPin)));
    int rawPinkie = max(0, (readSmooth(pinkiePin)));

    // Convert angles to map to desired range for Opengloves(4095->0)
    int thumbAngle  = max(0,(4095 - mapFlex(rawThumb)));
    int indexAngle  = max(0,(4095 - mapFlex(rawIndex)));
    int middleAngle = max(0,(4095 - mapFlex(rawMiddle)));
    int ringAngle   = max(0,(4095 - mapFlex(rawRing)));
    int pinkieAngle = max(0,(4095 - mapFlex(rawPinkie)));

    // Read controller button values 
    float joystick_x = map(analogRead(joystick_x_pin),0,4095,0,1024);
    float joystick_y = map(analogRead(joystick_y_pin),0,4095,0,1024);
    int joystick_pressed = digitalRead(joystick_button_pin);
    int a_button = digitalRead(a_button_pin);
    int b_button = digitalRead(b_button_pin);

    uint32_t buttonMask = (joystick_pressed << 2) | (a_button << 1) |(b_button);

    // Calculate trigger button passed of value of bending
    if(thumbAngle > 50 && indexAngle > 50)
    {
      // Set current 4 bit of bit mask to have trigger button
      buttonMask |= (1 << 3);
    }   

    /* Debug Printing
    Serial.print(thumbAngle);
    Serial.print(", ");
    Serial.print(indexAngle);
    Serial.print(", ");
    Serial.print(middleAngle);
    Serial.print(", ");
    Serial.print(ringAngle);
    Serial.print(", ");
    Serial.println(pinkeAngle);
    */

    //Send Data to Persistant State
    PersistentState::instance().setFingerAngle(0, (thumbAngle));
    PersistentState::instance().setFingerAngle(1, (indexAngle));
    PersistentState::instance().setFingerAngle(2, (middleAngle));
    PersistentState::instance().setFingerAngle(3, (ringAngle));   
    PersistentState::instance().setFingerAngle(4, (pinkieAngle));
    PersistentState::instance().setJoystick(joystick_x, joystick_y);
    PersistentState::instance().setButtonsBitmask(buttonMask);

    vTaskDelay(pdMS_TO_TICKS(2));; 
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
    /*
    PersistentState::instance().setServoTargetAngle(0, 0);
    PersistentState::instance().setServoTargetAngle(1, 0);
    PersistentState::instance().setServoTargetAngle(2, 0);
    PersistentState::instance().setServoTargetAngle(3, 0);
    PersistentState::instance().setServoTargetAngle(4, 0);
    */

    // Read persistant state and command servos
    thumbServo.write(PersistentState::instance().getServoTargetAngle(0));
    indexServo.write(PersistentState::instance().getServoTargetAngle(1));
    middleServo.write(PersistentState::instance().getServoTargetAngle(2));
    ringServo.write(PersistentState::instance().getServoTargetAngle(3));
    pinkieServo.write(PersistentState::instance().getServoTargetAngle(4));
    vTaskDelay(pdMS_TO_TICKS(50));;
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
        // Check if we should use plotter format 
        #ifdef USE_SERIAL_PLOTTER
      
        // Serial Plotter format
        Serial.printf("Thumb:%d Index:%d Middle:%d Ring:%d Pinkie:%d ",
                      PersistentState::instance().getFingerAngle(0),
                      PersistentState::instance().getFingerAngle(1),
                      PersistentState::instance().getFingerAngle(2),
                      PersistentState::instance().getFingerAngle(3),
                      PersistentState::instance().getFingerAngle(4));
        Serial.printf("\n");
        /*
        Serial.printf("Servo_Thumb:%.1f Servo_Index:%.1f Servo_Middle:%.1f Servo_Ring:%.1f Servo_Pinkie:%.1f ",
                      PersistentState::instance().getServoTargetAngle(0),
                      PersistentState::instance().getServoTargetAngle(1),
                      PersistentState::instance().getServoTargetAngle(2),
                      PersistentState::instance().getServoTargetAngle(3),
                      PersistentState::instance().getServoTargetAngle(4));
        
        Serial.printf("Vib_Thumb:%d Vib_Index:%d Vib_Middle:%d Vib_Ring:%d Vib_Pinkie:%d ",
                      PersistentState::instance().getVibrationRPM(0),
                      PersistentState::instance().getVibrationRPM(1),
                      PersistentState::instance().getVibrationRPM(2),
                      PersistentState::instance().getVibrationRPM(3),
                      PersistentState::instance().getVibrationRPM(4));
        
        float joyX, joyY;
        PersistentState::instance().getJoystick(joyX, joyY);
        Serial.printf("Joy_X:%.3f Joy_Y:%.3f ", joyX, joyY);
        
        uint32_t buttons = PersistentState::instance().getButtonsBitmask();
        Serial.printf("Joy_Btn:%d A_Btn:%d B_Btn:%d Trigger:%d ",
                      (buttons & 0b100) ? 0 : 1,  
                      (buttons & 0b010) ? 1 : 0,
                      (buttons & 0b001) ? 1 : 0,
                      (buttons & 0b1000) ? 1 : 0);
        
        Serial.printf("Battery:%d\n", PersistentState::instance().getBatteryPercent());
        */
        #else
        
        // Original Serial Monitor format
        Serial.println("\n=== PAYLOAD STATUS ===");

        // Finger angles
        Serial.println("Finger Angles (deg):");
        Serial.printf("  Thumb : %d\n",  PersistentState::instance().getFingerAngle(0));
        Serial.printf("  Index : %d\n",  PersistentState::instance().getFingerAngle(1));
        Serial.printf("  Middle: %d\n",  PersistentState::instance().getFingerAngle(2));
        Serial.printf("  Ring  : %d\n",  PersistentState::instance().getFingerAngle(3));
        Serial.printf("  Pinkie: %d\n",  PersistentState::instance().getFingerAngle(4));
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
        
        #endif
    }  
    vTaskDelay(pdMS_TO_TICKS(20)); 
  }
}


