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

json completeData (json data) {
    // Get timestamp with ms precision
    const auto now = std::chrono::system_clock::now();
    const auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    data["received_at"] = timestamp;

    return data;
}

void handleClient(int client_socket) {
    while (1) {
        // Read the header (1 byte for device type + 4 bytes for size + 8 bytes for timestamp)
        const size_t header_size = sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint64_t);
        std::vector<char> headerBuffer(header_size);

        // Read header first
        ssize_t bytesRead = readNBytes(client_socket, headerBuffer.data(), header_size);
        if (bytesRead <= 0) {
            std::cerr << "Client disconnected." << std::endl;
            close(client_socket);
            break;
        }

        // Extract device type, data size, and timestamp from header
        uint8_t deviceType;
        memcpy(&deviceType, headerBuffer.data(), sizeof(deviceType));

        uint32_t dataSize;
        memcpy(&dataSize, headerBuffer.data() + sizeof(deviceType), sizeof(dataSize));
        dataSize = ntohl(dataSize);

        uint64_t timestamp;
        memcpy(&timestamp, headerBuffer.data() + sizeof(deviceType) + sizeof(dataSize), sizeof(timestamp));
        timestamp = be64toh(timestamp);

        // Read the data based on dataSize
        std::vector<char> dataBuffer(dataSize + 1, 0);
        bytesRead = readNBytes(client_socket, dataBuffer.data(), dataSize);
        if (bytesRead <= 0) {
            std::cerr << "Failed to read data from client." << std::endl;
            close(client_socket);
            break;
        }

        try {
            // Parse the data as JSON
            json received_data;
            received_data["captured_at"] = timestamp;

            if (deviceType == 0x01) {
                received_data["data"] = json::parse(dataBuffer.data());
                json complete_data = completeData(received_data);

                std::cout << "Received RSS data." << std::endl;
                saveToCsv(complete_data);

            } else if (deviceType == 0x02 || deviceType == 0x03 || deviceType == 0x04 || deviceType == 0x06) { // If it is an image
                std::vector<unsigned char> img_data(dataBuffer.begin(), dataBuffer.end()); // Convert buffer to uchar
                received_data["data"] = img_data;
                received_data["type"] = deviceType;
                json complete_data = completeData(received_data);

                std::cout << "Received image data" << std::endl;
                saveToJpeg(complete_data);

            } else {
                std::cerr << "Unknown device type: " << static_cast<int>(deviceType) << std::endl;
            }

        } catch (const json::parse_error &e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
        }
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
