#pragma once

#include <string>
#include <netdb.h>

// Resolves the given port for server use not reallly necessary but its makes things simpler.
// Can also be used for dns lookup with slight modification
struct addrinfo* resolveAddress(const std::string& port);

// Creates a socket using address family, type, and protocol.
int CreateSocket(int family, int type, int protocol);

// Binds the socket to the address info.
void bindSocket(int sockfd, struct addrinfo* addr);

// Makes the socket listen for incoming connections.
void listenOnSocket(int sockfd, int backlog);
