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

#include "client.h"

// #define CAPTURE_INTERVAL 200 // Interval time between messages (ms)

#define IP_SERVER "10.0.0.20" // Orchestrator
#define PORT_SERVER 3990

int main(int argc, char *argv[]) {

    try {
        // Connect with orchestrator
        int orq_sock = connectWithServer(IP_SERVER, PORT_SERVER);
        if(orq_sock == -1){
            std::cerr << "Error: Could not connect to server at " << IP_SERVER << std::endl;
            return -1;
        }

        std::cout << "Succesfully connected to Orchestrator" << std::endl << std::endl;

        // Connect to camera and set resolution
        cv::VideoCapture cap(-1);
        if (!cap.isOpened()) {
            std::cerr << "Error: Could not open camera" << std::endl;
            return -1;
        }
        cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

        std::cout << "Device is ready." << std::endl;

        while(1){
            std::string capture_command = listenToServer(orq_sock);

            if (capture_command.empty()) {
                // std::cerr << "Server disconnected." << std::endl;
                break;
            }

            try {
                    int value = std::stoi(capture_command);

                    for (int i = 0; i < measure_times; i++) {
                        // Get RGB frame
                        std::vector<uchar> frame = getCamFrame(cap);
                        std::string frame_str(frame.begin(), frame.end());
                        sendData(orq_sock, frame_str, "rgb_ap");

                        // std::this_thread::sleep_until(next_time);
                        // next_time += wait_time; // increment absolute time
                    }
            } catch (...) {
                std::cerr << "Unknown message: " << capture_command << std::endl;
            }
        }
        
        close(orq_sock);
        std::cout << std::endl << "Orchestrator connection closed" << std::endl;
        
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}