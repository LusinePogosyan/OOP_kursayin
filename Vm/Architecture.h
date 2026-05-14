#ifndef ARCHITECTURE_H
#define ARCHITECTURE_H

#include <cstdint>
#include <string>

struct CpuArchitecture {
    std::string name = "RISC5";
    uint32_t xlenBits = 32;      // 32-bit registers
    uint32_t registerCount = 32; // x0..x31
    bool littleEndian = true;
};

#endif