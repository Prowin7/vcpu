# What the code does

Read this if `value & BYTE_MASK` looks like soup. It walks through the only real file we have written, in order, and explains what each part is *for*.

**Short version:** we have written the CPU's scratchpad. 189 lines. It cannot run a game yet, and it isn't supposed to.

---

## Part 1 — What a register is

A CPU has nowhere to put numbers except a few tiny slots called **registers**. Think of a workbench with 8 pigeonholes above it. Each pigeonhole holds one number, and the number can only be **0 to 255**. That's it. Every calculation a game does — your score, your position, whether a button is held — eventually becomes numbers moving between these pigeonholes.

The Game Boy has this set:

| Slot | What it's for |
| :--- | :--- |
| **A** | The main slot. Nearly all arithmetic ends up here. |
| **B C D E H L** | Six general-purpose slots. Just storage. |
| **F** | Not a number — four tiny on/off switches recording what the last calculation did. |
| **PC** | The "what's next" slot: which instruction to run next. |
| **SP** | The "stack top" slot: where saved return addresses live. |

That's the entire memory the CPU has to think with. Everything else — the game, the graphics, your save file — lives outside, in the 64 KB of main memory, and the CPU reaches it by *address*.

Two quirks make this file more than a list of variables:

**Slots can be read two at a time.** `BC` means "B and C as one big number". B is the high half, C is the low half. There is no `BC` pigeonhole — it's B and C standing next to each other wearing a trench coat. Same for `DE`, `HL`, and `AF`.

**Numbers overflow.** A slot holds 0–255. If you add 1 to 255, you don't get 256 — you get 0, and the carry flag flips on. That's not a bug, it's how the hardware works, and it's how games do math bigger than 255.

---

## Part 2 — The four symbols you'll keep seeing

Almost all the confusion in this file comes from four operators. Each one works on the *binary* form of the number, bit by bit.

**`&` AND — "keep only some bits"**

`value & 0xff` throws away everything except the bottom 8 bits.

```text
  0x1234 in binary: 0001 0010 0011 0100
  0xff   in binary: 0000 0000 1111 1111
  AND result:       0000 0000 0011 0100  = 52 = 0x34
```

**`|` OR — "combine two groups of bits"**

Used to glue two halves back into one number.

```text
  0x12 << 8  = 0001 0010 0000 0000
  0x34       = 0000 0000 0011 0100
  OR result  = 0001 0010 0011 0100  = 0x1234
```

**`<<` shift left — "move bits up, multiplying by 2"**

`0x12 << 8` pushes `0x12` up 8 places, leaving zeroes underneath. It's how a byte becomes the *high* half of a 16-bit number.

**`>>` shift right — "move bits down, dividing by 2"**

`0x1234 >> 8` gives `0x12`. It's how you pull the high half back out.

**So the pair trick is just:** `high << 8 | low` to join, and `value >> 8` to split. That's the whole idea. Nothing more complicated happens anywhere in this file.

---

## Part 3 — Every file we have, and why it exists

```text
package.json        Tells npm the project's name, that it uses modern modules,
                    and what `npm test` / `npm run build` should run.
tsconfig.json       Rules for the compiler. Mostly: be strict, and yell at me
                    when something might be undefined.
.gitignore          Files that must never be committed: installed packages,
                    build output, and game files.
src/index.ts        The front door. 7 lines. Re-exports what the package offers.
src/cpu/Registers.ts  THE CODE. Everything below is about this file.
tests/registers.test.ts  8 tests that prove the above works.
docs/               These explanations.
```

`src/index.ts` is 7 lines and looks pointless. It isn't: it decides what the outside world is allowed to touch. Everything in `src/` is private unless it's re-exported here.

---

## Part 4 — Walking `src/cpu/Registers.ts` line by line

### Lines 1–11: the header comment

Says what the file is and states the one rule it enforces: **a register holds 0–255, and that is checked here, in this file, in one place.**

Keep that sentence in mind. Every design choice below follows from it.

### Lines 13–20: the four flags

```ts
export const FLAG_Z = 0x80;
export const FLAG_N = 0x40;
export const FLAG_H = 0x20;
export const FLAG_C = 0x10;
```

