# Changelog

All notable changes to the FieldUnit library will be documented in this file.

## [Unreleased]

### Added
- **`cTcMachine` Office Abstraction (`src/cTcMachine.h`)**:
  - Implemented the supervisory office console counterpart to `ControlPoint`, modeling multi-column US&S Model 503 Centralized Traffic Control (CTC) machines.
  - Hardware Decoupling: Introduced abstract `PanelHardware` contract transforming `[read/write][column, function]` into physical I/O bit operations.
  - Fluent Chaining: `machine.addStation(name).inColumn(1)...inColumn(2)...` allows multi-column stations to be declared cleanly with zero throwaway local variables.
  - Atomic Code Button Processing: Pressing a station's code button harvests switch and signal lever demands across all constituent columns into an atomic AAR `ControlTransaction` snapshot.
  - Indication Fan-Out: Ingress AAR indication tokens automatically update corresponding switch correspondence lamps, signal jewels, and track model board LEDs across all columns.
  - Synonyms: Provided `CtcMachine` and `OfficeUnit` aliases matching railroad domain terminology.
- **Example SPCoast Dispatcher Desk (`examples/spcoast_ctc/`)**:
  - Added modern 14-column CTC machine sketch (`spcoast_ctc.ino`) controlling 7 Control Points (Gilroy through Watsonville staging).
  - Modular Hardware Adapters: Added `IO-I2C.h` (driving 14x MAX7313 expanders directly over I2C) and `IO-CMRI.h` (driving a cpNode-Xiao over C/MRI RS-485 serial), selectable via a single `#include`.
- **Unit Tests (`tests/test_ctc_machine.cpp`)**:
  - Added unit test suite verifying mock hardware binding, fluent multi-column station chaining, atomic code button demand compilation, and indication lamp fan-out.
