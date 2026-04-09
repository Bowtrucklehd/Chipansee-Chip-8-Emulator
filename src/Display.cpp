#include "Display.h"

#include <stdexcept>
#include <string>

Display::Display(int maxWidth, int maxHeight, int pixelScale)
    : m_maxWidth(maxWidth), m_maxHeight(maxHeight), m_scale(pixelScale),
      m_logicalWidth(maxWidth), m_logicalHeight(maxHeight),
      m_frameBuffer(maxWidth * maxHeight, false) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());

    m_window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        m_maxWidth  * m_scale,
        m_maxHeight * m_scale,
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

void Display::setHighRes(bool enabled) {
    m_highRes       = enabled;
    m_logicalWidth  = enabled ? m_maxWidth  : m_maxWidth  / 2;
    m_logicalHeight = enabled ? m_maxHeight : m_maxHeight / 2;
    m_frameBuffer.assign(m_logicalWidth * m_logicalHeight, false);
}

bool Display::invertPixel(int x, int y) {
    if (x < 0 || y < 0 || x >= m_logicalWidth || y >= m_logicalHeight)
        return false;
    bool current = m_frameBuffer[y * m_logicalWidth + x];
    m_frameBuffer[y * m_logicalWidth + x] = !current;
    return current;
}

void Display::clear() {
    std::fill(m_frameBuffer.begin(), m_frameBuffer.end(), false);
}

void Display::scrollUp(int n) {
    for (int y = 0; y < m_logicalHeight; ++y) {
        for (int x = 0; x < m_logicalWidth; ++x) {
            int src = y + n;
            m_frameBuffer[y * m_logicalWidth + x] = (src < m_logicalHeight)
                ? m_frameBuffer[src * m_logicalWidth + x]
                : false;
        }
    }
}

void Display::scrollDown(int n) {
    for (int y = m_logicalHeight - 1; y >= 0; --y) {
        for (int x = 0; x < m_logicalWidth; ++x) {
            int src = y - n;
            m_frameBuffer[y * m_logicalWidth + x] = (src >= 0)
                ? m_frameBuffer[src * m_logicalWidth + x]
                : false;
        }
    }
}

void Display::scrollLeft(int n) {
    for (int y = 0; y < m_logicalHeight; ++y) {
        for (int x = 0; x < m_logicalWidth; ++x) {
            int src = x + n;
            m_frameBuffer[y * m_logicalWidth + x] = (src < m_logicalWidth)
                ? m_frameBuffer[y * m_logicalWidth + src]
                : false;
        }
    }
}

void Display::scrollRight(int n) {
    for (int y = 0; y < m_logicalHeight; ++y) {
        for (int x = m_logicalWidth - 1; x >= 0; --x) {
            int src = x - n;
            m_frameBuffer[y * m_logicalWidth + x] = (src >= 0)
                ? m_frameBuffer[y * m_logicalWidth + src]
                : false;
        }
    }
}

void Display::render() {
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);

    // pixelSize scales logical pixels up to fill the fixed physical window
    int pixelSize = (m_maxWidth * m_scale) / m_logicalWidth;

    for (int y = 0; y < m_logicalHeight; ++y) {
        for (int x = 0; x < m_logicalWidth; ++x) {
            if (!m_frameBuffer[y * m_logicalWidth + x])
                continue;

            SDL_Rect pixel{ x * pixelSize, y * pixelSize, pixelSize, pixelSize };
            SDL_RenderFillRect(m_renderer, &pixel);
        }
    }

    SDL_RenderPresent(m_renderer);
}
