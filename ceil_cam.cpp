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

#include "sta.h"
#include "client.h"

#define CAPTURE_INTERVAL 200 // Interval time between messages (ms)

#define IP_SERVER "127.0.0.1"
// #define IP_SERVER "200.239.93.191" // Orquestrator
#define PORT_SERVER 3990


int main(int argc, char *argv[]) {

    try {
        // Connect with orquestrator
        int orq_sock = connectWithServer(IP_SERVER, PORT_SERVER);
        if(orq_sock == -1){
            std::cerr << "Error: Could not connect to server at " << IP_SERVER << std::endl;
            return -1;
        }

        std::cout << "Succesfully connected to Orquestrator" << std::endl << std::endl;

        // Connect to camera and set resolution
        cv::VideoCapture cap(-1);
        if (!cap.isOpened()) {
            std::cerr << "Error: Could not open camera" << std::endl;
            return -1;
        }
        cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

        // Config interval timer
        auto const start_time = std::chrono::steady_clock::now();
        auto const wait_time = std::chrono::milliseconds{CAPTURE_INTERVAL};
        auto next_time = start_time + wait_time;

        while(1){
            // Get RGB frame
            std::vector<uchar> frame = getCamFrame(cap);
            std::string frame_str(frame.begin(), frame.end());
            sendData(orq_sock, frame_str, "rgb_ceil");

            std::this_thread::sleep_until(next_time);
            next_time += wait_time; // increment absolute time
        }
        
        close(orq_sock);
        std::cout << std::endl << "Orquestrator connection closed";
        
    } catch (const std::invalid_argument &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}