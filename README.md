# vcpu

A Game Boy emulator written in C++, built to be looked inside.

The goal: load a game file (`.gb`), run it, and let you pause to see the CPU's registers, walk through memory, and watch the graphics memory change. Most emulators show you the game. This one is built to show you the machine behind it.

Plain C++17, no libraries.

## Status

Early, on purpose. Written and tested: the **registers**, the 64 KB **memory**, and the **CPU** (every instruction). `vcpu` can run a simple ROM headless and print its serial output and final registers.

Not done yet: interrupts, the timer, HALT waiting, cycle counts, graphics, buttons, cartridges over 32 KB, and the inspector. Nothing can show a game yet.

Next: run a community CPU test ROM (downloaded into `tests/roms/`) and fix what it reports. After that: timer and interrupts, graphics, buttons, and the inspector.

## Run it

Needs a C++17 compiler (`c++`, `g++` or `clang++`) and `make`. Nothing else to install.

```bash
make test                          # build and run the tests
make build                         # build build/vcpu
./build/vcpu                       # run game.gb from the current folder
make clean                         # delete build/
```

## Layout

```text
src/cpu/Registers.hpp      the CPU's registers and flags
src/cpu/Memory.hpp         the 64 KB of memory, ROM loading, serial output
src/cpu/Cpu.hpp            runs one instruction at a time
src/main.cpp               the `vcpu` command: load a ROM and run it
tests/registers_test.cpp   checks for the registers
tests/cpu_test.cpp         checks for memory and the CPU
Makefile                   builds, runs the tests, builds `vcpu`
docs/                      the two docs below
```

## Docs

| I want to… | Read |
| :--- | :--- |
| Understand the project, its rules, and the words it uses | [docs/how-it-works.md](docs/how-it-works.md) |
| Understand the code line by line | [docs/code-walkthrough.md](docs/code-walkthrough.md) |

## Rules

- **No game files, ever.** No ROMs, no boot ROM, no test ROM binaries. Test ROMs would be downloaded into `tests/roms/`, which git ignores.
- **Every claim in a comment should be checkable** by a test that would fail if the claim were false.

## What we're aiming at

The finished thing should be provably correct, not just "seems to run": it passes the community's free Game Boy test programs, draws the standard test image pixel for pixel, and runs free homebrew games.

## License

MIT. See [LICENSE](LICENSE). This project contains no game files and no proprietary code.
