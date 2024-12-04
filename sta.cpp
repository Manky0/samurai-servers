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
#include <librealsense2/rs.hpp> 

#include "sta.h"
#include "client.h"

#define CAPTURE_INTERVAL 200 // Interval time between messages (ms)

#define IP_SERVER "10.0.0.20" // Orquestrator
#define PORT_SERVER 3990

#define IP_RADIO "192.168.0.1" // Mikrotik
#define PORT_RADIO 8000


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

        std::cout << "Device is ready." << std::endl;

        while(1){
            int measure_times = listenToServer(orq_sock);
            if ( measure_times == 0 ) break;

            // Set interval timer
            auto start_time = std::chrono::steady_clock::now();
            auto wait_time = std::chrono::milliseconds{CAPTURE_INTERVAL};
            auto next_time = start_time + wait_time;

            for (int i = 0; i < measure_times; i++) {
                // Get radio data
                std::string currBeamRSS = getPerBeamRSS(radio_sock);
                sendData(orq_sock, currBeamRSS, "rss_sta"); // Send rss data to server

                // Get RGB frame
                std::vector<uchar> rgb_frame = getRGB(p, align_to_color);
                std::string rgb_str(rgb_frame.begin(), rgb_frame.end());
                sendData(orq_sock, rgb_str, "rgb_sta");

                // Get Depth frame
                std::vector<uchar> depth_frame = getDepth(p, align_to_color, depth_scale);
                std::string depth_str(depth_frame.begin(), depth_frame.end());
                sendData(orq_sock, depth_str, "depth_sta");

                std::this_thread::sleep_until(next_time);
                next_time += wait_time; // increment absolute time
            }
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