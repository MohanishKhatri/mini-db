#pragma once

#include <string>
#include <unordered_map>
#include <SocketUtils.hpp>
#include <miniDB.hpp>

class Server
{
public:
    Server(const std::string &port);
    ~Server();

    void initializeSocket();
    void acceptAndRespond();
    void runWithEpoll();

private:
    int serverSocketfd;
    std::string port;
    MiniDB database;

    int acceptClient();
    void sendResponse(int client_fd);
    void closeConnection(int client_fd);
    // Returns false when the client should be removed from epoll.
    bool handleClientData(int client_fd, std::unordered_map<int, std::string>& clientBuffers);
};
