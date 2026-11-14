# Status

Step 1 of 7 is underway. The register file is written and tested. Memory is
next.

Last updated: 14 November 2026.

This page is narrow on purpose: it states what works, what does not, and what
comes next. The reasoning behind the project, and the reasoning behind each
decision, live on the other pages.

## Working today

- The project builds and type-checks. Running the build produces a JavaScript
  file and type declarations in `dist/`.
- The register file exists and is tested: seven byte registers, four flag
  bits, two 16-bit pointers, and the four pairs the console reads as single
  numbers.
- Eight tests pass, covering the cases most likely to fail later.

These were verified by running the checks, not by inspection:

```text
npm run typecheck   → clean
npm test            → 8 passed
npm run build       → dist/ written
```

## Not started

- Memory (next)
- The instruction loop — the part that actually executes a program
- Graphics
- Timer and buttons
- The public wrapper, save states, and the hardware inspector
- The small assembler that lets tests be written in text instead of hex bytes

## Step 1 gate

Step 1 is "scaffolding plus the register file plus memory". It is complete when
all of the following are true:

- [x] Writing a 16-bit value into a pair splits it into the two bytes correctly
- [x] The flags register keeps its bottom four bits clear
- [x] The compiler reports no errors
- [x] The build produces its output files
- [ ] Writing a 16-bit value stores the low byte first, the way the console does
- [ ] Writing to the mirrored address region shows up in the original address

The last two are the next block of work.

## Next block: memory

The plan for the memory layer, agreed before it is written:

- A single 64 KB buffer, not a collection of separate objects
- Byte reads and writes, and 16-bit reads and writes
- 16-bit values stored with the low byte first, the way the console does
- The mirrored region: writes to one address show up at a different address,
  because that region of the console's address space is a copy of another
- An address watcher, so a later inspector layer can observe changes
- Tests for each of these, including behaviour at the far end of the address
  space

## Decisions

Design decisions are recorded individually in [decisions/](decisions/README.md),
one file each, with the situation, the choice, and its consequences. The short
list of reasons behind the register file is on the decision pages, not repeated
here.
