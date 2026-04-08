#include "Display.h"
#include <SDL2/SDL.h>
#include <cassert>
#include <spdlog/spdlog.h>

int main() {
    spdlog::set_pattern("[%H:%M:%S.%e] [%^%-5l%$] %v");

    Display display(10);

    // out-of-bounds returns false without drawing
    assert(display.invertPixel(64, 0)  == false);
    assert(display.invertPixel(0, 32)  == false);
    assert(display.invertPixel(99, 99) == false);
    spdlog::info("PASS: out-of-bounds returns false");

    // first invert turns pixel on, returns false (no collision)
    assert(display.invertPixel(0, 0) == false);
    spdlog::info("PASS: first invertPixel returns false");

    // second invert turns pixel off, returns true (was on = collision)
    assert(display.invertPixel(0, 0) == true);
    spdlog::info("PASS: second invertPixel returns true (collision)");

    // clear zeroes everything
    display.invertPixel(10, 10);
    display.invertPixel(20, 5);
    display.clear();
    // after clear, first invert should again return false
    assert(display.invertPixel(10, 10) == false);
    assert(display.invertPixel(20,  5) == false);
    spdlog::info("PASS: clear() resets all pixels");

    // visual check — draw a circle using midpoint circle algorithm
    display.clear();
    int cx = DISPLAY_WIDTH / 2, cy = DISPLAY_HEIGHT / 2, r = 12;
    int x = 0, y = r, d = 1 - r;
    auto plot = [&](int px, int py) { display.invertPixel(cx + px, cy + py); };
    while (x <= y) {
        plot( x,  y); if (x != 0) plot(-x,  y);
        plot( x, -y); if (x != 0) plot(-x, -y);
        if (x != y) {
            plot( y,  x); plot(-y,  x);
            if (x != 0) { plot( y, -x); plot(-y, -x); }
        }
        d < 0 ? d += 2 * x + 3 : (d += 2 * (x - y--) + 5);
        ++x;
    }

    display.render();
    spdlog::info("Visual check: circle drawn. Window will close in 2 seconds.");

    Uint32 deadline = SDL_GetTicks() + 2000;
    SDL_Event event;
    while (SDL_GetTicks() < deadline)
        SDL_PollEvent(&event);

    spdlog::info("All display tests passed.");
    return 0;
}
