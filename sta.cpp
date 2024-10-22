#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdint>
#include <chrono>
#include <thread>

#include "sta.h"
#include "client.h"

#define SLEEP_TIME 200 // Time between messages (ms)

#define IP_SERVER "127.0.0.1"
#define PORT_SERVER 8000

#define IP_RADIO "200.239.93.46"
#define PORT_RADIO 8000

int main(int argc, char *argv[]) {

    try {
        char response[1024];

        int sock = connectWithServer(IP_SERVER, PORT_SERVER);
        int radio_sock = connectWithServer(IP_RADIO, PORT_RADIO);

        std::cout << "Succesfully connected to Orquestrator and Mikrotik" << std::endl << std::endl;

        std::string currBeamRSS;
        char buffer[1024] = {0};

        while(1){
            // Get radio data
            currBeamRSS = getPerBeamRSS(radio_sock);
            strcpy(response, currBeamRSS.c_str());

            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME)); // Wait to send more data

            send(sock, response, strlen(response), 0); // Send message to server

            // read(sock, buffer, 1024); // Read response from server
            // std::cout << buffer << std::endl;
        }
        
        close(sock);
        close(radio_sock);
        std::cout << std::endl << "Connections closed";
        
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}