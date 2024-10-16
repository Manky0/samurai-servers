#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>

#include "radioFunctions.h"

int main() {
    std::string received_data;

    int counter = 0;
    while(counter < 5){
        received_data = getPerBeamRSS();

        std::cout << "Received new data: " << received_data << std::endl;

        counter++;
    }

    return 0;
}