# Progress

Last updated: 2026-11-14 · Milestone **M0 (scaffold + headless harness)** — in progress

## Done and verified

| Thing | State | Evidence |
| :--- | :--- | :--- |
| Package scaffold | ✅ | ESM, `type: module`, `engines: node >=20`, scripts `test` / `build` / `typecheck` |
| Strict TypeScript | ✅ | `strict` + `noUncheckedIndexedAccess`, `tsc --noEmit` exits 0 |
| Build pipeline | ✅ | `dist/index.js` (3.04 KB) + `dist/index.d.ts` (2.42 KB) |
| `src/cpu/Registers.ts` | ✅ | 10-byte register file, masked accessors, computed AF/BC/DE/HL, four flag accessors, `postBoot()` |
| `tests/registers.test.ts` | ✅ | 8/8 passing |
| Docs | ✅ | `docs/architecture.md`, this file |

Commands run to confirm, last at 2026-11-14:

```
npm run typecheck   → exit 0
npm test            → 1 file, 8 tests passed
npm run build       → ESM + DTS success, dist/index.js + dist/index.d.ts
```

## Not started

- `src/mmu/MemoryBus.ts` — next up
- `src/sdk/Headless.ts` — stub until the CPU exists
- everything below M0 (CPU, PPU, peripherals, SDK, assembler)

## M0 gate checklist

From the plan: green tests on a register file and a byte-addressed bus is the whole of M0.

- [x] writing `0x1234` to `BC` yields `B=0x12, C=0x34`
- [x] setting `AF` masks `F` to `0xF0`
- [ ] `write16(0xC000, 0xABCD)` writes `0xCD` at `0xC000` then `0xAB` at `0xC001` (little-endian)
- [ ] echo-RAM write to `0xE000` visible at `0xC000`
- [x] `tsc --noEmit` clean
- [x] `npm run build` emits `dist/index.js` + `dist/index.d.ts`

## Next block: `src/mmu/MemoryBus.ts`

Intended shape, to be argued line by line before it is written:

- one 64 KB `ArrayBuffer` + a `DataView`; no per-region objects
- `read8`/`write8`, `read16`/`write16` as little-endian pairs (low byte first)
- address masking (`& 0xFFFF`) so a runaway `PC` reads rather than throws
- echo RAM `0xE000–0xFDFF` mirrored onto `0xC000–0xDDFF`
- a watcher registry (`watch(addr, cb)`) — the hook the inspector is built on
- tests: little-endian order, echo mirroring both directions, wrap-around, watcher fires on write only

## Decisions log

| Date | Decision | Why |
| :--- | :--- | :--- |
| 2026-11-14 | Flag accessors are `zeroFlag` / `subtractFlag` / `halfCarryFlag` / `carryFlag` | `h` and `c` are already the H and C registers; the short names are a duplicate class member |
| 2026-11-14 | Pair accessors write through the byte setters, not the private fields | Keeps masking enforced in one place only |
| 2026-11-14 | TypeScript pinned to `^5` | `rollup-plugin-dts` fails to load TS 6.0.3; tsup DTS build errors out |
| 2026-11-14 | `postBoot()` is a static factory, not constructor defaults | `new Registers()` must stay all-zero for tests and for pre-boot semantics |
| 2026-11-14 | `"private": true` in package.json | Blocks an accidental publish before the M6 gate |
