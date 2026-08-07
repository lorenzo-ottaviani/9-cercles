#include "LPTF_Socket_Client.hpp"

#include <format>
#include <iostream>
#include <string>
#include <vector>

#include <winsock2.h>

LPTF_Socket_Client::LPTF_Socket_Client() : m_connectSocket(INVALID_SOCKET), m_id(-1) {
    m_connectSocket = initializeClient();
}

LPTF_Socket_Client::~LPTF_Socket_Client() {
    closeClient();
}

SOCKET LPTF_Socket_Client::getConnectSocket() const {
    return m_connectSocket;
}

void LPTF_Socket_Client::setConnectSocket(const SOCKET socket) {
    m_connectSocket = socket;
}

int LPTF_Socket_Client::getId() const {
    return m_id;
}

void LPTF_Socket_Client::setId(const int id) {
    m_id = id;
}

SOCKET LPTF_Socket_Client::initializeClient() {
    WSADATA wsaData;

    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return INVALID_SOCKET;
    }

    struct addrinfo *result = NULL;
    struct addrinfo *ptr = NULL;
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));

    // Define the requirements for our client
    hints.ai_family = AF_INET;       // IPv4 address family
    hints.ai_socktype = SOCK_STREAM; // Stream socket (TCP)
    hints.ai_protocol = IPPROTO_TCP; // TCP protocol

    // Resolve the Local Address and Port
    iResult = getaddrinfo(DEFAULT_SERVER_IP, DEFAULT_PORT, &hints, &result);

    if (iResult != 0) {
        std::cerr << "getaddrinfo failed with error: " << iResult << '\n';
        WSACleanup();
        return INVALID_SOCKET;
    }

    //  Create a Socket
    SOCKET ConnectSocket = INVALID_SOCKET;
    ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (ConnectSocket == INVALID_SOCKET) {
        std::cerr << "Error at socket(): " << WSAGetLastError() << '\n';
        freeaddrinfo(result);
        WSACleanup();
        return INVALID_SOCKET;
    }

    // Connect the Socket
    iResult = connect( ConnectSocket, result->ai_addr, (int)result->ai_addrlen);

    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }

    freeaddrinfo(result);

    // Verify if the connection was actually successful
    if (ConnectSocket == INVALID_SOCKET) {
        std::cerr << "Unable to connect to server! Error: " << WSAGetLastError() << '\n';
        WSACleanup();
        return INVALID_SOCKET;
    }

    std::cout << "Successfully connected to the server!" << std::endl;

    return ConnectSocket;
}

void LPTF_Socket_Client::runClient() {
    if (m_connectSocket == INVALID_SOCKET) {
        std::cerr << "Cannot run client: Connection socket is invalid.\n";
        return;
    }

    // Receive assigned ID from Server
    int myClientId = 0;

    int iResult = recv(m_connectSocket, reinterpret_cast<char*>(&myClientId), sizeof(myClientId), 0);

    if (iResult <= 0) {
        std::cerr << "Failed to receive client ID from server!" << '\n';
        closeClient();
        return;
    }

    m_id = myClientId;

    //  Send an Initial Buffer (Handshake)
    const int recvbuflen = DEFAULT_BUFLEN;
    char recvbuf[DEFAULT_BUFLEN];

    const std::string message = std::format("Sending test! I am the client {}", m_id);

    iResult = send(m_connectSocket, message.c_str(), static_cast<int>(message.length()), 0);

    // Verify if the buffer was correctly sent
    if (iResult == SOCKET_ERROR) {
        std::cerr << "Sending test failed with error: " << WSAGetLastError() << '\n';
        closeClient();
        return;
    }

    // Main Client Loop
    while (true) {
        // --- Step A: Action Input / Action Skip ---
        // TODO: Implement temporary user input via std::cin or hit space/enter to skip.
        // Later on, this section will silently query your application's state
        // (such as player movements, keystrokes, or actions) instead of blocking the console.

        // --- Step B: Silent Reception (recv) ---
        // TODO: Implement the background recv() call to grab server instructions.
        // Remember: Since this client is "silent" (sourde), we will parse these bytes internally
        // to update the client's local memory, without printing anything to the console.
    }
}

void LPTF_Socket_Client::closeClient() {

    if (m_connectSocket != INVALID_SOCKET) {
        closesocket(m_connectSocket);
        m_connectSocket = INVALID_SOCKET;
    }

    WSACleanup();
}
