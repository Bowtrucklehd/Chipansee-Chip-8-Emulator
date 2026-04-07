#include "Chip8.h"

#include <algorithm>
#include <fstream>
#include <stdexcept>

// Built-in font sprites (0x0 – 0xF), each character is 5 bytes.
// Stored at the conventional address 0x050 in CHIP-8 memory.
static constexpr uint16_t FONT_START_ADDRESS = 0x050;
static constexpr std::array<uint8_t, 80> FONT_DATA = {
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

Chip8::Chip8(Display& display, Input& input)
    : m_display(display), m_input(input)
{
    std::copy(FONT_DATA.begin(), FONT_DATA.end(), m_memory.begin() + FONT_START_ADDRESS);
}

void Chip8::loadROM(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file)
        throw std::runtime_error("Cannot open ROM: " + path);

    auto size = file.tellg();
    if (size > static_cast<std::streamsize>(MEMORY_SIZE - ROM_START))
        throw std::runtime_error("ROM too large to fit in CHIP-8 memory");

    file.seekg(0);
    file.read(reinterpret_cast<char*>(m_memory.data() + ROM_START), size);
}

void Chip8::cycle() {
    // TODO: Implement fetch-decode-execute
    // Fetch:   uint16_t opcode = (m_memory[m_pc] << 8) | m_memory[m_pc + 1]; m_pc += 2;
    // Decode:  switch on (opcode & 0xF000) >> 12
    // Execute: implement each opcode group
}
