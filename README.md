# CHIP8Emu
Emulator for CHIP8. Written with SDL2, but target is to emulate on console and maybe native macOS.

# Design
[CHIP8 design](https://www.cs.columbia.edu/~sedwards/classes/2016/4840-spring/designs/Chip8.pdf)

CHIP8 includes:
- 16 8-bit registers (V0-VF)
- 16-bit Index register
- 64-byte stack with 8-bit Stack Pointer
- 8-bit delay timer
- 8-bit sound timer
- 64x32 bit frame buffer
- 16 bit Program Counter

# Application Design
Emulator should have the ability to debug programs and load/unload them with GUI.
For now, only SDL2 implementation is planned.

# Targets
Later, it should build on multiple platforms, and multiple frameworks:
- SDL2
- Core Graphics (native macOS build)
- Terminal emulation (ncurses)