#include "Chip8.h"
#include "Display.h"
#include "Input.h"
#include <SDL_keycode.h>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <spdlog/spdlog.h>
#include <stdexcept>

Chip8::Chip8(Display& display, Input& input, Chip8Config& config) : display(display), input(input), pc(0x200), config(config), key_was_down(false) {
        if (config.high_res_support)
                display.setHighRes(false);
        uint8_t chip_fontset[80] = {
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

        uint8_t super_chip_fontset[160] = {
                0xff, 0xff, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xff, // 0
                0x18, 0x78, 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0xff, 0xff, // 1
                0xff, 0xff, 0x03, 0x03, 0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff, // 2
                0xff, 0xff, 0x03, 0x03, 0xff, 0xff, 0x03, 0x03, 0xff, 0xff, // 3
                0xc3, 0xc3, 0xc3, 0xc3, 0xff, 0xff, 0x03, 0x03, 0x03, 0x03, // 4
                0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff, 0x03, 0x03, 0xff, 0xff, // 5
                0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff, 0xc3, 0xc3, 0xff, 0xff, // 6
                0xff, 0xff, 0x03, 0x03, 0x06, 0x0c, 0x18, 0x18, 0x18, 0x18, // 7
                0xff, 0xff, 0xc3, 0xc3, 0xff, 0xff, 0xc3, 0xc3, 0xff, 0xff, // 8
                0xff, 0xff, 0xc3, 0xc3, 0xff, 0xff, 0x03, 0x03, 0xff, 0xff, // 9
                0x7e, 0xff, 0xc3, 0xc3, 0xc3, 0xff, 0xff, 0xc3, 0xc3, 0xc3, // A
                0xfc, 0xfc, 0xc3, 0xc3, 0xfc, 0xfc, 0xc3, 0xc3, 0xfc, 0xfc, // B
                0x3c, 0xff, 0xc3, 0xc0, 0xc0, 0xc0, 0xc0, 0xc3, 0xff, 0x3c, // C
                0xfc, 0xfe, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xfe, 0xfc, // D
                0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff, // E
                0xff, 0xff, 0xc0, 0xc0, 0xff, 0xff, 0xc0, 0xc0, 0xc0, 0xc0, // F
        };

        memcpy(memory + 0x0, chip_fontset, sizeof(chip_fontset));
        memcpy(memory + 0x50, super_chip_fontset, sizeof(super_chip_fontset));
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

void Chip8::decrement_delay() {
        if(delay_timer>0) {
                delay_timer--;
        }
}

void Chip8::decrement_sound() {
        if(delay_timer>0) {
                sound_timer--;
        }
}

bool Chip8::get_draw_flag() {
        return draw_flag;
}

void Chip8::set_draw_flag(bool value) {
        draw_flag = value;
}

void Chip8::send_vertical_blank_interrupt() {
        vertical_blank_interrupt = true;
}

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
                case 0x0: op_0x0(instruction); break;
                case 0x1: op_0x1(instruction); break;
                case 0x2: op_0x2(instruction); break;
                case 0x3: op_0x3(instruction); break;
                case 0x4: op_0x4(instruction); break;
                case 0x5: op_0x5(instruction); break;
                case 0x6: op_0x6(instruction); break;
                case 0x7: op_0x7(instruction); break;
                case 0x8: op_0x8(instruction); break;
                case 0x9: op_0x9(instruction); break;
                case 0xA: op_0xA(instruction); break;
                case 0xB: op_0xB(instruction); break;
                case 0xC: op_0xC(instruction); break;
                case 0xD: return op_0xD(instruction);
                case 0xE: op_0xE(instruction); break;
                case 0xF: op_0xF(instruction); break;
                default:  spdlog::warn("unknown opcode: {:#06x}", instruction.full_opcode); break;
        }
        return true;
}

