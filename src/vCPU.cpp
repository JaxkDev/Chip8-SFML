#include "vCPU.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <cstring>

vCPU::vCPU() :
    randGen(std::chrono::system_clock::now().time_since_epoch().count())
{
    randByte = std::uniform_int_distribution<uint8_t>(0, 255);

    // Load fonts manually into reserved memory space.
    // https://github.com/mattmikolay/chip-8/wiki/CHIP‐8-Technical-Reference#fonts
    const uint8_t fontset[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };
    for (unsigned int i = 0; i < 80; ++i) {
        memory[0x50 + i] = fontset[i]; // Stored from 0x50 to 0x9F.
    }

    // Load table of function pointers.
    table[0x0] = &vCPU::OP_NULL;
    table[0x1] = &vCPU::OP_1nnn;
    table[0x2] = &vCPU::OP_2nnn;
    table[0x3] = &vCPU::OP_3xnn;
    table[0x4] = &vCPU::OP_4xnn;
    table[0x5] = &vCPU::OP_5xy0;
    table[0x6] = &vCPU::OP_6xnn;
    table[0x7] = &vCPU::OP_7xnn;
    table[0x8] = &vCPU::OP_8xy0;
    table[0x9] = &vCPU::OP_9xy0;
    table[0xA] = &vCPU::OP_Annn;
    table[0xB] = &vCPU::OP_Bnnn;
    table[0xC] = &vCPU::OP_Cxnn;
    table[0xD] = &vCPU::OP_Dxyn;
    table[0xE] = &vCPU::OP_NULL;
    table[0xF] = &vCPU::OP_NULL;

    for(size_t i = 0; i <= 0xE; i++) {
        table0[i] = &vCPU::OP_NULL;
        table8[i] = &vCPU::OP_NULL;
        tableE[i] = &vCPU::OP_NULL;
    }

    table0[0xE] = &vCPU::OP_00EE;
    table0[0x0] = &vCPU::OP_00E0;

    table8[0x0] = &vCPU::OP_8xy0;
    table8[0x1] = &vCPU::OP_8xy1;
    table8[0x2] = &vCPU::OP_8xy2;
    table8[0x3] = &vCPU::OP_8xy3;
    table8[0x4] = &vCPU::OP_8xy4;
    table8[0x5] = &vCPU::OP_8xy5;
    table8[0x6] = &vCPU::OP_8xy6;
    table8[0x7] = &vCPU::OP_8xy7;
    table8[0xE] = &vCPU::OP_8xyE;

    tableE[0x1] = &vCPU::OP_ExA1;
    tableE[0xE] = &vCPU::OP_Ex9E;

    for(size_t i = 0; i <= 0x65; i++) {
        tableF[i] = &vCPU::OP_NULL;
    }
    tableF[0x07] = &vCPU::OP_Fx07;
    tableF[0x0A] = &vCPU::OP_Fx0A;
    tableF[0x15] = &vCPU::OP_Fx15;
    tableF[0x18] = &vCPU::OP_Fx18;
    tableF[0x1E] = &vCPU::OP_Fx1E;
    tableF[0x29] = &vCPU::OP_Fx29;
    tableF[0x33] = &vCPU::OP_Fx33;
    tableF[0x55] = &vCPU::OP_Fx55;
    tableF[0x65] = &vCPU::OP_Fx65;
}


void vCPU::loadROM(const char *filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (file.is_open()) {
        const std::streampos size = file.tellg();
        const auto buffer = new char[size];

        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();

        for (long i = 0; i < size; ++i) {
            this->memory[START_ADDRESS + i] = buffer[i];
        }

        delete[] buffer;
    } else {
        std::cerr << "Failed to open file: " << filename << std::endl;
        exit(1);
    }
}

// F-D-E Cycle
void vCPU::cycle() {
    //cycles at like 600Hz

    // Fetch
    opcode = memory[pc] << 8u | memory[pc + 1];
    pc += 2;

    // Decode and Execute
    (this->*table[(opcode & 0xF000u) >> 12u]) ();

    if (soundTimer > 0) --soundTimer;
    if (delayTimer > 0) --delayTimer;
}

// --- CPU Instructional Functions ---

void vCPU::OP_NULL() {
    // Do nothing.
}

void vCPU::OP_00E0() {
    // Clear the display.
    memset(video, 0, sizeof(video));
}

void vCPU::OP_00EE() {
    // Return from a subroutine.
    --sp;
    pc = stack[sp];
}

