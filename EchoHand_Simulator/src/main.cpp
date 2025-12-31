// Modern windows
#define _WIN32_WINNT 0x0600

// Force use ASCII characters since that's what TCP usually uses
#undef UNICODE

// Excludes GDI definitions (fixes Rectangle collision)
#define NOGDI
// Excludes User32 definitions (fixes CloseWindow/ShowCursor collision)
#define NOUSER

// Speeds up build process by reducing amount of windows headers we import from windows.h
#define WIN32_LEAN_AND_MEAN

#include "socket.h"
#include "serial.h"

// Undefine generic macros that might still leak through (like audio or text)
// PlaySound comes from mmsystem.h (linked via winmm), so NOGDI doesn't catch it
#undef PlaySound
#undef LoadImage
#undef DrawText
#undef DrawTextEx

#include "glove.h"

// Standard C libraries for mem management and io
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include <iostream>
#include <unistd.h>

int main(int argc, char *argv[])
{
    // Check if user wants to either serial port or TCP connection
    // Defualt will be TCP connection to Wokwi Echo Hand Simulator
    bool useTCPConnection = true;

    /* If user puts args:
    // Check if user completes all args correctly and if not tell them the correct usage
    // First possible arg: --serial or -s to use serial connection
    // Second arg: COM port name example: COM3, COM4, etc.
    // Third arg: Baud rate example: 115200, 9600, etc.
    // Fourth arg: Parity (0 = none, 1 = odd, 2 = even)
    // Fifth arg: Data bits (usually 8)
    // Sixth arg: Stop bits (1 or 2)
    // Seventh arg: Flow control (0 = none, 1 = hardware, 2 = software)
    // Second possible arg: --tcp or -t to use TCP connection (default)
    // first arg(not required): PORT number (usually 4000)
    */

    // Making variable to hold serial or tcp parameters if needed
    std::string portName;
    unsigned int baudRate = 115200;
    bool parity = false;
    int databits = 8;
    int stopbits = 1;
    bool flowcontrol = false;
    std::string ipAddress = LOCALHOST_IP;
    int portNumber = 4000;
    if (argc > 1)
    {
        std::string firstArg = argv[1];
        if (firstArg == "--serial" || firstArg == "-s")
        {
            // User wants serial connection
            useTCPConnection = false;
            if (argc != 8)
            {
                std::cout << "Usage: " << argv[0] << " --serial <PORT_NAME> <BAUD_RATE> <PARITY> <DATA_BITS> <STOP_BITS> <FLOW_CONTROL>\n";
                std::cout << "Example: " << argv[0] << " --serial COM3 115200 0 8 1 0\n";
                return -1;
            }
            else
            {
                portName = argv[2];
                baudRate = std::stoi(argv[3]);
                parity = (std::stoi(argv[4]) != 0);
                databits = std::stoi(argv[5]);
                stopbits = std::stoi(argv[6]);
                flowcontrol = (std::stoi(argv[7]) != 0);
            }
        }
        else if (firstArg == "--tcp" || firstArg == "-t")
        {
            // User wants TCP connection
            useTCPConnection = true;
            // We can have optional args for port and IP but not required
            if (argc >= 3)
            {
                portNumber = std::stoi(argv[2]);
            }
        }
        else
        {
            std::cout << "Invalid argument: " << firstArg << "\n";
            std::cout << "Usage: " << argv[0] << " [--serial | --tcp]\n";
            return -1;
        }
    }
    printf("Starting OpenGloves Echo Hand Simulator...\n");

    // Declare Raylib initialization flag
    bool isRaylibInit = false;

    if (useTCPConnection)
    {
        Socket EchoHandSocket = Socket(4000);
        if (!EchoHandSocket.connect())
        {
            return -1;
        }
        else
        {
            char echoHandPayload[256];
            std::string dataBuffer = "";

            // Now let's get the TCP continuous stream of data from the simulated ESP32
            // Keeping receing data until an error or disconnection occurs(aka not 0)
            while (EchoHandSocket.receiveData(echoHandPayload, 256))
            {
                dataBuffer += echoHandPayload;

                // Process all complete lines in the buffer
                size_t pos = 0;
                while ((pos = dataBuffer.find('\n')) != std::string::npos)
                {
                    std::string line = dataBuffer.substr(0, pos);
                    dataBuffer.erase(0, pos + 1);

                    // Update glove object with the complete line
                    if (!line.empty())
                    {
                        updateGloveObject(line);
                    }
                }

                // Create or update Raylib scene
                if (!isRaylibInit)
                {
                    // Create scene once and never again
                    isRaylibInit = true;
                    initializeWindow();
                    drawGloveObject();
                }
                else
                {
                    // Only break if user wants to close window(either alt-f4, ESC or close button)
                    if (userAttemptedClose())
                    {
                        break;
                    }
                    // Draw the updated scene (MUST be called every frame!)
                    drawGloveObject();
                }
            }
            EchoHandSocket.closeConnection();
            return 0;
        }
    }
    else
    {
        SerialDevice serialDevice = SerialDevice(portName, baudRate, parity, databits, stopbits, flowcontrol);
        if (!serialDevice.connect())
        {
            return -1;
        }
        else
        {
            // Now let's get the serial continuous stream of data from the real ESP32
            while (serialDevice.isConnected())
            {
                std::string echoHandPayload = serialDevice.readLine();
                if (!echoHandPayload.empty())
                {
                    // Create or update Raylib scene
                    if (!isRaylibInit)
                    {
                        // Create scene once and never again
                        isRaylibInit = true;
                        initializeWindow();
                        drawGloveObject();
                    }
                    else
                    {
                        // Only break if user wants to close window(either alt-f4, ESC or close button)
                        if (userAttemptedClose())
                        {
                            break;
                        }
                        // Update glove object with new payload data
                        updateGloveObject(echoHandPayload);
                        // Draw the updated scene (MUST be called every frame!)
                        drawGloveObject();
                    }
                }
            }
            serialDevice.close();
            return 0;
        }
    }
    // Close Raylib window if open
    if (isRaylibInit)
    {
        CloseWindow();
    }
    // Exit program
    printf("Exiting OpenGloves Echo Hand Simulator...\n");
    return 0;
}