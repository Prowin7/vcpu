# How it works

## The short version

An emulator is a program that pretends to be a machine. This one pretends to be the original 1989 Game Boy, one instruction at a time, fast enough to run games.

The Game Boy is a good target for two reasons. Its manual is public, so there is nothing to guess. And other people have already written test programs that say "you got it wrong, exactly here" — so we can check our work instead of trusting it.

## The parts

Six pieces, in the order they get built:

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
The console uses the single letters `H` and `C` for two registers *and* for two flags. Using the short names would collide, so the flags are `halfCarryFlag` and `carryFlag`. The short names stay reserved for the registers.

**The startup program is skipped.**
A real Game Boy runs a small hidden program on power-up that leaves the registers at known values. We just set those values directly. That skips needing a copy of Nintendo's startup code, which we could not legally ship.

**No game files are committed or published.**
Tests that need a game file download it into a folder that git ignores.

## Tools, and why

| Choice | Reason |
| :--- | :--- |
| TypeScript | Fast enough — the console runs at 4 MHz, and JavaScript emulators clear that comfortably. And bugs come with real stack traces and readable names, which a compiled language would take away. |
| TypeScript 5, not 6 | The tool that bundles the type definitions cannot read TypeScript 6 yet. Pinned until it can. |
| `tsup` with command-line flags | Two flags do the whole job. No config file to open and decode. |
| `strict` mode | Makes the compiler point at the places most likely to be wrong. |
| `vitest` | Runs TypeScript tests with no setup. |
| `"private": true` | Stops an accidental publish before the project is ready. |

## Words you will run into

Every term this project uses, in plain words, is in the glossary: [../reference/glossary.md](../reference/glossary.md).

## Where the deeper notes are

[what-is-possible.md](../../what-is-possible.md) covers what the surrounding landscape looks like and why the scope was cut where it was. The milestone plan is in [virtual_8_bit_cpu_browser_hardware_architecture_plan.md](../../virtual_8_bit_cpu_browser_hardware_architecture_plan.md).

Those two are research notes, written before any code existed, so they use the jargon freely. This page is the plain-language version.
