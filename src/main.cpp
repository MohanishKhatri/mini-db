#include "Server.hpp"

int main() {
    // Create server on port 8080
    Server Server("8080");

    // Initialize the socket
    Server.initializeSocket();

    // Start accepting clients
    Server.runWithEpoll();
    // Server.acceptAndRespond();


    return 0;
}
