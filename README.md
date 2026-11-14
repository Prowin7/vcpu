# vcpu

A Game Boy emulator written in TypeScript, built to be looked inside.

You drop a game file (`.gb`) onto a web page, the game starts, and you can
pause it to see the CPU's registers, walk through memory, and watch the
graphics memory change. Most emulators show you the game. This one is built to
show you the machine behind it.

It runs in a browser tab and in a terminal, with no third-party libraries at
runtime.

## Status

Early, on purpose. The register file — the CPU's small set of working slots —
is written and tested. Main memory is next. Nothing can run a game yet.

See [docs/status.md](docs/status.md) for exactly what works today.

## Documentation

| I want to… | Read |
| :--- | :--- |
| Understand the code that exists, line by line | [docs/explanation/what-the-code-does.md](docs/explanation/what-the-code-does.md) |
| Understand the project and its rules | [docs/explanation/how-it-works.md](docs/explanation/how-it-works.md) |
| Know what works today and what's next | [docs/status.md](docs/status.md) |
| Look up a term like "V-Blank" or "register" | [docs/reference/glossary.md](docs/reference/glossary.md) |
| Know why a choice was made | [docs/decisions/](docs/decisions/README.md) |

The full index is [docs/README.md](docs/README.md).

## Getting started

Requires Node.js 20 or newer.

```bash
npm install
npm test        # run the tests
npm run build   # compile to JavaScript in dist/
npm run typecheck
```

## Project layout

```text
src/
├── index.ts            public exports
├── cpu/Registers.ts    the CPU's registers and flags
├── mmu/  ppu/  io/     memory, graphics, timer and buttons (not yet written)
└── sdk/                headless runner and canvas renderer (not yet written)
tests/                  unit tests
docs/                   documentation, organised by purpose
```

## What we're aiming at

The finished thing should be provably correct, not just "seems to run":

- It runs known test programs written by the Game Boy homebrew community, and
  passes them.
- It draws a well-known test image exactly right, pixel for pixel.
- It runs real homebrew games that people have released for free.

Those tests already exist and are free to use. Passing them is the whole point
of imitating a real console instead of inventing our own.

## Contributing

Issues and pull requests are welcome. See [CONTRIBUTING.md](CONTRIBUTING.md).

## License

MIT. See [LICENSE](LICENSE).

This project contains no game files and no proprietary code. Bring your own
`.gb` file.

## Changelog

See [CHANGELOG.md](CHANGELOG.md).