void vCPU::OP_1nnn() {
    // Jump to address nnn.
    pc = opcode & 0x0FFFu;
}

void vCPU::OP_2nnn() {
    // Execute subroutine starting at nnn.
    stack[sp] = pc;
    ++sp;
    pc = opcode & 0x0FFFu;
}

void vCPU::OP_3xnn() {
    // Skip next instruction if value of register VX == nn.
    const auto X = (opcode & 0x0F00u) >> 8u;
    const auto nn = opcode & 0x00FFu;

    if (registers[X] == nn) {
        pc += 2;
    }
}

void vCPU::OP_4xnn() {
    // Skip next instruction if value of register VX != nn.
    const auto X = (opcode & 0x0F00u) >> 8u;
    const auto nn = opcode & 0x00FFu;

    if (registers[X] != nn) {
        pc += 2;
    }
}

void vCPU::OP_5xy0() {
    // Skip next instruction if value of register VX == value of register VY.
    const auto X = (opcode & 0x0F00u) >> 8u;
    const auto Y = (opcode & 0x00F0u) >> 4u;

    if (registers[X] == registers[Y]) {
        pc += 2;
    }
}

void vCPU::OP_6xnn() {
    // Set value of register VX to nn.
    const auto X = (opcode & 0x0F00u) >> 8u;
    const auto nn = opcode & 0x00FFu;

    registers[X] = nn;
}

void vCPU::OP_7xnn() {
    // Add nn to value of register VX.
    const auto X = (opcode & 0x0F00u) >> 8u;
    const auto nn = opcode & 0x00FFu;

    registers[X] += nn;
}

void vCPU::OP_8xy0() {
    // Set value of register VX to value of register VY.
    const auto X = (opcode & 0x0F00u) >> 8u;
    const auto Y = (opcode & 0x00F0u) >> 4u;

    registers[X] = registers[Y];
}

void vCPU::OP_8xy1() {
    // Set value of register VX to (value of register VX OR value of register VY).
    const auto X = (opcode & 0x0F00u) >> 8u;
    const auto Y = (opcode & 0x00F0u) >> 4u;

    registers[X] |= registers[Y];
}

void vCPU::OP_8xy2() {
    // Set value of register VX to (value of register VX AND value of register VY).
    const auto X = (opcode & 0x0F00u) >> 8u;
    const auto Y = (opcode & 0x00F0u) >> 4u;

    registers[X] &= registers[Y];
}

void vCPU::OP_8xy3() {
    // Set value of register VX to (value of register VX XOR value of register VY).
    const auto X = (opcode & 0x0F00u) >> 8u;
    const auto Y = (opcode & 0x00F0u) >> 4u;

    registers[X] ^= registers[Y];
}

void vCPU::OP_8xy4() {
    // Add value of register VY to register VX. Set VF to 1 if there is a carry, 0 if not.
    const auto X = (opcode & 0x0F00u) >> 8u;
    const auto Y = (opcode & 0x00F0u) >> 4u;

    const auto sum = registers[X] + registers[Y];

    registers[0xF] = sum > 255U ? 1 : 0;
    registers[X] = sum;
}

void vCPU::OP_8xy5() {
    // Subtract value of register VY from register VX. Set VF to 0 if there is a borrow, 1 if not.
    const auto X = (opcode & 0x0F00u) >> 8u;
    const auto Y = (opcode & 0x00F0u) >> 4u;

    registers[0xF] = registers[X] > registers[Y] ? 1 : 0;
    registers[X] -= registers[Y];
}

void vCPU::OP_8xy6() {
    // Store the least significant bit of register VX in register VF, then shift VX to the right by 1.
    const auto X = (opcode & 0x0F00u) >> 8u;

    registers[0xF] = registers[X] & 0x1u;
    registers[X] >>= 1;
}

void vCPU::OP_8xy7() {
    // Set register VX to value of register VY minus register VX. Set VF to 0 if there is a borrow, 1 if not.
    const auto X = (opcode & 0x0F00u) >> 8u;
    const auto Y = (opcode & 0x00F0u) >> 4u;

    registers[0xF] = registers[Y] > registers[X] ? 1 : 0;
    registers[X] = registers[Y] - registers[X];
}

void vCPU::OP_8xyE() {
    // Store the most significant bit of register VX in register VF, then shift VX to the left by 1.
    const auto X = (opcode & 0x0F00u) >> 8u;

    registers[0xF] = (registers[X] & 0x80u) >> 7u;
    registers[X] <<= 1;
}

