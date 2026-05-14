#include "VmMonitor.h"

namespace runtime {

VmMonitor::VmMonitor(size_t memoryBytes) : memory(memoryBytes), bus(memory), cpu(memory) {}

void VmMonitor::run(const std::vector<Instruction>& program, const std::vector<int32_t>& dataWords, uint32_t dataBaseAddress) {
    for (size_t i = 0; i < dataWords.size(); ++i) {
        bus.write32(dataBaseAddress + static_cast<uint32_t>(i * sizeof(int32_t)), static_cast<uint32_t>(dataWords[i]));
    }
    cpu.run(program);
}

int32_t VmMonitor::readRegister(int idx) const {
    return cpu.getReg(idx);
}

void VmMonitor::dumpRegisters() const {
    cpu.dumpRegisters();
}

CpuArchitecture VmMonitor::getArchitecture() const {
    return cpu.getArchitecture();
}

uint32_t VmMonitor::allocateDynamic(uint32_t bytes) {
    return memory.allocateDynamic(bytes);
}

uint32_t VmMonitor::allocateStatic(uint32_t bytes) {
    return memory.allocateStatic(bytes);
}

}  // namespace runtime
