#include "Chip8.h"
#include "Display.h"
#include "Input.h"

#include <SDL2/SDL.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

static constexpr int PIXEL_SCALE    = 10;
static constexpr int CPU_HZ         = 500;
static constexpr int TIMER_HZ       = 60;
static constexpr int CYCLES_PER_FRAME = CPU_HZ / TIMER_HZ;  // ~8 cycles per frame

int main(int argc, char* argv[]) {
    spdlog::set_pattern("[%H:%M:%S.%e] [%^%-5l%$] %v");
    spdlog::set_level(spdlog::level::debug);

    if (argc < 2) {
        spdlog::error("Usage: chip8 <rom_path>");
        return 1;
    }

    try {
        Display display(PIXEL_SCALE);
        Input   input;
        Chip8   chip8(display, input);

        chip8.loadRom(argv[1]);

        const uint32_t frameMs = 1000;//1000 / TIMER_HZ;

        while (true) {
            uint32_t frameStart = SDL_GetTicks();

            if (!input.pollEvents())
                break;

            for (int i = 0; i < CYCLES_PER_FRAME; ++i)
                chip8.cycle();
            
            display.render();

            uint32_t elapsed = SDL_GetTicks() - frameStart;
            if (elapsed < frameMs)
                SDL_Delay(frameMs - elapsed);
        }

    } catch (const std::exception& e) {
        spdlog::error("{}", e.what());
        return 1;
    }

    return 0;
}
