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

// Average of finger reading angles
int readSmooth(int pin) 
{
  long long sum = 0;
  for (int i = 0; i < FLEX_SENSOR_SAMPLE_RATE; i++) 
  {
    sum += abs(analogRead(pin));
  }
  return sum / FLEX_SENSOR_SAMPLE_RATE;
}

// Combined filter: blocks large anomalies AND small jitters
int bandPassFilter(int newVal, int& lastVal) 
{
    // Get difference between values
    int delta = abs(newVal - lastVal);
    
    // If change is too large, it's an anomaly
    if (delta > MAX_FINGER_STEP) 
    {
      return lastVal;
    }
    
    // If change is too small, it's just noise
    if (delta <= DEADBAND) 
    {
      return lastVal;
    }
    
    // Change is reasonable 
    lastVal = newVal;
    return newVal;
}

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
    0                 // Run on core 0
  );
}


void loop() 
{
  
}

//Description: Reads all Analog Data from sensors
//Parameters: pvParameters which is a place holder for any pointer to any type
//Return: none, it will simply pass the information on to the next core for processing
void TaskAnalogRead(void *pvParameters) 
{

  // Create Static variable of lastFinger
  static int lastFinger[5] = {0, 0, 0, 0, 0};

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

  
  // Finger Calibration value, no flex position is going to be assumbed when user flashes moule
  int thumbCalibrationAngle = 4095 - readSmooth(thumbPin);
  int indexCalibrationAngle = 4095 - readSmooth(indexPin);
  int middleCalibrationAngle = 4095 - readSmooth(middlePin);
  int ringCalibrationAngle = 4095 - readSmooth(ringPin);
  int pinkieCalibrationAngle = 4095 - readSmooth(pinkiePin);
  
  // Fetch analog data from sensors forever
  for (;;) 
  {
    
    //OpenGloves just wants raw adc val, however we will sample each pin 500 times and average the values, and subtract it by the calibration value, and if somehow negative,
    //due to noise, we will simply round it up to 0
    int rawThumb  = max(0, (4095 - readSmooth(thumbPin)) - thumbCalibrationAngle);
    int rawIndex  = max(0, (4095 - readSmooth(indexPin)) - indexCalibrationAngle);
    int rawMiddle = max(0, (4095 - readSmooth(middlePin)) - middleCalibrationAngle);
    int rawRing   = max(0, (4095 - readSmooth(ringPin)) - ringCalibrationAngle);
    int rawPinkie = max(0, (4095 - readSmooth(pinkiePin)) - pinkieCalibrationAngle);

    // Filter anomolies from circuit and update lastFingers
    int thumbAngle  = bandPassFilter(rawThumb, lastFinger[0]);
    int indexAngle  = bandPassFilter(rawIndex, lastFinger[1]);
    int middleAngle = bandPassFilter(rawMiddle, lastFinger[2]);
    int ringAngle   = bandPassFilter(rawRing, lastFinger[3]);
    int pinkieAngle = bandPassFilter(rawPinkie, lastFinger[4]);


    // Read controller button values 
    float joystick_x = map(analogRead(joystick_x_pin),0,4095,0,1024);
    float joystick_y = map(analogRead(joystick_y_pin),0,4095,0,1024);
    int joystick_pressed = digitalRead(joystick_button_pin);
    int a_button = digitalRead(a_button_pin);
    int b_button = digitalRead(b_button_pin);

    uint32_t buttonMask = (joystick_pressed << 2) | (a_button << 1) |(b_button);

    // Calculate trigger button passed of value of bending
    if(pinkieAngle > 50)
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
    PersistentState::instance().setFingerAngle(0, (rawThumb));
    PersistentState::instance().setFingerAngle(1, (rawIndex));
    PersistentState::instance().setFingerAngle(2, (rawMiddle));
    PersistentState::instance().setFingerAngle(3, (rawRing));   
    PersistentState::instance().setFingerAngle(4, (rawPinkie));
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
      
      vTaskDelay(pdMS_TO_TICKS(200)); 
  }
}


