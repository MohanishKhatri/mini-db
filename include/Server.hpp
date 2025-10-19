#pragma once

#include <string>
#include <SocketUtils.hpp>

class Server
{
public:
    Server(const std::string &port);
    ~Server();

    void initializeSocket();
    void acceptAndRespond();

private:
    int serverSocketfd;
    std::string port;

    int acceptClient();
    void sendResponse(int client_fd);
    void closeConnection(int client_fd);
};
