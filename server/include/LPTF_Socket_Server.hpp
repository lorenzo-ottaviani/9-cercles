#pragma once

#include <vector>

#include <winsock2.h>
#include <ws2tcpip.h>

struct Client {
    SOCKET socket;
    int id;
};

class LPTF_Socket_Server {
public:
    LPTF_Socket_Server();

    ~LPTF_Socket_Server();


    LPTF_Socket_Server(const LPTF_Socket_Server &) = delete;

    LPTF_Socket_Server &operator=(const LPTF_Socket_Server &) = delete;

    SOCKET getListenSocket() const;
    void setListenSocket(const SOCKET socket);

    const std::vector<Client> &getClients() const;
    void setClients(const std::vector<Client>& clients);

    void runServer();

    void closeServer();

private:
    static constexpr char DEFAULT_PORT[] = "27015";
    static constexpr int DEFAULT_BUFLEN = 512;

    SOCKET m_listenSocket = INVALID_SOCKET;
    std::vector<Client> m_clients;

    SOCKET initializeServer();
};
