# EchoHand

A haptic feedback glove system for VR/AR applications using an ESP32 microcontroller with flex sensors, servo motors, and Serial communication.

## Project Overview

EchoHand is a wearable haptic glove that captures hand and finger movements using flex sensors and provides tactile feedback through servo motors and vibration systems. The system communicates wirelessly via USB Serial to transmit real-time finger position data and receive haptic feedback commands.

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
- Stores current values for finger angles, servo positions, vibration motor RPMs, and button inputs

### Internal Systems

The internal processing layer handles data transformation and control logic. This component performs:

- **Sensor Mapping**: Averages raw ADC values (0-4095) to get more accurate values
- **Command Translation**: Translates serial commands PWM signals for servo motor control
- **State Updates**: Continuously updates the PersistentState with processed sensor data
- **Haptic Control**: Manages the relationship between virtual interactions and physical feedback through servo and vibration systems

### Information Handling

Data integrity is maintained through:
- Validation of sensor data ranges before processing
- Error checking on state updates to prevent corruption

### Communication

Current communication is done through USB Serial. 
- **Output Characteristic**: Transmits finger angle data and sensor readings
- **Input Characteristic**: Receives haptic feedback commands for servos and vibration motors

In the future, Bluetooth Low Energy (BLE) communication will be implemented using the NimBLE library.

- **BLE Server**: ESP32 operates as a GATT server
- **Output Characteristic**: Transmits finger angle data and sensor readings
- **Input Characteristic**: Receives haptic feedback commands for servos and vibration motors
- **Pairing**: Secure encrypted connection prevents unauthorized access

### Integrity & Resilience

Current security and reliability features include:
- **RTOS Task Management**: FreeRTOS ensures deterministic behavior and prevents task conflicts

Future security and reliability features include:
- **Encryption**: All BLE packets are encrypted to prevent man-in-the-middle attacks
- **Pairing Protocol**: Only authenticated devices can connect and exchange data

## Known Bugs

### 1. Flex sensors are not securely attached 
**Description**: Flex sensors need to be attached with a Amphenol FCI Clincher Connector (2 Position) to prevent disconnection.

### 2. Joystick Drift
**Description**: While the joystick is at the origin, the OpenHaptics program seem to be reading a small value in the X direction.

### 3. BLE Connection Inconsistency
**Description**: The BLE connection between the ESP32 and host device is not consistently stable.

### 4. Flex Sensor Angle Inaccuracy
**Description**: Flex sensor readings have about 80% accuracy.

## Future Work

#### 1. Implement Haptic Feedback
- **Servo Motor System**: Implement servo motors with spool-and-cable mechanism
  - Provides active resistance feedback to simulate object interaction
  - Dual-sensor approach: flex sensors + servo encoders for improved angle accuracy
  - Addresses current 20% inaccuracy issue through sensor fusion

#### 3. Solder or PCB Development 
- **Compact Size**: To be able to sit on the hand and arm comfortably, we will need to reduce the size of the circuit 
  - Allow better heat dissapation
  - Lighter weight
  - Less potential circuit damage from heavy use

#### 3. Connectivity Options
- **Dual-Mode Support**: Enable both wired (USB) and wireless (BLE) operation
  - Wired mode for low-latency, high-reliability applications
  - Wireless mode for freedom of movement
  - Automatic mode switching based on cable detection

#### 4. Vibration Motors
Attach vibration motors on the fingertips of the user to simulate touching an object.
- **Custom Unity Enviornment**: A custom Unity enviornment will send the current texture that the user's finger is touching, to the ESP32.
- **ESP32 Hashamp**: The ESP32 will receive the texture along with the finger index, and lookup the appropriate rpm to send the vibration motor.  

#### 5. 3D Printed Components
Design and print custom mounting solutions:
- **Adjustable Joystick Mount and Button Placement**: Ergonomic positioning based on hand size
  - Uses clamp mechanism around index finger
- **ESP32 Chassis**: Protective housing for the microcontroller
  - Mounts on back of palm toward wrist
  - Accommodates battery for wireless operation
