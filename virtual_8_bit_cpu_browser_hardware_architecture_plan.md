# Virtual 8-Bit CPU & Browser Hardware — Execution Plan (v2)

> **Status:** v2. Supersedes the v1 architecture plan. Research and decision record is in [`what-is-possible.md`](./what-is-possible.md).
>
> **v2 exists because v1 tried to be three projects at once** — a custom ISA, a Game Boy emulator, and a package — and specified a scope (audio, CGB, cycle-accurate PPU, MBC1, dual CJS/ESM) that cannot be finished. v2 commits to one project, one stack, one accuracy target, and a publishable end state.

---

## 1. What we are building

**A Game Boy (DMG) virtual machine in TypeScript that runs anywhere and can be inspected while it runs.**

The roots are unchanged: *"a user drops a legitimate `.gb` ROM onto a web page and the game boots and runs at ~60 FPS in the browser."* That hook is the product's whole demo, and everything in this plan serves it.

```
vcpu/
├── VirtualMachine        loadRom / stepFrame / stepInstruction / setButton
│                         saveState / loadState / getSnapshot / watch(addr, cb)
├── <GameBoy rom={…}/>    canvas embed + input binding for the browser
├── Assembler             two-pass assembler → emits a runnable ROM from TS
└── Headless              run N frames with no canvas: CI, tests, bug repro
```

Plus a **hardware inspector** (registers, flags, VRAM, tilemap, OAM, memory watch) — the thing none of the nine existing packages do well.

**Zero runtime dependencies. ESM only. Runs identically in the browser and in Node.**

### The claim we are building toward

> Passes blargg `cpu_instrs` 12/12, `instr_timing`, `mem_timing` 8/8, and renders `dmg-acid2` pixel-perfect — in a dependency-free TypeScript library, published on npm, that boots real ROMs a user drags in.

That sentence is the project. Every task below either produces it or is cut.

---

## 2. Decisions locked in v2

Stop revisiting these. They are already researched.

| Decision | Choice | Why |
| :--- | :--- | :--- |
| **Identity** | LR35902-compatible DMG, **not** a custom ISA | Free, brutal correctness oracles; bounded definition of done. See `what-is-possible.md` §2. |
| **Language** | **TypeScript**, pure ESM, zero runtime deps | See §2.1 below. |
| **Accuracy target** | **T3 / M2** — verifiable, not cycle-accurate | dmg-acid2's author states a line-based renderer is sufficient and it does not require T-cycle accuracy. |
| **PPU strategy** | Whole-frame render at V-Blank (line-based), no pixel FIFO | Same evidence. Skips the single largest time sink. |
| **Audio** | **Out of scope** | Separate subsystem + test suite = its own project. |
| **CGB / GBC** | **Out of scope** | Double-speed mode rewires global timing assumptions. |
| **MBC support** | ROM-only at first; **MBC1 before MBC3** | MBC1 covers the compatibility ladder step 3. Add only what the ladder needs. |
| **Boot ROM** | Skipped — hardcode DMG post-boot registers | Standard, accepted shortcut. `AF=0x01B0, BC=0x0013, DE=0x00D8, HL=0x014D, SP=0xFFFE, PC=0x0100`. |
| **Build** | `tsup` → `dist/index.js` + `dist/index.d.ts`, ESM only | Dual CJS/ESM is build tax and dual-package hazard for zero benefit. |
| **Tests** | `vitest` | Fast, zero-config, TS-native. |
| **Licence** | MIT | Two closest competitors are GPL; permissive is a genuine differentiator. |
| **Package name** | `@vcpu/core` if the scope is available, else unscoped `vcpu-core` | Decide at publish time, not now. |

### 2.1 Why TypeScript and not Rust/C++ → WASM

The origin pitch (screenshot doc) says *"C, C++, or Rust, compiled to WebAssembly."* That is one valid way to build this. We are not doing it, for four reasons:

