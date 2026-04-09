#include "Input.h"

// Standard CHIP-8 keypad layout mapped to keyboard:
//
//  Chip-8   Keyboard
//  1 2 3 C  ->  1 2 3 4
//  4 5 6 D  ->  Q W E R
//  7 8 9 E  ->  A S D F
//  A 0 B F  ->  Z X C V

Input::Input() {
    m_keyState.fill(false);
}

bool Input::pollEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
            return false;

        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            int index = chip8KeyToIndex(event.key.keysym.scancode);
            if (index >= 0)
                m_keyState[index] = (event.type == SDL_KEYDOWN);
        }
    }
    return true;
}

bool Input::isKeyDown(uint8_t chip8Key) const {
    if (chip8Key >= CHIP8_KEY_COUNT)
        return false;
    return m_keyState[chip8Key];
}

int Input::chip8KeyToIndex(SDL_Scancode scancode) {
    switch (scancode) {
        case SDL_SCANCODE_1: return 0x1;
        case SDL_SCANCODE_2: return 0x2;
        case SDL_SCANCODE_3: return 0x3;
        case SDL_SCANCODE_4: return 0xC;
        case SDL_SCANCODE_Q: return 0x4;
        case SDL_SCANCODE_W: return 0x5;
        case SDL_SCANCODE_E: return 0x6;
        case SDL_SCANCODE_R: return 0xD;
        case SDL_SCANCODE_A: return 0x7;
        case SDL_SCANCODE_S: return 0x8;
        case SDL_SCANCODE_D: return 0x9;
        case SDL_SCANCODE_F: return 0xE;
        case SDL_SCANCODE_Z: return 0xA;
        case SDL_SCANCODE_X: return 0x0;
        case SDL_SCANCODE_C: return 0xB;
        case SDL_SCANCODE_V: return 0xF;
        default:             return -1;
    }
}

int Input::getPressedKey() const {
    for (int key = 0; key < CHIP8_KEY_COUNT; ++key) {
        if (m_keyState[key])
            return key;
    }
    return -1;
}
