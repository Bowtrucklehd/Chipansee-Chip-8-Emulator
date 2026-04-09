#include "Chip8.h"
#include "Display.h"
#include "Input.h"

#include <SDL2/SDL.h>
#include <chrono>
#include <spdlog/spdlog.h>
#include <stdexcept>

static constexpr int         PIXEL_SCALE              = 10;
static constexpr int         MICROSECONDS_IN_A_SECOND = 1000000;
static const     std::string MENU_ROM_PATH             = "assets/roms/main_menu.ch8";

static Chip8Config ORIGINAL         = { .display_width = 64,  .display_height = 32, .cycles_per_second = 700,  .frames_per_second = 60, .high_res_support = false, .vf_reset_quirk = true,  .memory_quirk = true,  .display_wait_quirk = true,  .clipping_quirk = true,  .shifting_quirk = false, .jumping_quirk = false };
static Chip8Config CHIP48           = { .display_width = 128, .display_height = 64, .cycles_per_second = 700,  .frames_per_second = 60, .high_res_support = true,  .vf_reset_quirk = false, .memory_quirk = true,  .display_wait_quirk = true,  .clipping_quirk = false, .shifting_quirk = false, .jumping_quirk = false };
static Chip8Config SUPERCHIP_LEGACY = { .display_width = 128, .display_height = 64, .cycles_per_second = 1000, .frames_per_second = 60, .high_res_support = true,  .vf_reset_quirk = false, .memory_quirk = false, .display_wait_quirk = true,  .clipping_quirk = true, .shifting_quirk = true, .jumping_quirk = true };
static Chip8Config SUPERCHIP_MODERN = { .display_width = 128, .display_height = 64, .cycles_per_second = 1000, .frames_per_second = 60, .high_res_support = true,  .vf_reset_quirk = false, .memory_quirk = false, .display_wait_quirk = false, .clipping_quirk = true, .shifting_quirk = true, .jumping_quirk = true };
static Chip8Config ALL_QUIRKS       = { .display_width = 64,  .display_height = 32, .cycles_per_second = 700,  .frames_per_second = 60, .high_res_support = false, .vf_reset_quirk = true,  .memory_quirk = true,  .display_wait_quirk = true,  .clipping_quirk = true,  .shifting_quirk = true,  .jumping_quirk = true  };
static Chip8Config NO_QUIRKS        = { .display_width = 64,  .display_height = 32, .cycles_per_second = 700,  .frames_per_second = 60, .high_res_support = false, .vf_reset_quirk = false, .memory_quirk = false, .display_wait_quirk = false, .clipping_quirk = false, .shifting_quirk = false, .jumping_quirk = false };

static Chip8Config* config_from_menu_selection(uint8_t selection) {
    switch (selection) {
        case 1:  return &ORIGINAL;
        case 2:  return &CHIP48;
        case 4:  return &SUPERCHIP_MODERN;
        case 3:  return &SUPERCHIP_LEGACY;
        default:
            spdlog::warn("Unknown menu selection {:#04x}, defaulting to ORIGINAL", selection);
            return &ORIGINAL;
    }
}

static void run_emulator(const std::string& romPath, Chip8Config& config) {
    Display display(config.display_width, config.display_height, PIXEL_SCALE);
    Input   input;
    Chip8   chip8(display, input, config);
    chip8.loadRom(romPath);

    auto lastCycle = std::chrono::high_resolution_clock::now();
    auto lastTimer = std::chrono::high_resolution_clock::now();
    auto lastFrame = std::chrono::high_resolution_clock::now();

    while (true) {
        auto now = std::chrono::high_resolution_clock::now();

        if (!input.pollEvents())
            break;

        if (now - lastCycle >= std::chrono::microseconds(MICROSECONDS_IN_A_SECOND / config.cycles_per_second)) {
            chip8.cycle();
            lastCycle = now;
        }

        if (now - lastFrame >= std::chrono::microseconds(MICROSECONDS_IN_A_SECOND / config.frames_per_second)) {
            chip8.send_vertical_blank_interrupt();
            if (chip8.get_draw_flag()) {
                display.render();
                chip8.set_draw_flag(false);
            }
            lastFrame = now;
        }

        if (now - lastTimer >= std::chrono::microseconds(MICROSECONDS_IN_A_SECOND / config.frames_per_second)) {
            chip8.decrement_delay();
            chip8.decrement_sound();
            lastTimer = now;
        }
    }
}

static Chip8Config* run_menu() {
    Display display(ORIGINAL.display_width, ORIGINAL.display_height, PIXEL_SCALE);
    Input   input;
    Chip8   chip8(display, input, ORIGINAL);
    chip8.loadRom(MENU_ROM_PATH);

    auto lastCycle = std::chrono::high_resolution_clock::now();
    auto lastTimer = std::chrono::high_resolution_clock::now();
    auto lastFrame = std::chrono::high_resolution_clock::now();

    while (true) {
        auto now = std::chrono::high_resolution_clock::now();

        if (!input.pollEvents())
            return nullptr;

        if (now - lastCycle >= std::chrono::microseconds(MICROSECONDS_IN_A_SECOND / ORIGINAL.cycles_per_second)) {
            chip8.cycle();
            lastCycle = now;
        }

        if (now - lastFrame >= std::chrono::microseconds(MICROSECONDS_IN_A_SECOND / ORIGINAL.frames_per_second)) {
            chip8.send_vertical_blank_interrupt();
            if (chip8.get_draw_flag()) {
                display.render();
                chip8.set_draw_flag(false);
            }
            lastFrame = now;
        }

        if (now - lastTimer >= std::chrono::microseconds(MICROSECONDS_IN_A_SECOND / ORIGINAL.frames_per_second)) {
            chip8.decrement_delay();
            chip8.decrement_sound();
            lastTimer = now;
        }

        if (chip8.is_menu_done()) {
            uint8_t selection = chip8.get_memory_byte(0x842);
            spdlog::info("Menu selection: {:#04x}", selection);
            return config_from_menu_selection(selection);
        }
    }
}

int main(int argc, char* argv[]) {
    spdlog::set_pattern("[%H:%M:%S.%e] [%^%-5l%$] %v");
    spdlog::set_level(spdlog::level::debug);

    if (argc < 2) {
        spdlog::error("Usage: chip8 <rom_path> [original|chip48|superchip_legacy|superchip_modern|all_quirks|no_quirks]");
        return 1;
    }

    try {
        Chip8Config* config = nullptr;

        if (argc >= 3) {
            std::string mode = argv[2];
            if      (mode == "chip48")           config = &CHIP48;
            else if (mode == "superchip_legacy") config = &SUPERCHIP_LEGACY;
            else if (mode == "superchip_modern") config = &SUPERCHIP_MODERN;
            else if (mode == "all_quirks")       config = &ALL_QUIRKS;
            else if (mode == "no_quirks")        config = &NO_QUIRKS;
            else if (mode == "original")         config = &ORIGINAL;
            else {
                spdlog::warn("Unknown mode '{}', showing menu", mode);
            }
        }

        if (config == nullptr) {
            config = run_menu();
            if (config == nullptr)
                return 0;
        }

        spdlog::info("Running with config: {}x{} @ {}Hz",
            config->display_width, config->display_height, config->cycles_per_second);

        run_emulator(argv[1], *config);

    } catch (const std::exception& e) {
        spdlog::error("{}", e.what());
        return 1;
    }

    return 0;
}
