#include "serial.h"

SerialDevice::SerialDevice(const std::string &portName, unsigned int baudRate, bool parity, int databits, int stopbits, bool flowcontrol)
{
    this->portName = portName;
    this->baudRate = baudRate;
    this->parity = parity;
    this->databits = databits;
    this->stopbits = stopbits;
    this->flowcontrol = flowcontrol;
    hSerial = INVALID_HANDLE_VALUE;
}

SerialDevice::~SerialDevice()
{
    // Destructor implementation(simple making sure port is closed)
    close();
}
bool SerialDevice::connect()
{
    // Open serial port using windows stuff
    // 1st: port name ("COM3", "COM4", etc)
    // 2nd: access mode (read and write)
    // 3rd: share mode (0 = no sharing)
    // 4th: security attributes (NULL = default)
    // 5th: creation disposition (OPEN_EXISTING = open existing file/device)
    hSerial = CreateFileA(portName.c_str(),
                          GENERIC_READ | GENERIC_WRITE,
                          0,
                          NULL,
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL,
                          NULL);

    // Check if the port opened successfully
    if (hSerial == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Error opening serial port: " << portName << std::endl;
        return false;
    }
    else
    {
        std::cout << "Serial port opened successfully: " << portName << std::endl;
    }

    // Set Device parameters in DCB structure to set our reading parameters
    dcbSerialParams.DCBlength = sizeof(DCB);
    if (!GetCommState(hSerial, &dcbSerialParams))
    {
        std::cerr << "Error getting current serial parameters" << std::endl;
    }
    else
    {
        dcbSerialParams.BaudRate = baudRate;
        dcbSerialParams.ByteSize = databits;
        dcbSerialParams.StopBits = stopbits;
        dcbSerialParams.Parity = parity;

        if (!SetCommState(hSerial, &dcbSerialParams))
        {
            std::cerr << "Error setting serial port parameters" << std::endl;
            return false;
        }
        else
        {
            std::cout << "Serial port parameters set successfully" << std::endl;
        }
    }

    // Set timeouts
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    // See if there were any errors with the timouts
    if (!SetCommTimeouts(hSerial, &timeouts))
    {
        std::cerr << "Error setting timeouts" << std::endl;
    }
    else
    {
        std::cout << "Timeouts set successfully" << std::endl;
    }
    return true;
}

std::string SerialDevice::readLine()
{
    // Read a line from the serial port
    std::string line;
    char ch;
    DWORD bytesRead;

    // Read character until we reach a null terminator or error/no data
    // Else we keep adding to our running string line
    while (true)
    {
        if (!ReadFile(hSerial, &ch, 1, &bytesRead, NULL) || bytesRead == 0)
        {
            // Error or no data
            break;
        }
        if (ch == '\n')
        {
            // End of line(TRON)
            break;
        }
        line += ch;
    }
    return line;
}
bool SerialDevice::close()
{
    // Close the serial port
    if (isConnected() && CloseHandle(hSerial))
    {
        std::cout << "Serial port closed successfully" << std::endl;
        return true;
    }
    else
    {
        std::cerr << "Error closing serial port" << std::endl;
        return false;
    }
}
bool SerialDevice::isConnected()
{
    // Check if the serial port is connected
    return hSerial != INVALID_HANDLE_VALUE;
}