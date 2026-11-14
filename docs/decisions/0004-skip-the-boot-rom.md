# 0004 — Skip the console's startup program

**Status:** Accepted (14 November 2026)

## Context

A Game Boy runs a small hidden program on power-up. It scrolls the logo down,
plays a chime, checks that the cartridge is genuine, and leaves the registers
at known values before jumping to the game.

That program is Nintendo's copyrighted code. It cannot be shipped.

## Decision

Do not emulate it. Set the registers directly to the values the program would
have left behind (`AF=0x01B0`, `BC=0x0013`, `DE=0x00D8`, `HL=0x014D`,
`SP=0xFFFE`, `PC=0x0100`), in `Registers.postBoot()`.

## Consequences

**Good:** no copyrighted code in the repository, and no dependency on a file
the user would have to supply. Boot is instant.

**Bad:** the startup animation and logo check never happen. Games that inspect
the register state left by the boot program for copy protection will behave
differently. This is the standard shortcut taken by essentially every emulator,
and it is safe for the homebrew and test ROMs this project targets.

**Note:** a plain `new Registers()` remains all-zero, which is the true
pre-boot state, so the skip is explicit at the call site rather than a hidden
default.
