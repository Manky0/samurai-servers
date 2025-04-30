#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <mutex>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "orchestrator.h"

std::string session_dir;

#define CAPTURE_INTERVAL 200 // Interval time between messages (ms)
#define PORT 3990

std::mutex clients_mutex;
std::vector<int> client_sockets;

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
    // Add client to list
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        client_sockets.push_back(client_socket);
    }

    
    while (1) {
        // Read the header (1 byte for device type + 4 bytes for size + 8 bytes for timestamp)
        const size_t header_size = sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint64_t);
        std::vector<char> headerBuffer(header_size);

        // Read header first
        ssize_t bytesRead = readNBytes(client_socket, headerBuffer.data(), header_size);
        if (bytesRead <= 0) {
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                client_sockets.erase(std::remove(client_sockets.begin(), client_sockets.end(), client_socket), client_sockets.end());
            }   
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

        uint64_t captured_at;
        memcpy(&captured_at, headerBuffer.data() + sizeof(deviceType) + sizeof(dataSize), sizeof(captured_at));
        captured_at = be64toh(captured_at);

        // Read the data based on dataSize
        std::vector<char> dataBuffer(dataSize + 1, 0);
        bytesRead = readNBytes(client_socket, dataBuffer.data(), dataSize);
        if (bytesRead <= 0) {
            std::cerr << "Failed to read data from client." << std::endl;
            close(client_socket);
            break;
        }

        try {
            const auto now = std::chrono::system_clock::now();
            const auto received_at = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

            if (deviceType == 0x01) {
                json received_data = json::parse(dataBuffer.data()); // Parse the data as JSON

                std::cout << "Received RSS data." << std::endl;
                saveToCsv(received_data, captured_at, received_at);

            } else if (deviceType == 0x02 || deviceType == 0x03 || deviceType == 0x04 || deviceType == 0x06) { // If it is an image
                std::vector<unsigned char> img_data(dataBuffer.begin(), dataBuffer.end()); // Convert buffer to uchar

                std::cout << "Received image data" << std::endl;
                saveToJpeg(img_data, deviceType, captured_at, received_at);

            } else {
                std::cerr << "Unknown device type: " << static_cast<int>(deviceType) << std::endl;
            }

        } catch (const json::parse_error &e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
        }
    }

    
}

void sendToAllClients(const std::string& message) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (int client_socket : client_sockets) {
        send(client_socket, message.c_str(), message.size(), 0);
    }
}

void controlServer() {
    std::cout << "When all clients are connected, type how many measurements you want." << std::endl;

    while (true) {
        // ############### CLIENT SLEEP ###############
        // std::string command;
        // std::cin >> command;

        // json start_message = {{"command", command}};
        // sendToAllClients(start_message.dump()); 

        // ############### ORCHESTRATOR SLEEP ###############
        int n;
        std::cin >> n;

        int folder_count = 0;
        for (const auto& entry : std::filesystem::directory_iterator("./")) {
            if (entry.is_directory()) {
                folder_count++;
            }
        }

        // Set the session directory name based on the number of folders
        std::ostringstream ss;
        ss << std::setw(3) << std::setfill('0') << folder_count + 1;
        session_dir = "./" + ss.str() + "/";

        std::filesystem::create_directories(session_dir);

        // Set interval timer
        auto start_time = std::chrono::steady_clock::now();
        auto wait_time = std::chrono::milliseconds{CAPTURE_INTERVAL};
        auto next_time = start_time + wait_time;

        for (int i = 0; i < n; i++)
        {
            const auto now = std::chrono::system_clock::now();
            const auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
            std::cout << "Capture command sent at " << timestamp << std::endl;

            std::string capture_msg = "1\n";
            sendToAllClients(capture_msg);


            std::this_thread::sleep_until(next_time);
            next_time += wait_time; // increment absolute time
        }
          
    }
}

int main() {
    int server_fd, client_socket;
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

    // Thread for manual commands
    std::thread control_thread(controlServer);

    // Accept multiple clients
    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }
        std::cout << "Client connected" << std::endl;

        std::thread(handleClient, client_socket).detach();
    }

    control_thread.join();
    close(server_fd);
    return 0;
}
