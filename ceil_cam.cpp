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

#define CAPTURE_INTERVAL 200 // Interval time between messages (ms)

#define ORCH_IP "10.0.0.20" // Orchestrator
#define PORT_SERVER 3990

int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        std::cerr << "Insuficient arguments" << std::endl;
        std::cerr << "Usage: " << argv[0] << " <camera_ip> [orchestrator_ip=10.0.0.20]" << std::endl;
        std::cerr << "Example: " << argv[0] << " 200.239.93.231 127.0.0.1" << std::endl;
        return -1;
    }

    std::string video_ip = argv[1];                       // cam ip
    std::string orch_ip = (argc > 2) ? argv[2] : ORCH_IP; // arg or default value

    try
    {
        // Connect with orchestrator
        int orq_sock = connectWithServer(orch_ip.c_str(), PORT_SERVER);
        if (orq_sock == -1)
        {
            std::cerr << "Error: Could not connect to server at " << orch_ip << std::endl;
            return -1;
        }

        std::cout << "Succesfully connected to Orchestrator" << std::endl
                  << std::endl;

        // cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
        // cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

        std::cout << "Device is ready." << std::endl;

        while (1)
        {
            std::optional<std::string> result = listenToServer(orq_sock);

            if (!result.has_value())
            {
                break; // socket closed or error
            }

            if (result.value().empty())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(2)); // yield
                continue;                                                  // no full message yet, wait
            }

            std::string capture_command = result.value();

            try
            {
                int value = std::stoi(capture_command);

                for (int i = 0; i < value; i++)
                {
                    // Get RGB frame
                    // Connect to camera and set resolution
                    cv::VideoCapture cap("https://" + video_ip + ":8080/video");
                    if (!cap.isOpened())
                    {
                        std::cerr << "Error: Could not open camera" << std::endl;
                        return -1;
                    }
                    std::vector<uchar> frame = getCamFrame(cap);
                    std::string frame_str(frame.begin(), frame.end());
                    sendData(orq_sock, frame_str, "rgb_ceil");

                    // std::this_thread::sleep_until(next_time);
                    // next_time += wait_time; // increment absolute time
                }
            }
            catch (...)
            {
                std::cerr << "Unknown message: " << capture_command << std::endl;
            }
        }

        close(orq_sock);
        std::cout << std::endl
                  << "Orchestrator connection closed" << std::endl;
    }
    catch (const std::invalid_argument &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}