void Chip8::op_0x0(const Instruction& i) {
        switch (i.NN) {
                case 0xE0:

                if (config.display_wait_quirk) {
                        if (!vertical_blank_interrupt) {
                        spdlog::debug("[display_wait] draw stalled — waiting for vblank at pc={:#05x}", pc - 2);
                                pc -= 2;
                                return;
                        }
                        spdlog::debug("[display_wait] vblank received — proceeding with draw");
                        vertical_blank_interrupt = false;
                }
                        display.clear();
                        draw_flag = true;
                        break;
                case 0xEE:
                        if (stack.empty())
                                spdlog::warn("stack underflow at pc={:#05x}", pc);
                        else {
                                pc = stack.top();
                                stack.pop();
                        }
                        break;
                case 0xFE:
                        if (config.high_res_support) {
                                spdlog::debug("[high_res] switching to low-res (64x32)");
                                display.setHighRes(false);
                        }
                        break;
                case 0xFF:
                        if (config.high_res_support) {
                                spdlog::debug("[high_res] switching to high-res (128x64)");
                                display.setHighRes(true);
                        }
                        break;      
                case 0xFB:
                        if (config.display_wait_quirk && !display.isHighRes()) {
                                if (!vertical_blank_interrupt) {
                                        spdlog::debug("[display_wait] draw stalled — waiting for vblank at pc={:#05x}", pc - 2);
                                        pc -= 2;
                                        return;
                                }
                                spdlog::debug("[display_wait] vblank received — proceeding with draw");
                                vertical_blank_interrupt = false;
                        }
                        display.scrollRight(4);
                        draw_flag = true;
                        break;
                case 0xFC:
                        if (config.display_wait_quirk && !display.isHighRes()) {
                                if (!vertical_blank_interrupt) {
                                        spdlog::debug("[display_wait] draw stalled — waiting for vblank at pc={:#05x}", pc - 2);
                                        pc -= 2;
                                        return;
                                }
                                spdlog::debug("[display_wait] vblank received — proceeding with draw");
                                vertical_blank_interrupt = false;
                        }
                        display.scrollLeft(4);
                        draw_flag = true;
                        break;
                default:
                        if (i.y == 0xC) {
                                if (config.display_wait_quirk && !display.isHighRes()) {
                                        if (!vertical_blank_interrupt) {
                                                spdlog::debug("[display_wait] draw stalled — waiting for vblank at pc={:#05x}", pc - 2);
                                                pc -= 2;
                                                return;
                                        }
                                        spdlog::debug("[display_wait] vblank received — proceeding with draw");
                                        vertical_blank_interrupt = false;
                                }
                                display.scrollDown(i.N);
                                draw_flag = true;
                        } else if (i.y == 0xD) {
                                if (config.display_wait_quirk && !display.isHighRes()) {
                                        if (!vertical_blank_interrupt) {
                                                spdlog::debug("[display_wait] draw stalled — waiting for vblank at pc={:#05x}", pc - 2);
                                                pc -= 2;
                                                return;
                                        }
                                        spdlog::debug("[display_wait] vblank received — proceeding with draw");
                                        vertical_blank_interrupt = false;
                                }
                                display.scrollUp(i.N);
                                draw_flag = true;
                        } else {
                                spdlog::warn("unknown opcode: {:#06x}", i.full_opcode);
                        }
                        break;
        }
}

void Chip8::op_0x1(const Instruction& i) {
        pc = i.NNN;
}

void Chip8::op_0x2(const Instruction& i) {
        if (stack.size() >= 16)
                spdlog::warn("stack overflow at pc={:#05x}", pc);
        stack.push(pc);
        pc = i.NNN;
}

void Chip8::op_0x3(const Instruction& i) {
        if (variable_register[i.x] == i.NN)
                pc += 2;
}

void Chip8::op_0x4(const Instruction& i) {
        if (variable_register[i.x] != i.NN)
                pc += 2;
}

