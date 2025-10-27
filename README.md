# EchoHand

A haptic feedback glove system for VR/AR applications using an ESP32 microcontroller with flex sensors, servo motors, and Bluetooth Low Energy (BLE) communication.

## Project Overview

EchoHand is a wearable haptic glove that captures hand and finger movements using flex sensors and provides tactile feedback through servo motors and vibration systems. The system communicates wirelessly via BLE to transmit real-time finger position data and receive haptic feedback commands.

## System Architecture

### External Interface

The external interface layer is implemented in `EchoHand_Firmware.ino` and handles all sensor data acquisition. This component:

- Polls ADC values from flex sensors mounted on each finger
- Configures the FreeRTOS settings, including task stack sizes and priorities
- Configures serial communication settings for debugging and monitoring

### Persistent State

Data storage is managed through `PersistentState.h`, which implements a singleton pattern for centralized state management. This component:

- Uses `volatile` data types to ensure real-time accessibility across all system components
- Provides thread-safe getters and setters for all sensor and actuator values
- Implements a snapshot mechanism with revision counters for consistent multi-field reads
- Stores current values for finger angles, servo positions, vibration motor RPMs, and joystick inputs

### Internal Systems

The internal processing layer handles data transformation and control logic. This component performs:

- **Sensor Mapping**: Converts raw ADC values (0-4000) to finger angles (0-90 degrees)
- **Command Translation**: Translates BLE commands into PWM signals for servo motor control
- **State Updates**: Continuously updates the PersistentState with processed sensor data
- **Haptic Control**: Manages the relationship between virtual interactions and physical feedback through servo and vibration systems

### Information Handling

Data integrity is maintained through:

- Payload size verification for all BLE transmissions
- Validation of sensor data ranges before processing
- Error checking on state updates to prevent corruption

### Communication

Bluetooth Low Energy (BLE) communication is implemented using the NimBLE library.

- **BLE Server**: ESP32 operates as a GATT server
- **Output Characteristic**: Transmits finger angle data and sensor readings
- **Input Characteristic**: Receives haptic feedback commands for servos and vibration motors
- **Pairing**: Secure encrypted connection prevents unauthorized access

### Integrity & Resilience

Security and reliability features include:

- **Encryption**: All BLE packets are encrypted to prevent man-in-the-middle attacks
- **Pairing Protocol**: Only authenticated devices can connect and exchange data
- **RTOS Task Management**: FreeRTOS ensures deterministic behavior and prevents task conflicts

## Known Bugs

### 1. BLE Connection Inconsistency
**Description**: The BLE connection between the ESP32 and host device is not consistently stable.

### 2. Flex Sensor Angle Inaccuracy
**Description**: Flex sensor readings have about 80% accuracy.

## Future Work
#### 1. Input Controls
- **Joystick Integration**: Add analog joystick for additional user input (navigation, menu control)
- **Button Addition**: Integrate two programmable buttons for compability with most applications

#### 2. Enhanced Haptic Feedback
- **Servo Motor System**: Implement servo motors with spool-and-cable mechanism
  - Provides active resistance feedback to simulate object interaction
  - Dual-sensor approach: flex sensors + servo encoders for improved angle accuracy
  - Addresses current 20% inaccuracy issue through sensor fusion

#### 3. Connectivity Options
- **Dual-Mode Support**: Enable both wired (USB) and wireless (BLE) operation
  - Wired mode for low-latency, high-reliability applications
  - Wireless mode for freedom of movement
  - Automatic mode switching based on cable detection

#### 4. 3D Printed Components
Design and print custom mounting solutions:
- **Adjustable Joystick Mount and Button Placement**: Ergonomic positioning based on hand size
  - Uses clamp mechanism around index finger
- **ESP32 Chassis**: Protective housing for the microcontroller
  - Mounts on back of palm toward wrist
  - Accommodates battery for wireless operation
