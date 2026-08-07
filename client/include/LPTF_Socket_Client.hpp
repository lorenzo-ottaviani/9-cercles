#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>

class LPTF_Socket_Client {
public:
    LPTF_Socket_Client();

    ~LPTF_Socket_Client();

    LPTF_Socket_Client(const LPTF_Socket_Client &) = delete;

    LPTF_Socket_Client &operator=(const LPTF_Socket_Client &) = delete;

    SOCKET getConnectSocket() const;
    void setConnectSocket(const SOCKET socket);

    int getId() const;
    void setId(const int id);

    void runClient();

    void closeClient();

private:
    static constexpr char DEFAULT_SERVER_IP[] = "127.0.0.1";
    static constexpr char DEFAULT_PORT[] = "27015";
    static constexpr int DEFAULT_BUFLEN = 512;

    SOCKET m_connectSocket = INVALID_SOCKET;
    int m_id = -1;

    SOCKET initializeClient();
};
