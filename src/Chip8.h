#pragma once

#include "Display.h"
#include "Input.h"
#include <cstdint>
#include <random>
#include <stack>
#include <string>


struct Chip8Config {
        int display_width;
        int display_height;
        int cycles_per_second;
        int frames_per_second;

        bool high_res_support;
        bool vf_reset_quirk;
        bool memory_quirk;
        bool display_wait_quirk;
        bool clipping_quirk;
        bool shifting_quirk;
        bool jumping_quirk;
};

struct Instruction {
        uint16_t full_opcode;
        uint8_t  code;
        uint8_t  x;
        uint8_t  y;
        uint8_t  N;
        uint8_t  NN;
        uint16_t NNN;
};

class Chip8 {

        public:
                Chip8(Display& display, Input& input, Chip8Config& config);

                void loadRom(const std::string& path);
                void cycle();

                void decrementDelay();
                void decrementSound();

                bool getDrawFlag() const;
                void setDrawFlag(bool value);
                void sendVerticalBlankInterrupt();

                bool    isMenuDone() const;
                uint8_t getMemoryByte(uint16_t addr) const;

        private:
                uint16_t    fetch();
                Instruction decode(uint16_t opcode);
                void        execute(Instruction instruction);

                bool tryConsumeVBlank(bool lowResOnly = false);
                void storeRegisters(uint8_t upToRegister);
                void loadRegisters(uint8_t upToRegister);

                void op_0x0(const Instruction& i);
                void op_0x1(const Instruction& i);
                void op_0x2(const Instruction& i);
                void op_0x3(const Instruction& i);
                void op_0x4(const Instruction& i);
                void op_0x5(const Instruction& i);
                void op_0x6(const Instruction& i);
                void op_0x7(const Instruction& i);
                void op_0x8(const Instruction& i);
                void op_0x9(const Instruction& i);
                void op_0xA(const Instruction& i);
                void op_0xB(const Instruction& i);
                void op_0xC(const Instruction& i);
                void op_0xD(const Instruction& i);
                void op_0xE(const Instruction& i);
                void op_0xF(const Instruction& i);

                Display&     display;
                Input&       input;
                Chip8Config& config;

                uint8_t              memory[4096]          = {};
                uint16_t             index                 = 0;
                uint16_t             pc;
                uint8_t              variable_register[16] = {};
                std::stack<uint16_t> stack;
                uint8_t              delay_timer           = 0;
                uint8_t              sound_timer           = 0;

                bool    key_was_down     = false;
                uint8_t last_pressed_key = 0;

                bool draw_flag                = false;
                bool vertical_blank_interrupt = false;
                bool menu_selection_made      = false;

                std::mt19937 rng{std::random_device{}()};
};
