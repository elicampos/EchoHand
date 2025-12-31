#pragma once
#include <string>
#include <cstdint>
#include <windows.h>
#include <iostream>

// SerialDevice class for handling serial port communication
class SerialDevice
{
    // User Parameters
    std::string portName;
    unsigned int baudRate = 0;
    bool parity = false;
    int databits = 0;
    int stopbits = 0;
    bool flowcontrol = false;

    // Windows Serial COM Stuff
    HANDLE hSerial;
    DCB dcbSerialParams;
    COMMTIMEOUTS timeouts;

public:
    SerialDevice(const std::string &portName, unsigned int baudRate, bool parity, int databits, int stopbits, bool flowcontrol);
    ~SerialDevice();
    bool connect();
    std::string readLine();
    bool close();
    bool isConnected();
};