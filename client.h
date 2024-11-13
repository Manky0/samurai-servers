#include <opencv2/opencv.hpp>
#include <librealsense2/rs.hpp> 

int connectWithServer(const char *server_ip, int port);

std::vector<uchar> getCamFrame(cv::VideoCapture cap);

void resetCam();

std::vector<uchar> getRGB(rs2::pipeline p, rs2::align align_to_color);

std::vector<uchar> getDepth(rs2::pipeline p, rs2::align align_to_color);