#include <iostream>
#include <cstdlib>

#include "../sta.h"

void startUart() {
    // Bellow or use: "sudo usermod -aG dialout $USER" in bash and reboot
    system("sudo chmod 666 /dev/ttyS4"); 
    system("stty -F /dev/ttyS4 115200 cs8 -cstopb -parenb"); 
}

void startWalking() {
    system("echo \"start\" > /dev/ttyS4");
}

void stopWalking() {
    system("echo \"stop\" > /dev/ttyS4");
}

void startGPIO(int port) {
    int gpio_num = 922 + port;
    std::string gpio_str = std::to_string(gpio_num);

    std::string export_cmd = "echo " + gpio_str + " | sudo tee /sys/class/gpio/export > /dev/null 2>&1";
    std::string direction_cmd = "echo out | sudo tee /sys/class/gpio/gpio" + gpio_str + "/direction > /dev/null 2>&1";
    std::string active_low_cmd = "echo 0 | sudo tee /sys/class/gpio/gpio" + gpio_str + "/active_low > /dev/null 2>&1";

    system(export_cmd.c_str());
    system(direction_cmd.c_str());
    system(active_low_cmd.c_str());
}

void valueGPIO(int port, int value) {
    int gpio_num = 922 + port;
    std::string gpio_str = std::to_string(gpio_num);
    std::string value_str = std::to_string(value);

    std::string cmd = "echo " + value_str + " | sudo tee /sys/class/gpio/gpio" + gpio_str + "/value > /dev/null 2>&1";
    system(cmd.c_str());
}