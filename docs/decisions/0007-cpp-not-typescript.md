# 0007 — Write it in C++, superseding TypeScript

**Status:** Accepted (14 November 2026). Supersedes [0002](0002-typescript-not-wasm.md).

## Context

0002 chose TypeScript so the same build would run in a browser and in Node,
with readable stack traces. The project owner has decided to move the core to
C++17 instead.

## Decision

The emulator core is C++17, header-only for now, with no third-party
libraries. The code is kept deliberately plain, with `using namespace std;`
and no helper layers. It builds with the system compiler through a `Makefile`.

## Consequences

**Good:** a faster core, with no performance ceiling to worry about on weak
devices. Integer widths are real types (`uint8_t`, `uint16_t`), so the
hardware's "8 bits" and "16 bits" are part of the type system.

**Bad:** the browser is no longer a free target. Running in a tab now needs a
WebAssembly build (for example with Emscripten), plus glue code for the web
page. That is exactly the cost 0002 avoided, and it is not set up yet. A
compiled core is also harder to debug from the browser than plain JavaScript.

**Still true from 0005:** register setters take `int` and mask at the edge, so
overflow and negative values are truncated in one place.

**Follow-up:** decide how the inspector reaches the core (WebAssembly in the
browser, or a native front end) before the memory work grows a watch-an-address
hook that depends on the answer.
