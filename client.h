#include <opencv2/opencv.hpp>

int connectWithServer(const char *server_ip, int port);

std::vector<uchar> getCamFrame(cv::VideoCapture cap);