void Chip8::op_0x5(const Instruction& i) {
        if (variable_register[i.x] == variable_register[i.y])
                pc += 2;
}

void Chip8::op_0x6(const Instruction& i) {
        variable_register[i.x] = i.NN;
}

void Chip8::op_0x7(const Instruction& i) {
        variable_register[i.x] += i.NN;
}

void Chip8::op_0x8(const Instruction& i) {
        switch (i.N) {
                case 0x0:
                        variable_register[i.x] = variable_register[i.y];
                        break;
                case 0x1:
                        variable_register[i.x] |= variable_register[i.y];
                        if (config.vf_reset_quirk) {
                                spdlog::debug("[vf_reset] OR V{:X}={:#04x} — VF reset to 0", i.x, variable_register[i.x]);
                                variable_register[0xF] = 0;
                        }
                        break;
                case 0x2:
                        variable_register[i.x] &= variable_register[i.y];
                        if (config.vf_reset_quirk) {
                                spdlog::debug("[vf_reset] AND V{:X}={:#04x} — VF reset to 0", i.x, variable_register[i.x]);
                                variable_register[0xF] = 0;
                        }
                        break;
                case 0x3:
                        variable_register[i.x] ^= variable_register[i.y];
                        if (config.vf_reset_quirk) {
                                spdlog::debug("[vf_reset] XOR V{:X}={:#04x} — VF reset to 0", i.x, variable_register[i.x]);
                                variable_register[0xF] = 0;
                        }
                        break;
                case 0x4: {
                        uint8_t vx = variable_register[i.x];
                        uint8_t vy = variable_register[i.y];
                        variable_register[i.x] = vx + vy;
                        variable_register[0xF] = ((uint16_t)vx + vy > 0xFF) ? 1 : 0;
                        break;
                }
                case 0x5: {
                        uint8_t vx = variable_register[i.x];
                        uint8_t vy = variable_register[i.y];
                        variable_register[i.x] = vx - vy;
                        variable_register[0xF] = (vx >= vy) ? 1 : 0;
                        break;
                }
                case 0x6: {
                        if (!config.shifting_quirk) {
                                spdlog::debug("[shifting] SHR: V{:X}=V{:X}={:#04x} (copy source)", i.x, i.y, variable_register[i.y]);
                                variable_register[i.x] = variable_register[i.y];
                        } else {
                                spdlog::debug("[shifting] SHR: V{:X}={:#04x} (shift in-place)", i.x, variable_register[i.x]);
                        }
                        uint8_t lsb = variable_register[i.x] & 0x01;
                        variable_register[i.x] >>= 1;
                        variable_register[0xF] = lsb;
                        break;
                }
                case 0x7: {
                        uint8_t vx = variable_register[i.x];
                        uint8_t vy = variable_register[i.y];
                        variable_register[i.x] = vy - vx;
                        variable_register[0xF] = (vy >= vx) ? 1 : 0;
                        break;
                }
                case 0xE: {
                        if (!config.shifting_quirk) {
                                spdlog::debug("[shifting] SHL: V{:X}=V{:X}={:#04x} (copy source)", i.x, i.y, variable_register[i.y]);
                                variable_register[i.x] = variable_register[i.y];
                        } else {
                                spdlog::debug("[shifting] SHL: V{:X}={:#04x} (shift in-place)", i.x, variable_register[i.x]);
                        }
                        uint8_t msb = variable_register[i.x] >> 7;
                        variable_register[i.x] <<= 1;
                        variable_register[0xF] = msb;
                        break;
                }
                default:
                        spdlog::warn("unknown 8XYN opcode: {:#06x}", i.full_opcode);
                        break;
        }
}

void Chip8::op_0x9(const Instruction& i) {
        if (variable_register[i.x] != variable_register[i.y])
                pc += 2;
}

void Chip8::op_0xA(const Instruction& i) {
        index = i.NNN;
}