void vCPU::OP_9xy0() {
    // Skip next instruction if (value of register VX != value of register VY).
    const auto X = (opcode & 0x0F00u) >> 8u;
    const auto Y = (opcode & 0x00F0u) >> 4u;

    if (registers[X] != registers[Y]) {
        pc += 2;
    }
}

void vCPU::OP_Annn() {
    // Set index to address nnn.
    index = opcode & 0x0FFFu;
}

void vCPU::OP_Bnnn() {
    // Jump to address (nnn + value of register V0).
    pc = (opcode & 0x0FFFu) + registers[0];
}

void vCPU::OP_Cxnn() {
    // Set register VX to a random number with mask of nn.
    const auto X = (opcode & 0x0F00u) >> 8u;
    const auto nn = opcode & 0x00FFu;

    registers[X] = rand() & nn;
}

void vCPU::OP_Dxyn() {
    // Draw a sprite at position VX, VY with n bytes of sprite data starting at the address stored in index register.
    const auto X = registers[(opcode & 0x0F00u) >> 8u];
    const auto Y = registers[(opcode & 0x00F0u) >> 4u];
    const auto height = opcode & 0x000Fu;

    registers[0xF] = 0;

    for (unsigned int row = 0; row < height; ++row) {
        const auto sprite = memory[index + row];

        for (unsigned int col = 0; col < 8; ++col) {
            const auto pixel = sprite & (0x80u >> col);
            auto *screenPixel = &video[(Y + row) * 64 + (X + col)];

            if (pixel) {
                if (*screenPixel == 1) {
                    registers[0xF] = 1;
                }

                *screenPixel ^= 1;
            }
        }
    }
}

void vCPU::OP_Ex9E() {
    // Skip next instruction if key with the value of register VX is pressed.
    const auto X = (opcode & 0x0F00u) >> 8u;

    if (keypad[registers[X]] != 0) {
        pc += 2;
    }
}

void vCPU::OP_ExA1() {
    // Skip next instruction if key with the value of register VX is not pressed.
    const auto X = (opcode & 0x0F00u) >> 8u;

    if (keypad[registers[X]] == 0) {
        pc += 2;
    }
}

void vCPU::OP_Fx07() {
    // Set register VX to the value of the delay timer.
    const auto X = (opcode & 0x0F00u) >> 8u;

    registers[X] = delayTimer;
}

void vCPU::OP_Fx0A() {
    // Wait for a key press, store the value of the key in VX.
    const auto X = (opcode & 0x0F00u) >> 8u;

    for (unsigned int i = 0; i < 16; ++i) {
        if (keypad[i] != 0) {
            registers[X] = i;
            return;
        }
    }

    pc -= 2;
}

void vCPU::OP_Fx15() {
    // Set the delay timer to the value of register VX.
    const auto X = (opcode & 0x0F00u) >> 8u;

    delayTimer = registers[X];
}

void vCPU::OP_Fx18() {
    // Set the sound timer to the value of register VX.
    const auto X = (opcode & 0x0F00u) >> 8u;

    soundTimer = registers[X];
}

void vCPU::OP_Fx1E() {
    // Add the value of register VX to index.
    const auto X = (opcode & 0x0F00u) >> 8u;

    index += registers[X];
}

void vCPU::OP_Fx29() {
    // Set index to the location of the sprite for the character in register VX.
    const auto X = (opcode & 0x0F00u) >> 8u;
    index = registers[X] * 5;
}

void vCPU::OP_Fx33() {
    // Store the binary-coded decimal representation of the value of register VX at the addresses index, index+1, and index+2.
    const auto X = (opcode & 0x0F00u) >> 8u;
    const auto value = registers[X];

    memory[index] = value / 100;
    memory[index + 1] = (value / 10) % 10;
    memory[index + 2] = value % 10;
}

void vCPU::OP_Fx55() {
    // Store the values of registers V0 to VX inclusive in memory starting at the address in index register.
    const auto X = (opcode & 0x0F00u) >> 8u;

    for (unsigned int i = 0; i <= X; ++i) {
        memory[index + i] = registers[i];
    }
}

void vCPU::OP_Fx65() {
    // Fill registers V0 to VX inclusive with the values stored in memory starting at the address in index register.
    const auto X = (opcode & 0x0F00u) >> 8u;

    for (unsigned int i = 0; i <= X; ++i) {
        registers[i] = memory[index + i];
    }
}

// --- END ---
