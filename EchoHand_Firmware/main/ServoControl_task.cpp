#include "ServoControl_task.h"

// Description: Commands all haptic spools
// Parameters: pvParameters which is a place holder for any pointer to any type
// Return: none, it will simply pass the information on to the next core for processing
void TaskServoControl(void *pvParameters)
{

    // Set pin modes
    pinMode(THUMB_SERVO, OUTPUT);
    pinMode(INDEX_SERVO, OUTPUT);
    pinMode(MIDDLE_SERVO, OUTPUT);
    pinMode(RING_SERVO, OUTPUT);
    pinMode(PINKIE_SERVO, OUTPUT);

    // Setup servo objects
    Servo thumbServo;
    Servo indexServo;
    Servo middleServo;
    Servo ringServo;
    Servo pinkieServo;

    thumbServo.attach(THUMB_SERVO);
    indexServo.attach(INDEX_SERVO);
    middleServo.attach(MIDDLE_SERVO);
    ringServo.attach(RING_SERVO);
    pinkieServo.attach(PINKIE_SERVO);

    // To not get compiler unused variable error
    (void)pvParameters;

    // Reset all servo angles to 0 so user can use glove
    // Get servo angle from openhaptics
    thumbServo.write(180);
    indexServo.write(180);
    middleServo.write(180);
    ringServo.write(180);
    pinkieServo.write(180);

    // Fetch analog data from sensors forever
    for (;;)
    {
        // Wait until all servo values have been updates from the communcation task
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Get servo angle from openhaptics
        int thumbAngle = DataBroker::instance().getServoTargetAngle(0);
        int indexAngle = DataBroker::instance().getServoTargetAngle(1);
        int middleAngle = DataBroker::instance().getServoTargetAngle(2);
        int ringAngle = DataBroker::instance().getServoTargetAngle(3);
        int pinkieAngle = DataBroker::instance().getServoTargetAngle(4);

        /*
        if (thumbAngle <= SERVO_DEADZONE)
        {
            thumbAngle = 0;
        }
        if (indexAngle <= SERVO_DEADZONE)
        {
            indexAngle = 0;
        }
        if (middleAngle <= SERVO_DEADZONE)
        {
            middleAngle = 0;
        }
        if (ringAngle <= SERVO_DEADZONE)
        {
            ringAngle = 0;
        }
        if (pinkieAngle <= SERVO_DEADZONE)
        {
            pinkieAngle = 0;
        }
        */

        // Command servos
        thumbServo.write(180 - thumbAngle);
        indexServo.write(180 - indexAngle);
        middleServo.write(180 - middleAngle);
        ringServo.write(180 - ringAngle);
        pinkieServo.write(180 - pinkieAngle);
    }
}