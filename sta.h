#include <librealsense2/rs.hpp> 

std::string getPerBeamRSS(int radio_sock);

void resetCam();

std::vector<uchar> getRGB(rs2::pipeline p, rs2::align align_to_color);

std::vector<uchar> getDepth(rs2::pipeline p, rs2::align align_to_color);