1. **The publishable-package goal favours TS.** A `.wasm` blob needs bundler config, `wasm-bindgen`-style glue, and async instantiation. A `.js` file with `.d.ts` drops into any project with one `npm i`. This is the difference between "30 seconds to try it" and "read my README first."
2. **Performance is not the constraint.** Measured: a JS ColecoVision emulator sits at ~14% CPU in Firefox; CAMLBOY (OCaml→JS) hits 60 FPS on a 2018 phone; JS emulators using per-opcode closures exceed 8 MHz in-browser. We need 4.19 MHz.
3. **Debuggability is the constraint.** Exceptions with real stack traces, symbols readable in devtools, `console.log` on a register. In a WASM core every bug is `RuntimeError: unreachable` at wasm-function[412]. You will write 512 opcodes; you want good errors.
4. **The WASM path stays open.** If it ever matters, the hot loop (`CPU.step`) is a self-contained ~500 lines with no DOM, no async, no allocation. Porting it later is a week. Writing the whole system in Rust *first* costs weeks of toolchain friction before opcode 1 executes.

> If you specifically need the cross-compilation signal for interviews, port the CPU core to Rust/WASM **after** M2 and benchmark it. It becomes a *second* claim — "same semantics, two implementations, verified by the same test suites" — rather than a toolchain obstacle on day one.

---

## 3. Definition of Done (the publish gate)

The package is publishable when **all** of these are true:

- [ ] `npm test` passes: unit tests + blargg + dmg-acid2 diff, in CI on Node 20 and 22
- [ ] `npm run build` emits ESM + `.d.ts` with no errors, `tsc --noEmit` clean
- [ ] Fresh `npm i` into an empty Vite project: `VirtualMachine` loads a ROM and renders a frame in < 20 lines of code
- [ ] `npm pack` contains **no** ROMs, no test fixtures, no source maps pointing outside `dist/`
- [ ] README leads with an animated GIF of a real ROM running, then the accuracy table
- [ ] `saveState`/`loadState` round-trip verified byte-for-byte
- [ ] Headless mode documented — "test a ROM in CI in 5 lines"
- [ ] MIT licence, `repository`, `homepage`, `keywords` set; `provenance` enabled on publish

Publish at `0.1.0`. It is a preview, not a 1.0 — say so in the README.

---

## 4. Hardware reference (the mental model you need)

> Kept from v1 because it is genuinely the engineering context. Skim it; come back when a phase needs it.

### 4.1 Registers

- **8-bit:** `A B C D E H L`. `A` is the accumulator; nearly all arithmetic targets it.
- **16-bit pairs:** `AF BC DE HL`. **Not separate storage** — formed by packing:
  `BC = (B << 8) | C`, and unpacking `B = (v >> 8) & 0xFF`, `C = v & 0xFF`.
  `HL` is the primary memory pointer.
- **`PC`** — next instruction address. **`SP`** — stack top, grows downward.
- **`HL`** is worth internalising: `(HL)` in an opcode means "the byte at address HL", and it is how almost all array/pointer code works.

### 4.2 Flags (`F`, upper nibble only; low nibble hardwired `0`)

| Bit | Flag | Mask | Set when |
| :--- | :--- | :--- | :--- |
| 7 | **Z** Zero | `0x80` | Result is 0. Drives every conditional jump. |
| 6 | **N** Subtract | `0x40` | Last op was a subtraction. |
| 5 | **H** Half-carry | `0x20` | Carry out of bit 3 (or borrow from bit 4). Exists for BCD (`DAA`). |
| 4 | **C** Carry | `0x10` | Carry out of bit 7, borrow below 0, or bit shifted out. |

```
H_add(a,b) = ((a & 0x0F) + (b & 0x0F)) > 0x0F
H_sub(a,b) = (a & 0x0F) < (b & 0x0F)
```

**Quirks that break naive implementations:** `INC`/`DEC` never touch `C`. `ADD HL,r16` touches `N,H,C` but **not** `Z`. `AND` sets `H`; `OR`/`XOR` clear it. Always mask `F &= 0xF0` on write.

### 4.3 Memory map

The CPU only knows `read8(addr)` and `write8(addr, val)`. Devices are wired to address ranges — this is all MMIO is.

