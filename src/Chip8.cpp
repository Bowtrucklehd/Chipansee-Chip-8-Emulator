#include "Chip8.h"
#include "Display.h"
#include "Input.h"
#include <SDL_keycode.h>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <spdlog/spdlog.h>
#include <stdexcept>

Chip8::Chip8(Display& display, Input& input, Chip8Config& config) : display(display), input(input), pc(0x200), config(config) {
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
                                        if (stack.empty())
                                                spdlog::warn("stack underflow at pc={:#05x}", pc);
                                        else {
                                                pc = stack.top();
                                                stack.pop();
                                        }
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
                        if (stack.size() >= 16)
                                spdlog::warn("stack overflow at pc={:#05x}", pc);
                        stack.push(pc);
                        pc = instruction.NNN;
                        break;
                case 0x3:
                        if(variable_register[instruction.x] == instruction.NN) {
                                pc += 2;
                        }
                        break;
                case 0x4:
                        if(variable_register[instruction.x] != instruction.NN) {
                                pc += 2;
                        }
                        break;
                case 0x5:
                        if(variable_register[instruction.x] == variable_register[instruction.y]) {
                                pc += 2;
                        }
                        break;
                case 0x6:
                        variable_register[instruction.x] = instruction.NN;
                        break;
                case 0x7:
                        variable_register[instruction.x] += instruction.NN;
                        break;
                case 0x8:
                        switch (instruction.N) {
                                case 0x0:
                                        variable_register[instruction.x] = variable_register[instruction.y];
                                        break;
                                case 0x1:
                                        variable_register[instruction.x] = variable_register[instruction.x] | variable_register[instruction.y];
                                        break;
                                case 0x2:
                                        variable_register[instruction.x] = variable_register[instruction.x] & variable_register[instruction.y];
                                        break;
                                
                                case 0x3:
                                        variable_register[instruction.x] = variable_register[instruction.x] ^ variable_register[instruction.y];
                                        break;
                                
                                case 0x4: {
                                        uint8_t vx = variable_register[instruction.x];
                                        uint8_t vy = variable_register[instruction.y];
                                        bool overflow = (uint16_t)vx + vy > 0xFF;
                                        variable_register[instruction.x] = vx + vy;
                                        variable_register[0xF] = overflow ? 1 : 0;
                                        break;
                                }
                                case 0x5: {
                                        uint8_t vx = variable_register[instruction.x];
                                        uint8_t vy = variable_register[instruction.y];
                                        bool noBorrow = vx >= vy;
                                        variable_register[instruction.x] = vx - vy;
                                        variable_register[0xF] = noBorrow ? 1 : 0;
                                        break;
                                }
                                case 0x6:  {      
                                        if(!config.modern_shift) {
                                                variable_register[instruction.x] = variable_register[instruction.y];
                                        }
                                        uint8_t least_significant_bit = variable_register[instruction.x] & 0x01;
                                        variable_register[instruction.x] >>= 1;
                                        variable_register[0xF] = least_significant_bit;
                                        spdlog::debug("lsb: {}", least_significant_bit);
                                        break;          
                                }          
                                case 0x7: {
                                        uint8_t vx = variable_register[instruction.x];
                                        uint8_t vy = variable_register[instruction.y];
                                        bool noBorrow = vy >= vx;
                                        variable_register[instruction.x] = vy - vx;
                                        variable_register[0xF] = noBorrow ? 1 : 0;
                                        break;
                                }
                                case 0xE: {
                                        if(!config.modern_shift) {
                                                variable_register[instruction.x] = variable_register[instruction.y];
                                        }
                                        uint8_t most_significant_bit = variable_register[instruction.x] >> 7;
                                        variable_register[instruction.x] <<= 1;
                                        variable_register[0xF] = most_significant_bit;
                                        break;
                                }
                                default:
                                        spdlog::warn("unknown 8XYN opcode: {:#06x}", instruction.full_opcode);
                                        break;
                        }
                        break;
                case 0x9:
                        if(variable_register[instruction.x] != variable_register[instruction.y]) {
                                pc += 2;
                        }
                        break;
                case 0xA:
                        index = instruction.NNN;
                        break;
                case 0xB:
                        if(!config.modern_b_instruction) {
                                pc = instruction.NNN + variable_register[0x0];
                        } else {
                                pc = instruction.NNN + variable_register[instruction.x];
                        }
                        break;
                case 0xC: {
                        uint8_t random = rand() % 256;
                        variable_register[instruction.x] = random & instruction.NN;
                        break;
                }
                case 0xD: {
                        uint16_t x_coordinate = variable_register[instruction.x] % (DISPLAY_WIDTH-1);
                        uint16_t y_coordinate = variable_register[instruction.y] % (DISPLAY_HEIGHT-1);
                        uint16_t height = instruction.N;
                        uint16_t sprite_adress = index;

                        variable_register[0xF] = 0;
                        for(int y_offset = 0; y_offset<height; y_offset++) {
                                uint8_t sprite_byte = memory[sprite_adress + y_offset];
                                for(int x_offset = 0; x_offset < 8; x_offset++) {
                                        bool pixel = (sprite_byte >> (7 - x_offset)) & 0x01;  // ✅ isolate the bit
                                        if (pixel == 0) {
                                                continue;
                                        }

                                        bool collision = display.invertPixel(x_coordinate + x_offset, y_coordinate + y_offset);

                                        if(collision) {
                                                variable_register[0xF] = collision;
                                        }
                                }
                        }

                        break;
                }
                case 0xE:
                        switch (instruction.NN) {
                                case 0x9E:
                                        if(input.isKeyDown(variable_register[instruction.x])) {
                                                pc+=2;
                                        }
                                        break;
                                case 0xA1:
                                        if(!input.isKeyDown(variable_register[instruction.x])) {
                                                pc+=2;
                                        }
                                        break;
                                default:
                                        spdlog::warn("unknown EXNN opcode: {:#06x}", instruction.full_opcode);
                                        break;
                        }
                        break;
                case 0xF:
                        switch (instruction.NN) {
                                case 0x07:
                                        variable_register[instruction.x] = delay_timer;
                                        break;
                                case 0x15:
                                        delay_timer = variable_register[instruction.x];
                                        break;
                                case 0x18:
                                        sound_timer = variable_register[instruction.x];
                                        break;
                                case 0x1E:
                                        index += variable_register[instruction.x];
                                        break;
                                case 0x0A: {
                                        int pressed_key = input.getPressedKey();
                                        if(pressed_key == -1) {
                                                pc-=2;
                                        } else{
                                                variable_register[instruction.x] = (uint8_t) pressed_key;
                                        }
                                        break;
                                }
                                case 0x29:
                                        index = 0x050 + (variable_register[instruction.x] & 0x0F) * 5;
                                        break;
                                case 0x33:
                                        memory[index] = variable_register[instruction.x]/100;
                                        memory[index+1] = (variable_register[instruction.x]/10)%10;
                                        memory[index+2] = variable_register[instruction.x]%10;
                                        break;
                                case 0x55:
                                        for(int i = 0; i <= instruction.x; i++) {
                                                memory[index + i] = variable_register[i];
                                        }
                                        if(!config.modern_index_incrementation) {
                                                index += instruction.x + 1;
                                        }
                                        break;
                                case 0x65:
                                        for(int i = 0; i <= instruction.x; i++) {
                                                variable_register[i] = memory[index + i];
                                        }
                                        if(!config.modern_index_incrementation) {
                                                index += instruction.x + 1;
                                        }
                                        break;
                                default:
                                        spdlog::warn("unknown FXNN opcode: {:#06x}", instruction.full_opcode);
                                        break;
                        }
                        break;
                default:
                        spdlog::warn("unknown opcode: {:#06x}", instruction.full_opcode);
        }
        return true;
}