void Chip8::op_0xB(const Instruction& i) {
        if (!config.jumping_quirk) {
                spdlog::debug("[jumping] JUMP V0: pc={:#05x}+V0({:#04x})={:#05x}", i.NNN, variable_register[0x0], i.NNN + variable_register[0x0]);
                pc = i.NNN + variable_register[0x0];
        } else {
                spdlog::debug("[jumping] JUMP VX: pc={:#05x}+V{:X}({:#04x})={:#05x}", i.NNN, i.x, variable_register[i.x], i.NNN + variable_register[i.x]);
                pc = i.NNN + variable_register[i.x];
        }
}

void Chip8::op_0xC(const Instruction& i) {
        variable_register[i.x] = (rand() % 256) & i.NN;
}

bool Chip8::op_0xD(const Instruction& i) {
        if (config.display_wait_quirk) {
                if (!vertical_blank_interrupt) {
                        spdlog::debug("[display_wait] draw stalled — waiting for vblank at pc={:#05x}", pc - 2);
                        pc -= 2;
                        return true;
                }
                spdlog::debug("[display_wait] vblank received — proceeding with draw");
                vertical_blank_interrupt = false;
        }

        if(i.N == 0) {
                uint8_t x_start = variable_register[i.x] % display.width();
                uint8_t y_start = variable_register[i.y] % display.height();
        
                variable_register[0xF] = 0;
                for (int y_offset = 0; y_offset < 16; y_offset++) {
                        uint8_t first_sprite_byte = memory[index + y_offset * 2];
                        uint8_t second_sprite_byte = memory[index + y_offset * 2 + 1];
                        for (int x_offset = 0; x_offset < 8; x_offset++) {
                                if (!((first_sprite_byte >> (7 - x_offset)) & 0x01))
                                        continue;
                                int px = config.clipping_quirk ? x_start + x_offset : (x_start + x_offset) % display.width();
                                int py = config.clipping_quirk ? y_start + y_offset : (y_start + y_offset) % display.height();
        
                                if (!config.clipping_quirk && (x_start + x_offset >= display.width() || y_start + y_offset >= display.height()))
                                        spdlog::debug("[clipping] WRAP pixel ({},{}) -> ({},{})", x_start + x_offset, y_start + y_offset, px, py);
        
                                if (display.invertPixel(px, py))
                                        variable_register[0xF] = 1;
                        }

                        for (int x_offset = 0; x_offset < 8; x_offset++) {
                                if (!((second_sprite_byte >> (7 - x_offset)) & 0x01))
                                        continue;
                                int px = config.clipping_quirk ? x_start + x_offset + 8 : (x_start + x_offset + 8) % display.width();
                                int py = config.clipping_quirk ? y_start + y_offset : (y_start + y_offset) % display.height();
        
                                if (!config.clipping_quirk && (x_start + x_offset + 8 >= display.width() || y_start + y_offset >= display.height()))
                                        spdlog::debug("[clipping] WRAP pixel ({},{}) -> ({},{})", x_start + x_offset, y_start + y_offset, px, py);
        
                                if (display.invertPixel(px, py))
                                        variable_register[0xF] = 1;
                        }
                }
                draw_flag = true;
                return true;
        }

        uint8_t x_start = variable_register[i.x] % display.width();
        uint8_t y_start = variable_register[i.y] % display.height();

        variable_register[0xF] = 0;
        for (int y_offset = 0; y_offset < i.N; y_offset++) {
                uint8_t sprite_byte = memory[index + y_offset];
                for (int x_offset = 0; x_offset < 8; x_offset++) {
                        if (!((sprite_byte >> (7 - x_offset)) & 0x01))
                                continue;

                        int px = config.clipping_quirk ? x_start + x_offset : (x_start + x_offset) % display.width();
                        int py = config.clipping_quirk ? y_start + y_offset : (y_start + y_offset) % display.height();

                        if (!config.clipping_quirk && (x_start + x_offset >= display.width() || y_start + y_offset >= display.height()))
                                spdlog::debug("[clipping] WRAP pixel ({},{}) -> ({},{})", x_start + x_offset, y_start + y_offset, px, py);

                        if (display.invertPixel(px, py))
                                variable_register[0xF] = 1;
                }
        }
        draw_flag = true;
        return true;
}

