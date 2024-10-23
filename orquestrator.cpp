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

#define PORT 8000

void handleClient(int client_socket) {
    char buffer[1024] = {0};
    
    while (true) {
        memset(buffer, 0, sizeof(buffer)); // Clear buffer

        int bytes_read = read(client_socket, buffer, 1024); // Read from client
        
        if (bytes_read <= 0) {
            std::cerr << "Client disconnected." << std::endl << std::endl;
            close(client_socket);
            break;
        }

        std::cout << buffer << std::endl << std::endl; // Print received data

        json received_data = json::parse(buffer);

        if (received_data["device"] == "radio"){ // Save RSSI data to CSV
            saveToCsv(received_data["data"]);

        } else if (received_data["device"] == "camera") {

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
