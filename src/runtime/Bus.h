#ifndef RUNTIME_BUS_H
#define RUNTIME_BUS_H

#include "../../Vm/Memory.h"

namespace runtime {

class Bus {
public:
    explicit Bus(Memory& memoryRef) : memory(memoryRef) {}

    uint32_t read32(uint32_t address) const { return memory.read32(address); }
    void write32(uint32_t address, uint32_t value) { memory.write32(address, value); }

private:
    Memory& memory;
};

}  // namespace runtime

#endif