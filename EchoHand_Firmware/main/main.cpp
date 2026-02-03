#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.h"
#include "DataBroker.h"
#include <ESP32Servo.h>
#include <WiFi.h>
#include <Arduino.h>

// Import all tasks
#include "AnalogRead_task.h"
#include "Communication_task.h"
#include "DataBrokerPrint_task.h"
#include "ServoControl_task.h"

void setup()
{
    // Set BaudRate
    Serial.begin(115200);

    // Debug Print
    Serial.println("Starting FreeRTOS Setup...");

    // turn Wi-Fi off to make sure ADC2 doesn't get messed up
    WiFi.mode(WIFI_OFF);

    // Create the task
    xTaskCreatePinnedToCore(
        TaskAnalogRead, // Fucntion name of Task
        "AnalogRead",   // Name of Task
        8192,           // Stack size (bytes) for task
        NULL,           // Parameters(none)
        1,              // Priority level(1->highest)
        NULL,           // Task handle(for RTOS API maniuplation)
        1               // Run on core 1
    );

    xTaskCreatePinnedToCore(
        TaskCommunication, // Fucntion name of Task
        "Communication",   // Name of Task
        8192,              // Stack size (bytes) for task
        NULL,              // Parameters(none)
        2,                 // Priority level(1->highest)
        NULL,              // Task handle(for RTOS API maniuplation)
        0                  // Run on core 1
    );

    xTaskCreatePinnedToCore(
        TaskServoControl, // Fucntion name of Task
        "ServoControl",   // Name of Task
        8192,             // Stack size (bytes) for task
        NULL,             // Parameters(none)
        1,                // Priority level(1->highest)
        NULL,             // Task handle(for RTOS API maniuplation)
        0                 // Run on core 0
    );

    xTaskCreatePinnedToCore(
        TaskDataBrokerPrint, // Fucntion name of Task
        "DataBrokerPrint",   // Name of Task
        8192,                // Stack size (bytes) for task
        NULL,                // Parameters(none)
        1,                   // Priority level(1->highest)
        NULL,                // Task handle(for RTOS API maniuplation)
        1                    // Run on core 0
    );
}

extern "C" void app_main()
{
    // Initialize the Arduino framework
    initArduino();

    // Call your standard setup
    setup();

    // Delete task so that we don't hog any cpu time
    vTaskDelete(NULL);
}
