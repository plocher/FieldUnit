# FieldUnit

A modular C++ framework for implementing Model Railroad Control Points and Interlockings.

## Architecture

FieldUnit models railroad plants using prototype signaling principles:
- **Snapshot Execution**: Evaluates plant state and commands atomically.
- **Interlocking Control Tables**: Declarative routes with switch correspondence, occupancy checking, and speed/route aspect derivation.
- **Appliance Behavioral Contracts**: Clean interfaces for Turnouts, Crossovers, Track Circuits, and Signal Masts.
- **Zero Runtime Heap Allocation**: Safe operation with no dynamic allocation after `setup()`.
- **Protocol Independence**: Sits above network codecs (CMRInet, MQTT, bespoke CodeLine).

See [docs/CONTROL_POINT_ARCHITECTURE.md](docs/CONTROL_POINT_ARCHITECTURE.md) for the complete design specification.
