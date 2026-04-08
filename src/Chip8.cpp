#include "Chip8.h"
#include "Display.h"
#include "Input.h"
#include <SDL_keycode.h>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <spdlog/spdlog.h>
#include <stdexcept>

Chip8::Chip8(Display& display, Input& input) : display(display), input(input), pc(0x200) {
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

Chip8::~Chip8() {}

void Chip8::loadRom(const std::string& path) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file)
                throw std::runtime_error("Cannot open ROM: " + path);

        auto size = file.tellg();
        if (size > static_cast<std::streamsize>(sizeof(memory) - 0x200))
                throw std::runtime_error("ROM too large to fit in memory");

        file.seekg(0);
        file.read(reinterpret_cast<char*>(memory + 0x200), size);
        spdlog::info("ROM loaded: {} ({} bytes)", path, static_cast<int>(size));
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
}

Instruction Chip8::decode(uint16_t opcode) {
        Instruction instruction;
        instruction.full_opcode = opcode;
        instruction.code = (uint8_t) ((opcode & 0xF000) >> 12);
        instruction.x = (uint8_t) ((opcode & 0x0F00) >> 8);
        instruction.y = (uint8_t) ((opcode & 0x00F0) >> 4);
        instruction.N = (uint8_t) (opcode & 0x000F);
        instruction.NN = (uint8_t)(opcode & 0x00FF);
        instruction.NNN = (uint16_t) (opcode & 0x0FFF);
        return instruction;
};

bool Chip8::execute(Instruction instruction) {
        switch (instruction.code) {
                case 0x0:
                        switch(instruction.NN) {
                                case 0xE0:
                                        display.clear();
                                        break;
                                case 0xEE:
                                        pc = stack.top();
                                        stack.pop();
                                        break;
                                default:
                                        spdlog::warn("unknown opcode: {:#06x}", instruction.full_opcode);
                                        break;
                }
                        break;
                case 0x1:
                        pc = instruction.NNN;
                        break;
                case 0x2:
                        stack.push(pc);
                        pc = instruction.NNN;
                        break;
                case 0x3:
                        if(variable_register[instruction.x] == instruction.NN) {
                                pc += 2;
                        }
                        break;
                case 0x4:
                        break;
                case 0x5:
                        break;
                case 0x6:
                        variable_register[instruction.x] = instruction.NN;
                        break;
                case 0x7:
                        variable_register[instruction.x] += instruction.NN;
                        break;
                case 0x8:
                        break;
                case 0x9:
                        break;
                case 0xA:
                        index = instruction.NNN;
                        break;
                case 0xB:
                        break;
                case 0xC:
                        break;
                case 0xD: {
                        uint16_t x_coordinate = variable_register[instruction.x] % (DISPLAY_WIDTH-1);
                        uint16_t y_coordinate = variable_register[instruction.y] % (DISPLAY_HEIGHT-1);
                        uint16_t height = instruction.N;
                        uint16_t sprite_adress = index;

                        spdlog::debug("DRAW sprite at ({},{}) height={} sprite_addr={:#05x} VF_before={}",
                                x_coordinate, y_coordinate, height, sprite_adress, variable_register[0xF]);

                        variable_register[0xF] = 0;
                        for(int y_offset = 0; y_offset<height; y_offset++) {
                                uint8_t sprite_byte = memory[sprite_adress + y_offset];
                                for(int x_offset = 0; x_offset < 8; x_offset++) {
                                        bool pixel = (sprite_byte >> (7 - x_offset)) & 0x01;  // ✅ isolate the bit
                                        if (pixel == 0) {
                                                spdlog::debug("  pixel ({},{}) skipped (sprite byte=0)", x_coordinate + x_offset, y_coordinate + y_offset);
                                                continue;
                                        }

                                        bool collision = display.invertPixel(x_coordinate + x_offset, y_coordinate + y_offset);
                                        spdlog::debug("  pixel ({},{}) drawn sprite_byte={:#04x} collision={}",
                                                x_coordinate + x_offset, y_coordinate + y_offset, sprite_byte, collision);

                                        if(collision) {
                                                variable_register[0xF] = collision;
                                        }
                                }
                        }

                        spdlog::debug("DRAW done VF={}", variable_register[0xF]);
                        break;
                }
                case 0xE:
                        break;
                case 0xF:
                        break;
                default:
                        //add logging of unknown instruction here
        }
        return true;
}

