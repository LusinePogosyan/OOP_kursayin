#include "ExecutableFormat.h"

#include <fstream>
#include <stdexcept>

namespace linker_stage {

namespace {
constexpr uint32_t kMagic = 0x454C454E;  // "ELEN"
}

ExecutableImage ToolchainLinker::linkToImage(const std::vector<Instruction>& text, const std::vector<int32_t>& data) const {
    ExecutableImage image;
    image.header = {kMagic, static_cast<uint32_t>(text.size()), static_cast<uint32_t>(data.size())};
    image.text = text;
    image.data = data;
    return image;
}

void ToolchainLinker::writeExecutable(const ExecutableImage& image, const std::string& outputPath) const {
    std::ofstream out(outputPath, std::ios::binary);
    if (!out) {
        throw std::runtime_error("Unable to open output executable: " + outputPath);
    }

    out.write(reinterpret_cast<const char*>(&image.header), sizeof(ExecutableHeader));
    out.write(reinterpret_cast<const char*>(image.text.data()), static_cast<std::streamsize>(image.text.size() * sizeof(Instruction)));
    out.write(reinterpret_cast<const char*>(image.data.data()), static_cast<std::streamsize>(image.data.size() * sizeof(int32_t)));
}

}  // namespace linker_stage
