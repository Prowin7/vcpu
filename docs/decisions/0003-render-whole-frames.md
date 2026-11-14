# 0003 — Draw a whole frame at once instead of pixel-perfect timing

**Status:** Accepted (14 November 2026)

## Context

Real hardware draws the screen one scanline at a time and, within each line,
one pixel at a time. Games can exploit that: they change registers *while* a
line is being drawn to produce split-screen effects, parallax and wobble.

Reproducing that faithfully requires emulating the pixel pipeline, which is the
single largest time sink in the whole project — months, not weeks.

## Decision

Render the entire 160×144 frame during V-Blank, from whatever is in memory at
that moment. Report the timing flags plausibly, and support the one timing
feature that correctness actually depends on: the line-coincidence interrupt.

## Consequences

**Good:** the community's standard graphics test (`dmg-acid2`) exists precisely
to validate this simpler approach — its own author states that a line-based
renderer is sufficient and that it does not require pixel-level timing. So the
cut costs nothing that is being measured.

**Bad:** games that draw mid-frame will produce wrong output. Split-screen
status bars and parallax effects will not look right. This is a real, permanent
limitation and must be documented rather than glossed over.

**Rejected alternative:** cycle-accurate pixel rendering. Estimated at 60–120
hours *after* the 90–130 hours to get this far. Sunk cost for everything except
raster tricks.
