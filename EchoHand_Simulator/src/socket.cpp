#include "socket.h"

Socket::Socket(int portNumber)
{
    this->portNumber = portNumber;
}

Socket::~Socket()
{
    closeConnection();
}

bool Socket::connect()
{
    // Constructor implementation (if needed)
    // Struct that holds Winsock data
    WSAData wsaData;

    // Error testing var
    int resultCode;

    // Initalize one socket to connect to Wokawi server
    this->ConnectSocket = INVALID_SOCKET;

    // Pointer that will contain address info from windows when asked
    struct addrinfo *result = NULL;

    // Data structure for our settings for the socket(empty)
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    // Buffer to store opengloves payload(256 bytes should be enough)
    char recvbuf[256];

    // Max buffer size for echohand
    int recvbuflen = 256;

    // Load newtwork driver DLL's into program memroy
    // Using version 2.2 as stated in demo code from microsoft
    resultCode = WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Check if the dll's were loaded, if not return failure
    if (resultCode != 0)
    {
        printf("WSA Startup failed with error : %d\n", resultCode);
        return false;
    }

    // IPV4
    hints.ai_family = AF_UNSPEC;

    // TCP streaming socket
    hints.ai_socktype = SOCK_STREAM;

    // Ensure using TCP not UDP
    hints.ai_protocol = IPPROTO_TCP;

    // Call windows to get address info and store it in result based off
    // the settings from hints
    resultCode = getaddrinfo(LOCALHOST_IP, std::to_string(portNumber).c_str(), &hints, &result);

    // Verify that getaddrinfo call was successful
    if (resultCode != 0)
    {
        printf("getaddrinfo failed with error: %d\n", resultCode);
        WSACleanup();
        return false;
    }

    // Try to attempt to connect an address until one is successful
    for (addrinfo *ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
        // Create a socket based off of the address info we got from getaddrinfo
        this->ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        // Check if socket creation was successful
        if (this->ConnectSocket == INVALID_SOCKET)
        {
            printf("Socket creation failed with error: %d\n", WSAGetLastError());
            WSACleanup();
            return false;
        }

        // Try to connect to the server with the created socket and address info
        resultCode = ::connect(this->ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);

        // If connection was successful break out of loop
        if (resultCode == SOCKET_ERROR)
        {
            // Close the socket since it failed to connect
            closesocket(this->ConnectSocket);
            this->ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    // After binding free memory of address info
    freeaddrinfo(result);

    // Check if connection was successful
    if (this->ConnectSocket == INVALID_SOCKET)
    {
        printf("Unable to connect to Echohand Simulation!\n");
        WSACleanup();
        return false;
    }

    // Let user know we connected to simulated echohand
    printf("Successfully connected to Wokwi Echo Hand Simulator on %s:%d\n", LOCALHOST_IP, portNumber);
    return true;
}

bool Socket::receiveData(char *buffer, int length)
{
    // Receive data from the client and place it in buffer
    int resultCode = recv(this->ConnectSocket, buffer, length - 1, 0);
    if (resultCode > 0)
    {
        // Successfully received bytes so print payload of ASCII
        // Null terminate the string by overiding the next byte after the received data
        // Since arrays starts at 0, this is where we made our space for our null-terminator
        buffer[resultCode] = '\0';
        return true;
    }
    else if (resultCode == 0)
    {
        printf("Connection closed\n");
        return false;
    }
    else
    {
        printf("recv failed with error: %d\n", WSAGetLastError());
        return false;
    }
}

void Socket::closeConnection()
{
    // Close socket and do WSA cleanup
    closesocket(this->ConnectSocket);
    WSACleanup();
}