// client.cpp
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define IP_STA "127.0.0.1"
#define PORT_STA 8000

#define IP_AP "127.0.0.1"
#define PORT_AP 8001

#define IP_CAM "127.0.0.1"
#define PORT_CAM 8002

void communicateWithServer(const char *server_ip, int port, const char *message) {
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
    send(sock, message, strlen(message), 0);
    std::cout << "Message sent to server on port " << port << std::endl;

    // Read response from server
    read(sock, buffer, 10240);
    std::cout << "Response from server: " << buffer << std::endl;

    // Close the socket
    close(sock);
}

int main() {
// Communicate with server 1

    int counter = 0;
    while(counter < 5){
        communicateWithServer(IP_STA, PORT_STA, "start_capture");
        counter++;
    }


// Communicate with server 2
// communicateWithServer(IP_AP, PORT_AP);

// // Communicate with server 3
// communicateWithServer(IP_CAM, PORT_CAM);

return 0;
}