#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "orquestrator.h"

#define PORT 3990

ssize_t readNBytes(int sock, char *buffer, size_t n) {
    size_t bytesRead = 0;
    while (bytesRead < n) {
        ssize_t result = read(sock, buffer + bytesRead, n - bytesRead);
        if (result <= 0) return result;
        bytesRead += result;
    }
    return bytesRead;
}

void handleClient(int client_socket) {
    char sizeBuffer[sizeof(int32_t)];
    
    while (1) {
        // Read the size of the incoming data
        int bytesRead = readNBytes(client_socket, sizeBuffer, sizeof(int32_t));
        if (bytesRead <= 0) {
            std::cerr << "Client disconnected." << std::endl << std::endl;
            close(client_socket);
            break;
        }

        // Convert size buffer to integer
        int32_t dataSize;
        memcpy(&dataSize, sizeBuffer, sizeof(int32_t));
        dataSize = ntohl(dataSize); // Network to host byte order

        // Read the data based
        std::vector<char> buffer(dataSize + 1, 0); // Allocate buffer for data
        bytesRead = readNBytes(client_socket, buffer.data(), dataSize);
        if (bytesRead <= 0) {
            std::cerr << "Failed to read data from client." << std::endl;
            close(client_socket);
            break;
        }

        buffer[dataSize] = '\0'; // null-terminated for safe handling

        // std::cout << buffer.data() << std::endl << std::endl; // Print received data

        try {
            json received_data = json::parse(buffer.data());
            // Save RSSI data to CSV
            if (received_data["device"] == "radio") {
                std::cout << "Received RSS data." << std::endl;
                saveToCsv(received_data["data"]);

            // Save image
            } else if (received_data["device"] == "cam") {
                std::cout << "Received cam data." << std::endl;
                saveToJpeg(received_data["data"]);

            }
        } catch (const json::parse_error &e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
        }
    
        // Send response to client
        // const char* response = "Message received";
        // send(client_socket, response, strlen(response), 0);
    }
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int opt = 1;

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return -1;
    }

    // Allow reuse of the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::cerr << "Set socket options failed" << std::endl;
        close(server_fd);
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to ip and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 10) < 0) {
        std::cerr << "Listen failed" << std::endl;
        close(server_fd);
        return -1;
    }

    std::cout << "Server started on port " << PORT << std::endl;

    std::vector<std::thread> threads;

    // Accept multiple clients
    while (true) {
        int client_socket;
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }

        std::cout << "Client connected" << std::endl << std::endl;

        // Create a new thread for each client
        threads.push_back(std::thread(handleClient, client_socket));
        threads.back().detach();  // Detach the thread so it runs independently
    }

    close(server_fd);
    return 0;
}
