#ifndef VIRTUALMACHINE_H
#define VIRTUALMACHINE_H

#include <vector>
#include <cstdint>
#include "../Common/Instruction.h"
#include "Memory.h"
#include "Architecture.h"

class VirtualMachine {
private:
    int32_t registers[32]; // RISC-V x0-x31
    uint32_t pc;           // Program Counter
    Memory& memory;        // Հղում հիշողությանը
    CpuArchitecture arch;
    bool running;

    // Օժանդակ ֆունկցիա հրահանգը վերծանելու և կատարելու համար
    void executeInstruction(const Instruction& instr);

public:
    VirtualMachine(Memory& mem);
    
    // Ծրագրի բեռնում և գործարկում
    void run(const std::vector<Instruction>& program);
    
    // Ռեգիստրների վիճակը դուրս բերելու համար (Debug)
    void dumpRegisters() const;
    int32_t getReg(int idx) const { return registers[idx]; }
    const CpuArchitecture& getArchitecture() const { return arch; }
};

#endif