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

    return buf;
}
