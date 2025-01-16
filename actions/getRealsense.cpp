#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>

#include "../sta.h"

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

// Function to create a depth scale bar
cv::Mat createDepthScaleBar(int height, int width, float min_depth, float max_depth) {
    // Create an empty scale bar
    cv::Mat scale_bar(height, width, CV_8UC3, cv::Scalar(0, 0, 0));

    // Fill the scale bar with the colormap
    for (int i = 0; i < height; ++i) {
        float depth = min_depth + (max_depth - min_depth) * (i / static_cast<float>(height - 1));
        int color_value = static_cast<int>(255 * (1.0f - (i / static_cast<float>(height - 1)))); // Invert color mapping

        // Create a single-pixel grayscale image
        cv::Mat grayscale_pixel(1, 1, CV_8UC1, cv::Scalar(color_value));
        cv::Mat color_pixel;

        // Apply the colormap
        cv::applyColorMap(grayscale_pixel, color_pixel, cv::COLORMAP_PARULA);

        // Extract the color and fill the scale bar row
        cv::Vec3b color = color_pixel.at<cv::Vec3b>(0, 0);
        scale_bar.row(i).setTo(color);
    }

    // Add depth annotations (text) to the scale bar
    int font_thickness = 1;
    double font_scale = 0.4;
    int margin = 5; // Space between text and bar
    for (int i = 0; i <= 6; ++i) {
        float depth = min_depth + (max_depth - min_depth) * ((6 - i) / 6.0); // Reverse order of text annotations
        int y_pos = static_cast<int>(height * i / 6.0); // Position text at intervals

        // Ensure text is within bounds
        y_pos = std::clamp(y_pos, margin, height - margin);

        // Add the text annotation
        cv::putText(scale_bar, cv::format("%.1fm", depth), 
                    cv::Point(width / 6, y_pos), 
                    cv::FONT_HERSHEY_SIMPLEX, 
                    font_scale, 
                    cv::Scalar(255, 255, 255), 
                    font_thickness);
    }

    return scale_bar;
}

std::vector<uchar> getRGB(rs2::pipeline p, rs2::align align_to_color) {  
    rs2::frameset frames = p.wait_for_frames();
    frames = align_to_color.process(frames);

    rs2::video_frame color_frame = frames.get_color_frame();

    const int color_width = color_frame.get_width();
    const int color_height = color_frame.get_height();

    cv::Mat color_image(cv::Size(color_width, color_height), CV_8UC3, (void*)color_frame.get_data(), cv::Mat::AUTO_STEP);

    std::vector<uchar> buf;
    std::vector<int> compression_params = {cv::IMWRITE_JPEG_QUALITY, 90}; // Adjust compression quality
    cv::imencode(".jpg", color_image, buf, compression_params); // Compress to JPEG

    return buf;
}

float min_depth = 0.0f; // Minimum depth in meters
float max_depth = 6.0f; // Maximum depth in meters

std::vector<uchar> getDepth(rs2::pipeline p, rs2::align align_to_color, float depth_scale) {  
    rs2::frameset frames = p.wait_for_frames();

    rs2::depth_frame depth_frame = frames.get_depth_frame();

    // Convert depth data to CV_16U
    cv::Mat depth_image(cv::Size(depth_frame.get_width(), depth_frame.get_height()), CV_16UC1, (void*)depth_frame.get_data(), cv::Mat::AUTO_STEP);

    // Scale depth data to 8-bit and apply colormap
    cv::Mat depth_colormap, scaled_depth;
    depth_image.convertTo(scaled_depth, CV_8UC1, 255.0 / (max_depth / depth_scale));
    cv::applyColorMap(scaled_depth, depth_colormap, cv::COLORMAP_JET);

    // // Create a depth scale bar
    // cv::Mat scale_bar = createDepthScaleBar(depth_colormap.rows, 50, min_depth, max_depth);

    // // Combine depth colormap and scale bar
    // cv::Mat combined;
    // cv::hconcat(depth_colormap, scale_bar, combined);

    std::vector<uchar> buf;
    std::vector<int> compression_params = {cv::IMWRITE_JPEG_QUALITY, 90}; // Adjust compression quality
    cv::imencode(".jpg", depth_colormap, buf, compression_params); // Compress to JPEG

    return buf;
}
