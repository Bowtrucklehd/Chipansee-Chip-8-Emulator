#include "Display.h"

#include <stdexcept>
#include <string>

Display::Display(int pixelScale) : m_scale(pixelScale) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());

    m_window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        DISPLAY_WIDTH  * m_scale,
        DISPLAY_HEIGHT * m_scale,
        SDL_WINDOW_SHOWN
    );
    if (!m_window)
        throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_renderer)
        throw std::runtime_error(std::string("SDL_CreateRenderer failed: ") + SDL_GetError());
}

Display::~Display() {
    if (m_renderer) SDL_DestroyRenderer(m_renderer);
    if (m_window)   SDL_DestroyWindow(m_window);
    SDL_Quit();
}

void Display::setPixel(int x, int y, bool on) {
    x %= DISPLAY_WIDTH;
    y %= DISPLAY_HEIGHT;
    m_frameBuffer[y * DISPLAY_WIDTH + x] = on;
}

bool Display::getPixel(int x, int y) const {
    x %= DISPLAY_WIDTH;
    y %= DISPLAY_HEIGHT;
    return m_frameBuffer[y * DISPLAY_WIDTH + x];
}

void Display::clear() {
    m_frameBuffer.fill(false);
}

void Display::render() {
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);

    for (int y = 0; y < DISPLAY_HEIGHT; ++y) {
        for (int x = 0; x < DISPLAY_WIDTH; ++x) {
            if (!m_frameBuffer[y * DISPLAY_WIDTH + x])
                continue;

            SDL_Rect pixel{x * m_scale, y * m_scale, m_scale, m_scale};
            SDL_RenderFillRect(m_renderer, &pixel);
        }
    }

    SDL_RenderPresent(m_renderer);
}
