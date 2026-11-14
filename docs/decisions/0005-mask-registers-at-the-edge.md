# 0005 — Enforce bit widths inside the register setters

**Status:** Accepted (14 November 2026)

## Context

A register holds 0–255. An address register holds 0–65535. Nothing in the
language stops code from storing 300 in an 8-bit register, and once ~500
instructions exist, every one of them is a place that could do it.

The failure is not loud. A register quietly holding 300 produces nonsense
addresses, and the emulator wanders off into unallocated memory with no error.

## Decision

Store registers as private fields and expose them only through accessors that
mask: the `set_a` setter applies `& 0xff`, the `set_pc` setter applies `& 0xffff`.
Nothing outside the class can reach the raw field.

## Consequences

**Good:** the invariant is enforced in one place, so it cannot be violated by
any future instruction. The classic "program counter runs away" bug becomes
impossible to write rather than merely unlikely. Cost is about ten extra lines
per file.

**Bad:** slightly more code than public fields, and a small amount of call
overhead in the hottest part of the emulator. Neither matters at the target
speed.

**Alternative rejected:** mask inside each instruction. Cheaper to write once,
and every one of the 500 implementations becomes individually responsible for
the same correctness rule — the exact shape of bug that takes a week to find.

**Related:** the 16-bit pairs are computed from the byte registers rather than
stored separately, for the same reason — two copies of the same information
eventually disagree.
