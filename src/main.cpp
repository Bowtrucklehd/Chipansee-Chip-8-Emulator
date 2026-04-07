#include "Chip8.h"
#include "Display.h"
#include "Input.h"

#include <SDL2/SDL.h>
#include <iostream>
#include <stdexcept>

static constexpr int PIXEL_SCALE    = 10;
static constexpr int CPU_HZ         = 500;
static constexpr int TIMER_HZ       = 60;
static constexpr int CYCLES_PER_FRAME = CPU_HZ / TIMER_HZ;  // ~8 cycles per frame

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: chip8 <rom_path>\n";
        return 1;
    }

    try {
        Display display(PIXEL_SCALE);
        Input   input;
        Chip8   chip8(display, input);

        chip8.loadROM(argv[1]);

        const uint32_t frameMs = 1000 / TIMER_HZ;

        while (true) {
            uint32_t frameStart = SDL_GetTicks();

            if (!input.pollEvents())
                break;

            for (int i = 0; i < CYCLES_PER_FRAME; ++i)
                chip8.cycle();

            if (chip8.delayTimer > 0) --chip8.delayTimer;
            if (chip8.soundTimer > 0) --chip8.soundTimer;

            display.render();

            uint32_t elapsed = SDL_GetTicks() - frameStart;
            if (elapsed < frameMs)
                SDL_Delay(frameMs - elapsed);
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
