#include <iostream>
#include <cstring>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "../orquestrator.h"

void saveToCsv(json data, uint64_t captured_at, uint64_t received_at) {
    try {
        // Extract 'columns'
        uint64_t mac_addr = data["mac_addr"];
        std::vector<int> snr_data = data["snr_data"];

        std::string filename = std::to_string(mac_addr) + ".csv";

        // Open CSV file
        std::ofstream csvFile(filename, std::ios::app);
        if (csvFile.is_open()) {
            // Write timestamps
            csvFile << captured_at << ","; 
            csvFile << received_at;

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