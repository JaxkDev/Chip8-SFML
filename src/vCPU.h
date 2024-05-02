#pragma once

#include <cstdint>
#include <random>

// ReSharper disable CppMemberFunctionMayBeStatic
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

private:
    std::default_random_engine randGen;
    std::uniform_int_distribution<uint8_t> randByte;

    typedef void (vCPU::*opFunc)();
    opFunc table[0xF + 1]{};
    opFunc table0[0xE + 1]{};
    opFunc table8[0xE + 1]{};
    opFunc tableE[0xE + 1]{};
    opFunc tableF[0x65 + 1]{};

    void OP_NULL(); // Do nothing.
    void OP_00E0(); // Clear the display.
    void OP_00EE(); // Return from a subroutine.
    void OP_1nnn(); // Jump to address nnn.
    void OP_2nnn(); // Execute subroutine starting at nnn.
    void OP_3xnn(); // Skip next instruction if value of register VX == nn.
    void OP_4xnn(); // Skip next instruction if value of register VX != nn.
    void OP_5xy0(); // Skip next instruction if value of register VX == value of register VY.
    void OP_6xnn(); // Set value of register VX to nn.
    void OP_7xnn(); // Add nn to value of register VX.
    void OP_8xy0(); // Set value of register VX to value of register VY.
    void OP_8xy1(); // Set value of register VX to (value of register VX OR value of register VY).
    void OP_8xy2(); // Set value of register VX to (value of register VX AND value of register VY).
    void OP_8xy3(); // Set value of register VX to (value of register VX XOR value of register VY).
    void OP_8xy4(); // Add value of register VY to register VX. Set VF to 1 if there is a carry, 0 if not.
    void OP_8xy5(); // Subtract value of register VY from register VX. Set VF to 0 if there is a borrow, 1 if not.
    void OP_8xy6(); // Store the least significant bit of register VX in register VF, then shift VX to the right by 1.
    void OP_8xy7(); // Set register VX to value of register VY minus register VX. Set VF to 0 if there is a borrow, 1 if not.
    void OP_8xyE(); // Store the most significant bit of register VX in register VF, then shift VX to the left by 1.
    void OP_9xy0(); // Skip next instruction if (value of register VX != value of register VY).
    void OP_Annn(); // Set index to address nnn.
    void OP_Bnnn(); // Jump to address (nnn + value of register V0).
    void OP_Cxnn(); // Set register VX to a random number with mask of nn.
    void OP_Dxyn(); // Draw a sprite at position VX, VY with n bytes of sprite data starting at the address stored in index register.
    void OP_Ex9E(); // Skip next instruction if key with the value of register VX is pressed.
    void OP_ExA1(); // Skip next instruction if key with the value of register VX is not pressed.
    void OP_Fx07(); // Set register VX to the value of the delay timer.
    void OP_Fx0A(); // Wait for a key press, store the value of the key in register VX.
    void OP_Fx15(); // Set the delay timer to the value of register VX.
    void OP_Fx18(); // Set the sound timer to the value of register VX.
    void OP_Fx1E(); // Add the value of register VX to the index register.
    void OP_Fx29(); // Set the index register to the location of the sprite for the character in register VX.
    void OP_Fx33(); // Store the binary-coded decimal representation of the value of register VX at the addresses index, index+1, and index+2.
    void OP_Fx55(); // Store the values of registers V0 to VX inclusive in memory starting at the address in index register.
    void OP_Fx65(); // Fill registers V0 to VX inclusive with the values stored in memory starting at the address in index register.
};
