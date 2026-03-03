idf_component_register(
    # Source files to compile
    SRCS "AnalogRead_task.cpp" "ServoControl_task.cpp" "DataBrokerPrint.cpp" "main.cpp" "Communication_task.cpp" 

    # Header files to compile
    INCLUDE_DIRS "."

    # ESP-IDF component list
    # "arduino-esp32": Enables Arduino functions like Serial.begin() and delay()
    # "nvs_flash": Required for WiFi and Bluetooth to save settings.
    # "ESP32Servo": ESP32Servo library for servo control
    REQUIRES arduino-esp32 nvs_flash ESP32Servo
)