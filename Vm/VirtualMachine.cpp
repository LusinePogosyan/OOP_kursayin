#include "VirtualMachine.h"
#include <iostream>
#include <iomanip>

VirtualMachine::VirtualMachine(Memory& mem) : registers{0}, pc(0), memory(mem), running(false) {}

void VirtualMachine::executeInstruction(const Instruction& instr) {
    switch (instr.opcode) {
        case OpCode::ADD:
            registers[instr.rd] = registers[instr.rs1] + registers[instr.rs2];
            break;
        case OpCode::ADDI:
            registers[instr.rd] = registers[instr.rs1] + instr.imm;
            break;
        case OpCode::SUB:
            registers[instr.rd] = registers[instr.rs1] - registers[instr.rs2];
            break;
        case OpCode::MUL:
            registers[instr.rd] = registers[instr.rs1] * registers[instr.rs2];
            break;
        case OpCode::DIV:
            registers[instr.rd] = (registers[instr.rs2] == 0) ? 0 : (registers[instr.rs1] / registers[instr.rs2]);
            break;
        case OpCode::MOD:
            registers[instr.rd] = (registers[instr.rs2] == 0) ? 0 : (registers[instr.rs1] % registers[instr.rs2]);
            break;
        case OpCode::AND:
            registers[instr.rd] = registers[instr.rs1] & registers[instr.rs2];
            break;
        case OpCode::OR:
            registers[instr.rd] = registers[instr.rs1] | registers[instr.rs2];
            break;
        case OpCode::XOR:
            registers[instr.rd] = registers[instr.rs1] ^ registers[instr.rs2];
            break;
        case OpCode::SLL:
            registers[instr.rd] = registers[instr.rs1] << (registers[instr.rs2] & 0x1F);
            break;
        case OpCode::SRL:
            registers[instr.rd] = static_cast<uint32_t>(registers[instr.rs1]) >> (registers[instr.rs2] & 0x1F);
            break;
        case OpCode::LW:
            registers[instr.rd] = memory.read32(registers[instr.rs1] + instr.imm);
            break;
        case OpCode::SW:
            memory.write32(registers[instr.rs1] + instr.imm, registers[instr.rs2]);
            break;
        case OpCode::BEQ:
            if (registers[instr.rs1] == registers[instr.rs2]) {
                pc = static_cast<uint32_t>(static_cast<int32_t>(pc) + instr.imm - 1);
            }
            break;
        case OpCode::BNE:
            if (registers[instr.rs1] != registers[instr.rs2]) {
                pc = static_cast<uint32_t>(static_cast<int32_t>(pc) + instr.imm - 1);
            }
            break;
        case OpCode::BLT:
            if (registers[instr.rs1] < registers[instr.rs2]) {
                pc = static_cast<uint32_t>(static_cast<int32_t>(pc) + instr.imm - 1);
            }
            break;
        case OpCode::BGE:
            if (registers[instr.rs1] >= registers[instr.rs2]) {
                pc = static_cast<uint32_t>(static_cast<int32_t>(pc) + instr.imm - 1);
            }
            break;
        case OpCode::JAL:
            registers[instr.rd] = pc + 1;
            pc = instr.imm - 1; // -1 because loop increments pc
            break;
        case OpCode::JALR:
            registers[instr.rd] = pc + 1;
            pc = static_cast<uint32_t>(registers[instr.rs1] + instr.imm - 1);
            break;
        case OpCode::HALT:
            running = false;
            break;
        case OpCode::PRINT:
            std::cout << registers[instr.rs1] << "\n";
            break;
        default:
            running = false;
            break;
    }
    registers[0] = 0; // Hardwired zero
}

void VirtualMachine::run(const std::vector<Instruction>& prog) {
    pc = 0;
    running = true;
    while (running && pc < prog.size()) {
        executeInstruction(prog[pc]);
        pc++;
    }
}

void VirtualMachine::dumpRegisters() const {
    std::cout << "Register dump:\n";
    for (int i = 0; i < 32; ++i) {
        std::cout << std::setw(4) << RegNames[i] << "(x" << i << ") = " << registers[i] << "\n";
    }
}