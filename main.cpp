#include <iostream>
#include <vector>

#include <winsock2.h>
#include <ws2tcpip.h>

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

struct Client {
    SOCKET socket;
    int id;
};

int main(){
    WSADATA wsaData;

    // ==========================================
    // 1. Initialize Winsock
    // ==========================================
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }

    // ==========================================
    // 2. Setup Address Information (Hints)
    // ==========================================
    struct addrinfo *result = NULL;
    struct addrinfo *ptr = NULL;
    struct addrinfo hints;

    // Clear the memory of the hints structure to avoid garbage values
    ZeroMemory(&hints, sizeof(hints));

    // Define the requirements for our server
    hints.ai_family = AF_INET;       // IPv4 address family
    hints.ai_socktype = SOCK_STREAM; // Stream socket (TCP)
    hints.ai_protocol = IPPROTO_TCP; // TCP protocol
    hints.ai_flags = AI_PASSIVE;     // Passive mode (indicates this socket will listen for connections)


    // ==========================================
    // 3. Resolve the Local Address and Port
    // ==========================================
    // We pass NULL for the IP because AI_PASSIVE tells Windows to use our local IP.
    // This function allocates memory and fills the 'result' structure with network data.
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        std::cerr << "getaddrinfo failed with error: " << iResult << '\n';
        WSACleanup();
        return 1;
    }

    // ==========================================
    // 4. Create a Socket
    // ==========================================
    // Initialize a SOCKET variable to an invalid state for safety.
    SOCKET ListenSocket = INVALID_SOCKET;

    // Call the socket() system call to create the endpoint for communication.
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    // Check if the socket creation failed
    if (ListenSocket == INVALID_SOCKET) {
        // WSAGetLastError() fetches the specific error code from Windows
        std::cerr << "Error at socket(): " << WSAGetLastError() << '\n';

        // Clean up everything before exiting
        freeaddrinfo(result);      // Free the getaddrinfo memory
        WSACleanup();              // Shut down Winsock
        return 1;
    }

    // ==========================================
    // 5. Bind the Socket
    // ==========================================
    // Associate the socket with the specific local IP address and port (27015).
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);

    // Check if the bind failed
    if (iResult == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << '\n';

        // Clean up everything before exiting
        freeaddrinfo(result);      // Free the getaddrinfo memory
        closesocket(ListenSocket); // Close the socket we just created
        WSACleanup();              // Shut down Winsock
        return 1;
    }

    // ==========================================
    // 6. Free Address Information
    // ==========================================
    // We don't need the 'result' structure anymore because the socket is now bound.
    freeaddrinfo(result);


    // ==========================================
    // 7. Listen
    // ==========================================
    // Place the socket in a state where it is listening for incoming connections.
    // SOMAXCONN is a special constant that tells Winsock to allow a maximum
    // reasonable number of pending connections in the queue.
    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << '\n';

        // Clean up everything before exiting
        closesocket(ListenSocket); // Close the socket
        WSACleanup();              // Shut down Winsock
        return 1;
    }

    std::cout << "Server is now listening on port " << DEFAULT_PORT << "..." << std::endl;

    // ==========================================
    // 8. Client Management Setup
    // ==========================================
    // Dynamic array (vector) to keep track of all active client sockets.
    std::vector<Client> clients;
    int nextClientId = 1; // The counter that determines the next ID

    // Structure to store the network address of incoming clients.
    sockaddr_in socketAddress;
    int AddressSize = sizeof(socketAddress);

    // ==========================================
    // 9. Main Server Loop (Single-Threaded Multiplexing)
    // ==========================================
    while (true) {
        // fd_set structures are bit arrays used to group sockets for monitoring.
        fd_set readfds;   // Sockets ready to read data (or new incoming connections)
        fd_set writefds;  // Sockets ready to write data without blocking
        fd_set exceptfds; // Sockets with exceptional/error conditions

        // Clear all the sets. This is mandatory because they contain garbage values.
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_ZERO(&exceptfds);

        // Add the main listening socket to the read & exception sets.
        // If ListenSocket triggers 'readfds', it means a new client wants to connect.
        FD_SET(ListenSocket, &readfds);
        FD_SET(ListenSocket, &exceptfds);

        // Add all currently connected clients to the sets.
        for (const Client& client : clients)
        {
            FD_SET(client.socket, &readfds);
            FD_SET(client.socket, &writefds);
            FD_SET(client.socket, &exceptfds);
        }

        // ==========================================
        // 10. The select() System Call
        // ==========================================
        // This is the core multiplexing call. It blocks the program execution
        // until at least one socket in any set changes its state (receives data, connects, etc.).
        // The first argument is ignored on Windows (usually 0), and the last is NULL (no timeout).
        int result = select(0, &readfds, &writefds, &exceptfds, NULL);

        if (result == SOCKET_ERROR) {
            std::cerr << "select() failed: " << WSAGetLastError() << '\n';
            // Note: In a production server, we would handle this error and exit/cleanup.
        }

        // ==========================================
        // 11. Handle New Incoming Connections
        // ==========================================
        // Check if our listening socket is flagged as "ready to read".
        if (FD_ISSET(ListenSocket, &readfds)) {

            // accept() retrieves the connection from the queue and creates a new dedicated socket.
            SOCKET ClientSocket = accept(ListenSocket, (sockaddr*)&socketAddress, &AddressSize);


            if (ClientSocket == INVALID_SOCKET) {
                std::cerr << "accept failed: " << WSAGetLastError() << '\n';
            } else {
                int assignedId = nextClientId;
                nextClientId++; // Increment for the next connection

                // Send the ID to the client in binary format (4 bytes)
                // We cast the address of our int to a char* so send() can process it
                send(ClientSocket, (char*)&assignedId, sizeof(assignedId), 0);

                // Add the client structure to our list
                clients.push_back({ClientSocket, assignedId});
                std::cout << "Client " << assignedId << " connected and registered!\n";
            }

        }

        // ==========================================
        // 12. Handle Existing Client Data / Messages
        // ==========================================
        char recvbuf[DEFAULT_BUFLEN];

        // Use a safe iterator loop to allow removing clients from the vector on disconnect
        for (auto it = clients.begin(); it != clients.end(); ) {

            // Check if this specific client has sent any data
            if (FD_ISSET(it->socket, &readfds)) {

                // Leave 1 byte at the end of the buffer for the null-terminator safely
                iResult = recv(it->socket, recvbuf, DEFAULT_BUFLEN - 1, 0);

                if (iResult > 0) {
                    // Secure the buffer by adding a null-terminator to safely print it as a C-string
                    recvbuf[iResult] = '\0';
                    std::cout << "Client " << it->id << " sent this message: " << recvbuf << "\n";
                    ++it; // Move to the next client
                }
                else {
                    // iResult == 0 means graceful disconnect, < 0 means a socket error
                    if (iResult == 0) {
                        std::cout << "Client " << it->id << " disconnected gracefully.\n";
                    } else {
                        std::cerr << "Receiving error from client " << it->id << ": " << WSAGetLastError() << '\n';
                    }

                    // Clean up the disconnected socket and remove it from our active vector
                    closesocket(it->socket);
                    it = clients.erase(it); // erase() returns the next valid iterator, no manual increment needed
                }
            } else {
                ++it; // This client didn't send anything, move to the next one
            }
        }
    }

    closesocket(ListenSocket);
    WSACleanup();
    return 0;
}
