#pragma once

#include "Display.h"
#include "Input.h"

#include <array>
#include <cstdint>
#include <string>

static constexpr int MEMORY_SIZE     = 4096;
static constexpr int REGISTER_COUNT  = 16;
static constexpr int STACK_SIZE      = 16;
static constexpr int ROM_START       = 0x200;

class Chip8 {
public:
    Chip8(Display& display, Input& input);

    void loadROM(const std::string& path);
    void cycle();

    uint8_t delayTimer = 0;
    uint8_t soundTimer = 0;

private:
    Display& m_display;
    Input&   m_input;

    std::array<uint8_t,  MEMORY_SIZE>    m_memory{};
    std::array<uint8_t,  REGISTER_COUNT> m_registers{};
    std::array<uint16_t, STACK_SIZE>     m_stack{};

    uint16_t m_pc  = ROM_START;
    uint16_t m_I   = 0;
    uint8_t  m_sp  = 0;
};
