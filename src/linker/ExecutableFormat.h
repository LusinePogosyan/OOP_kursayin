#ifndef EXECUTABLE_FORMAT_H
#define EXECUTABLE_FORMAT_H

#include <cstdint>
#include <string>
#include <vector>

#include "../../Common/Instruction.h"

namespace linker_stage {

struct ExecutableHeader {
    uint32_t magic;
    uint32_t textSize;
    uint32_t dataSize;
};

struct ExecutableImage {
    ExecutableHeader header;
    std::vector<Instruction> text;
    std::vector<int32_t> data;
};

class ToolchainLinker {
public:
    ExecutableImage linkToImage(const std::vector<Instruction>& text, const std::vector<int32_t>& data) const;
    void writeExecutable(const ExecutableImage& image, const std::string& outputPath) const;
};

}  // namespace linker_stage

#endif