The F register doesn't store a number. It stores four on/off switches, one per bit, in the *top* four bits of a byte:

```text
  bit:   7  6  5  4  3  2  1  0
        [Z][N][H][C][0][0][0][0]
  0x80 = 1000 0000  → the Z switch
  0x40 = 0100 0000  → the N switch
```

`0x80` is just the number whose 7th bit is on. So these constants aren't flag *values*, they're **masks**: a tool for picking out one switch.

Here's what each means, in game terms:

- **Z (zero)** — "the answer was 0". Games check this constantly: *did the counter reach zero?*
- **N (subtract)** — "the last calculation was a subtraction". Only used to make decimal arithmetic work.
- **H (half-carry)** — "carrying out of the 4th bit". Exists purely for decimal arithmetic.
- **C (carry)** — "it overflowed past 255, or borrowed below 0".

### Lines 22–24: three masks with names

```ts
const BYTE_MASK = 0xff;   // 0000 0000 1111 1111
const WORD_MASK = 0xffff; // 1111 1111 1111 1111
const FLAG_MASK = 0xf0;   // 1111 0000
```

`BYTE_MASK` keeps 8 bits. `WORD_MASK` keeps 16. `FLAG_MASK` keeps the top 4 and wipes the bottom 4.

They're named because the alternative is `& 0xff` scattered through 500 future instructions, where a single typo of `& 0xfff` creates a bug you'd hunt for a week. A name can be searched; a magic number can't.

### Lines 27–36: the actual storage

```ts
private _a = 0;
private _b = 0;
...
private _pc = 0;
private _sp = 0;
```

Ten private numbers, all starting at 0. **This is the entire state of the CPU.** Everything the game knows lives here, or in the 64 KB of memory outside.

`private` matters: no other file can reach `_a` directly. The only way in is the setter below.

### Lines 38–43: one register, fully, as the pattern for all ten

```ts
get a(): number {
  return this._a;
}
set a(value: number) {
  this._a = value & BYTE_MASK;
}
```

Reading `reg.a` returns the stored number. Writing `reg.a = 300` stores `44`, because `300 & 0xff = 44`.

**Why bother?** Because otherwise ~500 instructions each have to remember to trim their own numbers, and one that forgets silently corrupts everything downstream. Here, the trimming happens once, and no code anywhere can put an invalid number into A. The bug becomes impossible rather than unlikely.

This is the trade in one line: **10 extra lines of code, in exchange for an entire category of bug that can never be written.**

### Lines 45–85: the other six registers

Identical pattern. Nothing new. Skim them.

### Lines 87–96: F, which is special

```ts
get f(): number {
  return this._f;
}
set f(value: number) {
  this._f = value & FLAG_MASK;
}
```

The bottom four bits of F **do not exist** on real hardware — there's nothing behind them. So they're forced to zero on write.

Note the asymmetry: the setter masks, the getter doesn't. That's deliberate. Because the setter is the only door in, `_f` can never hold rubbish, so the getter has nothing to clean up. One mask, at the door, is enough.

### Lines 98–110: PC and SP

Same pattern, but masked to 16 bits (`0xffff`) because they hold addresses, which are 0–65535.

The payoff: `pc` can't go negative or past the end of memory. Write `70000` into it and you get `4464`. That single line prevents the classic emulator disaster where the program counter runs off into nowhere and the emulator hangs with no explanation.

### Lines 112–143: the pairs

```ts
get bc(): number {
  return (this._b << 8) | this._c;
}
set bc(value: number) {
  this.b = value >> 8;
  this.c = value;
}
```

Read it as: **to get BC, push B up 8 places and drop C underneath.** To set BC, split it.

Worked example — `reg.bc = 0x1234`:

```text
  value = 0x1234
  value >> 8  = 0x12  →  stored in B
  value       = 0x34  →  stored in C  (truncated to 8 bits by the setter)
```

and reading it back:

```text
  (0x12 << 8) | 0x34  =  0x1200 | 0x34  =  0x1234
```

