#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdint>
#include <chrono>
#include <thread>
#include <opencv2/opencv.hpp>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "sta.h"
#include "client.h"

#define SLEEP_TIME 200 // Interval time between messages (ms)

#define IP_SERVER "127.0.0.1"
// #define IP_SERVER "200.239.93.192" // Orquestrator
#define PORT_SERVER 3990

#define IP_RADIO "200.239.93.46" // Mikrotik
#define PORT_RADIO 8000


void sendData (int socket, std::string data, std::string device_type) {
    // Get timestamp with ms precision
    const auto now = std::chrono::system_clock::now();
    const auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    json data_json;
    data_json["captured_at"] = timestamp;
    data_json["device"] = device_type;
    data_json["data"] = json::parse(data.data());

    std::string data_send = data_json.dump();
    // std::cout << data_send << std::endl;

    uint32_t dataSize = htonl(data_send.size());
    send(socket, &dataSize, sizeof(dataSize), 0); // Send data size

    send(socket, data_send.c_str(), data_send.size(), 0); // Send data
}

int main(int argc, char *argv[]) {

    try {
        // Connect with servers
        int orq_sock = connectWithServer(IP_SERVER, PORT_SERVER);
        int radio_sock = connectWithServer(IP_RADIO, PORT_RADIO);

        std::cout << "Succesfully connected to Orquestrator and Mikrotik" << std::endl << std::endl;

        // Connect to camera and set resolution
        cv::VideoCapture cap(-1);
        if (!cap.isOpened()) {
            std::cerr << "Error: Could not open camera" << std::endl;
            return -1;
        }
        cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

        std::string currBeamRSS;

        while(1){
            // Get radio data
            currBeamRSS = getPerBeamRSS(radio_sock);
            sendData(orq_sock, currBeamRSS, "radio"); // Send rss data to server

            // Get camera frame
            std::vector<uchar> frame = getCamFrame(cap);

            // TODO: find a better way to parse from vector to string
            json frameJSON;
            frameJSON["frame"] = frame;

            sendData(orq_sock, frameJSON["frame"].dump(), "cam"); // Send cam data to server

            std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME)); // Wait to send more data

            // read(orq_sock, buffer, 1024); // Read response from server
            // std::cout << buffer << std::endl;
        }
        
        close(orq_sock);
        close(radio_sock);
        std::cout << std::endl << "Connections closed";
        
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}