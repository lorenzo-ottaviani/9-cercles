#include "LPTF_Socket_Server.hpp"

#include <iostream>
#include <vector>

#include <winsock2.h>

LPTF_Socket_Server::LPTF_Socket_Server() {
    m_listenSocket = initializeServer();
}

LPTF_Socket_Server::~LPTF_Socket_Server() {
    closeServer();
}

SOCKET LPTF_Socket_Server::getListenSocket() const {
    return m_listenSocket;
}

void LPTF_Socket_Server::setListenSocket(const SOCKET socket) {
    m_listenSocket = socket;
}

const std::vector<Client>& LPTF_Socket_Server::getClients() const {
    return m_clients;
}

void LPTF_Socket_Server::setClients(const std::vector<Client>& clients) {
    m_clients = clients;
}

SOCKET LPTF_Socket_Server::initializeServer() {
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

    // Define the requirements for our server
    hints.ai_family = AF_INET;       // IPv4 address family
    hints.ai_socktype = SOCK_STREAM; // Stream socket (TCP)
    hints.ai_protocol = IPPROTO_TCP; // TCP protocol
    hints.ai_flags = AI_PASSIVE;     // Passive mode

    // Resolve the Local Address and Port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);

    if (iResult != 0) {
        std::cerr << "getaddrinfo failed with error: " << iResult << '\n';
        WSACleanup();
        return INVALID_SOCKET;
    }

    //  Create a Socket
    SOCKET ListenSocket = INVALID_SOCKET;
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (ListenSocket == INVALID_SOCKET) {
        std::cerr << "Error at socket(): " << WSAGetLastError() << '\n';
        freeaddrinfo(result);
        WSACleanup();
        return INVALID_SOCKET;
    }

    // Bind the Socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);

    if (iResult == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << '\n';
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return INVALID_SOCKET;
    }

    freeaddrinfo(result);

    // Listen for incoming connections
    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << '\n';
        closesocket(ListenSocket);
        WSACleanup();
        return INVALID_SOCKET;
    }

    std::cout << "Server is now listening on port " << DEFAULT_PORT << "..." << std::endl;

    return ListenSocket;
}

void LPTF_Socket_Server::runServer() {

    if (m_listenSocket == INVALID_SOCKET) {
        std::cerr << "Cannot run server: Listen socket is invalid.\n";
        return;
    }

    // Client Management Setup
    int nextClientId = 1;
    sockaddr_in socketAddress;
    int AddressSize = sizeof(socketAddress);

    // Main Server Loop
    while (true) {

        fd_set readfds, writefds, exceptfds;
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_ZERO(&exceptfds);

        // Add the main listening socket to the read & exception sets.
        FD_SET(m_listenSocket, &readfds);
        FD_SET(m_listenSocket, &exceptfds);

        // Add all currently connected clients to the sets.
        for (const Client& client : m_clients)
        {
            FD_SET(client.socket, &readfds);
            FD_SET(client.socket, &writefds);
            FD_SET(client.socket, &exceptfds);
        }

        // The select() System Call
        int result = select(0, &readfds, &writefds, &exceptfds, NULL);

        if (result == SOCKET_ERROR) {
            std::cerr << "select() failed: " << WSAGetLastError() << '\n';
            break;
        }

        // Handle New Incoming Connections
        if (FD_ISSET(m_listenSocket, &readfds)) {

            // Retrieves the connection from the queue and creates a new dedicated socket.
            SOCKET ClientSocket = accept(m_listenSocket, (sockaddr*)&socketAddress, &AddressSize);


            if (ClientSocket == INVALID_SOCKET) {
                std::cerr << "accept failed: " << WSAGetLastError() << '\n';
            } else {
                int assignedId = nextClientId;
                nextClientId++; // Increment for the next connection

                // Send the ID to the client
                send(ClientSocket, (char*)&assignedId, sizeof(assignedId), 0);

                // Add the client structure to our list
                m_clients.push_back({ClientSocket, assignedId});
                std::cout << "Client " << assignedId << " connected and registered!\n";
            }

        }

        // Handle Existing Client Data / Messages
        char recvbuf[DEFAULT_BUFLEN];

        // Use a safe iterator loop to allow removing clients from the vector on disconnect
        for (auto it = m_clients.begin(); it != m_clients.end(); ) {

            // Check if this specific client has sent any data
            if (FD_ISSET(it->socket, &readfds)) {

                int iResult = recv(it->socket, recvbuf, DEFAULT_BUFLEN - 1, 0);

                // Case client sent some data and still connected
                if (iResult > 0) {
                    recvbuf[iResult] = '\0';
                    std::cout << "Client " << it->id << " sent this message: " << recvbuf << "\n";
                    ++it;
                }
                else {
                    // Case client has disconnected gracefully
                    if (iResult == 0) {
                        std::cout << "Client " << it->id << " disconnected gracefully.\n";
                    }

                    // Case an error occurred for this client socket
                    else {
                        std::cerr << "Receiving error from client " << it->id << ": " << WSAGetLastError() << '\n';
                    }

                    closesocket(it->socket);
                    it = m_clients.erase(it);
                }
            } else {
                ++it;
            }
        }
    }
}

void LPTF_Socket_Server::closeServer() {

    // Close all clients sockets
    for (const auto& client : m_clients) {
        closesocket(client.socket);
    }
    m_clients.clear();

    // Close the listenSocket
    if (m_listenSocket != INVALID_SOCKET) {
        closesocket(m_listenSocket);
        m_listenSocket = INVALID_SOCKET;
    }

    WSACleanup();
}
