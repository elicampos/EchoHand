idf_component_register(
    # Source files to compile
    SRCS "src/ESP32Servo.cpp"
         "src/ESP32PWM.cpp"
    # Header files to compile
    INCLUDE_DIRS "src"

    # ESP-IDF component list
    REQUIRES arduino-esp32
)
