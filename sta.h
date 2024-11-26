#include <librealsense2/rs.hpp> 

std::string getPerBeamRSS(int radio_sock);

void resetCam();

std::vector<unsigned char> getRGB(rs2::pipeline p, rs2::align align_to_color);

std::vector<unsigned char> getDepth(rs2::pipeline p, rs2::align align_to_color);