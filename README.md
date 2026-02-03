# EchoHand

A haptic feedback glove for VR applications utilizing an ESP32-S3 microcontroller. 

## Project Overview

EchoHand is a wearable haptic glove that captures hand and finger movements, and provides haptic feedback through the servo motors and vibration systems. The system communicates via USB Serial and Bluetooth Serial to transmit all the sensor data to the computer in real-time.

## System Architecture

### External Interface

The external interface layer is implemented in `main.cpp` and configures and runs every Freertos task. This component:

- Polls ADC values from flex sensors or potentiometer on each finger
- Configures the FreeRTOS settings, including task stack sizes and priorities
- Configures serial communication settings for debugging and monitoring

### DataBroker

Data storage is managed through `DataBroker.h`, which implements a singleton pattern for centralized state management. This component:

- Provides thread-safe getters and setters for all sensor and actuator values
- Implements a snapshot mechanism with revision counters for consistent multi-field reads
- Stores current values for finger angles, servo positions, vibration motor RPMs, and button inputs

### Internal Systems

The internal processing layer handles data transformation and control logic. This component performs:

- **Sensor Mapping**: Averages raw ADC values (0-4095) to get more accurate values
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
- **Encryption**: All BLE packets are encrypted to prevent man-in-the-middle attacks
- **Pairing Protocol**: Only authenticated devices can connect and exchange data

## Known Bugs

### 1. Random Jitter
**Description**: Due to everything still being on a breadboard there is some noise when polling the ADCs.

### 1.Grabbing Hitter
**Description**: Held objects can sometimes fall randomly due to the "L" not being set in the payload consistently. 

## Future Work

#### 1. Solder or PCB Development(Waiting for final beta 3d-printed parts)
- **Compact Size**: To be able to sit on the hand and arm comfortably, we will need to reduce the size of the circuit 
  - Allow better heat dissapation
  - Lighter weight
  - Less potential circuit damage from heavy use

#### 2. Vibration Motors
Attach vibration motors on the fingertips of the user to simulate touching an object.
- **Custom Unity Enviornment**: A custom Unity enviornment will send the current texture that the user's finger is touching, to the ESP32.
- **ESP32 Hashamp**: The ESP32 will receive the texture along with the finger index, and lookup the appropriate rpm to send the vibration motor.  

#### 3. 3D Printed Components
Design and print custom mounting solutions:
- **Adjustable Joystick Mount and Button Placement**: Ergonomic positioning based on hand size
  - Uses clamp mechanism around index finger

