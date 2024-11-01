#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "../orquestrator.h"

void saveToJpeg(json data) {
    try {
        std::vector<uchar> img_data = data["data"];
        uint64_t captured_at = data["captured_at"];
        uint64_t received_at = data["received_at"];

        std::string img_name = std::to_string(captured_at) + "_" + std::to_string(received_at) + ".jpg";
        std::string img_path = "./images/" + img_name;
        
        // // Decode the compressed image back to cv::Mat
        cv::Mat img = cv::imdecode(img_data, cv::IMREAD_COLOR);

        // // Save the decoded image to a file
        cv::imwrite(img_path, img);

        } catch (const std::exception &e) {
            std::cerr << "Error saving JPEG image: " << e.what() << std::endl;
    }
}