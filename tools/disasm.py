#!/usr/bin/env python3
"""
Chip-8 / SUPER-CHIP disassembler.
Usage: python3 disasm.py <rom.ch8>
"""

import sys

REGISTER_NAMES = {i: f"V{i:X}" for i in range(16)}
REGISTER_NAMES[0xF] = "VF"


def reg(n: int) -> str:
    return REGISTER_NAMES[n]


def describe(op: int) -> str:
    nib = (op >> 12) & 0xF
    x   = (op >> 8)  & 0xF
    y   = (op >> 4)  & 0xF
    n   = op & 0xF
    nn  = op & 0xFF
    nnn = op & 0xFFF

    if op == 0x00E0: return "Clear the screen"
    if op == 0x00EE: return "Return from subroutine"
    if op == 0x00FE: return "Switch to low-resolution mode (64x32)"
    if op == 0x00FF: return "Switch to high-resolution mode (128x64)"
    if op == 0x00FB: return "Scroll screen right by 4 pixels"
    if op == 0x00FC: return "Scroll screen left by 4 pixels"
    if (op & 0xFFF0) == 0x00C0: return f"Scroll screen down by {n} pixel{'s' if n != 1 else ''}"
    if (op & 0xFFF0) == 0x00D0: return f"Scroll screen up by {n} pixel{'s' if n != 1 else ''}"

    if nib == 0x1: return f"Jump to address {nnn:#05x}"
    if nib == 0x2: return f"Call subroutine at {nnn:#05x}"
    if nib == 0x3: return f"Skip next instruction if {reg(x)} == {nn:#04x}"
    if nib == 0x4: return f"Skip next instruction if {reg(x)} != {nn:#04x}"
    if nib == 0x5 and n == 0: return f"Skip next instruction if {reg(x)} == {reg(y)}"
    if nib == 0x6: return f"Set {reg(x)} = {nn:#04x}"
    if nib == 0x7: return f"Add {nn:#04x} to {reg(x)} (no carry flag)"

    if nib == 0x8:
        if n == 0: return f"Set {reg(x)} = {reg(y)}"
        if n == 1: return f"Set {reg(x)} = {reg(x)} OR {reg(y)}"
        if n == 2: return f"Set {reg(x)} = {reg(x)} AND {reg(y)}"
        if n == 3: return f"Set {reg(x)} = {reg(x)} XOR {reg(y)}"
        if n == 4: return f"Set {reg(x)} = {reg(x)} + {reg(y)}, VF = carry"
        if n == 5: return f"Set {reg(x)} = {reg(x)} - {reg(y)}, VF = 1 if no borrow"
        if n == 6: return f"Shift {reg(x)} right by 1, VF = shifted-out bit"
        if n == 7: return f"Set {reg(x)} = {reg(y)} - {reg(x)}, VF = 1 if no borrow"
        if n == 0xE: return f"Shift {reg(x)} left by 1, VF = shifted-out bit"

    if nib == 0x9 and n == 0: return f"Skip next instruction if {reg(x)} != {reg(y)}"
    if nib == 0xA: return f"Set index register I = {nnn:#05x}"
    if nib == 0xB: return f"Jump to address {nnn:#05x} + V0"
    if nib == 0xC: return f"Set {reg(x)} = random byte AND {nn:#04x}"
    if nib == 0xD:
        h = f"{n}-byte sprite" if n > 0 else "16x16 sprite (SUPER-CHIP)"
        return f"Draw {h} from [I] at ({reg(x)}, {reg(y)}), VF = collision"

    if nib == 0xE and nn == 0x9E: return f"Skip next instruction if key {reg(x)} is pressed"
    if nib == 0xE and nn == 0xA1: return f"Skip next instruction if key {reg(x)} is NOT pressed"

    if nib == 0xF:
        if nn == 0x07: return f"Set {reg(x)} = delay timer"
        if nn == 0x0A: return f"Wait for any key press, store key in {reg(x)} (blocking)"
        if nn == 0x15: return f"Set delay timer = {reg(x)}"
        if nn == 0x18: return f"Set sound timer = {reg(x)}"
        if nn == 0x1E: return f"Add {reg(x)} to index register I"
        if nn == 0x29: return f"Set I = address of built-in font character for digit in {reg(x)}"
        if nn == 0x33: return f"Store BCD of {reg(x)} at [I], [I+1], [I+2]"
        if nn == 0x55: return f"Store V0 through {reg(x)} in memory starting at [I]"
        if nn == 0x65: return f"Read V0 through {reg(x)} from memory starting at [I]"
        if nn == 0x75: return f"Store V0 through {reg(x)} in RPL user flags (SUPER-CHIP)"
        if nn == 0x85: return f"Read V0 through {reg(x)} from RPL user flags (SUPER-CHIP)"

    return f"Unknown opcode {op:#06x}"


def disassemble(path: str):
    with open(path, "rb") as f:
        rom = f.read()

    print(f"Disassembly of: {path}")
    print(f"ROM size: {len(rom)} bytes\n")
    print(f"{'Address':<10} {'Bytes':<8}  Instruction")
    print("-" * 60)

    i = 0
    while i < len(rom) - 1:
        addr = 0x200 + i
        op   = (rom[i] << 8) | rom[i + 1]
        print(f"  {addr:#05x}    {op:04X}     {describe(op)}")
        i += 2

    if len(rom) % 2 != 0:
        print(f"\n  (trailing odd byte: {rom[-1]:#04x})")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: python3 {sys.argv[0]} <rom.ch8>")
        sys.exit(1)
    disassemble(sys.argv[1])
