#include "Display.h"

#include <stdexcept>
#include <string>

Display::Display(int width, int height, int pixelScale)
    : m_width(width), m_height(height), m_scale(pixelScale),
      m_frameBuffer(width * height, false) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());

    m_window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        m_width  * m_scale,
        m_height * m_scale,
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

bool Display::invertPixel(int x, int y) {
    if (x < 0 || y < 0 || x >= m_width || y >= m_height)
        return false;
    bool current = m_frameBuffer[y * m_width + x];
    m_frameBuffer[y * m_width + x] = !current;
    return current;
}

void Display::clear() {
    std::fill(m_frameBuffer.begin(), m_frameBuffer.end(), false);
}

void Display::render() {
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);

    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            if (!m_frameBuffer[y * m_width + x])
                continue;

            SDL_Rect pixel{x * m_scale, y * m_scale, m_scale, m_scale};
            SDL_RenderFillRect(m_renderer, &pixel);
        }
    }

    SDL_RenderPresent(m_renderer);
}
