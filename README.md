# vcpu

A Game Boy emulator written in TypeScript.

The idea is simple: you drop a game file (`.gb`) onto a web page, the game starts, and you can pause it and look inside — see the CPU's registers, walk through memory, watch the graphics memory change. Most emulators show you the game. This one is built to show you the machine behind it.

It works in a browser tab and in a terminal, with no third-party libraries at runtime.

## Where it is right now

Early days, on purpose. The register file — the CPU's small set of working slots that hold numbers — is written and tested. Main memory is next.

- [docs/what-the-code-does.md](docs/what-the-code-does.md) — **start here.** Line-by-line tour of the code that exists, in plain words
- [docs/progress.md](docs/progress.md) — what works today, what's next
- [docs/how-it-works.md](docs/how-it-works.md) — the parts, the rules, and a glossary

## Running it

```bash
npm install
npm test        # run the tests
npm run build   # compile to JavaScript in dist/
npm run typecheck
```

## What we're aiming at

The finished thing should be provably correct, not just "seems to run":

- It runs known test programs written by the Game Boy homebrew community, and passes them.
- It draws a well-known test image exactly right, pixel for pixel.
- It runs real homebrew games that people have released for free.

Those tests exist already and are free to use. Passing them is the whole point of choosing a real console to imitate instead of inventing our own.

## Licence

This project contains no game files and no proprietary code. Bring your own `.gb` file.

MIT.
