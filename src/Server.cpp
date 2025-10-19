#include <Server.hpp>
#include <SocketUtils.hpp>
#include <iostream>
#include <unistd.h>
#include <cstring>


Server::Server(const std::string& port){
    this->port=port;
    serverSocketfd=-1;  

}
Server::~Server(){
    if(serverSocketfd!=-1){
        close(serverSocketfd);
    }
    std::cout<<"Server Shutdown Successfully\n";
}

void Server::initializeSocket(){
    struct addrinfo *res=resolveAddress(port);
    serverSocketfd=CreateSocket(res->ai_family,res->ai_socktype,res->ai_protocol);
    bindSocket(serverSocketfd,res);
    listenOnSocket(serverSocketfd,10); //backlog size 10(size of queue)

    freeaddrinfo(res);
}


void Server::acceptAndRespond(){
    std::cout << "Waiting for clients on port " << port << "...\n";
    while (true)
    { // temporary fix to avoid closing after one conection
        int client_fd = acceptClient();
        if (client_fd == -1)
            continue; // Try next client

        sendResponse(client_fd);
        closeConnection(client_fd);
    }

}

int Server::acceptClient(){
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    int client_fd = accept(serverSocketfd, (struct sockaddr*)&client_addr, &addr_size);
    if (client_fd == -1)
    {
        std::cerr << "Accept Error. errno: " << errno << "\n";
        std::cerr << "Error message : " << std::strerror(errno) << "\n";
    }else{

        std::cout << "Accepted a new client connection\n";
    }
    return client_fd;
}

// void Server::sendResponse(int client_fd) {
//     std::string greeting = "Hi Hello!\n";
//     send(client_fd, greeting.c_str(), greeting.size(), 0);
//     std::cout << "Sent greeting to client.\n";
// }
void Server::sendResponse(int client_fd) {
    char buffer[4096];
    std::string input;
    ssize_t bytes;
    // Read until newline or buffer becomes full
    while ((bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes] = '\0';
        input += buffer;
        // Stops reading if newline is found 
        if (input.find('\n') != std::string::npos) {
            break;
        }
    }
    if (bytes < 0) {
        std::cerr<<"Recv Error. errno: "<<errno<<"\n";
        std::cerr<<"Error message : "<<std::strerror(errno)<<"\n";
        return;
    }

    // Use the server’s MiniDB instance
    std::string response = database.executeCommand(input);

    send(client_fd, response.c_str(), response.size(), 0);
    std::cout<<"Sent response to client.\n";
}


void Server::closeConnection(int client_fd) {
    close(client_fd);
    std::cout << "Closed Client connection\n";
}