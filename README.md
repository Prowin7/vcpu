# vcpu

A Game Boy (DMG) virtual machine in TypeScript: zero runtime dependencies, ESM only, runs the same in the browser and in Node, and can be inspected while it runs.

**Status: M0 in progress.** Registers are done and tested. Memory bus is next. See [docs/progress.md](./docs/progress.md) for the exact state and [docs/architecture.md](./docs/architecture.md) for how the pieces fit.

Target: passes blargg `cpu_instrs`, `instr_timing`, `mem_timing`, and renders `dmg-acid2` pixel-perfect.

```bash
npm install
npm test        # unit tests
npm run build   # ESM + .d.ts into dist/
npm run typecheck
```

This project contains no ROMs and no proprietary code. Supply your own.

MIT.
