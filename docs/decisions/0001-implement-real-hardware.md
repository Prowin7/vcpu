# 0001 — Imitate real Game Boy hardware, not a custom CPU

**Status:** Accepted (14 November 2026)

## Context

Two very different projects could wear the same description. One invents an
8-bit CPU, its own instruction set, and programs to run on it. The other
reimplements the CPU that is already inside a 1989 Game Boy.

A custom instruction set has no wrong answers, which sounds easier and is
actually harder: there is nothing to check the work against, so every bug is
discovered by staring at it. Real hardware comes with a published manual and
with test programs written by the homebrew community that report their own
failures.

## Decision

Implement the Sharp LR35902, the CPU in the original Game Boy.

## Consequences

**Good:** correctness becomes checkable rather than arguable. Real games and
homebrew titles can run. The definition of "done" is bounded by suites that
already exist.

**Bad:** the niche is crowded — there are nine or more JavaScript Game Boy
emulators, two published within a month of starting. There is no novelty in
the emulator itself.

**Mitigation:** the novelty has to come from what surrounds it — the headless
test harness and the hardware inspector — not from the emulation core. This is
why those are milestones, not extras.

**Rejected alternative:** a small custom instruction set with its own
assembler would have been genuinely original, and was cut because "original
with no oracle" is worth less here than "verified against someone else's
standard".
