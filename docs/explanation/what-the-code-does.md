# What the code does

This page walks through the only source file that exists so far, in file order,
and explains what each part does and why it is written that way.

In short: the file holds the CPU's working storage — seven byte-sized
registers, one set of four flags, two 16-bit pointers, and the four
flag/register pairs that the Game Boy reads as single numbers. It cannot run a
game, and it is not supposed to.

---

## What the file represents

A CPU has very limited storage of its own. Everything it operates on lives in a
small set of registers, each named and each holding one byte, so a value from 0
to 255. The Game Boy has seven of these: A, B, C, D, E, H, L. Two more, PC
and SP, hold 16-bit addresses: the current instruction position, and the stack
pointer.

The registers are the only state the CPU "owns". Everything else — the game,
the screen data, the save file — lives outside, in the 64 KB of main memory,
and the CPU reaches it by address.

Two behaviours make this file non-obvious.

First, some registers are read as pairs. The console reads B and C together as
one 16-bit number called BC, with B as the high byte and C as the low byte.
Treatments of this kind exist for DE, HL and AF as well.

Second, registers wrap. A byte register stores 0–255, so an arithmetic result
larger than 255 loses the extra bits and sets a carry flag. This is not a
shortcoming; it is how the hardware behaves, and how larger arithmetic is built
out of 8-bit operations.

---

## The constants

The file begins by naming the four flags as bit masks:

```ts
export const FLAG_Z = 0x80;
export const FLAG_N = 0x40;
export const FLAG_H = 0x20;
export const FLAG_C = 0x10;
```text

Bits are usually numbered from 0 at the bottom. The four flag bits sit at 7, 6,
5 and 4:

```text
  bit:   7  6  5  4  3  2  1  0
         [Z][N][H][C][0][0][0][0]
```text

Each constant is the byte with exactly that one bit set, so `FLAG_Z` is `0x80`,
the value `1000 0000` in binary. They are masks, not flag values: they are the
tools used to test or change a single bit in a byte.

Their meanings:

- **Z** — the result of the last operation was zero. A program checks this
  frequently, for example to see whether a counter reached zero.
- **N** — the last operation was a subtraction. Used by a specific decimal
  adjustment instruction and by very little else.
- **H** — half-carry. A carry out of bit 3. Exists for the decimal adjustment
  correct, and reads from it almost nowhere else.
- **C** — carry. A carry out of bit 7, or a borrow below zero.

---

## The masks

Three masks follow, so bit patterns can be named rather than repeated as hex
literals:

```ts
const BYTE_MASK = 0xff;   // 0000 0000 1111 1111
const WORD_MASK = 0xffff; // 1111 1111 1111 1111
const FLAG_MASK = 0xf0;   // 1111 0000
```text

`BYTE_MASK` keeps 8 bits, `WORD_MASK` keeps 16, `FLAG_MASK` keeps the top 4 and
clears the bottom 4. They are named so the intent is readable in the code and so
the pattern can be changed in one place if it ever needs to be.

---

## The storage

The file declares ten private fields, all initialised to zero:

```ts
private _a = 0;
private _b = 0;
private _c = 0;
private _d = 0;
private _e = 0;
private _h = 0;
private _l = 0;
private _f = 0;
private _pc = 0;
private _sp = 0;
```text

This is the entire state of the CPU: ten numbers, and that is all. Nothing
outside this class can read or write these fields directly. Access is possible
only through the property accessors below.

---

## One register, in full

Each register gets a getter and a setter. This is the pattern for A:

```ts
get a(): number {
  return this._a;
}
set a(value: number) {
  this._a = value & BYTE_MASK;
}
```text

Reading `a` gives the stored value. Writing `a = 300` stores 44, because `300
& 0xff` is 44.

This exists so that no code can ever put an out-of-range value into a register.
With 500 instructions to write in the future, each one doing arithmetic into
these registers, a single instruction that forgets to trim its result would
create an invalid register value and gradually send the system to nonsense
addresses. Enforcing the range in the setter means that fault is removed as a
possibility, rather than requiring every instruction to do it correctly.

That is the trade the whole file is built around: a small amount of extra code
at the boundary, in return for making a whole category of bug structurally
impossible.

The other six byte registers — B, C, D, E, H, L — use the same pattern
identically.

---

## The flags register

The flags register differs from the byte registers because only its top four bits
exist on the hardware:

```ts
get f(): number {
  return this._f;
}
set f(value: number) {
  this._f = value & FLAG_MASK;
}
```text

