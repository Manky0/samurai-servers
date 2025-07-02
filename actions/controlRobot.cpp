#include <iostream>
#include <cstdlib>

#include "../orchestrator.h"

void startUart() {
    system("sudo chmod 666 /dev/ttyS4"); 
    system("stty -F /dev/ttyS4 115200 cs8 -cstopb -parenb"); 
}

void startWalking() {
    system("echo \"start\" > /dev/ttyS4");
}

void stopWalking() {
    system("echo \"stop\" > /dev/ttyS4");
}