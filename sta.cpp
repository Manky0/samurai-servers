#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "sta.h"

#define PORT 8000

enum Command : uint8_t {
    START_MEASUREMENTS = 0x01,
    CAPTURE_IMAGE = 0x02,
    MOVE_FORWARD = 0x0E,
    MOVE_BACKWARD = 0x0F,
};

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    uint8_t command;
    char response[1024];
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

        std::string currBeamRSS;

        // Read the command (1 byte) from the client
        int bytes_read = read(new_socket, &command, sizeof(command));

        if (bytes_read > 0) {
            switch (command) {
                case START_MEASUREMENTS:
                    currBeamRSS = getPerBeamRSS();
                    strcpy(response, currBeamRSS.c_str());
                    break;

                case CAPTURE_IMAGE:
                    strcpy(response, "Capturing image");
                    break;

                case MOVE_FORWARD:
                    strcpy(response, "Moving forward");
                    break;

                case MOVE_BACKWARD:
                    strcpy(response, "Moving backward");
                    break;

                default:
                    strcpy(response, "Unknown command");
                    break;
            }

            send(new_socket, response, strlen(response), 0);
            // std::cout << "Response sent to client: " << response << std::endl;
        } else {
            std::cerr << "Failed to read from client" << std::endl;
        }

        close(new_socket);
        std::cout << "\n";
    }

    close(server_fd);
    return 0;
}
