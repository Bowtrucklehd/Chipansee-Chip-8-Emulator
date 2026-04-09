#pragma once

#include <SDL2/SDL.h>
#include <cstdint>
#include <vector>

class Display {
public:
    explicit Display(int maxWidth, int maxHeight, int pixelScale);
    ~Display();

    Display(const Display&)            = delete;
    Display& operator=(const Display&) = delete;

    bool invertPixel(int x, int y);
    void clear();
    void render();
    void setHighRes(bool enabled);

    void scrollUp(int n);
    void scrollDown(int n);
    void scrollLeft(int n);
    void scrollRight(int n);

    int  width()     const { return m_logicalWidth; }
    int  height()    const { return m_logicalHeight; }
    bool isHighRes() const { return m_highRes; }

private:
    int m_maxWidth;
    int m_maxHeight;
    int m_scale;
    int m_logicalWidth;
    int m_logicalHeight;
    bool m_highRes = false;
    std::vector<bool> m_frameBuffer;

    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
};
