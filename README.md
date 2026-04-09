# CHIP-8 Emulator

CHIP-8 is a simple interpreted language from the 1970s, originally designed to make game development easier on early microcomputers. This project is a from-scratch emulator that can run original CHIP-8 games as well as programs written for later variants like CHIP-48 and SUPER-CHIP.

The emulator handles the differences between platforms through configurable quirk profiles — small behavioural switches that control how ambiguous instructions behave on each system. If you're not sure which one to use, just launch without arguments and the built-in platform selector will guide you through it. This my first publicly available Project, so if you'd like to leave a star as feedback I'd be quite happy :)

---

## Features

- Full CHIP-8 instruction set
- SUPER-CHIP extensions — high-resolution mode (128×64), scrolling, large font
- Configurable quirk profiles for accurate per-platform behaviour
- Interactive platform selector menu (loaded from ROM)
- Structured logging via spdlog
- ROM disassembler tool

---

## Supported Platforms

| Profile | Resolution | Notes |
|---|---|---|
| `original` | 64×32 | CHIP-8 as on the COSMAC VIP |
| `chip48` | 128×64 | CHIP-48 on HP-48 calculators |
| `superchip_legacy` | 128×64 | SUPER-CHIP 1.1 with display-wait |
| `superchip_modern` | 128×64 | SUPER-CHIP without display-wait |
| `all_quirks` | 64×32 | All quirks enabled (Used for debugging)|
| `no_quirks` | 64×32 | All quirks disabled (Used for debugging)|

---

## Requirements

- CMake ≥ 3.20
- C++20 compiler (clang / GCC)
- SDL2 (`brew install sdl2` on macOS)
- Internet connection on first build (spdlog fetched via CMake FetchContent)

---

## Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

The `chip8` binary is placed in `build/`.

---

## Usage

### With platform selector menu

Run without specifying a platform to open the interactive menu:

```bash
./build/chip8 <rom_path>
```

The menu lets you choose the platform with the keyboard:

| Key | Action |
|---|---|
| `E` | Move selection up |
| `F` | Move selection down |
| `A` | Confirm |

### With explicit platform

Skip the menu by passing a platform name directly:

```bash
./build/chip8 <rom_path> <platform>
```

**Example:**
```bash
./build/chip8 assets/roms/Games/chip8/br8kout.ch8 original
./build/chip8 assets/roms/Games/chip8/snake.ch8 superchip_modern
```

---

## CHIP-8 Key Mapping

The original CHIP-8 hex keypad maps to:

```
CHIP-8    Keyboard
1 2 3 C   1 2 3 4
4 5 6 D   Q W E R
7 8 9 E   A S D F
A 0 B F   Z X C V
```

---

## Project Structure

```
.
├── src/
│   ├── main.cpp          # Entry point, emulation loop, platform selection
│   ├── Chip8.cpp/h       # CPU, memory, opcode execution
│   ├── Display.cpp/h     # SDL2 window, framebuffer, hi-res/lo-res rendering
│   └── Input.cpp/h       # SDL2 keyboard input
├── assets/
│   └── roms/
│       ├── main_menu.ch8 # Platform selector ROM (patched 5-quirks)
│       ├── test_suite/   # Chip-8 test ROMs
│       └── Games/        # Playable ROMs
└── tools/
    └── disasm.py         # ROM disassembler
```

---

## ROM Disassembler

Disassemble any `.ch8` file to readable output:

```bash
python3 tools/disasm.py <rom_path>

# Save to file
python3 tools/disasm.py assets/roms/main_menu.ch8 > output.txt
```

---

## Quirks

CHIP-8 implementations vary significantly across hardware. The following quirks are configurable per platform:

| Quirk | Description |
|---|---|
| `vf_reset` | `8XY1/2/3` reset VF to 0 |
| `memory` | `FX55/65` increment I |
| `display_wait` | `DXYN` stalls until vertical blank |
| `clipping` | Sprites clip at screen edges instead of wrapping |
| `shifting` | `8XY6/E` use VY as source |
| `jumping` | `BXNN` uses VX instead of V0 |

---

## Dependencies

| Library | Version | Purpose |
|---|---|---|
| [SDL2](https://www.libsdl.org/) | system | Windowing, rendering, input |
| [spdlog](https://github.com/gabime/spdlog) | v1.14.1 | Structured logging |
