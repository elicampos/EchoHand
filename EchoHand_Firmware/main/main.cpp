#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.h"
#include "DataBroker.h"
#include <ESP32Servo.h>
#include <WiFi.h>
#include <Arduino.h>

// Import all tasks
#include "AnalogRead_task.h"
#include "SerialCommunication_task.h"
#include "DataBrokerPrint_task.h"
#include "ServoControl_task.h"
#include "WifiCommuncation.h"

// Allocate memory for servo task handler
TaskHandle_t xServoTaskHandle = NULL;

void setup()
{
    // Set BaudRate
    Serial.begin(115200);

    // Debug Print
    Serial.println("Starting FreeRTOS Setup...");

    if (DEBUG_PRINT)
    {
        xTaskCreatePinnedToCore(
            TaskDataBrokerPrint, // Fucntion name of Task
            "DataBrokerPrint",   // Name of Task
            8192,                // Stack size (bytes) for task
            NULL,                // Parameters(none)
            0,                   // Priority level(0->Lowest)
            NULL,                // Task handle(for RTOS API maniuplation)
            0                    // Run on core 0
        );
    }

    // Create the task
    xTaskCreatePinnedToCore(
        TaskAnalogRead, // Fucntion name of Task
        "AnalogRead",   // Name of Task
        8192,           // Stack size (bytes) for task
        NULL,           // Parameters(none)
        0,              // Priority level(0->lowest)
        NULL,           // Task handle(for RTOS API maniuplation)
        1               // Run on core 1
    );

    if (COMMUNCATION <= 1)
    {
        xTaskCreatePinnedToCore(
            TaskSerialCommunication, // Fucntion name of Task
            "SerialCommunication",   // Name of Task
            8192,                    // Stack size (bytes) for task
            NULL,                    // Parameters(none)
            0,                       // Priority level(1->highest)
            NULL,                    // Task handle(for RTOS API maniuplation)
            0                        // Run on core 0
        );
    }
    else
    {
        xTaskCreatePinnedToCore(
            TaskWifiCommunication, // Fucntion name of Task
            "WifiCommunication",   // Name of Task
            8192,                  // Stack size (bytes) for task
            NULL,                  // Parameters(none)
            0,                     // Priority level(1->highest)
            NULL,                  // Task handle(for RTOS API maniuplation)
            0                      // Run on core 0
        );
    }

    xTaskCreatePinnedToCore(
        TaskServoControl,  // Fucntion name of Task
        "ServoControl",    // Name of Task
        8192,              // Stack size (bytes) for task
        NULL,              // Parameters(none)
        1,                 // Priority level(1->highest)
        &xServoTaskHandle, // Task handle(for RTOS API maniuplation)
        1                  // Run on core 1
    );
}

extern "C" void app_main()
{
    // Initialize the Arduino framework
    initArduino();

    // Call the standard setup
    setup();

    // Delete task so that we don't hog any cpu time
    vTaskDelete(NULL);
}
