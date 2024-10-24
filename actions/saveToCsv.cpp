#include <iostream>
#include <cstring>
#include <fstream>
#include <nlohmann/json.hpp>

#include "../orquestrator.h"

using json = nlohmann::json;

void saveToCsv(json data) {
    try {
            // Extract 'columns'
            uint64_t mac_addr = data["mac_addr"];
            std::vector<int> snr_data = data["snr_data"];
            uint64_t timestamp = data["timestamp"];

            std::string filename = std::to_string(mac_addr) + ".csv";

            // Open CSV file
            std::ofstream csvFile(filename, std::ios::app);
            if (csvFile.is_open()) {
                csvFile << timestamp; // Write timestamp

                // Write snr_data
                for (const auto& snr : snr_data) {
                    csvFile << "," << snr;
                }

                csvFile << "\n";  // Write linebreak for the next sample

                csvFile.close();
            } else {
                std::cerr << "Unable to open file for writing." << std::endl;
            }
        } catch (const json::exception &e) {
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
    }
}