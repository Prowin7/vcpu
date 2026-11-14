# Decisions

Each file here records one design decision in the
[ADR](https://cognitect.com/blog/2011/11/15/documenting-architecture-decisions)
format: the situation, the choice, and what it costs. Decisions are never
rewritten; when one changes, a new record supersedes it.

| # | Decision | Status |
| :--- | :--- | :--- |
| [0001](0001-implement-real-hardware.md) | Imitate real Game Boy hardware, not a custom CPU | Accepted |
| [0002](0002-typescript-not-wasm.md) | Write it in TypeScript, not Rust/C++ compiled to WebAssembly | Accepted |
| [0003](0003-render-whole-frames.md) | Draw a whole frame at once instead of pixel-perfect timing | Accepted |
| [0004](0004-skip-the-boot-rom.md) | Skip the console's startup program | Accepted |
| [0005](0005-mask-registers-at-the-edge.md) | Enforce bit widths inside the register setters | Accepted |
| [0006](0006-spell-out-flag-names.md) | Name flags in full rather than `Z N H C` | Accepted |
