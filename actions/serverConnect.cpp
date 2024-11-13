#include <iostream>
#include <arpa/inet.h>

#include "../client.h"


int connectWithServer(const char *server_ip, int port) {
    int sock = 0;
    struct sockaddr_in serv_addr;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return -1;
    }

    return sock;
}

void sendData (int socket, std::string data, std::string device_type) {
    // Get timestamp with ms precision
    const auto now = std::chrono::system_clock::now();
    const auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    uint8_t device_type_byte;
    if (device_type == "rss") {
        device_type_byte = 0x01;
    } else if (device_type == "rgb") {
        device_type_byte = 0x02;
    } else if (device_type == "depth") {
        device_type_byte = 0x03;
    } else {
        device_type_byte = 0xFF; // Unknown
    }

    // Prepare message structure
    uint32_t dataSize = htonl(data.size());
    uint64_t timestamp_net = htobe64(timestamp);

    // Create buffer to hold the entire message
    std::vector<char> message(sizeof(device_type_byte) + sizeof(dataSize) + sizeof(timestamp_net) + data.size());

    memcpy(message.data(), &device_type_byte, sizeof(device_type_byte)); // device type
    memcpy(message.data() + sizeof(device_type_byte), &dataSize, sizeof(dataSize)); // data size
    memcpy(message.data() + sizeof(device_type_byte) + sizeof(dataSize), &timestamp_net, sizeof(timestamp_net)); // timestamp
    memcpy(message.data() + sizeof(device_type_byte) + sizeof(dataSize) + sizeof(timestamp_net), data.c_str(), data.size()); // data

    // Send the complete message at once
    send(socket, message.data(), message.size(), 0);
}