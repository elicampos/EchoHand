#include "DataBrokerPrint_task.h"

// Description: Print out current values from persistant state
// Parameters: pvParameters which is a place holder for any pointer to any type
// Return: none, it will simply pass the information on to the next core for processing
void TaskDataBrokerPrint(void *pvParameters)
{
    // To not get compiler unused variable error
    (void)pvParameters;

    for (;;)
    {
        if (DEBUG_PRINT)
        {
            // Check if we should use plotter format
            if (USE_SERIAL_PLOTTER)
            {
                // Serial Plotter format
                Serial.printf("Thumb:%d Index:%d Middle:%d Ring:%d Pinkie:%d ",
                              DataBroker::instance().getFingerAngle(0),
                              DataBroker::instance().getFingerAngle(1),
                              DataBroker::instance().getFingerAngle(2),
                              DataBroker::instance().getFingerAngle(3),
                              DataBroker::instance().getFingerAngle(4));
                Serial.printf("\n");
            }
            else
            {
                // Original Serial Monitor format
                Serial.println("\n=== PAYLOAD STATUS ===");

                // Finger angles
                Serial.println("Finger Angles (deg):");
                Serial.printf("  Thumb : %d\n", DataBroker::instance().getFingerAngle(0));
                Serial.printf("  Index : %d\n", DataBroker::instance().getFingerAngle(1));
                Serial.printf("  Middle: %d\n", DataBroker::instance().getFingerAngle(2));
                Serial.printf("  Ring  : %d\n", DataBroker::instance().getFingerAngle(3));
                Serial.printf("  Pinkie: %d\n", DataBroker::instance().getFingerAngle(4));
                Serial.println();

                // Servo targets

                Serial.println("Servo Targets (deg):");
                Serial.printf("  Thumb : %.1f\n", DataBroker::instance().getServoTargetAngle(0));
                Serial.printf("  Index : %.1f\n", DataBroker::instance().getServoTargetAngle(1));
                Serial.printf("  Middle: %.1f\n", DataBroker::instance().getServoTargetAngle(2));
                Serial.printf("  Ring  : %.1f\n", DataBroker::instance().getServoTargetAngle(3));
                Serial.printf("  Pinkie: %.1f\n", DataBroker::instance().getServoTargetAngle(4));
                Serial.println();

                // Vibration motors
                Serial.println("Vibration RPM:");
                Serial.printf("  Thumb : %d\n", DataBroker::instance().getVibrationRPM(0));
                Serial.printf("  Index : %d\n", DataBroker::instance().getVibrationRPM(1));
                Serial.printf("  Middle: %d\n", DataBroker::instance().getVibrationRPM(2));
                Serial.printf("  Ring  : %d\n", DataBroker::instance().getVibrationRPM(3));
                Serial.printf("  Pinkie: %d\n", DataBroker::instance().getVibrationRPM(4));
                Serial.println();

                // Joystick
                float joyX, joyY;
                DataBroker::instance().getJoystick(joyX, joyY);
                Serial.println("Joystick:");
                Serial.printf("  X: %.3f\n", joyX);
                Serial.printf("  Y: %.3f\n", joyY);
                Serial.println();

                // Buttons
                uint32_t buttons = DataBroker::instance().getButtonsBitmask();
                Serial.println("Buttons:");
                Serial.printf("  Bitmask      : 0b%03lu (0x%02lX)\n", buttons, buttons);
                Serial.printf("  Joystick Btn : %s\n", (buttons & 0b100) ? "released" : "PRESSED");
                Serial.printf("  A Button     : %s\n", (buttons & 0b010) ? "PRESSED" : "released");
                Serial.printf("  B Button     : %s\n", (buttons & 0b001) ? "PRESSED" : "released");
                Serial.println();

                // Battery
                Serial.printf("Battery: %d%%\n", DataBroker::instance().getBatteryPercent());
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}