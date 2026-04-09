#include "Chip8.h"
#include "Display.h"
#include "Input.h"

#include <SDL2/SDL.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

static constexpr int PIXEL_SCALE      = 10;
static constexpr int CPU_HZ           = 500;
static constexpr int TIMER_HZ         = 60;
static constexpr int CYCLES_PER_FRAME = CPU_HZ / TIMER_HZ;

static Chip8Config ORIGINAL  = { .modern_shift = false, .modern_b_instruction = false, .modern_index_incrementation = false };
static Chip8Config CHIP48    = { .modern_shift = true,  .modern_b_instruction = true,  .modern_index_incrementation = true  };
static Chip8Config SUPERCHIP = { .modern_shift = true,  .modern_b_instruction = true,  .modern_index_incrementation = true  };

int main(int argc, char* argv[]) {
    spdlog::set_pattern("[%H:%M:%S.%e] [%^%-5l%$] %v");
    spdlog::set_level(spdlog::level::debug);

    if (argc < 2) {
        spdlog::error("Usage: chip8 <rom_path> [original|chip48|superchip]");
        return 1;
    }

    Chip8Config* config = &ORIGINAL;
    if (argc >= 3) {
        std::string mode = argv[2];
        if      (mode == "chip48")    config = &CHIP48;
        else if (mode == "superchip") config = &SUPERCHIP;
        else if (mode != "original") {
            spdlog::warn("Unknown mode '{}', defaulting to original", mode);
        }
    }
    spdlog::info("Mode: {}", argc >= 3 ? argv[2] : "original");

    try {
        Display display(PIXEL_SCALE);
        Input   input;
        Chip8   chip8(display, input, *config);

        chip8.loadRom(argv[1]);

        const uint32_t frameMs = 1000 / TIMER_HZ;

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
