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
- Strict TypeScript project setup with `tsup` for the ESM build and `.d.ts`
  emission, and `vitest` for tests.
- Documentation set: explanation pages for the project and for the code that
  exists, a reference glossary, a status page, and architecture decision
  records.

### Notes

- Not published to npm. The package is marked `"private": true` until the
  definition-of-done checklist is satisfied.
