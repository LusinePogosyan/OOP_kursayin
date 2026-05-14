#ifndef VM_MONITOR_H
#define VM_MONITOR_H

#include <vector>

#include "../../Common/Instruction.h"
#include "../../Vm/Memory.h"
#include "../../Vm/VirtualMachine.h"
#include "../../Vm/Architecture.h"
#include "Bus.h"

namespace runtime {

class VmMonitor {
public:
    explicit VmMonitor(size_t memoryBytes = 65536);
    void run(const std::vector<Instruction>& program, const std::vector<int32_t>& dataWords = {}, uint32_t dataBaseAddress = 4096);
    int32_t readRegister(int idx) const;
    void dumpRegisters() const;
    CpuArchitecture getArchitecture() const;
    uint32_t allocateDynamic(uint32_t bytes);
    uint32_t allocateStatic(uint32_t bytes);

private:
    Memory memory;
    Bus bus;
    VirtualMachine cpu;
};

}  // namespace runtime

#endif