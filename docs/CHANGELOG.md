# Changelog

All notable changes to the FieldUnit library will be documented in this file.

## [Unreleased]

### Added
- **`AarTextCodec`**: Human-readable, self-documenting AAR symbolic token codec for CodeLine and MQTT transports:
  - **Conservative Producer**: Emits indication snapshot vectors strictly in declared order using authentic `'K'` suffix (`1NWK`, `(1RWK)`, `1T1K`, `2NGK`, `(2SGK)`, `(2TEK)`, `MC1K`).
  - **Liberal Consumer**: Parses control messages in any sequence with whitespace and case tolerance.
  - **Mandatory Suffixes**: Enforces `'S'` suffix on inbound controls (`1NWS`, `1RWS`, `2SGS`, `2NGS`, `2HS`, `MC1S`) and rejects invalid/ambiguous tokens.
  - **Vital Isolation ("Don't Poke a Sleeping Bear")**: Conflicting vital commands flag `vitalValid = false` on `ControlTransaction` without mutating received demands. `ControlPoint` skips all vital appliance invocation while still applying non-vital commands (maintainer call).
  - **Fault Monitoring**: Tracks unknown symbol counts, last unknown symbol, and vital conflict counts.
- **Appliance Self-Indexing**: `Switch`, `SignalControl`, and `TrackCircuit` store their registered index within `ControlPoint` upon creation, eliminating hardcoded magic integers in codec mappings.
- **`BitPackedCodec`**: Dedicated binary bitstream codec for C/MRI IB/OB dense byte arrays, supporting `padToByte()` and `skipBits(N)` alignment helpers.
- **Unit Tests**: Added `tests/test_wire_codec.cpp` covering symbolic parsing, indication formatting, vital isolation, and binary streaming. Added `tests/test_corporal_sketch.cpp` for end-to-end sketch verification of CP Corporal.

### Changed
- Refactored `examples/CP_Christopher/CP_Christopher.ino` and `examples/CP_Corporal/CP_Corporal.ino` to use authentic AAR appliance names (`"1"`, `"3"`, `"5"`, `"2"`, `"4"`) and fluent `AarTextCodec` declarations.
- Updated `tests/test_christopher_sketch.cpp` to verify end-to-end sketch behavior driven by AAR text snapshots.
- Updated documentation and tutorials to use authentic AAR appliance identifiers.
