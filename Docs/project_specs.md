# Chip-8 Emulator — Project Specs

## Overview

A Chip-8 emulator written in C++20 using SDL2 for rendering and input.

## Build

```bash
cmake -B build
cmake --build build
./build/chip8 <path/to/rom>
```

Requires SDL2: `brew install sdl2`

## Architecture

```
main.cpp
  └── Display   (SDL2 window + 64×32 pixel buffer)
  └── Input     (SDL2 event polling + Chip-8 key mapping)
  └── Chip8     (CPU state, memory, stack — fetch/decode/execute)
```

### Display

- Native resolution: **64 × 32** pixels
- Rendered at **10× scale** by default (640×320 window)
- Frame buffer: `std::array<bool, 64*32>` — `true` = white pixel
- API: `setPixel`, `getPixel`, `clear`, `render`

### Input

Chip-8 keypad (0x0–0xF) mapped to keyboard:

| Chip-8 | Key |
|--------|-----|
| 1 2 3 C | 1 2 3 4 |
| 4 5 6 D | Q W E R |
| 7 8 9 E | A S D F |
| A 0 B F | Z X C V |

### Chip8 (CPU)

- **Memory**: 4 KB (`0x000`–`0xFFF`)
- **ROM loaded at**: `0x200`
- **Font sprites**: `0x050`–`0x09F`
- **Registers**: 16 × 8-bit (`V0`–`VF`), 1 × 16-bit index (`I`)
- **Stack**: 16 levels deep
- **Timers**: `delayTimer`, `soundTimer` (decremented at 60 Hz in `main.cpp`)

## Main Loop Timing

- CPU: **500 Hz** (~8 cycles per 60 Hz frame)
- Render + timers: **60 Hz**
- Frame cap via `SDL_Delay`

## Status

| Component | Status |
|-----------|--------|
| CMake build | Done |
| Display (SDL2) | Done |
| Input (keyboard mapping) | Done |
| Chip8 stub + ROM loading | Done |
| Chip8 opcode implementation | TODO |
