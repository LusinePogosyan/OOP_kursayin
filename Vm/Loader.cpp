#include "Loader.h"
#include "../Linker/ObjectFile.h"
#include <fstream>
#include <stdexcept>

std::vector<Instruction> Loader::loadExecutable(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot open file");

    ObjectFileHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(ObjectFileHeader));

    if (header.magic != 0x454C454E) {
        throw std::runtime_error("Invalid magic number");
    }

    std::vector<Instruction> program(header.code_size);
    file.read(reinterpret_cast<char*>(program.data()), header.code_size * sizeof(Instruction));

    return program;
}