#pragma once

#include "Display.h"
#include "Input.h"
#include <SDL_stdinc.h>
#include <cstdint>
#include <stack>
#include <string>


struct Chip8Config {
        int display_width;
        int display_height;
        int cycles_per_second;
        int frames_per_second;

        bool vf_reset_quirk;
        bool memory_quirk;
        bool display_wait_quirk;
        bool clipping_quirk;
        bool shifting_quirk;
        bool jumping_quirk;
};

struct Instruction {
        uint16_t full_opcode; // full opcode
        uint8_t code; // first nibble
        uint8_t x; // second nibble
        uint8_t y; // third nibble
        uint8_t N; // fourth nibble
        uint8_t NN; // second byte
        uint16_t NNN; // second, third and fourth nibble
};

class Chip8 {

        public:
                Chip8(Display& display, Input& input, Chip8Config& config);
                ~Chip8();

                void loadRom(const std::string& path);
                void cycle();

                void decrement_delay();
                void decrement_sound();

                bool get_draw_flag();
                void set_draw_flag(bool value);
                void send_vertical_blank_interrupt();
        private:
                uint16_t fetch();
                Instruction decode(uint16_t opcode);
                bool execute(Instruction instruction);

                Display& display;
                Input& input;
                Chip8Config& config;

                uint8_t memory[4096];
                uint16_t index;
                uint16_t pc;
                uint8_t variable_register[16];
                std::stack<uint16_t> stack;
                uint8_t delay_timer;
                uint8_t sound_timer;

                bool key_was_down;

                bool draw_flag;
                bool vertical_blank_interrupt;
};