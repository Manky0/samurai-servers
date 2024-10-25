#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

#include "../client.h"

std::vector<uchar> getCamFrame(cv::VideoCapture cap) {
    cv::Mat frame;
    cap >> frame; // Capture frame

    // if (frame.empty()) {
    //     std::cerr << "Error: Failed to capture frame" << std::endl;
    //     return -1;
    // }

    std::vector<uchar> buf;
    std::vector<int> compression_params = {cv::IMWRITE_JPEG_QUALITY, 60}; // Adjust compression quality
    cv::imencode(".jpg", frame, buf, compression_params); // Compress to JPEG

    // Print the size of the compressed image buffer
    // std::cout << "Compressed image size: " << buf.size() << " bytes" << std::endl;

    // // Decode the compressed image back to cv::Mat
    // cv::Mat img = cv::imdecode(buf, cv::IMREAD_COLOR);

    // if (img.empty()) {
    //     std::cerr << "Error: Failed to decode image" << std::endl;
    //     return -1;
    // }

    // // Save the decoded image to a file
    // cv::imwrite("teste.jpg", img);

    return buf;
}
