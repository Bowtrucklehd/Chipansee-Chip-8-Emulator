#pragma once

#include <SDL2/SDL.h>
#include <array>
#include <cstdint>

static constexpr int CHIP8_KEY_COUNT = 16;

class Input {
public:
    Input();

    bool pollEvents();
    bool isKeyDown(uint8_t chip8Key) const;
    int getPressedKey() const;

private:
    std::array<bool, CHIP8_KEY_COUNT> m_keyState{};

    static int chip8KeyToIndex(SDL_Scancode scancode);
};
