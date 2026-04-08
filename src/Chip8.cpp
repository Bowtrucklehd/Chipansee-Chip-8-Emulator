#include "Chip8.h"
#include "Display.h"
#include "Input.h"
#include <SDL_keycode.h>
#include <cstdint>
#include <cstring>
#include <spdlog/spdlog.h>

Chip8::Chip8(Display& display, Input& input) : display(display), input(input) {
        uint8_t font[80] = {
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
        memcpy(memory + 0x50, font, sizeof(font));
}

void Chip8::cycle() {
        uint16_t opcode = fetch();
        Instruction instruction = decode(opcode);
        execute(instruction);
};

uint16_t Chip8::fetch() {
        uint16_t opcode = (uint16_t) memory[pc] << 8 | memory[pc+1];
        pc+=2;
        return opcode;
};

Instruction Chip8::decode(uint16_t opcode) {
        Instruction instruction;
        instruction.code = (uint8_t) ((opcode & 0xF000) >> 12);
        instruction.x = (uint8_t) ((opcode & 0x0F00) >> 8);
        instruction.y = (uint8_t) ((opcode & 0x00F0) >> 4);
        instruction.N = (uint8_t) (opcode & 0x000F);
        instruction.NN = (instruction.y << 4) | instruction.N;
        instruction.NNN = (uint16_t) (instruction.x << 8) | instruction.NN;
        pc+=2;
        return instruction;
};

bool Chip8::execute(Instruction instruction) {
        switch (instruction.code) {
                case 0x0:
                        break;
                case 0x1:
                        break;
                case 0x2:
                        break;
                case 0x3:
                        break;
                case 0x4:
                        break;
                case 0x5:
                        break;
                case 0x6:
                        break;
                case 0x7:
                        break;
                case 0x8:
                        break;
                case 0x9:
                        break;
                case 0xA:
                        break;
                case 0xB:
                        break;
                case 0xC:
                        break;
                case 0xD:
                        break;
                case 0xE:
                        break;
                case 0xF:
                        break;
                default:
                        //add logging of unknown instruction here
        }
}

