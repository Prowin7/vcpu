# What the code does

Read this if `value & BYTE_MASK` looks like soup. It walks through the only real file we have written, in order, and explains what each part is *for*.

**Short version:** we have written the CPU's scratchpad. about 130 lines. It cannot run a game yet, and it isn't supposed to.

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
Makefile                 Builds the test with the C++ compiler and runs it.
.gitignore               Files that must never be committed: build output and
                         game files.
src/cpu/Registers.hpp    THE CODE. Everything below is about this file.
tests/registers_test.cpp The checks that prove the above works.
docs/                    These explanations.
```

`Registers.hpp` is a header file. Any file that writes `#include "Registers.hpp"` gets the whole class, because the code lives in the header itself.

---

## Part 4 — Walking `src/cpu/Registers.hpp`

### The top

```cpp
#pragma once

#include <cstdint>
using namespace std;
```

- `#pragma once` — read this file only once, even if several files include it.
- `#include <cstdint>` — gives us `uint8_t` (a number that is exactly 8 bits, so 0–255) and `uint16_t` (exactly 16 bits, 0–65535).
- `using namespace std;` — lets us write `uint8_t` instead of `std::uint8_t`. Because it is in a header, every file that includes this one gets it too. That is fine for a small project like this one.

### The four flags

```cpp
const int FLAG_Z = 0x80;
const int FLAG_N = 0x40;
const int FLAG_H = 0x20;
const int FLAG_C = 0x10;
```

The F register doesn't store a number. It stores four on/off switches, one per bit, in the *top* four bits of a byte:

```text
  bit:   7  6  5  4  3  2  1  0
        [Z][N][H][C][0][0][0][0]
  0x80 = 1000 0000  → the Z switch
  0x40 = 0100 0000  → the N switch
```

`0x80` is just the number whose 7th bit is on. So these constants aren't flag *values*, they're **masks**: a tool for picking out one switch.

- **Z (zero)** — "the answer was 0". Games check this constantly: *did the counter reach zero?*
- **N (subtract)** — "the last calculation was a subtraction". Only used to make decimal arithmetic work.
- **H (half-carry)** — "carrying out of the 4th bit". Exists purely for decimal arithmetic.
- **C (carry)** — "it overflowed past 255, or borrowed below 0".

### The storage (bottom of the file)

```cpp
 private:
  uint8_t a_ = 0;
  uint8_t b_ = 0;
  ...
  uint16_t pc_ = 0;
  uint16_t sp_ = 0;
```

Ten private numbers, all starting at 0. **This is the entire state of the CPU.** Everything the game knows lives here, or in the 64 KB of memory outside.

`private` means no other file can touch `a_` directly. The only way in is the setter. The underscore at the end is just a habit that says "this is private data".

### One register, as the pattern for all ten

```cpp
uint8_t a() { return a_; }
void set_a(int value) { a_ = value & 0xff; }
```

`a()` returns the stored number. `set_a(300)` stores `44`, because `300 & 0xff = 44`.

The setter takes an `int` so that instructions can hand it any result — too big, or even negative — and the setter cuts it down to 8 bits. `set_a(-1)` stores `255`.

**Why bother?** Otherwise ~500 instructions each have to remember to trim their own numbers, and one that forgets silently corrupts everything downstream. Here the trimming happens once, in the setter, so no code can put an invalid number into A.

The other six registers (B, C, D, E, H, L) are identical. Skim them.

### F, which is special

```cpp
void set_f(int value) { f_ = value & 0xf0; }
```

The bottom four bits of F **do not exist** on real hardware, so they are forced to zero. `0xf0` keeps the top 4 bits and wipes the bottom 4.

### PC and SP

Same pattern, but `& 0xffff` because they hold addresses, which are 0–65535. Write `70000` into PC and you get `4464`, so PC can never run off the end of memory.

### The pairs

```cpp
uint16_t bc() { return (b_ << 8) | c_; }
void set_bc(int value) {
  set_b(value >> 8);
  set_c(value);
}
```

