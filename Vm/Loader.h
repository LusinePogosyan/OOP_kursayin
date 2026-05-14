#ifndef LOADER_H
#define LOADER_H

#include <vector>
#include <string>
#include "../Common/Instruction.h"
#include "../Linker/ObjectFile.h" // Ներառեք սա, որպեսզի ObjectFileHeader-ը լինի defined

class Loader {
public:
    static std::vector<Instruction> loadExecutable(const std::string& filename);
};

#endif