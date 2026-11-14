# 0006 — Name flags in full rather than `Z N H C`

**Status:** Accepted (14 November 2026)

## Context

The console's documentation calls the four flag bits `Z`, `N`, `H` and `C`.
It also calls two of the registers `H` and `C`. Both namings are correct, and
they collide.

## Decision

Name the flag accessors `zeroFlag`, `subtractFlag`, `halfCarryFlag` and
`carryFlag`. Reserve the single letters for the registers.

## Consequences

**Good:** `reg.c` is unambiguous. The code compiles, and a reader does not have
to reconstruct which `c` was meant from context.

**Bad:** flag handling reads more verbosely — `reg.carryFlag = true` instead of
`reg.c = true` — in code that will be written hundreds of times.

**Note:** this was not a theoretical concern. The first implementation used the
short names and failed to compile with a duplicate member. The collision is
inherent to the hardware, not a style preference.
