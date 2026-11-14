# vcpu

A Game Boy emulator written in TypeScript, built so the internals are visible
while it runs.

The idea is straightforward: load a `.gb` game file, run it, and be able to
pause it to inspect the CPU's registers, memory, graphics memory, tiles and
sprites. Most emulators show the game. This one is built to show the machine
behind it.

It is designed to run the same way in a browser tab and in a terminal, with no
third-party libraries at runtime.

## Status

Early, by design. The register file — the CPU's working storage — is written
and tested. Main memory is next. Nothing can run a game yet.

See [docs/status.md](docs/status.md) for an exact, current account.

## Documentation

| I want to… | Read |
| :--- | :--- |
| Understand the project and why it is built this way | [docs/explanation/how-it-works.md](docs/explanation/how-it-works.md) |
| Understand the code that exists, line by line | [docs/explanation/what-the-code-does.md](docs/explanation/what-the-code-does.md) |
| Know what works today and what is next | [docs/status.md](docs/status.md) |
| Look up a term like "register" or "V-Blank" | [docs/reference/glossary.md](docs/reference/glossary.md) |
| Know why a design choice was made | [docs/decisions/](docs/decisions/README.md) |

The full index is [docs/README.md](docs/README.md).

## Getting started

Requires Node.js 20 or newer.

```bash
npm install
npm test        # run the tests
npm run build   # compile to JavaScript in dist/
npm run typecheck
```text

## Project layout

```text
src/
├── index.ts            public exports
├── cpu/Registers.ts    the CPU's registers and flags
├── mmu/  ppu/  io/     memory, graphics, timer and buttons (not yet written)
└── sdk/                headless runner and canvas renderer (not yet written)
tests/                  unit tests
docs/                   documentation, organised by purpose
```text

## What is the point

A project is worth finishing when its correctness is checkable, not when it
seems to run. The finished emulator should therefore:

- Run known test programs written by the Game Boy homebrew community, and pass
  them.
- Draw a well-known graphics test image exactly right, pixel for pixel.
- Run real homebrew titles.

Those test programs already exist and are freely redistributable. That existing
standard is why the project imitates a real console instead of inventing one.

## Contributing

Issues and pull requests are welcome. See [CONTRIBUTING.md](CONTRIBUTING.md).

## License

MIT. See [LICENSE](LICENSE).

This project contains no game files and no proprietary code. Supply your own
`.gb` file.

## Changelog

See [CHANGELOG.md](CHANGELOG.md).
