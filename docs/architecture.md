# Architecture

## What this is

A Sharp LR35902 (Game Boy DMG) implementation in TypeScript. Not a custom ISA — the CPU is the documented one, which means correctness is checked against ROMs someone else wrote (`blargg cpu_instrs`, `instr_timing`, `mem_timing`, `dmg-acid2`) instead of against our own opinion.

Scope is locked. **In:** DMG CPU, 64 KB bus, frame-at-V-Blank PPU, timer, joypad, interrupts, ROM-only + MBC1, save states, headless mode, inspector, assembler, one demo game. **Out:** audio, CGB, MBC2/3/5, RTC, link cable, cycle-accurate PPU.

The design record behind those cuts is in [`../what-is-possible.md`](../what-is-possible.md); the milestone plan is [`../virtual_8_bit_cpu_browser_hardware_architecture_plan.md`](../virtual_8_bit_cpu_browser_hardware_architecture_plan.md).

## Module map

```
src/
├── index.ts              public exports
├── cpu/
│   ├── Registers.ts      ✅ register file, flags, pairs, post-boot state
│   ├── ALU.ts            ⬜ add/sub/adc/sbc/and/or/xor/cp/inc/dec, shifts, 16-bit add
│   ├── CPU.ts            ⬜ fetch-decode-execute, stack, control flow, interrupts, T-states
│   └── Opcodes.ts        ⬜ dispatch tables decoded from the octal/bit encoding
├── mmu/
│   ├── MemoryBus.ts      ⬜ 64 KB ArrayBuffer, read8/write8/read16/write16, echo RAM, watchers
│   └── Cartridge.ts      ⬜ ROM-only, then MBC1
├── ppu/
│   ├── PPU.ts            ⬜ LY/STAT, render whole frame at V-Blank
│   └── TileDecoder.ts    ⬜ 2BPP row decode
├── io/
│   ├── Timer.ts          ⬜ DIV/TIMA/TMA/TAC
│   └── Joypad.ts         ⬜ active-low 2×4 matrix at 0xFF00
└── sdk/
    ├── Headless.ts       ⬜ run N frames, no canvas
    ├── CanvasRenderer.ts ⬜ ImageData + rAF frame pacing
    └── Assembler.ts      ⬜ two-pass, labels resolved at compile()
```

Data flows one way: `CPU.step()` asks the bus for bytes, the bus routes by address to cartridge / VRAM / WRAM / I/O, and each device consumes the T-states the CPU reports. Nothing reaches around the bus.

## Invariants we have committed to

These are the decisions that are expensive to reverse later. Each one is enforced in code, not by convention.

**1. Bit widths are enforced at the edge, in accessors, not at use sites.**
`Registers` stores private fields and exposes `get`/`set` that mask. `a = 0x1234` stores `0x34`; `pc = 0x10000` stores `0x0000`. The alternative — public fields masked inside each opcode — makes ~500 opcode implementations individually responsible for the same invariant, and a single missed mask is a runaway `PC`. That bug is the first entry in the plan's landmine table, so it gets a structural fix rather than a careful one.

Tests: `tests/registers.test.ts` ("keeps only the low byte", "wraps DE to 16 bits").

**2. 16-bit pairs are computed, never stored.**
Hardware has no `BC` register — it has B and C read as one number. `bc` is `(b << 8) | c`; writes split through the byte setters. Storing the pair as its own number would create two sources of truth for the same bits, and any path updating one without the other yields a phantom register that only shows up in specific opcode combinations. One shift and one OR is free at 4.19 MHz.

Tests: "splits BC into B and C", "joins HL back into one number".

**3. `F` only has four bits.**
Low nibble is hardwired zero, enforced by the `f` setter (`& 0xf0`), so reads can return the raw field and still be correct. Because the setter is the only door in, there is no state with a dirty low nibble.

Tests: "cannot hold anything in the low nibble", "masks F when written through AF".

**4. Flag accessors are spelled out, never `z`/`n`/`h`/`c`.**
`h` and `c` are the H and C *registers*. Using the short names for flags is a duplicate class member and a compile error — hardware genuinely reuses both names for both things. The accessors are `zeroFlag`, `subtractFlag`, `halfCarryFlag`, `carryFlag`.

**5. The boot ROM is skipped.**
`Registers.postBoot()` returns the documented post-boot DMG state (`AF=0x01B0`, `BC=0x0013`, `DE=0x00D8`, `HL=0x014D`, `SP=0xFFFE`, `PC=0x0100`). No Nintendo code is shipped or required. A bare `new Registers()` stays all-zero, which is the real pre-boot state and what tests want.

**6. No ROMs in the repository or the package.**
`tests/roms/` is gitignored and excluded from `files`. Test ROMs are fetched by script.

## Toolchain decisions

| Choice | Reason |
| :--- | :--- |
| TypeScript 5.x (pinned) | tsup's `.d.ts` bundler (`rollup-plugin-dts`) cannot load TypeScript 6 yet. Unpin when it can. |
| `tsup` via CLI flags, no `tsup.config.ts` | Two flags do the whole job; a config file would be a file to read for no benefit. Revisit if the build grows. |
| `strict` + `noUncheckedIndexedAccess` | Indexed reads type as `T \| undefined`, which forces explicit handling in exactly the places a bus/opcode table needs it. |
| `vitest` | No config file needed for TS. |
| `"private": true` | Prevents accidentally publishing before M6. Flip at publish. |
