#ifndef OBJECTFILE_H
#define OBJECTFILE_H

#include <vector>
#include <cstdint>
#include "../Common/Instruction.h"

struct ObjectFileHeader {
    uint32_t magic;      // "ELEN"
    uint32_t code_size;  // Սա այն է, ինչ փնտրում է Loader-ը
    uint32_t data_size;
};

struct ObjectFile {
    ObjectFileHeader header;
    std::vector<Instruction> textSection; // Ավելացրու սա
    std::vector<int32_t> dataSection;     // Ավելացրու սա
};

#endif