void Chip8::op_0xE(const Instruction& i) {
        switch (i.NN) {
                case 0x9E:
                        if (input.isKeyDown(variable_register[i.x]))
                                pc += 2;
                        break;
                case 0xA1:
                        if (!input.isKeyDown(variable_register[i.x]))
                                pc += 2;
                        break;
                default:
                        spdlog::warn("unknown EXNN opcode: {:#06x}", i.full_opcode);
                        break;
        }
}

void Chip8::op_0xF(const Instruction& i) {
        switch (i.NN) {
                case 0x07:
                        variable_register[i.x] = delay_timer;
                        break;
                case 0x15:
                        delay_timer = variable_register[i.x];
                        break;
                case 0x18:
                        sound_timer = variable_register[i.x];
                        break;
                case 0x1E:
                        index += variable_register[i.x];
                        break;
                case 0x0A: {
                        int pressed_key = input.getPressedKey();
                        if (pressed_key == -1) {
                                if (key_was_down) {
                                        variable_register[i.x] = (uint8_t)pressed_key;
                                        key_was_down = false;
                                } else {
                                        pc -= 2;
                                }
                        } else {
                                key_was_down = true;
                                pc -= 2;
                        }
                        break;
                }
                case 0x29:
                        index = variable_register[i.x] * 5;
                        break;
                case 0x30:
                        index = 0x50 + variable_register[i.x] * 10;
                        break;     
                case 0x33:
                        memory[index]     = variable_register[i.x] / 100;
                        memory[index + 1] = (variable_register[i.x] / 10) % 10;
                        memory[index + 2] = variable_register[i.x] % 10;
                        break;
                case 0x55:
                        for (int r = 0; r <= i.x; r++)
                                memory[index + r] = variable_register[r];
                        if (config.memory_quirk) {
                                spdlog::debug("[memory] STORE: I advanced from {:#05x} to {:#05x}", index, index + i.x + 1);
                                index += i.x + 1;
                        } else {
                                spdlog::debug("[memory] STORE: I unchanged at {:#05x}", index);
                        }
                        break;
                case 0x65:
                        for (int r = 0; r <= i.x; r++)
                                variable_register[r] = memory[index + r];
                        if (config.memory_quirk) {
                                spdlog::debug("[memory] LOAD: I advanced from {:#05x} to {:#05x}", index, index + i.x + 1);
                                index += i.x + 1;
                        } else {
                                spdlog::debug("[memory] LOAD: I unchanged at {:#05x}", index);
                        }
                        break;
                case 0x75:
                        for (int r = 0; r <= i.x; r++)
                                memory[index + r] = variable_register[r];
                        if (config.memory_quirk) {
                                spdlog::debug("[memory] STORE: I advanced from {:#05x} to {:#05x}", index, index + i.x + 1);
                                index += i.x + 1;
                        } else {
                                spdlog::debug("[memory] STORE: I unchanged at {:#05x}", index);
                        }
                        break;
                case 0x85:
                        for (int r = 0; r <= i.x; r++)
                                variable_register[r] = memory[index + r];
                        if (config.memory_quirk) {
                                spdlog::debug("[memory] LOAD: I advanced from {:#05x} to {:#05x}", index, index + i.x + 1);
                                index += i.x + 1;
                        } else {
                                spdlog::debug("[memory] LOAD: I unchanged at {:#05x}", index);
                        }
                        break;
                default:
                        spdlog::warn("unknown FXNN opcode: {:#06x}", i.full_opcode);
                        break;
        }
}

