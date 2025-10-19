#include "SocketUtils.hpp"
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <unistd.h>


struct addrinfo* resolveAddress(const std::string& port){
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints); 
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags=AI_PASSIVE;  // tells getaddressinfo to send address to bind to

    struct addrinfo *res;
    int status=getaddrinfo(nullptr,port.c_str(),&hints,&res);
    if(status!=0){
        std::cerr<<"getaddrinfo error "<<gai_strerror(status)<<std::endl;
        exit(1);
    }
    return res;

}

int CreateSocket(int family,int type,int protocol){ // family is ipv4 or ipv6     // type of socket req //protocol here is tcp/udp
    int socketfd=socket(family,type,protocol);
    if(socketfd==-1){
        std::cerr<<"Socket failed. errno: "<<errno<<"\n";
        std::cerr<<"Error message : "<<std::strerror(errno)<<"\n";
        exit(1);
    }
    // Allows to reuse port which is in time_wait state
    int yes = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        std::cerr << "setsockopt error: " << strerror(errno) << "\n";
        close(socketfd);
        exit(1);
    }

    return socketfd;

}

void bindSocket(int socketfd,struct addrinfo* addressinfo){
    if(bind(socketfd,addressinfo->ai_addr,addressinfo->ai_addrlen)==-1){
        std::cerr<<"Bind Error. errno: "<<errno<<"\n";
        std::cerr<<"Error message : "<<std::strerror(errno)<<"\n";
        close(socketfd);
        exit(1);

    }
}

void listenOnSocket(int socketfd,int backlog){ //backlog is basically like size waiting queue
    if(listen(socketfd,backlog)==-1){
        std::cerr<<"Listen Error. errno: "<<errno<<"\n";
        std::cerr<<"Error message : "<<std::strerror(errno)<<"\n";
        close(socketfd);
        exit(1);
        
    }

}