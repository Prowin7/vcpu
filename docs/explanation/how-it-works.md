# How it works

## The short version

An emulator is software that behaves like a piece of hardware. This project
behaves like the original 1989 Game Boy: it reads the game's instructions one
at a time and carries them out, fast enough to run real games.

The Game Boy is a deliberate choice. A public specification describes exactly
what its CPU does, which removes guesswork from the implementation. And test
programs already exist that point at the exact instruction run wrong, which
means the emulator can be checked against an independent standard instead of
trusted on the basis of "it seems to run".

## The parts

The program is split into layers that talk to each other in one direction:
the CPU fetches instructions, the memory layer answers them, and the graphics,
timer and button layers all sit behind the memory layer.

| Part | What it does |
| :--- | :--- |
| CPU | Reads the next instruction and executes it: arithmetic, comparisons, jumps, subroutine calls. |
| Memory | 64 KB of numbered space. The game, the screen data, the buttons, everything — all lives at a numbered address. |
| Graphics | Reads the screen data from memory and produces 160×144 pixels, 60 times a second. |
| Timer | Counts up continuously on its own clock, so games can measure time. |
| Buttons | Reports whether each of the eight buttons is pressed. |
| Wrapper | The public API: load a game, run a frame, save a snapshot. |

Only the wrapper is public. The other layers exist so a reader can watch the
whole system, because everything goes through one door to reach memory.

## The rules the code follows

These are decisions made once that make specific bugs impossible to introduce by
mistake. Each one costs a little extra code in exchange for removing a class of
fault that is hard to find once it appears.

**A register holds a single byte and invalidates anything outside 0–255.**

Registers are the CPU's working storage. Each holds one byte, so 0 to 255. If a
value larger than 255 is assigned, the extra bits are discarded, which is how
the hardware behaves. That sounds like a limitation, but it is also the reason a
common emulator failure is avoidable: if every instruction had to remember to
trim its own results, one instruction that forgot would silently send the CPU to
an invalid address, with no error. With trimming in the setter, that failure
cannot occur, because no code path can ever store a value outside the valid
range.

The same reasoning applies to the program counter and stack pointer, which hold
16-bit addresses: writing a value outside 0–65535 wraps it back into the valid
range rather than producing an impossible address.

**16-bit pairs are reconstructed from the bytes, not stored separately.**

The Game Boy has pairs named BC, DE, HL and AF. BC is the register B with the
register C attached as the upper half, so the two bytes B and C read back as one
16-bit number. The emulator does not store a separate copy of "BC". It builds
the pair on demand from B and C, and reconstructs B and C from a pair value on
write.

Storing a separate copy would mean two places holding the same information. If a
later change updated B without updating the pair, the system would quietly hold
two conflicting values of "what BC is". That discrepancy is hard to find because
each individual instruction looks correct. Computing it removes that entire class
of inconsistency.

**The flags register has only four meaningful bits.**

Arithmetic instructions set four flags to record what they did: was the result
zero, was it a subtraction, was there a carry from the lower half of the byte,
was there a carry out of the top bit. The lower four bits of the flags register
have no hardware meaning and are kept at zero on write. This is enforced by the
only way to write the register, so reads can return the stored value without
further correction.

**Flag names are spelled out in full.**

The hardware uses the single letters `Z`, `N`, `H`, `C` for the four flags. It
also uses `H` and `C` for two registers. The collision in the code base is a
compilation error, not a style choice: the short form for the flags is rejected
in favour of `zeroFlag`, `subtractFlag`, `halfCarryFlag`, `carryFlag`, leaving
the letters free for the registers.

**The startup program is skipped.**

A real Game Boy runs a small hidden program when powered on. It scrolls the logo
a few times, plays a sound, checks the cartridge, and then jumps to the game.
That program is Nintendo's code, which cannot be distributed. The emulator skips
it and sets the registers to the values the program would have left behind.

**No game files are committed or published.**

Games and the boot program are not part of the project. Test programs needed by
the test suite are downloaded into a folder that git ignores, so they never
enter version control.

## Tools, and why

| Choice | Reason |
| :--- | :--- |
| TypeScript | Correctness matters more than the speed ceiling here. The console runs at 4 MHz, a level JavaScript emulators routinely clear. TypeScript also gives bugs real stack traces and readable names, which a compiled language would obscure. |
| TypeScript 5, not 6 | The tool that bundles the type declarations cannot read TypeScript 6 yet. Pinned until it can. |
| `tsup` with command-line flags | The build is small enough that two flags cover it. A config file adds context without adding value while the project is small. |
| `strict` mode | Makes the compiler flag the places most likely to be wrong, rather than silently assuming. |
| `vitest` | Runs TypeScript test files with no additional configuration. |
| `"private": true` | Prevents an accidental publish before the project is ready. The flag is removed at publish time. |
