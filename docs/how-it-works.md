# How it works

## The short version

An emulator is a program that pretends to be a machine. This one pretends to be the original 1989 Game Boy, one instruction at a time, fast enough to run games.

The Game Boy is a good target for two reasons. Its manual is public, so there is nothing to guess. And other people have already written test programs that say "you got it wrong, exactly here" — so we can check our work instead of trusting it.

## The parts

Six pieces, in the order they get built. **CPU** and **Memory** exist; the rest do not yet.

| Part | What it actually does | Talks to the outside world via |
| :--- | :--- | :--- |
| **CPU** | Reads instructions from memory and does them: add, compare, jump, call. The brain. | asks memory for bytes |
| **Memory** | 64 KB of addressable space. Everything — the game, the screen data, the buttons — lives at a numbered address in it. | `read(addr)` / `write(addr, value)` |
| **Graphics** | Turns the data in memory into 160×144 pixels, 60 times a second. | a pixel buffer |
| **Timer** | Counts up on its own clock so games can keep time. | an interrupt |
| **Buttons** | Reports which of the 8 buttons are held down. | `setButton(name, pressed)` |
| **Wrapper** | The thing you actually hold: load a game, run a frame, save a snapshot. | the public API |

Only the wrapper is public. Everything else is reachable only through it, and everything reaches memory through one door — that is what makes it possible to watch what happens.

## The rules the code follows

These are choices we made once so we never have to think about them again. Each one exists to make a specific unpleasant bug impossible to write.

**A register is a number from 0 to 255, and that is enforced in one place.**
Registers are the CPU's working slots. If code puts 300 in one, it silently becomes 44. That sounds bad, but the alternative is worse: if every instruction had to remember to trim its own values, one forgotten trim sends the program running off into nonsense addresses. So the trimming happens in the register setters, and nowhere else.

This is also why the CPU's main position pointer can't wrap to a negative number or past the end of memory — writing to it just cannot produce nonsense.

**Two 8-bit registers read as one 16-bit number are calculated, never stored twice.**
The console has a pair called BC, which is the two registers B and C read as one number (B is the high part). We do not keep a separate copy of "BC". We build it when asked. Keeping a copy would mean two places holding the same truth, and any code that updates one and forgets the other gives you a register that quietly disagrees with itself — the hardest kind of bug to find, because each instruction looks correct on its own.

**The flags register only has four bits.**
Flags are single bits that record what the last arithmetic did: was the result zero, was it a subtraction, did it carry, did it carry out of half a byte. Four bits, nothing else. The bottom four bits of that register are hardwired to zero on real hardware, and we enforce that on write.

**Flag names are spelled out in full.**
The console uses the single letters `H` and `C` for two registers *and* for two flags. Using the short names would collide, so the flags are `half_carry_flag` and `carry_flag`. The short names stay reserved for the registers.

**The startup program is skipped.**
A real Game Boy runs a small hidden program on power-up that leaves the registers at known values. We just set those values directly. That skips needing a copy of Nintendo's startup code, which we could not legally ship.

**The CPU runs one instruction per `step()`, and nothing else.**
`step()` reads the byte at PC, does what it says, and returns. There are no cycle counts, interrupts or timer yet. That keeps the instruction code easy to read and test, and it is why only simple ROMs run so far.

**Each instruction has its own named case in one `switch`.**
`case 0x3e: set_register(REG_A, fetch_byte());` says exactly what it does. The cost is a long file; the gain is that you can read any line without knowing a bit trick. Only the two big rows of look-alike opcodes (`0x40`-`0xBF`) use simple division to pick the registers.

**All memory goes through `read` and `write`.**
Echo-region mirroring and serial capture happen in `write`, in one place. The inspector will later watch addresses from the same place.

**No game files are committed or published.**
Tests that need a game file download it into a folder that git ignores.

## Choices we made, and why

- **Real Game Boy hardware, not a made-up CPU.** A made-up CPU has no wrong answers, so nothing to check against. The real one has a public manual and test programs that report their own failures.
- **C++17.** Fast, and integer sizes are real types: a register is a `uint8_t`, a program counter is a `uint16_t`. The cost: running in a browser would need a WebAssembly build, which is not set up. (The project started in TypeScript and was moved to C++.)
- **Header-only, no libraries.** One `.hpp` per part, nothing to link or install. `make` and the system compiler are enough.
- **Strict compiler flags** (`-Wall -Wextra -Wpedantic -Werror`). Any warning stops the build.
- **A hand-written test runner.** A few dozen checks do not need a framework.
- **Draw a whole frame at once.** The real console draws pixel by pixel, which takes months to copy. We draw the full 160×144 picture during V-Blank. Games that change the screen mid-frame will look wrong. That is a known, permanent limit.

## Words you will run into

| Term | Meaning |
| :--- | :--- |
| **Address** | A number that names one byte of memory, like a house number. |
| **Byte** | One number, 0–255. |
| **Flag** | A single bit recording what the last calculation did. |
| **Frame** | One complete picture. The screen redraws about 60 times a second. |
| **Half-carry** | A carry out of the 4th bit. Exists only so decimal arithmetic works. |
| **Hex (`0xFF00`)** | How addresses are written. `0xFF00` is 65280. |
| **Instruction** | One thing the CPU can do, like "add these two numbers". |
| **Interrupt** | The hardware tapping the CPU on the shoulder: "deal with me now". |
| **Mask** | A number used to keep or clear specific bits, e.g. `0xff` keeps the bottom 8 bits. |
| **Register** | A tiny slot inside the CPU holding one number, 0–255. |
| **ROM / game file** | The game itself, a single `.gb` file. |
| **Snapshot** | A copy of the whole machine at one instant, so you can return to it. |
| **Sprite / tile** | A sprite is a small picture that moves (a character). A tile is an 8×8 picture the screen is built from. |
| **V-Blank** | The short gap after each frame, when the screen is idle and safe to redraw. |
| **Word (16-bit)** | Two bytes read as one number, 0–65535. |

## Where the deeper notes are

[what-is-possible.md](../what-is-possible.md) covers the surrounding landscape and why the scope was cut where it was. The milestone plan is in [virtual_8_bit_cpu_browser_hardware_architecture_plan.md](../virtual_8_bit_cpu_browser_hardware_architecture_plan.md). Those two are research notes written before any code existed, so they use jargon freely.
