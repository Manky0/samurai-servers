#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>
#include <nlohmann/json.hpp>

#include "sta.h"
using json = nlohmann::json;

// Buffer size for receiving data
const int BUFFER_SIZE = 1024;

std::string radioSendAndReceive(int radio_sock, const json &dataToSend, int buf_len, std::string &lastReceived) {
    // Serialize JSON data to string and send it
    std::string message = dataToSend.dump();
    send(radio_sock, message.c_str(), message.length(), 0);

    char buffer[BUFFER_SIZE];
    std::string received_data;
    int len = 0;

    // Receive data until the termination character
    while ((len = recv(radio_sock, buffer, buf_len, 0)) > 0) {
        received_data.append(buffer, len);
        if (buffer[len - 1] == ')' || buffer[len - 1] == ']' || buffer[len - 1] == '}') {
            break;
        }
    }

    // If the data is not the same as last received, process it
    // if (received_data != lastReceived) {
    //     lastReceived = received_data;
    //     std::cout << "Received new data: " << received_data << std::endl;
    // }
    return received_data;
}

std::string getPerBeamRSS() {
    // radio connection parameters
    std::string radio_ip = "200.239.93.46";
    int radio_port = 8000;
    int measurement_time = 1; // Time in seconds
    int measurement_interval = 30; // Interval between measurements

    // Create socket
    int radio_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (radio_sock < 0) {
        std::cerr << "Socket creation failed!" << std::endl;
        return "-1";
    }

    // radio address structure
    struct sockaddr_in radio_addr;
    radio_addr.sin_family = AF_INET;
    radio_addr.sin_port = htons(radio_port);

    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, radio_ip.c_str(), &radio_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return "-1";
    }

    // Connect to the radio
    std::cout << "Connecting to the radio..." << std::endl;
    if (connect(radio_sock, (struct sockaddr *)&radio_addr, sizeof(radio_addr)) < 0) {
        std::cerr << "Connection to radio failed" << std::endl;
        return "-1";
    }
    std::cout << "Connected to the radio!" << std::endl;

    json start_scan_cmd, per_beam_rss_cmd;
    start_scan_cmd["cmd"] = "start_scan";
    start_scan_cmd["args"] = {};
    
    per_beam_rss_cmd["cmd"] = "per_beam_rss_v2x";
    per_beam_rss_cmd["args"] = {};

    std::string lastReceivedData;
    int counter = 0;
    std::string received_data;
    // auto end_time = std::chrono::steady_clock::now() + std::chrono::seconds(measurement_time);

    // Main loop to send commands and receive responses
    try {
        counter++;
        // Send start_scan command and receive response
        std::cout << "Sending start_scan command..." << std::endl;
        radioSendAndReceive(radio_sock, start_scan_cmd, BUFFER_SIZE, lastReceivedData);

        // Short sleep before the next command
        std::this_thread::sleep_for(std::chrono::milliseconds(measurement_interval/2));

        // Send per_beam_rss command and receive response
        std::cout << "Sending per_beam_rss_v2x command..." << std::endl;
        received_data = radioSendAndReceive(radio_sock, per_beam_rss_cmd, BUFFER_SIZE, lastReceivedData);

        // Sleep for the remaining half of the interval
        // std::this_thread::sleep_for(std::chrono::milliseconds(measurement_interval/2));
    } catch (const std::exception &e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
    }

    // Close the socket connection
    close(radio_sock);
    std::cout << "Connection closed." << std::endl;

    return received_data;
}
