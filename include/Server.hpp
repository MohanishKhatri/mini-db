#pragma once

#include <string>
#include <SocketUtils.hpp>
#include <miniDB.hpp>

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
    MiniDB database;

    int acceptClient();
    void sendResponse(int client_fd);
    void closeConnection(int client_fd);
};