| Range | Size | Region |
| :--- | :--- | :--- |
| `0x0000–0x3FFF` | 16 KB | ROM bank 0 (fixed) |
| `0x4000–0x7FFF` | 16 KB | ROM bank N (switchable) |
| `0x8000–0x97FF` | 6 KB | VRAM tile data (2BPP, 16 B/tile) |
| `0x9800–0x9FFF` | 2 KB | VRAM tile maps (two 32×32 grids) |
| `0xA000–0xBFFF` | 8 KB | Cartridge RAM |
| `0xC000–0xDFFF` | 8 KB | WRAM (game state, entities) |
| `0xE000–0xFDFF` | 7.5 KB | Echo RAM → mirrors `0xC000–0xDDFF` |
| `0xFE00–0xFE9F` | 160 B | OAM — 40 sprites × 4 B (`Y, X, tile, flags`) |
| `0xFF00–0xFF7F` | 128 B | I/O registers |
| `0xFF80–0xFFFE` | 127 B | HRAM |
| `0xFFFF` | 1 B | `IE` — interrupt enable |

### 4.4 Timing

- CPU: 4,194,304 T-states/sec. One frame = **70,224 T-states** = 154 scanlines × 456.
- Instructions cost 4–24 T-states. **Conditional branches differ taken vs. not taken** (`JR NZ` = 12 taken, 8 not) — `instr_timing` checks exactly this.
- Real frame rate is **59.7275 Hz**, not 60. "60 FPS" is marketing; the loop must be frame-paced, not wall-clock.

### 4.5 PPU (our simplified model)

Real hardware draws scanline by scanline in four modes with a 172-cycle pixel-transfer window. **We render the whole frame at V-Blank instead** and set `STAT` mode bits plausibly. What we keep because dmg-acid2 tests it:

- 2BPP decoding: 2 bytes per row → 8 pixels. `colorId = (hi_byte's bit << 1) | lo_byte's bit`, bit 7 = leftmost pixel.
- Background + window layers from tile maps, with `SCX/SCY/WX/WY` scroll and `BGP` palette.
- Sprites: max 10 per scanline, OAM-index priority, X-flip (`bit 5`) / Y-flip (`bit 6`), color 0 transparent, 8×8.
- `LY` (0–153), `LYC` coincidence flag, and the **STAT interrupt on `LY == LYC`** — dmg-acid2 uses this to write registers on specific rows, so this one *is* required.

### 4.6 Interrupts

`IF` (`0xFF0F`) is set by hardware; `IE` (`0xFFFF`) is the mask; `IME` is the master switch. Service when `(IE & IF & 0x1F) != 0`:

1. `IME = false`, clear the serviced `IF` bit, wake from `HALT`
2. push `PC` (`SP -= 2`), jump to vector
3. 20 T-states

Vectors: V-Blank `0x40`, LCD STAT `0x48`, Timer `0x50`, Serial `0x58`, Joypad `0x60`.

---

## 5. Compatibility ladder (the "blow up" hook, staged)

You cannot ship ROMs. The demo is a file picker / drag-drop. These are the rungs — each is a milestone gate, not a wish:

| Rung | Target | Requires | Status gate |
| :--- | :--- | :--- | :--- |
| 0 | `dmg-acid2`, blargg suites | CPU + PPU correctness | Must pass — M2 |
| 1 | **Our own assembled ROM** | Assembler + PPU + joypad | Must pass — M5 |
| 2 | **Tetris** (ROM-only cart) | Timer, joypad, interrupts, sprite priority | M3 exit criteria |
| 3 | **Super Mario Land** (MBC1) | MBC1 banking | M3 stretch |
| 4 | **Pokémon Red** (MBC3, no RTC needed) | Faithful sprite priority, WRAM banking, timer behaviour | Stretch — *do not promise this* |

**Honesty rule for the README and for interviews:** say *"runs Tetris and open-source homebrew; passes the reference test suites."* Do not claim "runs Pokémon Red" until you have personally played it to the first save. Commercial-ROM compatibility is a claim you earn rom by rom.

---

## 6. Milestones

Each has an exit gate with a command. **Do not start a milestone before the previous gate is green.** If a gate fails for more than a day, cut scope from that milestone rather than extending it.

---

