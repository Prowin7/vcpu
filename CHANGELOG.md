# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project follows [Semantic Versioning](https://semver.org/spec/v2.0.0.html)
once it is published.

## [Unreleased]

### Added

- Register file: ten 8-bit/16-bit registers with masking enforced in the
  accessors, AF/BC/DE/HL read as pairs, four named flag accessors, and the
  post-boot state that removes the need for a boot ROM.
- C++17 project setup: a `Makefile` that builds with warnings as errors, and a
  dependency-free test runner.
- Documentation set: explanation pages for the project and for the code that
  exists, a reference glossary, a status page, and architecture decision
  records.

### Changed

- Moved the project from TypeScript to C++17 ([ADR 0007](docs/decisions/0007-cpp-not-typescript.md)).
  The register file keeps the same behavior; accessors are now `a()` /
  `set_a(value)`, flags are `zero_flag()` / `set_zero_flag(on)`, and
  `postBoot()` is now `set_post_boot_values()`.

### Removed

- The Node.js toolchain: `package.json`, the lockfile, `tsconfig.json` and
  `src/index.ts`.