**Why not just store BC as its own number?** Because then B, C and BC would all be holding the same information, and any code that updates one pair of them and forgets the third leaves the register disagreeing with itself. That bug is invisible: every individual instruction looks right, and only a specific combination betrays it.

Notice line 125 writes `this.b`, **not** `this._b`. It goes through the setter, so the trimming rule still applies. Same reason as before: one place, one rule.

### Lines 145–173: the four flag switches

```ts
get zeroFlag(): boolean {
  return (this._f & FLAG_Z) !== 0;
}
set zeroFlag(on: boolean) {
  this.f = on ? this._f | FLAG_Z : this._f & ~FLAG_Z;
}
```

Reading: `f & 0x80` leaves only the Z bit. If it's non-zero, the switch is on.

Writing: if turning it on, OR the bit in. If turning it off, AND with the inverse (clearing just that bit, leaving the other three alone).

The comment on line 145 explains the naming. The console calls these `Z N H C`, but **`H` and `C` are also the names of two registers**. Using the short names would collide — TypeScript would refuse to compile, and more importantly, a human reading `reg.c` couldn't tell which one was meant. So the flags are `zeroFlag`, `subtractFlag`, `halfCarryFlag`, `carryFlag`. The single letters stay reserved for registers.

### Lines 175–188: `postBoot()`

```ts
static postBoot(): Registers {
  const r = new Registers();
  r.af = 0x01b0;
  ...
  return r;
}
```

A real Game Boy runs a small hidden program when you switch it on. That program does some checks and leaves the registers at specific known values. Everyone emulating a Game Boy skips it and jumps straight to those values — because that startup program is Nintendo's code, and shipping it isn't legal.

This function is those values. `pc = 0x0100` is the entry point of the game.

It's a **static** function (called as `Registers.postBoot()`) rather than the default state, so that a plain `new Registers()` stays all-zero — which is what hardware actually looks like before that program runs, and what the tests want.

---

## Part 5 — The tests, and what they're actually for

`tests/registers.test.ts`, 8 tests. They aren't there to "get coverage". Each one exists because a specific thing would break invisibly without it:

| Test | The bug it catches |
| :--- | :--- |
| `a = 0x1234` → `0x34` | Someone removes the trimming |
| `l = -1` → `0xff` | Trimming works on negative numbers too |
| `bc = 0x1234` → B=0x12, C=0x34 | The split is backwards |
| `h`, `l` set → `hl` joins correctly | `<<` got changed to `>>` |
| `f = 0xff` → `0xf0` | The unused bits come back to life |
| `af = 0x00ff` → F=0xf0 | The mask survives being written through a pair |
| setting one flag leaves the others alone | An instruction clobbers a flag it wasn't supposed to touch |
| `postBoot()` values | Someone "cleans up" those magic numbers |

That last flag test is the important one for what comes next. Once there are 500 instructions all setting flags, "did the add instruction quietly clear the carry flag?" is the question that eats a week. The test is one line of insurance.

---

## Part 6 — What the commands actually do

```bash
npm test
```

Runs every file in `tests/`, shows a pass/fail per test. No game runs, no browser. Just: does the register file still behave?

```bash
npm run typecheck
```

Reads every source file and reports type mistakes without producing any output. Catches "you're treating something possibly-empty as if it has a value" before it becomes a crash.

```bash
npm run build
```

Produces `dist/index.js` (plain JavaScript anyone can run) and `dist/index.d.ts` (notes telling other TypeScript projects what's available). This is the thing that would eventually be published.

---

## Part 7 — What does *not* exist yet

Worth being blunt, so nothing here feels mysterious:

- **Nothing runs a game.** There is no instruction loop, no graphics, no sound, no file loading.
- **The 64 KB of memory doesn't exist.** The CPU has its scratchpad and nothing to scratch at.
- **Only the scratchpad is tested.** Everything else is a plan.

The order is deliberate: the scratchpad is where every other bug would eventually show up, so it gets built and frozen first. When the instruction loop lands next, "is this new bug in the registers or in the new code?" already has half its answer.

Next block: memory — the 64 KB, the reads and writes, the mirrored region, and the address watcher the inspector will use.