To get BC, push B up 8 places and put C underneath. To set BC, split the number.

Worked example — `set_bc(0x1234)`:

```text
  value >> 8  = 0x12  →  stored in B
  value       = 0x34  →  stored in C  (the setter keeps only the low 8 bits)
```

and reading it back:

```text
  (0x12 << 8) | 0x34  =  0x1200 | 0x34  =  0x1234
```

**Why not store BC as its own number?** Then B, C and BC would all hold the same information, and code that updates one and forgets the others leaves the register disagreeing with itself.

The setter calls `set_b` and `set_c`, not `b_ = ...`, so the trimming rule still applies.

### The four flag switches

```cpp
bool zero_flag() { return (f_ & FLAG_Z) != 0; }
void set_zero_flag(bool on) {
  if (on) {
    set_f(f_ | FLAG_Z);
  } else {
    set_f(f_ & ~FLAG_Z);
  }
}
```

Reading: `f_ & 0x80` leaves only the Z bit. If that is not zero, the switch is on.

Writing: to turn it on, OR the bit in. To turn it off, AND with the opposite (`~`), which clears that one bit and leaves the other three alone.

The other three flags repeat this exactly, with their own mask.

The flags are named in full (`zero_flag`, `carry_flag`, ...) because the console calls them `Z N H C`, and **`H` and `C` are also the names of two registers**. A reader seeing `c()` couldn't tell which one was meant.

### `set_post_boot_values()`

```cpp
void set_post_boot_values() {
  set_af(0x01b0);
  set_bc(0x0013);
  ...
  set_pc(0x0100);
}
```

A real Game Boy runs a small hidden program when you switch it on. It leaves the registers at known values. Everyone emulating a Game Boy skips it and sets those values directly — the startup program is Nintendo's code, and shipping it isn't legal.

`pc = 0x0100` is where games begin. A plain `Registers r;` stays all-zero, which is what the hardware looks like before the startup program runs.

---

## Part 5 — The tests, and what they're actually for

`tests/registers_test.cpp` calls a small `check(name, actual, expected)` function and prints `pass` or `FAIL` for each one. Each group exists because a specific thing would break invisibly without it:

| Check | The bug it catches |
| :--- | :--- |
| `set_a(0x1234)` → `0x34` | Someone removes the trimming |
| `set_l(-1)` → `0xff` | Trimming works on negative numbers too |
| `set_bc(0x1234)` → B=0x12, C=0x34 | The split is backwards |
| `h`, `l` set → `hl()` joins correctly | `<<` got changed to `>>` |
| `set_f(0xff)` → `0xf0` | The unused bits come back to life |
| `set_af(0x00ff)` → F=0xf0 | The mask survives being written through a pair |
| setting one flag leaves the others alone | An instruction clobbers a flag it wasn't supposed to touch |
| `set_post_boot_values()` | Someone "cleans up" those magic numbers |

The flag check is the important one for what comes next. Once there are 500 instructions all setting flags, "did the add instruction quietly clear the carry flag?" is the question that eats a week.

---

## Part 6 — What the command does

```bash
make test
```

Compiles the test, then runs it. The compiler runs with `-Wall -Wextra -Wpedantic -Werror`: every warning is turned on, and any warning stops the build. No game runs, no browser. Just: does the register file still behave?

```bash
make clean
```

Deletes the `build/` folder.

---

## Part 7 — What does *not* exist yet

Worth being blunt, so nothing here feels mysterious:

- **Nothing runs a game.** There is no instruction loop, no graphics, no sound, no file loading.
- **The 64 KB of memory doesn't exist.** The CPU has its scratchpad and nothing to scratch at.
- **Only the scratchpad is tested.** Everything else is a plan.

The order is deliberate: the scratchpad is where every other bug would eventually show up, so it gets built and frozen first. When the instruction loop lands next, "is this new bug in the registers or in the new code?" already has half its answer.

Next block: memory — the 64 KB, the reads and writes, the mirrored region, and the address watcher the inspector will use.
