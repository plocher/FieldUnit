# 0001. MQTT Broker and AAR Tokens for the CodeLine Interface

- Status: accepted
- Date: 2026-09-12

## Context

Centralized Traffic Control (cTc) office machines communicate with trackside control points across the CodeLine Interface.
Historically, dispatcher consoles were either custom desktop software (such as JMRI or custom sockets) or dedicated physical hardware machines (such as 16-column US&S Model 503 consoles driven by microcontrollers).
In prior implementations, these consoles suffered from severe "Don't Repeat Yourself" (DRY) violations:
- Physical consoles (such as `test-ctc-south.ino`) hardcoded hundreds of binary packet offsets and pin mappings.
- Virtual panels duplicated field naming and layout logic in application memory.
- Adding or modifying an appliance in a trackside field unit required manual, error-prone updates across field code, virtual panels, and physical console firmware.

The architecture requires an open message bus and unified data contract where:
1. FieldUnit Studio's virtual cTc console and physical hardware cTc panels are both **first-class peers**.
2. A single authoritative, machine-readable specification defines each Control Point without code duplication.
3. Consoles support a robust operational lifecycle: dynamic discovery, cold-start state synchronization, atomic control execution, and health telemetry.
4. The transport supports physical layouts with network brokers, offline workstations without external networks, and automated CI test runners.

## Decision

We select an MQTT message broker, symbolic Association of American Railroads (AAR) text tokens, and declarative JSON Control Point schemas for the CodeLine Interface.

### Topic Structure

The system uses this topic hierarchy:
`/layout/<layoutname>/codeline/<controlpointname>/<channel>`

The hierarchy defines five channels:
1. `controls`: Transmits dispatcher requests. Messages are not retained.
2. `indications`: Transmits verified field plant states. Messages are retained (QoS 1).
3. `json`: Transmits the complete Control Point schema and topology. Messages are retained (QoS 1).
4. `telemetry`: Transmits heartbeat, connection state, and operational health. The Last Will and Testament (LWT) marks offline state.
5. `info`: Transmits hardware mappings, firmware versions, and diagnostic logs.

### Payload Encoding

The system encodes plant controls and indications as comma-separated AAR text tokens:
- Control tokens end with `S` (such as `1NWS`, `(1RWS)`, `2NGS`, `2HS`).
- Indication tokens end with `K` (such as `1NWK`, `(1RWK)`, `1T1K`, `2NGK`, `2TEK`).
- An unparenthesized token means asserted or true.
- A parenthesized token means unasserted or false.

### First-Class Panel Entities (Virtual and Physical)

FieldUnit Studio (virtual software console) and microcontroller-driven hardware desks (physical US&S/GRS consoles) are symmetrical, first-class clients of the CodeLine Interface:
- **Zero Interlocking Logic in Panels**: Neither panel type evaluates vital safety logic. Both operate purely in the supervisory Control Plane.
- **Unified Schema Consumption**: Both panels read the authoritative definition from the `json` channel.
- **Concurrent Coexistence**: Physical and virtual consoles can run simultaneously on the same layout. A command transmitted by a physical panel updates the virtual panel lamps, and vice-versa, because both listen to the authoritative field `indications` channel.

### JSON-Based Control Point Schemas

The `json` topic publishes a declarative, versioned specification of each Control Point:
- Enumerates all appliances (switches, crossovers, track circuits, signal masts, maintainer calls).
- Defines console column layout (associated switch levers, signal levers, and code button addresses).
- Documents expected control tokens and indication tokens.

Field devices or Studio publish this schema at startup with MQTT retention. Physical consoles (using an embedded JSON parser such as ArduinoJson) and Studio parse this payload dynamically to provision their panel columns, eliminating hardcoded bit tables.

### Unified Panel Lifecycle

Both virtual and physical panels follow an identical operational lifecycle:
1. **Connect & Discover**: The panel connects to the MQTT broker and subscribes to `/layout/+/codeline/+/json`. It provisions panel columns and lever bindings directly from received schemas.
2. **Cold-Start Alignment**: The panel subscribes to `/layout/+/codeline/+/indications`. Because indication messages are retained, the panel immediately aligns all switch correspondence lamps, signal indicators, and track model LEDs with current field reality before any lever is moved.
3. **Atomic Execution (Code Button)**: Moving panel levers establishes operator intent locally. When the operator pushes the column's physical or virtual "Code" button, the panel compiles active demands into a `ControlTransaction`, serializes it to AAR control tokens, and publishes to `controls`.
4. **Presence & Fault Tolerance**: Both consoles and field units publish heartbeat messages to `telemetry`. If a field unit disconnects, the broker fires its LWT, causing the panels to display communications failure indicators.

## Consequences

- **DRY Compliance**: Eliminates duplicate bit-mapping tables, hardcoded hex addresses, and parallel C++ definitions across field sketches and consoles.
- **Hardware Console Portability**: Physical CTC desks become generic I/O controllers driven by layout schemas rather than bespoke, layout-specific firmware sketches.
- **Instant Synchronization**: New consoles (physical or virtual) powering up mid-session synchronize immediately via retained indication topics without querying field units.
- **Offline and Simulation Parity**: Automated tests and offline simulations in Studio use the exact same MQTT payloads and topics as production layout hardware.
- **Client Library Requirement**: Both Studio (via Rust `rumqttc`) and physical console microcontrollers require MQTT client and JSON parser libraries.
