#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>
#include <filesystem>

extern std::atomic<int> capture_counter; // Image name increment (from orchestrator.cpp)

using json = nlohmann::json;
namespace fs = std::filesystem;

#include "../orchestrator.h"

void saveToJpeg(std::vector<unsigned char> img_data, uint8_t device_type, uint64_t captured_at, uint64_t received_at) {
    try {
        std::string dir_path;

        extern std::string session_dir;  // Declare the global session_dir

        if (device_type == 0x02){
            dir_path = session_dir + "images/rgb_sta/";
        } else if (device_type == 0x03) {
            dir_path = session_dir + "images/depth_sta/";
        } else if (device_type == 0x04) {
            dir_path = session_dir + "images/rgb_ap/";
        } else if (device_type == 0x06) {
            dir_path = session_dir + "images/rgb_ceil/";
        }

        fs::create_directories(dir_path);

        // Count files in the folder then define image name
        int current_index = std::distance(fs::directory_iterator(dir_path), fs::directory_iterator{});

        std::string img_name = std::to_string(current_index + 1) + "_" + std::to_string(captured_at) + "_" + std::to_string(received_at) + ".jpg";
        std::string img_path = dir_path + img_name;
        
        // Decode the compressed image back to cv::Mat
        cv::Mat img = cv::imdecode(img_data, cv::IMREAD_COLOR);

        // Save the decoded image to a file
        cv::imwrite(img_path, img);
        
        } catch (const std::exception &e) {
            std::cerr << "Error saving JPEG image: " << e.what() << std::endl;
    }
}