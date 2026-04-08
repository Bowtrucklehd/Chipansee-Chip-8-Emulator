#pragma once

#include "Display.h"
#include "Input.h"
#include <SDL_stdinc.h>
#include <cstdint>
#include <stack>
#include <string>

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
                Chip8(Display& display, Input& input);
                ~Chip8();

                void loadRom(const std::string& path);
                void cycle();

                void decrement_delay();
                void decrement_sound();
        private:
                uint16_t fetch();
                Instruction decode(uint16_t opcode);
                bool execute(Instruction instruction);

                Display& display;
                Input& input;

                uint8_t memory[4096];
                uint16_t index;
                uint16_t pc;
                uint16_t variable_register[16];
                std::stack<uint16_t> stack;
                uint8_t delay_timer;
                uint8_t sound_timer;
};