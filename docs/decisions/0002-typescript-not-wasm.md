# 0002 — Write it in TypeScript, not Rust/C++ compiled to WebAssembly

**Status:** Accepted (14 November 2026)

## Context

Emulators are usually written in C, C++ or Rust and compiled to WebAssembly
for the browser, because that is the fastest option. The CPU being emulated
runs at 4,194,304 ticks per second, which is not a demanding target for modern
JavaScript.

## Decision

TypeScript, no runtime dependencies, one ESM build that runs unchanged in a
browser and in Node.

## Consequences

**Good:** bugs arrive with real stack traces and readable names. A compiled
core reports `RuntimeError: unreachable` at an anonymous function index, which
is a miserable place to debug 500 instructions from. Distribution is a single
JavaScript file plus type definitions — no bundler configuration, no
asynchronous module setup, no glue code.

**Bad:** measurably slower than a compiled core, roughly 1.3–1.7× on desktop
and more on older mobile hardware. If the project ever targets weak devices,
this is the constraint that shows up first.

**Mitigation:** the hot loop is self-contained and has no DOM or async
dependencies, so porting it later is a bounded job rather than a rewrite.

**Rejected alternative:** a WASM core would have moved the performance ceiling
up and the debuggability ceiling down. For a project whose point is to be
understood and inspected, that trade is backwards.
