#include "vCPU.h"

#include <fstream>
#include <iostream>

vCPU::vCPU() {
    pc = START_ADDRESS;
}


void vCPU::loadROM(const char *filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (file.is_open()) {
        const std::streampos size = file.tellg();
        const auto buffer = new char[size];

        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();

        for (long i = 0; i < size; ++i) {
            this->memory[START_ADDRESS + i] = buffer[i];
        }

        delete[] buffer;
    } else {
        std::cerr << "Failed to open file: " << filename << std::endl;
        exit(1);
    }
}
