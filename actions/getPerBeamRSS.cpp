#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>
#include <nlohmann/json.hpp>

#include "../sta.h"
#include "../client.h"

using json = nlohmann::json;

const int BUFFER_SIZE = 1024;
std::string lastReceived;

json radioSendAndReceive(int radio_sock, const json &dataToSend, int buf_len) {
    // Serialize JSON data to string and send it
    std::string message = dataToSend.dump();
    send(radio_sock, message.c_str(), message.length(), 0);

        char buffer[BUFFER_SIZE];
        json response;
        
        std::string received_data;
        int len = 0;

        // Receive data until the termination character
        while ((len = recv(radio_sock, buffer, buf_len, 0)) > 0) {
            received_data.append(buffer, len);
            if (buffer[len - 1] == '}') {
                break;
            }
        }

        if (received_data != lastReceived) { // Ignore duplicates

            // Process RSSI data (ignore start_scan)
            if (dataToSend["cmd"] != "start_scan"){
                lastReceived = received_data;

                // Get timestamp with ms precision
                const auto now = std::chrono::system_clock::now();
                const auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

                json rssi_data = json::parse(received_data);
                rssi_data["timestamp"] = timestamp;

                response["device"] = "radio";
                response["data"] = rssi_data;
            }

            return response;
            // std::cout << "Received new data: " << received_data << std::endl;
        }

    return 0;
}

std::string getPerBeamRSS(int radio_sock) {
    json start_scan_cmd, per_beam_rss_cmd;
    start_scan_cmd["cmd"] = "start_scan";
    start_scan_cmd["args"] = {};
    
    per_beam_rss_cmd["cmd"] = "per_beam_rss_v2x";
    per_beam_rss_cmd["args"] = {};

    json response;

    try { // Send commands and receive response
        std::cout << "Sending start_scan command..." << std::endl;
        radioSendAndReceive(radio_sock, start_scan_cmd, BUFFER_SIZE);

        std::cout << "Sending per_beam_rss_v2x command..." << std::endl << std::endl;
        response = radioSendAndReceive(radio_sock, per_beam_rss_cmd, BUFFER_SIZE);

    } catch (const std::exception &e) {
        std::cerr << "Exception occurred: " << e.what() << std::endl;
    }

    return response.dump(); // Return JSON as string
}
