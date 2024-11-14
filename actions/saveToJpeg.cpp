#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

#include "../orquestrator.h"

void saveToJpeg(json data) {
    try {
        std::vector<uchar> img_data = data["data"];
        uint64_t captured_at = data["captured_at"];
        uint64_t received_at = data["received_at"];

        std::string img_name = std::to_string(captured_at) + "_" + std::to_string(received_at) + ".jpg";

        std::string dir_path;

        if (data["type"] == 0x02){
            dir_path = "./images/rgb_sta/";
        } else if (data["type"] == 0x03) {
            dir_path = "./images/depth_sta/";
        } else if (data["type"] == 0x04) {
            dir_path = "./images/rgb_ap/";
        } else if (data["type"] == 0x06) {
            dir_path = "./images/rgb_ceil/";
        }

        fs::create_directories(dir_path);
        std::string img_path = dir_path + img_name;
        
        // Decode the compressed image back to cv::Mat
        cv::Mat img = cv::imdecode(img_data, cv::IMREAD_COLOR);

        // Save the decoded image to a file
        cv::imwrite(img_path, img);
        
        } catch (const std::exception &e) {
            std::cerr << "Error saving JPEG image: " << e.what() << std::endl;
    }
}