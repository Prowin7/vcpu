# Status

**Where we are:** step 1 of 7. The register file is written and tested. Memory is next.

Last updated: 14 November 2026.

This page is deliberately narrow: it says what works, what doesn't, and what
comes next. For why the project is shaped this way, read
[explanation/how-it-works.md](explanation/how-it-works.md). For why a specific
choice was made, read [decisions/](decisions/README.md).

## Working today

> New to the code? Read
> [explanation/what-the-code-does.md](explanation/what-the-code-does.md) first —
> it walks the register file line by line.

- The project builds and type-checks. Compiling produces a runnable JavaScript
  file plus type definitions.
- The **register file** exists: the CPU's working slots that hold one number
  each, plus the four flags, plus the shortcut that reads two of them as one
  bigger number.
- **8 tests pass**, covering the cases most likely to break later.

Checked by running, not assumed:

```text
npm run typecheck   → clean
npm test            → 8 passed
npm run build       → dist/ written
```

## Not started

- Memory (next)
- The instruction loop — the part that actually runs a program
- Graphics
- Timer and buttons
- The public wrapper, snapshots, and the inspector
- The small assembler that lets us write test programs in text instead of
  hand-typed numbers

## The plan, as a checklist

The project is split into 7 steps. Step 1 is "scaffolding plus the register
file plus memory", and it is done when these are all true.

- [x] Storing a two-part number into one register pair splits it correctly
- [x] The flags register refuses to hold anything in its unusable bottom four
      bits
- [x] The compiler reports no errors
- [x] The build produces its output files
- [ ] Writing two bytes as one larger number stores them small-end-first
- [ ] A write to the mirrored copy of memory shows up in the original

Those last two are the next block of work.

## Next up: memory

The plan for the next block, agreed before writing it:

- one flat 64 KB buffer, not a pile of separate objects
- read and write one byte, and read and write two bytes as one number
- two-byte numbers stored lowest byte first, the way the console does it
- the "mirror" region: part of the address space is a copy of another part, so
  writing to one shows up in the other
- the ability to watch an address and be told when it changes — this is the
  hook the inspector is built on
- tests for each of the above, including what happens at the very top of the
  address space

## Decisions

Design decisions are recorded as ADRs in [decisions/](decisions/README.md), one
file each, with the situation, the choice, and what it costs.
