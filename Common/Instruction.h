#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <cstdint>
#include <string>

// RISC-V հիմնական հրահանգների տիպերը
enum class OpCode : uint8_t {
    // Arithmetic (R-type / I-type)
    ADD, ADDI, SUB, MUL, DIV, MOD,
    AND, OR, XOR, SLL, SRL, // Shift Logical Left/Right
    
    // Memory (I-type / S-type)
    LW,  // Load Word
    SW,  // Store Word
    
    // Control Flow (B-type / J-type)
    BEQ, BNE, BLT, BGE, // Branches
    JAL, JALR,          // Jump and Link (for functions & switch)
    
    // System
    PRINT,
    ECALL, // For Print or Exit
    HALT   // Custom instruction to stop VM
};

struct Instruction {
    OpCode opcode;
    uint8_t rd;  // Destination Register (x0 - x31)
    uint8_t rs1; // Source Register 1
    uint8_t rs2; // Source Register 2
    int32_t imm; // Immediate value / Offset
    
    Instruction(OpCode op = OpCode::HALT, uint8_t d = 0, uint8_t s1 = 0, uint8_t s2 = 0, int32_t i = 0)
        : opcode(op), rd(d), rs1(s1), rs2(s2), imm(i) {}
};

// Ռեգիստրների անունները՝ հեշտ դեբագ անելու համար
const std::string RegNames[] = {
    "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

#endif