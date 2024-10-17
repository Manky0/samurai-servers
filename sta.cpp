#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdint>

#include "sta.h"

enum Command : uint8_t {
    START_MEASUREMENTS = 0x01,
    CAPTURE_IMAGE = 0x02,
    MOVE_FOWARD = 0x0E,
    MOVE_BACKWARD = 0x0F,
};

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

    while (true) {
        // Accept incoming connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue; // Skip this iteration and wait for the next connection
        }

        memset(buffer, 0, sizeof(buffer));  // Clear buffer before reading
        std::string currBeamRSS;

        int bytes_read = read(new_socket, buffer, 1024);
        if (bytes_read > 0) {
            switch (bytes_read)
            {
            case START_MEASUREMENTS:
                currBeamRSS = getPerBeamRSS();
                strcpy(response, currBeamRSS.c_str());
                break;
    
            default:
                break;
            }
            // // Compare received message and respond accordingly
            // if (strcmp(buffer, "start_capture") == 0) {
            //     currBeamRSS = getPerBeamRSS();
            //     strcpy(response, currBeamRSS.c_str());
            // } else {
            //     strcpy(response, "Unknown message");
            // }

            // // Send response to client
            // send(new_socket, response, strlen(response), 0);
            // std::cout << "Response sent to client: " << response << std::endl;
        } else {
            std::cerr << "Failed to read from client" << std::endl;
        }

        // Close client socket after communication is done
        close(new_socket);
        std::cout << "\n";
    }

    close(server_fd);
    return 0;
}