The setter keeps only the top four bits and discards the rest. The getter
returns the internal value unchanged, because the setter guarantees nothing else
is there. This asymmetry is deliberate: the mask is applied once, at the only
valid write path.

---

## PC and SP

Program counter and stack pointer follow the same pattern, but they are
16-bit so they use `WORD_MASK`:

```ts
get pc(): number {
  return this._pc;
}
set pc(value: number) {
  this._pc = value & WORD_MASK;
}
```text

The effect is that a program counter can never hold an out-of-range address. A
value written outside 0–65535 wraps into range automatically, which prevents the
emulator from wandering off into invalid memory with no reasoning.

---

## The 16-bit pairs

The console reads pairs of registers as single 16-bit values. The file provides
accessors for them, reconstructing them from the byte registers rather than
storing them separately. For BC:

```ts
get bc(): number {
  return (this._b << 8) | this._c;
}
set bc(value: number) {
  this.b = value >> 8;
  this.c = value;
}
```text

Reading BC combines B shifted up 8 bits with C dropped below: B becomes the high
byte, C the low byte.

Writing BC splits a value back: the high byte of `value` goes to B, the low byte
to C. The write goes through the byte setters, so trimming still applies.

The other pairs, DE and HL, use the same pattern. AF pairs A with the flags
register instead.

This is important for the same reason the getters exist: a separate stored copy
of each pair would mean two sources of truth for the same bits. If one source
were updated and the other was not, the registers would disagree quietly, and
each individual instruction would still look correct. Computing the pair removes
that whole fault.

---

## The flag switches

The four flags can also be read and set as individual on/off values rather than
as bit masks:

```ts
get zeroFlag(): boolean {
  return (this._f & FLAG_Z) !== 0;
}
set zeroFlag(on: boolean) {
  this.f = on ? this._f | FLAG_Z : this._f & ~FLAG_Z;
}
```text

Reading asks whether the Z bit is set. Writing turns just that bit on, or just
that bit off, leaving the others untouched.

Why the flags are not called `Z`, `N`, `H`, `C` is a practical point: the
hardware uses those exact letters for both two flags and two registers, so the
names would collide in the code. The flags take the longer forms, and the
single-letter names remain free for the registers.

---

## `postBoot()`

The file ends with a static function that returns the registers in the state the
console's startup program would have left them:

```ts
static postBoot(): Registers {
  const r = new Registers();
  r.af = 0x01b0;
  r.bc = 0x0013;
  r.de = 0x00d8;
  r.hl = 0x014d;
  r.sp = 0xfffe;
  r.pc = 0x0100;
  return r;
}
```text

A real Game Boy runs a hidden startup program at power-on. That program is
Nintendo's code, so it cannot be included. Instead, the emulator skips it and
jumps directly to the state the program would have produced, with the program
counter at `0x0100`, the game's entry point.

The function is static rather than the class default, so `new Registers()` stays
zero throughout — the real pre-boot state, and the state the tests want — while
`postBoot()` is used explicitly when it matters.

---

## The tests

Eight tests exist for this file, each covering one thing that would otherwise
break invisibly:

| Test | What it catches |
| :--- | :--- |
| Writing 0x1234 to A gives 0x34 | Trimming is removed |
| Writing -1 to L gives 0xff | Trimming works on negative values |
| Writing 0x1234 to BC splits into B=0x12, C=0x34 | High/low split is correct |
| Setting H and L then reading HL gives the joined value | The join is correct, not reversed |
| Writing 0xff to F gives 0xf0 | The unused bits stay zero |
| Writing 0x00ff to AF gives A=0, F=0xf0 | The mask survives writing through a pair |
| Setting one flag leaves the others unchanged | An instruction does not clobber unrelated flags |
| `postBoot()` returns the documented values | The startup state is not accidentally changed |

The last flag test matters most for what comes next. Once there are hundreds of
instructions setting flags, the question "did this instruction clear the carry
flag when it should not" becomes the kind of question that is hard to answer
without a targeted test.

---

## What does not exist yet

It is worth stating plainly what is missing, so the file is not over-read.

- No instruction loop runs. The CPU has storage, not execution.
- No memory exists. There is a scratchpad and no place to store larger program
  data.
- No graphics, timer, buttons or public wrapper exist.
- Only this one file is tested.

The order is deliberate. This is the layer where every later bug would
eventually show up, so it is written and tested before anything else. When the
instruction loop arrives next, the question "is this new bug in the instruction
logic or in the register storage" is already partly answered.

The next block is memory: 64 KB of storage, byte and word reads and writes,
the mirrored address region used by the console for parts of its own space, and
an address watcher that the later inspector layer will use.
