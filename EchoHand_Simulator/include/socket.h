#pragma once

// Windows header
#include <windows.h>

// Import Window Sockets headers
#include <winsock2.h>

// Import IP helper functions
#include <ws2tcpip.h>

// Basic C++ libraries
#include <string>
#include <iostream>

#define LOCALHOST_IP "127.0.0.1"

// Tell Mr.Compiler to include the ws2_32.lib file (winsock library)
#pragma comment(lib, "Ws2_32.lib")

class Socket
{
    int portNumber = 0;
    SOCKET ConnectSocket = INVALID_SOCKET;
    int resultCode = 0;

public:
    Socket(int portNumber);
    ~Socket();

    bool connect();
    bool receiveData(char *buffer, int length);
    void closeConnection();
};