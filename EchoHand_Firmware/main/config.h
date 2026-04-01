#pragma once

// Stops outputting in openglove format and outputs the persistant state values read
#define DEBUG_PRINT 0

// Serial Plot values instead of having a readable format
#define USE_SERIAL_PLOTTER 0


// Use for setting UP AT commands on external bluetooth device
#define BLUETOOTH_SETUP 0

// Setup communcation method
// 0->Wired over Serial
// 1->Bluetooth Serial
// 2-> WIFI (ESPNOW)
// Enable WIFI mode(note bluetooth serial must be set to 0)
#define COMMUNCATION 2

// Averages values read from flex sensor by x amount
#define POT_SAMPLE_RATE 16

// Enable which noise reducation algo for flex sensor adc polling
// 0-> No average
// 1-> Average
// 2-> Median
// 3->Trimmed Mean
#define POLL_METHOD 2

// Calibration Phase Method
// 0 -> Average
// 1-> Most extreme value
#define CALIBRATION_METHOD 1

// If running Wokawai Simulation
#define SIMULATION 0

// Servo deadzone used for haptics
#define SERVO_DEADZONE 0

//-------Circuit Pin Connections-------//

// Potentiometers
inline constexpr uint8_t THUMB_POT = 8;
inline constexpr uint8_t INDEX_POT = 4;
inline constexpr uint8_t MIDDLE_POT = 7;
inline constexpr uint8_t RING_POT = 5;
inline constexpr uint8_t PINKIE_POT = 6;

// Servos
inline constexpr uint8_t THUMB_SERVO = 45;
inline constexpr uint8_t INDEX_SERVO = 35;
inline constexpr uint8_t MIDDLE_SERVO = 36;
inline constexpr uint8_t RING_SERVO = 37;
inline constexpr uint8_t PINKIE_SERVO = 38;

// Joystick and buttons
inline constexpr uint8_t JOYSTICK_BUTTON = 11;
inline constexpr uint8_t JOYSTICK_X = 9;
inline constexpr uint8_t JOYSTICK_Y = 10;
inline constexpr uint8_t A_BUTTON = 12;
inline constexpr uint8_t B_BUTTON = 15;

// Bluetooth Buttons
inline constexpr uint8_t BLUETOOTH_RX = 18;
inline constexpr uint8_t BLUETOOTH_TX = 17;
