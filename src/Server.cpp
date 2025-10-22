#include <Server.hpp>
#include <SocketUtils.hpp>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/epoll.h>
#include <unordered_map>
#include <chrono>
#include "miniDBParser.hpp"
#include "TTLManager.hpp"


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
    int bytes;
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

bool Server::handleClientData(int client_fd, std::unordered_map<int, std::string>& clientBuffers){
    // Read any newly arrived bytes for this client.
    char buffer[4096];
    int bytes = recv(client_fd, buffer, sizeof(buffer), 0);
    // if all fine bytes is size of data

    if(bytes <= 0){ //-ve if any error occurs
        if(bytes == 0){
            std::cout<<"Client disconnected: "<<client_fd<<"\n";
        }else{
            std::cerr<<"Recv Error. errno: "<<errno<<"\n";
            std::cerr<<"Error message : "<<std::strerror(errno)<<"\n";
        }
        return false;
    }

    clientBuffers[client_fd].append(buffer, bytes);

    MiniDBParser parser;
    std::vector<std::string> parsed = parser.parse(clientBuffers[client_fd]);

    if(parsed.empty()){
        return true; // need more data
    }

    std::string response = database.executeCommand(clientBuffers[client_fd]);
    int sent = send(client_fd, response.c_str(), response.size(), 0);

    if(sent == -1){
        std::cerr<<"Send Error. errno: "<<errno<<"\n";
        std::cerr<<"Error message : "<<std::strerror(errno)<<"\n";
        return false;
    }

    std::cout<<"Sent response to client "<<client_fd<<"\n";
    clientBuffers[client_fd].clear();
    return true;
}

void Server::runWithEpoll(){
    std::cout<<"Starting server with epoll on port "<<port<<"...\n";
    
    int epoll_fd = epoll_create1(0);
    if(epoll_fd == -1){
        std::cerr<<"Epoll Create Error. errno: "<<errno<<"\n";
        std::cerr<<"Error message : "<<std::strerror(errno)<<"\n";
        return;
    }
    
    // Add server socket to epoll
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = serverSocketfd;
    
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverSocketfd, &event) == -1){
        std::cerr<<"Epoll CTL ADD Error. errno: "<<errno<<"\n";
        std::cerr<<"Error message : "<<std::strerror(errno)<<"\n";
        close(epoll_fd);
        return;
    }
    
    struct epoll_event events[10];
    std::unordered_map<int, std::string> clientBuffers;
    
    auto last_purge = std::chrono::steady_clock::now();
    const int purgeIntervalSeconds = 1;
    const int epollTimeoutMs = 500; // wake up every second even if no events
    
    std::cout<<"Waiting for clients...\n";
    
    while(true){
        int ready = epoll_wait(epoll_fd, events, 10, epollTimeoutMs);
        
        if(ready == -1){
            std::cerr<<"Epoll Wait Error. errno: "<<errno<<"\n";
            std::cerr<<"Error message : "<<std::strerror(errno)<<"\n";
            continue;
        }
        
        // Handle ready events (could be 0 if timeout)
        for(int i = 0; i < ready; i++){
            int fd = events[i].data.fd;
            
            if(fd == serverSocketfd){
                // new client connecting
                int client_fd = acceptClient();
                if(client_fd != -1){
                    struct epoll_event client_event;
                    client_event.events = EPOLLIN;
                    client_event.data.fd = client_fd;
                    
                    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1){
                        std::cerr<<"Epoll CTL ADD Client Error. errno: "<<errno<<"\n";
                        std::cerr<<"Error message : "<<std::strerror(errno)<<"\n";
                        close(client_fd);
                    }else{
                        clientBuffers[client_fd] = ""; // Initialize buffer
                        std::cout<<"Added client "<<client_fd<<" to epoll\n";
                    }
                }
            }
            else{
                // existing client sending data
                bool keepConnection = handleClientData(fd, clientBuffers);
                if(!keepConnection){
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                    close(fd);
                    clientBuffers.erase(fd);
                }
            }
        }
        
        // Periodic cleanup
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_purge).count();
        
        if (elapsed >= purgeIntervalSeconds) {
            database.purgeExpiredKeys();
            last_purge = now;
        }
    }
    
    close(epoll_fd);
}