### **M0 — Scaffold + headless harness** · 4–6 h

**Build:**
- `package.json` (ESM, `type: module`, `exports`, `files: ["dist"]`, Node 20+), `tsconfig.json` (strict, `noUncheckedIndexedAccess`), `tsup.config.ts`, `vitest.config.ts`
- `src/types/hardware.ts` — snapshot and watcher interfaces
- `src/cpu/Registers.ts` — masked `number` fields (`a…l`, `pc`, `sp`), `f` with `& 0xF0`, packed accessors `af/bc/de/hl`, flag helpers, post-boot defaults
- `src/mmu/MemoryBus.ts` — 64 KB `ArrayBuffer`, `read8/write8/read16/write16` (little-endian), echo-RAM mirroring, watcher registry
- `src/sdk/Headless.ts` — run a ROM for N frames, capture serial output and `0xA000` result byte
- `tests/registers.test.ts`, `tests/mmu.test.ts`

**Gate:**
- `npm test` green: writing `0x1234` to `BC` yields `B=0x12, C=0x34`; `write16(0xC000, 0xABCD)` writes `0xCD` then `0xAB`; setting `AF` masks `F` to `0xF0`; echo-RAM write to `0xE000` visible at `0xC000`.
- `npm run build` produces `dist/index.js` + `dist/index.d.ts`.
- `tsc --noEmit` clean.

> The harness exists *before* the CPU on purpose: from M1 onward, "did I break it?" is a command, not a guess.

---

### **M1 — CPU core** · 20–30 h

**Build:**
- `src/cpu/ALU.ts` — `add8/sub8/adc8/sbc8/and/or/xor/cp/inc/dec`, all 8 shift/rotate, `add16`, `addSpSigned8`, exact flag semantics
- `src/cpu/CPU.ts` — fetch-decode-execute, stack `push16/pop16`, control flow (`JR/JP/CALL/RET` + conditions), `HALT`, `EI/DI`, interrupt dispatch, **returns exact T-state count per instruction**
- `src/cpu/Opcodes.ts` — dispatch via the octal encoding, not 512 hand-written cases:
  - `0x40–0x7F`: `LD r,r'` = dst `bits 5..3`, src `bits 2..0` (index `0..7` = `B C D E H L (HL) A`; `0x76` is `HALT`)
  - `0x80–0xBF`: ALU on the same index
  - `0xCB xx`: op = `bits 7..6`, bit index = `bits 5..3`, target = `bits 2..0`
- `src/sdk/Assembler.ts` (minimal, ~40 opcodes) — **instruction list + two-pass label resolution**, emitting into a `Uint8Array`. Used to write M1 test programs without hex arrays.
- `tests/alu.test.ts` (exhaustive boundary cases), `tests/cpu.test.ts`
- `tests/roms/` — blargg ROMs fetched by a script, **gitignored**, never published

**Gate:**
- blargg `cpu_instrs` **11/11 individual sub-tests + combined ROM** pass in the headless harness
- `instr_timing` passes (this forces exact cycle counts)
- Hand-written assembled program: sums 1..10 in a loop, calls a subroutine, pushes/pops, halts — asserted from registers

---

### **M2 — PPU (T3)** · 15–25 h

**Build:**
- `src/ppu/PPU.ts` — `framebuffer: Uint8ClampedArray(160*144*4)`, `step(cycles)` advancing `LY`/`STAT`, `renderFrame()` at V-Blank
- `src/ppu/TileDecoder.ts` — 2BPP row decode, 8×16 sprite mode skipped (not needed for the ladder)
- BG, window, OAM sprite compositing with priority + flips; `BGP/OBP0/OBP1`; `LCDC` bit handling
- `LYC` coincidence + STAT interrupt; OAM DMA at `0xFF46`
- `src/sdk/CanvasRenderer.ts` + `requestAnimationFrame` loop with **time-accumulator frame pacing and catch-up clamp**