- **FieldUnit Studio Architecture & Design Specification (`docs/FIELDUNIT_STUDIO_DESIGN_SPEC.md`)**:
  - Comprehensive architectural specification for FieldUnit Studio (Issue #6) covering four integrated workspaces: EDA-style Schematic Track Designer, Automated Route Synthesizer & Control Table Engine, Virtual cTc Machine (Simulation & Live Layout Dual-Use), and Code/Firmware/Documentation Generator.
  - Documents domain ontology and taxonomy from Territory/Subdivision down to appliances and hardware profiles.
  - Formalized interface "A" (Supervisory CodeLine: Controls-as-Demands, Indications-as-Truth) and interface "B" (Field I/O) decoupled operating model.
  - Added Geographic/Physical terms (Territory, Control Point, Interlocking Limits, CP Boundary, Plant, Island Block, Fouling Point, Bungalow) and Operational/Signaling terms (Interfaces A & B, Controls-as-Demands, Indications-as-Truth, Correspondence, Out of Correspondence, Approach Time Locking, Fleeting, Call-On, Engine Return) to `docs/GLOSSARY.md`.
- **`FieldUnitConsole` & `FieldUnit_Tracer` Test Jig**:
  - Implemented USB CDC serial multiplexer router (`FieldUnitConsole.h`) inspired by CMRInet's `TracerHost`/`TracerNode`.
  - Multiplexes a single serial stream between Command & Control (C&C) verbs (`load json <payload>`, `dump json`, `status`, `shunt <tc>`, `clear <tc>`, `throw <sw> <pos>`, `reset`) and AAR CodeLine transaction snapshots (`1NWS, 2NGS` $\longleftrightarrow$ `1NWK, 2NGK...`).
  - Added `examples/FieldUnit_Tracer/FieldUnit_Tracer.ino`: Interactive test jig firmware for Seeed Xiao ESP32 and USB microcontrollers.
  - Added unit test suite `tests/test_console.cpp` verifying stream line framing, verb dispatch, dynamic configuration, and CodeLine transactions.
- **Dynamic Plant Serialization & Runtime Deserialization (`PlantSerializer.h`)**:
  - Export: `cp.serialize(buffer, maxLen, pretty)` generates clean, self-contained JSON representing the entire plant topology (appliances, detector locks, crossovers, signal masts, rulebook policies, and routes).
  - Import: `cp.deserialize(json)` reconstructs the entire interlocking plant dynamically at boot from LittleFS, SPIFFS, SD card, or network flash storage.
  - Universal Binary: Enables a single universal microcontroller firmware binary (`examples/Universal_FieldUnit/Universal_FieldUnit.ino`) to control any trackside bungalow plant simply by reading its local configuration file.
  - Zero Dependencies: Implemented with an in-place, zero-allocation scanner requiring no external JSON library dependencies.
  - Added unit test suite `tests/test_plant_serializer.cpp` verifying 100% round-trip fidelity, vital route clearing, and signal knockdown on deserialized plants.
- **`MqttApplianceBus`**: High-Level Semantic Device Interface connecting FieldUnit logical appliances directly to discrete MQTT domain topics (e.g. JMRI MQTT schemas: `track/sensor/<name>`, `track/turnout/<name>`, `track/signalmast/<name>`):
  - Ingress: Implements `onUnpack(topic, payload)` to translate sensor occupancy (`ACTIVE`/`INACTIVE`) and turnout point feedback (`CLOSED`/`THROWN`) into appliance updates.
  - Egress: Implements `onPack(handler)` and `sync()` to publish turnout movement commands and signal mast aspect changes (`"Clear"`, `"Approach"`, `"Stop"`).
  - Auto-binding: `bus.bind(cp)` automatically registers all declared track circuits, switches, and masts in a Control Point using standard topic schemas.
  - Two Core Interfaces Architecture: Formalized the CodeLine Interface (Transactional Snapshot Interface) and the Device Interface (Low-Level Electrical Pins & Signals vs. High-Level Semantic Appliances) in `CONTROL_POINT_ARCHITECTURE.md`.
- **`AarTextCodec`**: Human-readable, self-documenting AAR symbolic token codec for CodeLine and MQTT transports:
  - **Conservative Producer**: Emits indication snapshot vectors strictly in declared order using authentic `'K'` suffix (`1NWK`, `(1RWK)`, `1T1K`, `2NGK`, `(2SGK)`, `(2TEK)`, `MC1K`).
  - **Liberal Consumer**: Parses control messages in any sequence with whitespace and case tolerance.
  - **Mandatory Suffixes**: Enforces `'S'` suffix on inbound controls (`1NWS`, `1RWS`, `2SGS`, `2NGS`, `2HS`, `MC1S`) and rejects invalid/ambiguous tokens.
  - **Vital Isolation ("Don't Poke a Sleeping Bear")**: Conflicting vital commands flag `vitalValid = false` on `ControlTransaction` without mutating received demands. `ControlPoint` skips all vital appliance invocation while still applying non-vital commands (maintainer call).
  - **Fault Monitoring**: Tracks unknown symbol counts, last unknown symbol, and vital conflict counts.
- **Appliance Self-Indexing**: `Switch`, `SignalControl`, and `TrackCircuit` store their registered index within `ControlPoint` upon creation, eliminating hardcoded magic integers in codec mappings.
- **`BitPackedCodec`**: Dedicated binary bitstream codec for C/MRI IB/OB dense byte arrays, supporting `padToByte()` and `skipBits(N)` alignment helpers.
- **Sectional Route Release**:
  - Implemented sequential block progression and progressive switch unlocking in `ControlTable.h` (`RouteState` and `SectionState`).
  - As a train traverses an interlocking plant across multiple switches and crossovers, trailing switches release progressively (`SwitchLock::ROUTE_LOCKED` dropped) as their specific fouling track circuit is vacated, freeing switches behind the train for conflicting moves while maintaining route locking on downstream switches.
  - Trailing switches in vacant downstream sections maintain `SwitchLock::ROUTE_LOCKED` after signal knockdown until the train traverses and clears them.
  - Automatic detector circuit pairing via `ControlPoint::bindDetectorLock()` with optional explicit releasing block assignment in `Route::aligns()`.
- **`SignalAspectPolicy`**: Pluggable rulebook policy subsystem for `SignalMast`, supporting distinct railroad and era signaling practices:
  - Built-in policies: `defaultRoute`, `sp1969` (Rule 290 Red over Lunar), `sp1985` (Rule 290 Red over Flashing Red with 1 Hz pulse), `gcorSpeed`, `nycSpeed` (Eastern 3-head speed signaling), `prrPositionLight` (Pennsylvania Railroad amber position lights), `upperQuadrantSemaphore`, and `boCpl` (Baltimore & Ohio Color-Position-Light with orbital markers).
  - Extensible: Accepts custom `AspectResolver` functions and lambdas for arbitrary railroad practices.
  - Extended standard `Indication` and `Aspect` enums with common North American indications and flashing aspects.
- **`CplMastDriver`**: Dedicated hardware driver for Baltimore & Ohio (B&O) Color-Position-Light (CPL) signal masts, driving central cluster lamp pairs (Red horizontal, Yellow diagonal, Green vertical, Lunar diagonal) with 1 Hz flashers and up to six perimeter orbital markers (12, 2, 4, 6, 8, 10 o'clock) for speed signaling.
- **`SemaphoreDriver`**: Dedicated hardware driver for mechanical semaphore signals actuated by hobby servos (PCA9685 / PWM pins), supporting independent Stop, Approach, and Clear angles per blade.
- **`IOBus::writeAngle()`**: Hardware abstraction method for driving analog/servo channel angles.
- **`Crossover`**: First-class crossover appliance inheriting from `Switch`, allowing direct route alignment (`Route::aligns({ {xover, SwitchPosition::REVERSE} })`) while driving and proving two physical machines in unison.
- **`ElectricSwitchLock` (`WLS` / `WLK`)**: Integrated dispatcher electric lock demands (`WLS`) and field indications (`WLK`) into `ControlTransaction`, `IndicationVector`, `ControlPoint` safety arbitration, `AarTextCodec`, and `BitPackedCodec`.
- **Production `CodeLine` Transports**: Added `StreamCodeLine` (for Arduino HardwareSerial/SoftwareSerial streams) and `MqttCodeLine` (for platform-neutral MQTT publish/subscribe bridging).
- **Declarative String-Based Configuration**: Added configuration-time name lookup methods to `ControlPoint` (`findTrackCircuit`, `findSwitch`, `findSignalControl`, `findSignalMast`) and string overloads to `Route` (`governedBy`, `displays`, `aligns`, `clears`, `entrance`, `approaching`, `engineReturn`). Resolves strings once during setup to maintain $O(1)$ raw pointer execution during vital runtime cycles.
- **Unit Tests**: Added `tests/test_wire_codec.cpp` covering symbolic parsing, indication formatting, vital isolation, and binary streaming. Added `tests/test_corporal_sketch.cpp` for end-to-end sketch verification of CP Corporal. Added Test 5, 6, and 7 to `tests/test_hardware_drivers.cpp` verifying semaphores and Eastern rulebooks. Added Tests 8 and 9 to `tests/test_cp_christopher.cpp` verifying `Crossover` and `MqttCodeLine`.

### Changed
- Standardized terminology across documentation: replaced arbitrary lettered interfaces with descriptive names (**CodeLine Interface** and **Device Interface**) and eliminated programming jargon ("seams").
- Updated `docs/adr/0001-mqtt-aar-codeline-interface-a.md` and `docs/FIELDUNIT_STUDIO_DESIGN_SPEC.md` to establish Studio virtual cTc panels and physical microcontroller-driven hardware desks as first-class, symmetrical peers.
- Expanded `docs/CONTROL_POINT_ARCHITECTURE.md` Section 3.2 and 3.4 to clearly contrast Centralized Traffic Control (CTC - Rule 261) with Interlocking Tower / Local Station regimes (TT&TO - Rule 251, train orders, telegraph/telephony, and fascia CodeLine-analogues).
- Added CodeLine Interface, Device Interface, Timetable & Train Order (TT&TO), and CTC definitions to `docs/GLOSSARY.md`.
- Refactored `examples/CP_Christopher/CP_Christopher.ino` and `examples/CP_Corporal/CP_Corporal.ino` to use declarative string configuration, eliminating all file-scope pointer variables and making plant definitions completely self-contained within `ControlPoint`.
- Refactored example sketches to use authentic AAR appliance names (`"1"`, `"3"`, `"5"`, `"2"`, `"4"`) and fluent `AarTextCodec` declarations.
- Updated `tests/test_christopher_sketch.cpp` to verify end-to-end sketch behavior driven by AAR text snapshots.
- Updated documentation and tutorials to use authentic AAR appliance identifiers.

### Fixed
- **Switch Time Locking (`SwitchLock::TIME_LOCKED`)**: Enforced switch freeze on cancelled routes while `SignalControl::isTimeLocked()` is active.
- **Dynamic Approach Cancellation**: Signal cancellation checks route approach circuit occupancy, enabling immediate safe release when vacant and engaging timed release only when occupied.
- **Signal Knockdown Robustness**: Added `.entrance(TrackCircuit*)` to `Route` so signal knockdown triggers reliably regardless of the order in `.clears()`.
- **Route-Specific Fleeting**: Fleeting restores based on the vacancy of blocks on the commanded route instead of requiring all plant track circuits to be clear.
- **Telemetry Decoupling**: Added `masts[]` array to `IndicationVector` and `compositeAspect()` to `SignalMast`, decoupling wayside mast aspects from dispatcher lever indices. Added `encodeMast()` to `WireCodec`.
- **Sketch Examples & Test Benches**:
  - Corrected CP Corporal Route 4 (`IND-NB`) clearing track to `2SAT` (MT2) and added Derail 5 to `.aligns()`.
  - Renamed CP Corporal Route 2 to `"MT-NB-REV"`.
  - Added Arduino hardware drivers and sampling stubs to `CP_Corporal.ino`.
  - Added `encodeMaintainer(0)` to `CP_Christopher.ino`.
  - Fixed integration test benches (`test_corporal_sketch.cpp` and `test_christopher_sketch.cpp`) by initializing baseline vacant track state.
- **Documentation**: Deduplicated Tutorial 1, corrected route builder syntax in `CONTROL_POINT_ARCHITECTURE.md`, and updated electric lock naming to `HAND_LOCKED`.
