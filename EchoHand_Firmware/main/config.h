#pragma once

// Stops outputting in openglove format and outputs the persistant state values read
#define DEBUG_PRINT 0

// Serial Plot values instead of having a readable format
#define USE_SERIAL_PLOTTER 0

// Enable Bluetooth Serial Communication instead of USB Serial
#define USE_BLUETOOTH_SERIAL 0

// Averages values read from flex sensor by x amount
#define POT_SAMPLE_RATE 16

// Enable which noise reducation algo for flex sensor adc polling
// 0-> No average
// 1-> Average
// 2-> Median
// 3->Trimmed Mean
#define POLL_METHOD 1

// Circuit Pin Connections

// Potentiometers
inline constexpr uint8_t THUMB_POT = 8;
inline constexpr uint8_t INDEX_POT = 3;
inline constexpr uint8_t MIDDLE_POT = 12;
inline constexpr uint8_t RING_POT = 13;
inline constexpr uint8_t PINKIE_POT = 14;

// Servos
inline constexpr uint8_t THUMB_SERVO = 45;
inline constexpr uint8_t INDEX_SERVO = 35;
inline constexpr uint8_t MIDDLE_SERVO = 36;
inline constexpr uint8_t RING_SERVO = 37;
inline constexpr uint8_t PINKIE_SERVO = 38;

// Calibration Phase Method
// 0 -> Average
// 1-> Most extreme value
#define CALIBRATION_METHOD 0

// If running Wokawai Simulation
#define SIMULATION 0

// Servo deadzone used for haptics
#define SERVO_DEADZONE 30
