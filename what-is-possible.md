# What Is Actually Possible — Research Notes

> **Purpose of this document.** Your `virtual_8_bit_cpu_browser_hardware_architecture_plan.md` describes *one* way to build this. This document maps the *whole* possibility space around it, grounded in what already exists, what can be verified, and what genuinely can't be done in a reasonable amount of time. Read this before committing to a phase order.

---

## 1. The uncomfortable finding, up front

**The niche you are aiming at is already occupied — thoroughly, and recently.**

| Package | Stack | Status | Notes |
| :--- | :--- | :--- | :--- |
| [`wasmboy`](https://www.npmjs.com/package/wasmboy) | AssemblyScript → WASM | 0.7.1, **last publish 5 years ago**, GPL-3.0, ~1,380 downloads/week | DMG+GBC, canvas + Web Audio, save states, debugger demo. Author's own README says "the emulator is still not very accurate." |
| [`gameboy-emulator`](https://www.npmjs.com/package/gameboy-emulator) (roblouie) | TypeScript, zero deps | 1.1.2, **4 years ago**, ISC, ~50 downloads/week | Clean API from a well-known blog series. |
| [`typescript-gameboy-emulator`](https://github.com/davisernst04/typescript_gameboy_emulator) | TypeScript | 2025, MIT, 1 star | MBC0/1/3/5, browser + Node, scanline renderer, DMG sprite priority. |
| [`gb.ts`](https://github.com/obolton/gb.ts) | TypeScript | Active, MIT | Audio, timing, MBC1/2/3/5, pnpm workspace. |
| [`gameboy.js`](https://github.com/juchi/gameboy.js) | JavaScript | 2015, MIT | Runs test ROMs, "some tests fail." |
| [`@c-boy-please/c-boy`](https://github.com/C-Boy-Please/c-boy) | C++ → WASM | **Oct 2026**, GPL-2.0 | `<game-boy>` custom element, DMG + CGB, touch controls, "any framework." |
| [`GB.js`](https://raw.org/software/libraries/gb-js/) (Robert Eisele) | TypeScript | **Nov 1, 2026** — two weeks ago | "Accuracy-focused… separates deterministic emulation cores from presentation, storage, and input policy, so the same package can power browser players, cartridge tools, automated tests, and custom frontends." ESM + CJS + browser bundles + types. |
| [`binjgb`](https://github.com/gbdev/rgbds-live) (C → WASM) | C | Mature | The core behind gbdev's official `rgbds-live` in-browser assembler. |
| [`retroboy`](https://github.com/smparsons/retroboy) | Rust → WASM | 1,038 stars | Cycle-accurate, wasm-pack. |

Two conclusions, both important:

1. **"Publish a Game Boy emulator on npm" is not a novel contribution.** GB.js published the *exact* positioning I would otherwise have recommended — deterministic core, presentation decoupled, reusable across players/tools/tests — two weeks ago, with better packaging than a first attempt will have.
2. **Even good packages get almost no traction.** 1,380 downloads/week for the most popular one, 50/week for the cleanest TypeScript one. A cold npm publish will be invisible. That's not a failure mode to avoid — it's just reality, and it means **adoption must not be the success metric.**

**Therefore: this is a learning + portfolio project that happens to ship a package. Judge it on what you can claim to have built and verified, not on installs.**

---

## 2. The two identities you can pick (and cannot have both)

Your plan currently claims the first while specifying the second. You must pick one, because they have different economics.

| | **A. Custom ISA** ("I designed a CPU") | **B. LR35902-compatible** ("I implemented a real CPU") |
| :--- | :--- | :--- |
| Novelty | High — almost nobody ships a JS-hosted *original* ISA as a library | Low — see the table above |
| Correctness oracle | **None.** You are the specification. Your unit tests are the only truth. | **Free and brutal** — blargg, Mooneye, dmg-acid2 |
| Existing software to run | Zero. Every program is one you wrote. | Real ROMs, 900+ open-source homebrew titles, GB Studio exports |
| Effort to "impressive" | Moderate (small ISA, tiny PPU, own assembler) | High (512 opcodes, exact timing), but *bounded by known answers* |
| Interview story | "I designed an instruction set and its toolchain" | "I implemented a documented ISA and validated it against the community's reference suites" |
| Ceiling | Ends at "cool demo" — nothing else can ever run on it | Ends at "runs real software" — but it's a crowded field |
| Best for | teaching, uniqueness, a weekend-scale win | credibility, verification, employability signal |

**Recommendation: B, with A's assembler.** You get free correctness oracles and a bounded definition of done, which is the single biggest risk-reducer available. The assembler from Phase 6 stays yours and stays the interesting part.

> **If you pick A anyway**, shrink it hard: 8 registers, one 64 KB flat address space, no banking, no interrupts in v1, ~40 opcodes, a 128×128 2BPP screen. That is a genuinely finishable, genuinely original project. Do not attempt A at DMG scale — you would be re-deriving a spec you could have just read.

---

## 3. Accuracy tiers: what is actually achievable, and what each one buys

This is the most important table in the document. **"How accurate" is not one decision — it is five, each with a price and a payoff.** Pick a tier and stop.

| Tier | What it includes | Effort (incremental) | What you can then honestly claim | Verifiable by |
| :--- | :--- | :--- | :--- | :--- |
| **T0 — Toy** | Registers, ALU, ~60 opcodes, no interrupts, no PPU, dumps memory | 8–12 h | "A working fetch-decode-execute loop" | Your own tests only |
| **T1 — Runs your own ROMs** | All 256 base + 256 CB opcodes, stack, control flow, full MMU, interrupts, frame-at-VBlank PPU | +30–45 h | "A complete ISA and a working display" | `cpu_instrs` subset; visual sanity |
| **T2 — Passes the CPU suites** ⭐ | Exact cycle counts on every opcode, timer, halt bug basics | +15–25 h | "All 12 `cpu_instrs`, `instr_timing`, `mem_timing` pass" | blargg suites |
| **T3 — Pixel-correct PPU** ⭐⭐ | OAM scan, sprite priority/flip, window layer, palettes, 10-sprite limit, `LY=LYC` STAT interrupt, mode-2 writes | +15–25 h | "`dmg-acid2` renders correctly" | dmg-acid2 |
| **T4 — Cycle-accurate PPU** | Pixel FIFO, mid-scanline register writes, mode-3 access blocking, OAM corruption bug | +60–120 h | "Mealybug Tearoom passes; 90+/93 Mooneye" | Mooneye, Mealybug |
| **T5 — Full system** | APU (4 channels, envelope, mixing), CGB (double speed, 16 palettes, HDMA) | +150–300 h | "Runs commercial games with sound" | `dmg_sound`, `cgb-acid2` |

**The critical evidence for stopping at T3:** the dmg-acid2 author's own README states —

> "A simple line based renderer is sufficient to generate the correct output. This is **NOT** a PPU timing torture test requiring T-cycle accuracy, and does **NOT** perform register writes during the PPU's mode 3."

That is the PPU equivalent of an official blessing to skip T4. It confirms the frame-at-VBlank / line-based renderer cut from my earlier review: you lose only raster tricks, and you keep pixel-perfect sprites, background, window, palettes and priority.

**Where T4 becomes mandatory:** games that write registers *during* scanline rendering — split-screen status bars, parallax, "wobble" effects. If your game doesn't do that, T4 is pure sunk cost.

**Recommendation: build to T3. Skip T4 and T5 entirely.** T1→T3 is the whole project. T4/T5 are a different project with a different lifespan, and PixiJS-era browsers do not care.

---

## 4. The free correctness oracles (use these, they are the point)

Everything in Tier 2+ is verifiable by ROMs that already exist and are freely redistributable.

| Suite | Source | Validates | Protocol |
| :--- | :--- | :--- | :--- |
| **blargg `cpu_instrs`** | [retrio/gb-test-roms](https://github.com/retrio/gb-test-roms) | All instructions except STOP + 11 illegal opcodes, via CRC-32 of register state | Prints pass/fail to screen; also serial out |
| **blargg `instr_timing`** | same | Cycle count of every instruction, taken and not-taken branches. **Requires working TAC/TIMA/TMA** | text on screen |
| **blargg `mem_timing` / `-2`** | same | Read/write timing per instruction | text |
| **blargg `halt_bug`** | same | The HALT bug specifically | screen-verified |
| **`dmg-acid2`** | [mattcurrie/dmg-acid2](https://github.com/mattcurrie/dmg-acid2) | PPU correctness in one image: sprites, priority, window, 10-sprite limit, flip, off-screen clipping | **Pixel-diff the framebuffer against the reference PNG** |
| **Mooneye** | [Gekkio/mooneye-test-suite](https://github.com/Gekkio/mooneye-test-suite) | Timing edge cases + MBC behaviour. ~93 DMG-applicable tests | register/memory result code |
| **`c-sp/game-boy-test-roms`** | [c-sp/game-boy-test-roms](https://github.com/c-sp/game-boy-test-roms) | Prebuilt release bundles of *all* of the above | — |
| **GBEmulatorShootout** | [gbdev/GBEmulatorShootout](https://github.com/gbdev/GBEmulatorShootout) | Automated cross-emulator comparison table with screenshots | submit your results |

Two practical notes from other implementations' writeups:

- **Headless harness first.** You need a mode that runs a ROM for N frames with no canvas, captures blargg's serial output or the `0xA000` result byte, and exits 0/1. That harness is ~80 lines and turns "did I break the CPU?" into `npm test`. Build it *before* the PPU.
- **A reference PNG diff** for dmg-acid2 (quantize to shade ranks, count mismatched pixels, expect 0/23040) is the cheapest possible proof your PPU is right and costs an hour.

**This is what separates the project from every emulator in the table in §1.** "Passes blargg 12/12, instr_timing, mem_timing 8/8, dmg-acid2 pixel-perfect" is a falsifiable claim. Nothing in a portfolio beats that.

---

## 5. Performance envelope: what's comfortably possible in JS

Roughly, browser-side, single-threaded:

| Approach | Feasibility | Evidence |
| :--- | :--- | :--- |
| Interpreted TS on the main thread | **Fine on desktop, marginal on old mobile** | 8bitworkshop measured a JS ColecoVision emulator at ~14% CPU in Firefox; CAMLBOY (OCaml→JS) reports 60 FPS on a Galaxy S9 and ~1000 FPS native |
| JS in a Web Worker + transfer frames | **Comfortable** — this is what WasmBoy does | keeps input responsive during GC pauses |
| JS with per-opcode JIT (cache a JS closure per decoded opcode) | **Fast, and underused** | noted in the 8bitworkshop comparison: an ARM emulator using this technique exceeds 8 MHz in-browser |
| WASM core | ~1.35–1.67× faster on desktop; up to ~11.7× on mobile Safari | WasmBoy's own benchmark, and the peer-reviewed follow-up |

**Implication: pure TypeScript is a defensible choice** — you don't need Rust/WASM to hit 60 FPS, and you get debuggability that WASM cores don't have. But **plan for a Web Worker from day one** (each core constructible with no canvas attached), even if v1 runs on the main thread. Retrofitting that later is a rewrite.

**Practical rAF loop correction:** don't run `while (cyclesThisFrame < 70224)` uncritically. On a 120 Hz monitor or in a background tab that drifts. Accumulate real elapsed time, run at most N frames per callback, and clamp the catch-up.

---

## 6. Delivery shapes that are possible (pick 2–3, not 6)

| Surface | Cost | Worth it? |
| :--- | :--- | :--- |
| `VirtualMachine` class: `loadRom`, `stepFrame`, `setButton`, `getSnapshot` | Low — it's your internal API | **Required** |
| `attachCanvas()` with `ImageData`/`putImageData` | Low | **Required** |
| `saveState()` / `loadState()` as a single `Uint8Array` | Low | **Yes** — it's the cheapest "wow" feature and unlocks everything below |
| Headless mode (`stepFrame()` with no canvas, run N frames) | Very low | **Yes** — this is your test harness *and* a real differentiator |
| `<GameBoy rom={…}/>` React component | Medium | Yes, if you want a demo page worth looking at |
| Devtools panel: register/VRAM/tilemap/OAM viewers, memory watch | Medium | **This is the real differentiator.** Almost nobody does it well |
| Deterministic input replay (record button-per-frame, replay to reproduce a bug) | Low once save states exist | Nice; genuinely rare, genuinely useful for bug reports |
| Teaching mode: single-step with a human-readable explanation of each opcode | Medium | The most *open* niche — see §7 |
| Audio (APU) | High | **No** — explicitly out of scope |
| Link cable / serial emulation | Low value | No |
| Monorepo (`@x/core` + `@x/react` + `@x/asm`) | Adds tooling tax | No — one package first, split later if ever |

---

## 7. Where the genuinely open space is

Looking at what exists (§1) versus what doesn't:

| Slot | Occupied? | Assessment |
| :--- | :--- | :--- |
| Browser player library | **Saturated** | 9+ packages, 2 of them shipped within the last month |
| `<game-boy>` web component | **Taken** (c-boy, Oct 2026) | — |
| Accuracy-focused decoupled core | **Taken** (GB.js, Nov 1 2026) | Explicitly took the exact positioning worth having |
| **Headless CI harness for GB ROMs** | **Thin** | Real audience: GB Studio users, GB Compo entrants (113 entries in 2025), Homebrew Hub (918 GB + 451 GBC entries). "Does my ROM still boot and reach the menu in CI?" Nobody ships this well in npm. |
| **Embedded devtools / observability** | **Thin** | WasmBoy has a debugger *demo*; no library treats inspection as the product |
| **Teaching / single-step visualization** | **Genuinely open** | Best fit for a portfolio piece: it makes the internals visible instead of hidden |
| **Custom ISA + its toolchain** | **Empty** | Nobody does it because nothing can run on it. Which is exactly why it's novel. |
| **JS-native assembler** | Thin | RGBDS-live runs RGBDS *in WASM*; a pure-TS assembler usable in Node/CI without a C toolchain is a small, real gap — but small |
| APU / CGB accuracy | Occupied by the WASM cores | Don't compete |

**The strategic read:** you cannot win the emulator slot, so **don't compete on emulation — compete on visibility.** The thing all nine packages above are bad at is *showing you the machine*. That's also the thing an interviewer can see in ten seconds, and the thing that makes the project feel like it taught you low-level systems rather than that you typed 512 opcode cases.

---

## 8. Combination constraints (what is *not* possible)

Short, blunt list. Each of these is a trap in the current plan.

1. **Custom ISA + runs real ROMs.** Mutually exclusive. Pick A or B (§2).
2. **T-cycle accuracy + finishes this term.** T4 alone is 60–120 h *after* the 90–130 h to reach T3.
3. **Audio in v1.** The APU is a fourth subsystem with its own timing, envelopes and mixing. `dmg_sound` is a whole project.
4. **CGB support in v1.** Double-speed mode changes CPU/PPU/timer clock relationships globally. Half of CGB is retrofitting timing assumptions.
5. **Real-game compatibility as a claim.** Without testing against real commercial titles you cannot say "runs games." You can say "runs open-source homebrew and passes the reference suites" — which is true and defensible.
6. **Distribution of ROMs or the Nintendo boot ROM.** Not legal. Ship no ROMs; accept ROMs via file picker. Skip the boot ROM entirely — your hardcoded post-boot registers (`AF=0x01B0`, `PC=0x0100`) are the standard, accepted shortcut.
7. **npm adoption as a success metric.** See §1. 1,380 downloads/week is the market leader, and it's been unmaintained for 5 years.
8. **A game engine on top of an 8-bit CPU.** Your game is a demo, not a product; 200 lines of assembly, one screen, one sprite, one score.
9. **The "we designed an architecture" interview framing.** Interviewers will ask "is this your ISA?" Have the honest answer ready: *"I implemented the LR35902 and validated it against blargg and dmg-acid2; the parts I designed are the assembler and the inspection layer."* That answer is stronger than the original, because it's checkable.

---

## 9. Scope tiers — choose a target, not an aspiration

| Tier | Contents | Total effort | Deliverable claim |
| :--- | :--- | :--- | :--- |
| **M0 — Weekend** | Registers + ALU + 40 opcodes + a tiny assembler, headless only | 10–15 h | "I wrote an 8-bit CPU and an assembler. Here's a program it runs." |
| **M1 — Playable core** | T1: full ISA, MMU, interrupts, frame-at-VBlank PPU, canvas | 60–80 h | "I built a working virtual machine that runs ROMs I wrote." |
| **M2 — Verified** ⭐ | M1 + T2 + T3, headless harness, dmg-acid2 pixel-diff, save states | 90–130 h | "Passes blargg 12/12, instr_timing, mem_timing, and renders dmg-acid2 correctly." |
| **M3 — Tooled** | M2 + assembler + devtools panel + React embed + input replay | 130–180 h | "…and ships an SDK with live hardware inspection." |
| **M4 — Aspirational** | M3 + T4 + APU | 350 h+ | "Cycle-accurate emulator." Please don't. |

**Realistic target: M2, then M3's assembler + devtools if time allows.** M2 is the smallest version that produces a falsifiable, resume-worthy claim.

---

## 10. Suggested milestone order (revised from the original plan)

The original phase order (MMU → ALU → CPU → PPU → IO → SDK) is **correct** and should be kept. Three modifications:

1. **Headless test harness goes before the PPU** (with the blargg runners), not with the SDK.
2. **A ~40-opcode assembler moves into Phase 3**, not Phase 6 — hand-typing hex arrays to debug opcodes is the single biggest time sink you can avoid. Grow it to full coverage in Phase 6.
3. **Assembler internals: instruction list + two-pass label resolution.** A fluent builder that emits bytes as you chain cannot resolve forward `jp label` references, which real assembly needs constantly. Methods push items to an array; `compile()` assigns addresses then patches. Same surface, correct underneath.

Then: joypad + timer last (they're small and you need them for `instr_timing`), and devtools after that.

---

## 11. Licensing and hygiene

- **Blargg's ROMs:** freely redistributable freeware — but keep them out of the published package; fetch them as a test-only dev dependency or in CI.
- **dmg-acid2:** MIT (Matt Currie). Mooneye: MIT (Joonas Javanainen). Fine to reference, better to fetch than vendor.
- **Package licence:** pick MIT or Apache-2.0. Note that two of the closest competitors are GPL — if you ever want a company to use this, permissive matters.
- **README discipline:** "This project contains no ROMs and no proprietary code. Supply your own." One line, prevents the only legal problem you could have.

---

## 12. Sources

- Landscape: [wasmboy](https://www.npmjs.com/package/wasmboy), [gameboy-emulator](https://www.npmjs.com/package/gameboy-emulator), [typescript_gameboy_emulator](https://github.com/davisernst04/typescript_gameboy_emulator), [gb.ts](https://github.com/obolton/gb.ts), [gameboy.js](https://github.com/juchi/gameboy.js), [c-boy](https://github.com/C-Boy-Please/c-boy), [GB.js](https://raw.org/software/libraries/gb-js/), [retroboy](https://github.com/smparsons/retroboy), [rgbds-live](https://github.com/gbdev/rgbds-live)
- Test suites: [c-sp/game-boy-test-roms](https://github.com/c-sp/game-boy-test-roms), [mooneye-test-suite](https://github.com/Gekkio/mooneye-test-suite), [dmg-acid2](https://github.com/mattcurrie/dmg-acid2), [blargg cpu_instrs readme](https://github.com/L-P/blargg-test-roms/blob/master/cpu_instrs/readme.txt), [GBEmulatorShootout](https://github.com/gbdev/GBEmulatorShootout)
- Community: [gbdev.io](https://gbdev.io/), [GB Studio](https://www.gbstudio.dev/), [Homebrew Hub](https://hh.gbdev.io/), [GB Compo 2025](https://itch.io/jam/gbcompo25), [awesome-gbdev](https://github.com/gbdev/awesome-gbdev)
- Performance: [8bitworkshop: WebAssembly vs JavaScript](https://8bitworkshop.com/docs/posts/2021/webassembly-vs-javascript-emulator-performance.html), [WasmBoy benchmark article](https://medium.com/@torch2424/webassembly-is-fast-a-real-world-benchmark-of-webassembly-vs-es6-d85a23f8e193), [CAMLBOY](https://spread-x.github.io/CAMLBOY)

---

## TL;DR

- Build it — but as **a verified LR35902 implementation + your own assembler + an inspection layer**, not as "an npm emulator."
- Target **T3 / M2**: `dmg-acid2` explicitly says line-based rendering is enough, so **skip cycle-accurate PPU**. Skip audio, skip CGB, skip MBC beyond what the test ROMs need.
- **blargg + dmg-acid2 are the project's credibility.** Nothing else in this document matters as much.
- The open space is **headless CI, devtools/observability, and teaching** — not the emulator itself.
- Expect **90–130 hours** to M2. Everything above that is a different, larger project.
