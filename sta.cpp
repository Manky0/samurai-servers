#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "radioFunctions.h"

#define PORT 8000 // Use unique port for each server

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char response[1024];  // Change to mutable response
    int opt = 1;

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return -1;
    }

    // Set SO_REUSEADDR to allow reuse of the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::cerr << "Set socket options failed" << std::endl;
        close(server_fd);
        return -1;
    }

    // Define the server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the network address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        close(server_fd);
        return -1;
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed" << std::endl;
        close(server_fd);
        return -1;
    }

    std::cout << "Server started on port " << PORT << std::endl;

    // Infinite loop to continuously accept client connections
    while (true) {
        // Accept incoming connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue; // Skip this iteration and wait for the next connection
        }

        // Read message from client
        memset(buffer, 0, sizeof(buffer));  // Clear buffer before reading
        std::string currBeamRSS;

        int bytes_read = read(new_socket, buffer, 1024);
        if (bytes_read > 0) {
            // Compare received message and respond accordingly
            if (strcmp(buffer, "start_capture") == 0) {
                currBeamRSS = getPerBeamRSS();
                strcpy(response, currBeamRSS.c_str());
            } else {
                strcpy(response, "Unknown message");
            }

            // Send response to client
            send(new_socket, response, strlen(response), 0);
            std::cout << "Response sent to client: " << response << std::endl;
        } else {
            std::cerr << "Failed to read from client" << std::endl;
        }

        // Close client socket after communication is done
        close(new_socket);
        std::cout << "\n";
    }

    // Close server socket (though this line will never be reached in the infinite loop)
    close(server_fd);
    return 0;
}
