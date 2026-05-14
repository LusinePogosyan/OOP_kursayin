#include "Linker.h"
#include <fstream>

void Linker::link(const std::vector<ObjectFile>& objects, const std::string& outputPath) {
    ObjectFile finalExe;

    // Միավորում ենք բոլոր օբյեկտային ֆայլերի սեկցիաները
    for (const auto& obj : objects) {
        finalExe.textSection.insert(finalExe.textSection.end(), obj.textSection.begin(), obj.textSection.end());
        finalExe.dataSection.insert(finalExe.dataSection.end(), obj.dataSection.begin(), obj.dataSection.end());
    }

    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile) return;

    // Գրում ենք Header-ը
    ObjectFileHeader header = {0x454C454E, (uint32_t)finalExe.textSection.size(), (uint32_t)finalExe.dataSection.size()};
    outFile.write(reinterpret_cast<const char*>(&header), sizeof(ObjectFileHeader));

    // Գրում ենք Code (text)
    outFile.write(reinterpret_cast<const char*>(finalExe.textSection.data()), finalExe.textSection.size() * sizeof(Instruction));

    // Գրում ենք Data
    outFile.write(reinterpret_cast<const char*>(finalExe.dataSection.data()), finalExe.dataSection.size() * sizeof(int32_t));

    outFile.close();
}