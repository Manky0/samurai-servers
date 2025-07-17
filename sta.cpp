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
#include <optional>
#include <librealsense2/rs.hpp>

#include "sta.h"
#include "client.h"

#define ORCH_IP "10.0.0.20" // Orchestrator
#define ORCH_PORT 3990

#define RADIO_IP "192.168.0.1" // Mikrotik
#define RADIO_PORT 8000

int main(int argc, char *argv[])
{

    if (argc < 1)
    {
        std::cerr << "Insuficient arguments" << std::endl;
        std::cerr << "Usage: " << argv[0] << " [orchestrator_ip=10.0.0.20]" << std::endl;
        std::cerr << "Example: " << argv[0] << " 127.0.0.1" << std::endl;
        return -1;
    }

    std::string orch_ip = (argc > 1) ? argv[1] : ORCH_IP; // arg or default value

    try
    {
        // Connect with orchestrator
        int orq_sock = connectWithServer(orch_ip.c_str(), ORCH_PORT);
        if (orq_sock == -1)
        {
            std::cerr << "Error: Could not connect to server at " << orch_ip << std::endl;
            return -1;
        }

        // Connect with STA radio
        int radio_sock = connectWithServer(RADIO_IP, RADIO_PORT);
        if (radio_sock == -1)
        {
            std::cerr << "Error: Could not connect to radio at " << RADIO_IP << std::endl;
            return -1;
        }

        std::cout << "Succesfully connected to Orchestrator and STA radio" << std::endl
                  << std::endl;

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

        // Start GPIO ports
        startGPIO(72); // GPIO_UART1_RTS

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

                if (value == -4)
                {
                    valueGPIO(72, 1);
                    stopWalking();
                    valueGPIO(72, 0);
                }
                else if (value == -3)
                {
                    valueGPIO(72, 1);
                    startWalking();
                    valueGPIO(72, 0);
                }
                else if (value == -2)
                {
                    startUart();
                }
                else
                { // Normal capture procedure

                    for (int i = 0; i < value; i++)
                    {
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
                    }
                }
            }
            catch (...)
            {
                std::cerr << "Unknown message: " << capture_command << std::endl;
            }
        }

        close(orq_sock);
        close(radio_sock);
        p.stop(); // RealSense pipeline
        std::cout << std::endl
                  << "Connections closed" << std::endl;
    }
    catch (const std::invalid_argument &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}