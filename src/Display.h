#pragma once

#include <SDL2/SDL.h>
#include <cstdint>
#include <vector>

class Display {
public:
    explicit Display(int width, int height, int pixelScale);
    ~Display();

    Display(const Display&)            = delete;
    Display& operator=(const Display&) = delete;

    bool invertPixel(int x, int y);
    void clear();
    void render();

    int width()  const { return m_width; }
    int height() const { return m_height; }

private:
    int m_width;
    int m_height;
    int m_scale;
    std::vector<bool> m_frameBuffer;

    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
};
