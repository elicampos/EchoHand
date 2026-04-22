# EchoHand

A haptic feedback glove for VR applications utilizing ESP32-S3 microcontrollers. 

## Project Overview

EchoHand is a wearable haptic glove that captures hand and finger movements, and provides haptic feedback through the servo motors. The system communicates primary through WI-FI but also supports USB and Bluetooth Serial to transmit all the sensor data to the computer in real-time through the OpenGloves application.

## System Architecture

### External Interface

The external interface layer is executed primarily in `SerialCommunication.cpp` or `WifiCommuncation.cpp` and is responsible for receving and sending packets to the computer running the virtual reality enviornment.

- Configures Bluetooth Serial Communcation and has some variables to toggle AT mode for baudrates changes
- Configures ESP-NOW which allows the ESP32-S3 to talk to the computer quickly and asynchronously.
- Configures USB Serial by printing through default COM port

### DataBroker

Data storage is managed through `DataBroker.h`, which implements a singleton pattern for centralized state management. This component:

- Provides getters and setters for all sensor and actuator values
- Implements a snapshot mechanism with revision counters for consistent multi-field reads
- Stores current values for finger angles, servo positions, and button inputs

### Internal Systems

The internal processing layer handles data transformation and control logic. This component performs:

- **Sensor Mapping**: Get's center of raw ADC values (0-4095) to get more accurate values
- **Command Translation**: Translates serial commands PWM signals for servo motor control
- **State Updates**: Continuously updates the DataBroker with processed sensor data
- **Haptic Control**: Manages the relationship between virtual interactions and physical feedback through servo systems

### Information Handling

Data integrity is maintained through:
- Validation of sensor data ranges before processing
- Error checking on state updates to prevent corruption
- Verifying presence of the null terminator character in strings to prevent overflows
- Ensuring we never read outside of the bounds of C-style strings

### Communication

Communication is done through WI-FI, USB Serial, Bluetooth Serial and is dependent on the values in `config.h`.
- **Output Characteristic**: Transmits finger angle data and button values
- **Input Characteristic**: Receives haptic feedback commands for servos

### Integrity & Resilience

Current security and reliability features include:
- **RTOS Task Management**: FreeRTOS ensures deterministic behavior 

Future security and reliability features include:
- **Encryption**: All Bluetooth packets are encrypted(after pairing) to prevent man-in-the-middle attacks
- **Pairing Protocol**: Only authenticated devices can connect and exchange data

## Known Bugs

### 1. Random Finger position Jitter
**Description**: Due to no low-pass filter on the slider pins. Implementing this via software will cause too much of a delay.

## Future Work

#### 1. User-facing application for easy firmware changes
- **Description**: Create GUI software to allow users to send custom or pre-configured payloads to the EchoHand to update the current EchoHand settings during run-time.

