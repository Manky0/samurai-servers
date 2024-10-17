// client.cpp
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdint>

enum Command : uint8_t {
    START_MEASUREMENTS = 0x01,
    CAPTURE_IMAGE = 0x02,
    MOVE_FOWARD = 0x0E,
    MOVE_BACKWARD = 0x0F,
};

#define IP_STA "127.0.0.1"
#define PORT_STA 8000

#define IP_AP "127.0.0.1"
#define PORT_AP 8001

#define IP_CAM "127.0.0.1"
#define PORT_CAM 8002

void communicateWithServer(const char *server_ip, int port, Command cmd) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[10240] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    std::cerr << "Socket creation error" << std::endl;
    return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
    std::cerr << "Invalid address/ Address not supported" << std::endl;
    return;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    std::cerr << "Connection Failed" << std::endl;
    return;
    }

    // Send message to server
    send(sock, &cmd, sizeof(cmd), 0);
    std::cout << "Message sent to server on port " << port << std::endl;

    // Read response from server
    read(sock, buffer, 10240);
    std::cout << "Response from server: " << buffer << std::endl;

    // Close the socket
    close(sock);
}

int main() {
    int counter = 0;
    while(counter < 1){
        communicateWithServer(IP_STA, PORT_STA, START_MEASUREMENTS);
        counter++;
    }
return 0;
}