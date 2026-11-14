# Contributing

This is a learning project in early development, but issues and pull requests
are welcome.

## Getting set up

Requires Node.js 20 or newer.

```bash
npm install
npm test
npm run typecheck
```

## Before opening a pull request

- `npm test` passes.
- `npm run typecheck` reports nothing.
- `npm run build` succeeds.

## House rules

**No ROMs, ever.** No game files, no boot ROM, no test ROM binaries. Test
suites are fetched by script into `tests/roms/`, which git ignores. A pull
request containing a ROM will be closed.

**Every claim in a comment or doc should be checkable.** If a comment says
"this cannot overflow", there should be a test that would fail if it could.

**One idea per file.** See [docs/](docs/README.md) for how the docs are
organised, and follow the same split for new pages.

**Decisions get recorded.** If you change a design decision, add or supersede
an entry in [docs/decisions/](docs/decisions/README.md) rather than editing
history.
