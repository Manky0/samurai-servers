#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>

#include "../client.h"

void resetCam() {
    std::cout << "RealSense reset start" << std::endl;
    rs2::context ctx;
    auto devices = ctx.query_devices();
    for (auto&& dev : devices) {
        std::cout << "Resetting RealSense device: " << dev.get_info(RS2_CAMERA_INFO_NAME) << std::endl;
        dev.hardware_reset();
    }
    std::cout << "Waiting for RealSense device to reset..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));  // Wait after reset
    std::cout << "RealSense reset done" << std::endl;
}

std::vector<uchar> getRGB(rs2::pipeline p, rs2::align align_to_color) {  
        rs2::frameset frames = p.wait_for_frames();
        frames = align_to_color.process(frames);

        rs2::video_frame color_frame = frames.get_color_frame();

        const int color_width = color_frame.get_width();
        const int color_height = color_frame.get_height();

        cv::Mat color_image(cv::Size(color_width, color_height), CV_8UC3, (void*)color_frame.get_data(), cv::Mat::AUTO_STEP);

        std::vector<uchar> buf;
        std::vector<int> compression_params = {cv::IMWRITE_JPEG_QUALITY, 60}; // Adjust compression quality
        cv::imencode(".jpg", color_image, buf, compression_params); // Compress to JPEG

        return buf;
}

std::vector<uchar> getDepth(rs2::pipeline p, rs2::align align_to_color) {  
        rs2::frameset frames = p.wait_for_frames();
        frames = align_to_color.process(frames);

        rs2::depth_frame depth_frame = frames.get_depth_frame();

        const int depth_width = depth_frame.get_width();
        const int depth_height = depth_frame.get_height();

        cv::Mat depth_image(cv::Size(depth_width, depth_height), CV_16UC1, (void*)depth_frame.get_data(), cv::Mat::AUTO_STEP);

        // Convert depth image to an 8-bit image for display purposes
        cv::Mat depth_colormap;
        cv::convertScaleAbs(depth_image, depth_colormap, 0.03);
        cv::applyColorMap(depth_colormap, depth_colormap, cv::COLORMAP_JET);

        std::vector<uchar> buf;
        std::vector<int> compression_params = {cv::IMWRITE_JPEG_QUALITY, 60}; // Adjust compression quality
        cv::imencode(".jpg", depth_colormap, buf, compression_params); // Compress to JPEG

        return buf;
}
