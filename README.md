# EchoHand

A haptic feedback glove for VR applications utilizing an ESP32-S3 or ESP32-WROOM-32 microcontroller. 

## Project Overview

EchoHand is a wearable haptic glove that captures hand and finger movements, and provides haptic feedback through the servo motors and vibration systems. The system communicates via USB Serial and Bluetooth Serial to transmit all the sensor data to the computer in real-time.

## System Architecture

### External Interface

The external interface layer is implemented in `main.cpp` and configures and runs every Freertos task. This component:

- Configures the FreeRTOS settings, including task stack sizes and priorities
- Configures serial communication settings for debugging and monitoring

### DataBroker

Data storage is managed through `DataBroker.h`, which implements a singleton pattern for centralized state management. This component:

- Provides thread-safe getters and setters for all sensor and actuator values
- Implements a snapshot mechanism with revision counters for consistent multi-field reads
- Stores current values for finger angles, servo positions, vibration motor RPMs, and button inputs

### Internal Systems

The internal processing layer handles data transformation and control logic. This component performs:

- **Sensor Mapping**: Get's center of raw ADC values (0-4095) to get more accurate values
- **Command Translation**: Translates serial commands PWM signals for servo motor control
- **State Updates**: Continuously updates the DataBroker with processed sensor data
- **Haptic Control**: Manages the relationship between virtual interactions and physical feedback through servo and vibration systems

### Information Handling

Data integrity is maintained through:
- Validation of sensor data ranges before processing
- Error checking on state updates to prevent corruption

### Communication

Communication is done through USB Serial or Bluetooth Serial dependent on the value in `config.h`.
- **Output Characteristic**: Transmits finger angle data and sensor readings
- **Input Characteristic**: Receives haptic feedback commands for servos and vibration motors

### Integrity & Resilience

Current security and reliability features include:
- **RTOS Task Management**: FreeRTOS ensures deterministic behavior and prevents task conflicts

Future security and reliability features include:
- **Encryption**: All Bluetooth packets are encrypted(after pairing) to prevent man-in-the-middle attacks
- **Pairing Protocol**: Only authenticated devices can connect and exchange data

## Known Bugs

### 1. Random Finger position Jitter
**Description**: Due to no low-pass filter on the slider pins. Implementing this via software will cause too much of a delay.

### 2. Unstable SteamVR Connections
**Description**: Sometimes SteamVR will crash or fail to connect to the Echohand requiring a restart on both devices.

## Future Work

#### 1. Create a new PCB with an ESP32-WROOM-32
- **New ESP32-WROOM-32 Circuit**: Reduce size, weight and cost
  - Has Bluetooth Serial integrated in the chip itself (reducing cost, weight and size)
  - More pins on ADC1 compared to the ESP32-S3 allowing use all components and use WIFI

#### 2. 3D Printed Components
- **Design and print custom mounting solutions**: To be able to use the joystick and button comfortably 
  - Uses clamp mechanism around index finger

#### 3. Improve Setup Time
- **Develop or modify code**: Create software or update Unity enviornment to reduce user setup time
  - Create a process that runs in the background to manage existing communcation between the ESP32 and Opengloves
  or modify the Opengloves steam driver itself

#### 4. Reduce Latency
- **Modify current software stack**: Improve speed of packet generation and transmission on the ESP32
  - Support ESP-NOW
  - Make singleton more efficent
  - Support DMA for ADC while still utilizing ESP32 efuse information
