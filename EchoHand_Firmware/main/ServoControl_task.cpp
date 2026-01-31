#include "ServoControl_task.h"

// Description: Commands all haptic spools
// Parameters: pvParameters which is a place holder for any pointer to any type
// Return: none, it will simply pass the information on to the next core for processing
void TaskServoControl(void *pvParameters)
{

    // Servo finger pins
    static const int thumbPin = 45;
    static const int indexPin = 35;
    static const int middlePin = 36;
    static const int ringPin = 37;
    static const int pinkiePin = 38;

    // Set pin modes
    pinMode(thumbPin, OUTPUT);
    pinMode(indexPin, OUTPUT);
    pinMode(middlePin, OUTPUT);
    pinMode(ringPin, OUTPUT);
    pinMode(pinkiePin, OUTPUT);

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
    (void)pvParameters;

    // Fetch analog data from sensors forever
    for (;;)
    {
        // Read persistant state and command servos
        thumbServo.write(180 - DataBroker::instance().getServoTargetAngle(0));
        indexServo.write(180 - DataBroker::instance().getServoTargetAngle(1));
        middleServo.write(180 - DataBroker::instance().getServoTargetAngle(2));
        ringServo.write(180 - DataBroker::instance().getServoTargetAngle(3));
        pinkieServo.write(180 - DataBroker::instance().getServoTargetAngle(4));
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}