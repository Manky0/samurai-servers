#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdint>

enum Command : uint8_t {
    START_MEASUREMENTS = 0x01,
    CAPTURE_IMAGE = 0x02,
    MOVE_FORWARD = 0x0E,
    MOVE_BACKWARD = 0x0F,
};

// Temp while we use in the same computer

#define IP_STA "127.0.0.1"
#define PORT_STA 8000

#define IP_AP "127.0.0.1"
#define PORT_AP 8001

#define IP_CAM "127.0.0.1"
#define PORT_CAM 8002

Command getCommandFromArg(const std::string &arg) {
    if (arg == "rss") {
        return START_MEASUREMENTS;
    } else if (arg == "cam") {
        return CAPTURE_IMAGE;
    } else if (arg == "movef") {
        return MOVE_FORWARD;
    } else if (arg == "moveb") {
        return MOVE_BACKWARD;
    } else {
        throw std::invalid_argument("Invalid command argument");
    }
}

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

    // Read response from server
    read(sock, buffer, 10240);
    std::cout << buffer << std::endl;

    close(sock);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <command> <times>\n";
        std::cerr << "Commands: rss, cam, movef, moveb\n";
        return -1;
    }

    try {
        int counter = 0;
        Command cmd = getCommandFromArg(argv[1]);

        while(counter < std::stoi(argv[2])){
            communicateWithServer(IP_STA, PORT_STA, cmd);
            counter++;
        }
        
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}