#pragma once

#include <SDL2/SDL.h>
#include <array>
#include <cstdint>

static constexpr int DISPLAY_WIDTH  = 64;
static constexpr int DISPLAY_HEIGHT = 32;

class Display {
public:
    explicit Display(int pixelScale);
    ~Display();

    Display(const Display&)            = delete;
    Display& operator=(const Display&) = delete;

    bool invertPixel(int x, int y);
    void clear();
    void render();

private:
    int m_scale;
    std::array<bool, DISPLAY_WIDTH * DISPLAY_HEIGHT> m_frameBuffer{};

    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
};
