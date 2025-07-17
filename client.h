#include <opencv2/opencv.hpp>
#include <optional>

int connectWithServer(const char *server_ip, int port);

void sendData(int socket, std::string data, std::string device_type);

std::optional<std::string> listenToServer(int sock);

std::vector<uchar> getCamFrame(cv::VideoCapture cap);