**Gate:**
- **`dmg-acid2` renders with 0/23040 mismatched pixels** against the reference PNG (quantise to shade ranks before diffing)
- Our own assembled program: tile data → tilemap → a sprite that moves with the D-pad, on a canvas
- Frame time < 16 ms in Chrome on your machine (measure, don't assume)

---

### **M3 — Peripherals + first real game** · 8–12 h

**Build:**
- `src/io/Timer.ts` — `DIV` (every 256 T-states, write resets), `TIMA/TMA/TAC` with the four selectable frequencies, overflow → modulo + Timer interrupt
- `src/io/Joypad.ts` — 2×4 active-low matrix at `0xFF00` (bit 4 selects d-pad, bit 5 selects buttons)
- `src/mmu/Cartridge.ts` — ROM-only + MBC1 (all four banking modes)
- `setButton()` API and keyboard binding

**Gate:**
- `instr_timing` and `mem_timing` (8/8) still pass *with the timer wired in* — this is the real test that the timer is right
- **Tetris boots to its title screen and plays** with keyboard input, ~59.7 FPS
- (stretch) Super Mario Land boots with MBC1

---

### **M4 — SDK + inspector** · 12–20 h

> This milestone is the differentiator. It is what no existing package does well.

**Build:**
- `saveState(): Uint8Array` / `loadState()` — CPU + 64 KB + PPU + timer + MBC state
- `stepInstruction()`, `getSnapshot()`, `watch(addr, cb)`
- `attachCanvas()`, `setButton()`, pause/resume, speed control
- Inspector UI (separate `demo/` app, not in the package bundle): registers+flags, disassembly of next 10 instructions, VRAM tile viewer, tilemap viewer, OAM table, memory hex + watch list

**Gate:**
- `saveState` → mutate → `loadState` returns byte-identical state (test)
- `stepInstruction` on Tetris advances the disassembly correctly and the frame stays consistent
- Inspector shows live values while Tetris runs (visual check, recorded as a GIF for the README)

---

### **M5 — Assembler + demo game** · 15–25 h

**Build:**
- Full assembler: all opcodes, labels (forward + backward), `.org`, `.db/.dw`, tile-data macro, `compile(): Uint8Array` with a valid cartridge header + checksum
- `demo/game.ts` — one screen: tiled background, one sprite, D-pad movement, BCD score, V-Blank-synced loop. ~200 lines of assembly-through-TS.
- `npm run game` builds the ROM and loads it in the demo page

**Gate:**
- `compile()` output boots on **our** VM *and* on a known-good external emulator (binjgb or SameBoy) — cross-validation proves the assembler, not just the VM
- The game is playable and holds ~59.7 FPS
- `npm run game` is one command from clone to playing

---

### **M6 — Publish** · 3–5 h

**Do:** README (GIF first, accuracy table second, 20-line quickstart, headless-in-CI example), LICENSE (MIT), `files`/`exports` audit, `npm pack` inspection, GitHub Actions CI matrix (Node 20/22 + `cpu_instrs` + acid2 diff), tag `v0.1.0`, `npm publish --provenance`.

**Gate:** the §3 Definition of Done checklist is fully ticked.

---

## 7. Total scope

| Milestone | Hours | Cumulative |
| :--- | :--- | :--- |
| M0 Scaffold + harness | 4–6 | 6 |
| M1 CPU | 20–30 | 36 |
| M2 PPU | 15–25 | 61 |
| M3 Peripherals + Tetris | 8–12 | 73 |
| M4 SDK + inspector | 12–20 | 93 |
| M5 Assembler + game | 15–25 | 118 |
| M6 Publish | 3–5 | 123 |

**~75–125 hours.** At 10 h/week that is roughly three months. The first publishable artifact (`0.1.0`, package exists, Tetris runs, suites pass) lands at the end of **M3 — about 70 hours in**, which is the milestone to actually commit to publicly.

---

## 8. Cut list (agreed in advance)

When you are behind schedule — and you will be — cut from the top of this list, not by extending deadlines:

1. MBC3/MBC5, RTC — *never needed for the ladder*
2. Audio — already cut
3. Inspector VRAM/tilemap viewers — keep registers + memory, drop the fancy ones
4. Assembler macros — hand-emit tile bytes
5. The demo game — ship the SDK with Tetris + acid2 as the proof
6. `stepInstruction` disassembly — keep raw register stepping
7. CGB — already cut

**Never cut:** the blargg/acid2 gates, `saveState`, the headless harness. Those *are* the project.

---

## 9. Known landmines

Look here first when something is subtly wrong.

| Symptom | Likely cause |
| :--- | :--- |
| CPU spirals, `PC` runs away | Unmasked 16-bit arithmetic; a `JP` target not `& 0xFFFF` |
| `instr_timing` fails only on branches | Not returning taken-vs-untaken cycle counts |
| Everything works, then dies after a `CALL` | Stack direction (`SP` decrements) or non-little-endian push order |
| Sprites flicker/jitter | Rendering mid-frame instead of at V-Blank; OAM read during copy |
| First sprite hidden behind background | BG color-0 priority / OAM-index priority order |
| `dmg-acid2` shows a shifted image | `SCX/SCY` applied to window, or window `WX` off-by-7 |
| Game freezes right after boot | `IE`/`IF` mismatch, or `HALT` not waking on pending interrupt |
| Timer drifts | `DIV`/`TIMA` updated once per instruction instead of per T-state |
| Infinite loop inside an interrupt handler | `IME` not cleared on service; `RETI` not restoring it |

Not implementing, and should be documented as such: the HALT bug's exact quirk, OAM corruption during mode 2, `STOP` power states, mid-scanline register writes, the 11 illegal opcodes (they lock the CPU, which is what hardware does).

---

## 10. How to describe it (honest version)

The v1 doc's interview script overclaims ("we designed an architecture"). Replace it with claims that survive a follow-up question:

1. **"I implemented the Sharp LR35902 instruction set — 512 opcodes across the base and `0xCB`-prefixed tables — and validated it against blargg's `cpu_instrs` (12/12), `instr_timing`, and `mem_timing`."**
   → Falsifiable. The interviewer can ask you to run it.

2. **"I decoded the ISA algorithmically rather than with copy-paste: the `LD r,r'` and ALU blocks encode destination and source as 3-bit fields, so 128 opcodes collapse into index arithmetic; the `0xCB` table decodes to op/bit/target from three bit-ranges."**
   → This is the bitwise-understanding claim, and it is true.

3. **"The memory bus is a single 64 KB `ArrayBuffer` with MMIO routing, echo-RAM mirroring, and an OAM DMA path — all O(1) address transforms, no per-region objects."**
   → Systems-design claim.

4. **"I synchronised a 4.19 MHz virtual clock to a 59.7275 Hz display: the CPU returns exact T-states per instruction, the PPU and timer consume those cycles, and V-Blank raises a hardware interrupt serviced via the stack."**
   → Timing claim.

5. **"I wrote a two-pass assembler with symbolic label resolution and cross-validated its output against a third-party emulator, then shipped a game on my own VM."**
   → Tooling + verification claim. The cross-validation is the part people don't expect.

6. **"It ships as a zero-dependency TypeScript package with deterministic save states, single-instruction stepping, and a live hardware inspector."**
   → DX claim, and the one that maps to day-job work.

---

## 11. Locked scope reference (do not re-open)

**In:** DMG CPU, 64 KB bus, frame-at-V-Blank PPU, timer, joypad, interrupts, ROM-only + MBC1, save states, headless mode, inspector, assembler, one demo game, MIT, npm.

**Out:** audio, CGB, MBC2/3/5, RTC, link cable, serial, rewind, websocket multiplayer, custom ISA, cycle-accurate PPU, mobile touch controls, React Native, Electron, database, accounts, telemetry.

If a task is not in the **In** list and not in a milestone gate: it does not exist.

---

## 12. Immediate next step

**M0**, in this order:

```
1. npm init + TS strict + tsup + vitest + tsconfig/exports
2. src/cpu/Registers.ts      → tests/registers.test.ts
3. src/mmu/MemoryBus.ts      → tests/mmu.test.ts
4. src/sdk/Headless.ts       → stub (runs frames once CPU exists)
5. npm test && npm run build → both green
```

Nothing else. No PPU, no canvas, no README, no publish config. Green tests on a register file and a byte-addressed bus — that is M0, and it is the whole first session.
