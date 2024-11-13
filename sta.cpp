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
#include <librealsense2/rs.hpp> 

using json = nlohmann::json;

#include "sta.h"
#include "client.h"

#define CAPTURE_INTERVAL 200 // Interval time between messages (ms)

#define IP_SERVER "127.0.0.1"
// #define IP_SERVER "200.239.93.191" // Orquestrator
#define PORT_SERVER 3990

#define IP_RADIO "200.239.93.45" // Mikrotik
#define PORT_RADIO 8000


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

int main(int argc, char *argv[]) {

    try {
        // Connect with orquestrator
        int orq_sock = connectWithServer(IP_SERVER, PORT_SERVER);
        if(orq_sock == -1){
            std::cerr << "Error: Could not connect to server at " << IP_SERVER << std::endl;
            return -1;
        }

        // Connect with STA radio
        int radio_sock = connectWithServer(IP_RADIO, PORT_RADIO);
        if(radio_sock == -1){
            std::cerr << "Error: Could not connect to radio at " << IP_RADIO << std::endl;
            return -1;
        }

        std::cout << "Succesfully connected to Orquestrator and STA radio" << std::endl << std::endl;

        // Connect to RealSense and configure
        resetCam();

        rs2::pipeline p;
        rs2::config cfg;
        cfg.enable_stream(RS2_STREAM_DEPTH, 640, 480, RS2_FORMAT_Z16, 30);
        cfg.enable_stream(RS2_STREAM_COLOR, 640, 480, RS2_FORMAT_BGR8, 30);

        rs2::pipeline_profile profile = p.start(cfg);

        rs2::device dev = profile.get_device();
        rs2::depth_sensor depth_sensor = dev.first<rs2::depth_sensor>();
        float depth_scale = depth_sensor.get_depth_scale();
        std::cout << "RealSense Depth Scale is: " << depth_scale << std::endl;

        rs2::align align_to_color(RS2_STREAM_COLOR);

        // Config interval timer
        auto const start_time = std::chrono::steady_clock::now();
        auto const wait_time = std::chrono::milliseconds{CAPTURE_INTERVAL};
        auto next_time = start_time + wait_time;

        while(1){
            // Get radio data
            std::string currBeamRSS = getPerBeamRSS(radio_sock);
            sendData(orq_sock, currBeamRSS, "rss"); // Send rss data to server

            // Get RGB frame
            std::vector<uchar> rgb_frame = getRGB(p, align_to_color);
            std::string rgb_str(rgb_frame.begin(), rgb_frame.end());
            sendData(orq_sock, rgb_str, "rgb");

            // Get Depth frame
            std::vector<uchar> depth_frame = getDepth(p, align_to_color);
            std::string depth_str(depth_frame.begin(), depth_frame.end());
            sendData(orq_sock, depth_str, "depth");

            std::this_thread::sleep_until(next_time);
            next_time += wait_time; // increment absolute time
        }
        
        close(orq_sock);
        close(radio_sock);
        p.stop(); // RealSense pipeline
        std::cout << std::endl << "Connections closed";
        
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}