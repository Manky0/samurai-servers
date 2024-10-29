#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "../orquestrator.h"

void saveToJpeg(json data) {
    try {
        std::vector<uchar> img_data = data["data"];
        
        // // Decode the compressed image back to cv::Mat
        cv::Mat img = cv::imdecode(img_data, cv::IMREAD_COLOR);

        // // Save the decoded image to a file
        cv::imwrite("cam.jpg", img);

        } catch (const std::exception &e) {
            std::cerr << "Error saving JPEG image: " << e.what() << std::endl;
    }
}