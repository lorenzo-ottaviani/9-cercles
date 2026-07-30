#include <format>
#include <iostream>
#include <vector>
#include <string>

#include <winsock2.h>
#include <ws2tcpip.h>

#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512

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

    // ==========================================
    // 3. Resolve the Local Address and Port
    // ==========================================
    // We pass "127.0.0.1" (localhost) for the client, who needs a specific target IP address to connect to.
    // This function allocates memory and fills the 'result' structure with network data.
    iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        std::cerr << "getaddrinfo failed with error: " << iResult << '\n';
        WSACleanup();
        return 1;
    }

    // ==========================================
    // 4. Create a Socket
    // ==========================================
    // Initialize a SOCKET variable to an invalid state for safety.
    SOCKET ConnectSocket = INVALID_SOCKET;

    // Call the socket() system call to create the endpoint for communication.
    ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    // Check if the socket creation failed
    if (ConnectSocket == INVALID_SOCKET) {
        // WSAGetLastError() fetches the specific error code from Windows
        std::cerr << "Error at socket(): " << WSAGetLastError() << '\n';

        // Clean up everything before exiting
        freeaddrinfo(result);      // Free the getaddrinfo memory
        WSACleanup();              // Shut down Winsock
        return 1;
    }

    // ==========================================
    // 5. Connect the Socket
    // ==========================================
    // The connect() system call initiates the TCP 3-way handshake with the server.
    // It blocks until the server accepts the connection or a timeout occurs.
    iResult = connect( ConnectSocket, result->ai_addr, (int)result->ai_addrlen);

    if (iResult == SOCKET_ERROR) {

        // If connection failed, close the socket and reset it to invalid
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }

    // Once connect() is called, we no longer need the address info structure
    freeaddrinfo(result);

    // Verify if the connection was actually successful
    if (ConnectSocket == INVALID_SOCKET) {
        std::cerr << "Unable to connect to server! Error: " << WSAGetLastError() << '\n';
        WSACleanup();
        return 1;
    }

    std::cout << "Successfully connected to the server!" << std::endl;

    // ==========================================
    // 6 Receive assigned ID from Server
    // ==========================================
    int myClientId = 0;

    // We expect the server to send exactly 4 bytes (the size of an int) representing our ID
    iResult = recv(ConnectSocket, (char*)&myClientId, sizeof(myClientId), 0);

    if (iResult == 0) {
        std::cerr << "Failed to receive client ID from server!" << '\n';
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // ==========================================
    // 7. Send an Initial Buffer (Handshake)
    // ==========================================
    int recvbuflen = DEFAULT_BUFLEN;
    char recvbuf[DEFAULT_BUFLEN];

    // Build the dynamic string using std::format
    std::string message = std::format("Sending test! I am the client {}", myClientId);

    // We get the raw pointer (.c_str()) and its size from the std::string
    iResult = send(ConnectSocket, message.c_str(), (int)message.length(), 0);

    // Verify if the buffer was correctly sent
    if (iResult == SOCKET_ERROR) {
        std::cerr << "Sending test failed with error: " << WSAGetLastError() << '\n';
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // ==========================================
    // 8. Main Client Loop
    // ==========================================
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

    // Cleanup (Keep these ready for when you implement a loop exit condition)
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}