#pragma once

#include <cstdint>

class vCPU {
public:
    const unsigned int START_ADDRESS = 0x200;
    // Program counter starts at 0x200, as the first 512 bytes are reserved for the interpreter.

    vCPU();

    void loadROM(const char *filename);
    void cycle();

    uint8_t memory[4096]{}; // 4096 8-bit Memory (4KB).
    uint8_t registers[16]{}; // 16 8-bit Registers.
    uint16_t index = 0; // 1 16-bit Register.
    uint16_t pc = START_ADDRESS; // 1 16-bit Program Counter.
    uint16_t stack[16]{}; // 16 16-bit Call Stack.
    uint8_t sp = 0; // 1 8-bit Stack Pointer.
    uint8_t delayTimer = 0; // 1 8-bit Delay Timer.
    uint8_t soundTimer = 0; // 1 8-bit Sound Timer.
    uint8_t keypad[16]{}; // 16 8-bit Keypad.
    uint8_t video[64 * 32]{}; // 64x32 8-bit Video Memory.
    uint16_t opcode = 0; // 1 16-bit Opcode.
};
