# AGENTS.md

This file provides guidance to agentic coders when working with code in this repository.

## Project Scope and Deployment Targets

FieldUnit provides a modular C++17 foundation for model railroad automations based on three core domain abstractions:
1. **Dispatcher cTc Machines** (`cTcMachine.h`): Office consoles modeling US&S Model 503 panels, column lever harvest, code button edge latching, and indication lamp fan-out.
2. **Interlocking Towers**: Local tower plants modeling leverframe locking beds and manual/pistol-grip control.
3. **Interlocking Plants** (`InterlockingPlant.h`): Autonomous field plants enforcing prototype Association of American Railroads (AAR) vital safety rules and route locking.

The codebase is engineered to support two distinct deployment environments from the same codebase:
- **Embedded Arduino Sketches**: Microcontroller firmwares (ESP32, RP2040, AVR) located in `examples/` driving physical layout GPIO, I2C port expanders (MCP23017, MAX7313), PWM servos, and cpNodes.
- **Standalone Native Applications & CI**: Centralized host processes, command-line utilities, simulation loops, and automated desktop test benches running natively on macOS/Linux without Arduino framework dependencies.

Related repos:
- FieldUnit-Studio - GUI Plant creation, simulation and validation
- FieldUnit-Subdivision - Dispatcher and cTc machine conglomeration of multiple interlockings into a terratory or subdivision
- CMRInet - low level bit/byte oriented distributed I/O system

## Key Documentation and Domain Glossary

Extensive architectural specifications and prototype signaling primers reside in `docs/`. When designing features or naming entities, consult these primary sources:
- **`docs/GLOSSARY.md`**: Authoritative domain dictionary covering physical track topology, operational rules, and AAR relay naming conventions. Always check this glossary to preserve authentic railroad nomenclature.
- **`docs/CONTROL_POINT_ARCHITECTURE.md`**: Complete 4-tier architectural specification written in ASD-STE100 controlled English.
- **`docs/AAR_SIGNALING_PRIMER.md`**: Detailed technical primer explaining the vital relay chains behind switches (`TR`, `WLR`, `WR`, `NWCR`, `RWCR`, `KR`) and signals (`HSR`, `HR`, `DR`, `ASR`, `FSR`).
- **`docs/FIELDUNIT_STUDIO_DESIGN_SPEC.md`**: Architectural specification for virtual and physical cTc panel parity, layout schemas, and route synthesis.
- **`docs/adr/`**: Architectural Decision Records (e.g., `0001-mqtt-aar-codeline-interface-a.md` establishing the MQTT AAR CodeLine topic schema and payload contracts).
- **`docs/how-to/` & `docs/tutorials/`**: Wiring guides, JMRI/CMRI integration, electric locks, and aspect configuration tutorials.

## Build and Test Commands

FieldUnit is a header-only C++17 library located in `src/`. The test suite consists of standalone native C++ test programs in `tests/` using standard assertions (`assert.h`).

### Run Native C++ Tests when changing code

Not required when only changing documentation

Compile and execute with `clang++` (or `g++`) using C++17 and the `src/` include directory:

- **Run all unit & integration tests:**
  ```bash
  for t in tests/*.cpp; do clang++ -std=c++17 -I src "$t" -o "/tmp/$(basename "$t" .cpp)" && "/tmp/$(basename "$t" .cpp)" || exit 1; done
  ```

- **Run a single test suite:**
  ```bash
  clang++ -std=c++17 -I src tests/test_wire_codec.cpp -o /tmp/test_wire_codec && /tmp/test_wire_codec
  clang++ -std=c++17 -I src tests/test_ctc_machine.cpp -o /tmp/test_ctc_machine && /tmp/test_ctc_machine
  clang++ -std=c++17 -I src tests/test_sectional_release.cpp -o /tmp/test_sectional_release && /tmp/test_sectional_release
  clang++ -std=c++17 -I src tests/test_plant_serializer.cpp -o /tmp/test_plant_serializer && /tmp/test_plant_serializer
  ```

- **Compile with diagnostic warnings:**
  ```bash
  clang++ -std=c++17 -Wall -Wextra -I src tests/test_wire_codec.cpp -o /tmp/test_wire_codec
  ```

### Compile Arduino Sketches with `arduino-cli`

Example sketches in `examples/` target microcontrollers and rely on sibling libraries located in the parent directory (`..`):

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 --library . --libraries .. examples/CP_Corporal/CP_Corporal.ino
```

### Python Test Runner

The interactive test desk runner for MQTT CodeLine simulation is in `tools/`:

```bash
# Monitor desk controls over MQTT
python3 tools/test_ctc_desk.py --monitor

# Closed-loop ghost plant simulator (2-second switch travel simulation)
python3 tools/test_ctc_desk.py --loopback

# Exercise all 7 station lamps
python3 tools/test_ctc_desk.py --walk
```

---

## High-Level Architecture and System Structure

FieldUnit models North American railroad signaling, Centralized Traffic Control (CTC - Rule 261), and interlocking towers under Association of American Railroads (AAR) vital safety principles.

The system is organized into a four-tier decoupled architecture:

```
┌─────────────────────────────────────────────────────────────┐
│                 1. CodeLine Interface                       │
│    (AarTextCodec, BitPackedCodec, StreamCodeLine, MQTT)     │
└──────────────────────────────┬──────────────────────────────┘
                               │ ControlTransaction / IndicationVector
                               ▼
┌─────────────────────────────────────────────────────────────┐
│            2. Control Point Vital Safety Engine             │
│   (ControlPoint, ControlTable, Route Locking, Approach,     │
│       Sectional Release, Fleeting, Engine Return)           │
└──────────────────────────────┬──────────────────────────────┘
                               │ Logical Commands / Feedback
                               ▼
┌─────────────────────────────────────────────────────────────┐
│               3. Logical Railroad Appliances                │
│   (TrackCircuit, Switch, Crossover, SignalControl, Mast)    │
└──────────────────────────────┬──────────────────────────────┘
                               │
                               ▼
┌─────────────────────────────────────────────────────────────┐
│            4. Device Interface & Hardware Drivers           │
│   (IOBus, IOBit, DriverPolicy, MqttApplianceBus, Servos)    │
└─────────────────────────────────────────────────────────────┘
```

### 1. The Two Primary Boundaries

- **CodeLine Interface (Supervisory Plane)**:
  Exchanges synchronized transactional snapshots between the dispatcher office and field units.
  - Ingress: `ControlTransaction` (desired demands for all plant appliances).
  - Egress: `IndicationVector` (verified physical truth and lock status).
  - Codecs: `AarTextCodec` formats and parses human-readable AAR tokens (e.g., `1NWS, (1RWS), 2NGS` $\longleftrightarrow$ `1NWK, 1T1K, 2NGK`). `BitPackedCodec` packs dense bitstreams for C/MRI input/output byte arrays.
  - Symmetrical Office/Field Design: Both field bungalows (`ControlPoint`) and office consoles (`cTcMachine`) use the same codec layer and data structures without duplicated logic.

- **Device Interface (Trackside Plane)**:
  Decouples logical appliance safety models from physical actuation and telemetry.
  - Low-Level Electrical: `IOBus`, `InputBit`, and `OutputBit` handle pin numbers, port offsets, and active-high vs active-low polarity across GPIO, MCP23017 I2C expanders, and shift registers.
  - High-Level Semantic: `MqttApplianceBus` maps appliances directly to discrete MQTT topics (e.g. JMRI MQTT schemas: `track/sensor/`, `track/turnout/`, `track/signalmast/`).
  - Driver Policies: `ControlPoint::setDefaultDriverPolicy()` runs atomic `sampleAll(nowMs)` and `driveAll(nowMs)` during scan ticks. `mockSwitch()` and `overrideDriver()` allow hybrid bench-testing of individual appliances before physical track installation.

### 2. Vital Interlocking Engine (`ControlPoint`, `ControlTable`)

- **Execution Model**:
  - Zero dynamic heap allocation after startup. All appliance lookups occur once during configuration; runtime cycles use $O(1)$ raw pointer dereferences.
  - Encapsulated scan cycle: `cp.tick(nowMs)` executes atomically:
    $$\text{Sample Inputs} \longrightarrow \text{Vital Safety Rules} \longrightarrow \text{Drive Outputs}$$
  - No queuing or conversational negotiation: invalid or unsafe switch commands are silently ignored (vital lock prevents movement); the controlling client learns this by observing reported indications.

- **Safety Invariants**:
  - **Detector Locking**: Prevents throwing a switch when its associated track circuit (`TR`) is occupied.
  - **Route Locking & Opposing Authority**: Prevents throwing switches or granting conflicting signal authorities along an active path.
  - **Approach Time Locking (`TIME_LOCKED`)**: If a cleared signal is knocked down before a train enters, switch points freeze until the approach timer expires, unless downstream approach tracks are vacant (dynamic approach cancellation).
  - **Sectional Route Release**: As a train traverses an interlocking across multiple switches, trailing switches release progressively as their specific fouling track circuit is vacated, freeing them for conflicting moves while maintaining locking ahead.
  - **Vital Stick Relays**: Standard signal stick drops upon entrance shunt (`knockdown`). Fleeting mode (`FSR`) automatically re-clears authority for following trains when route blocks vacate. Engine Return Stick (`ERS`) permits low-speed return moves to cars standing on dark/approach tracks.

### 3. Office Machine Abstraction (`cTcMachine`)

- Models multi-column US&S Model 503 Centralized Traffic Control dispatcher consoles.
- `PanelHardware`: Abstract interface decoupling physical buttons, levers, and LEDs (`[read/write][column, function]`) from machine logic.
- `PanelColumn`: Fluent declaration binding physical column slots to switch levers, signal levers, code pushbuttons, and track diagram lamps.
- `OneShot`: Hardware edge latch that arms when a code pushbutton is pressed and latches `TRIGGERED` upon release until consumed, avoiding edge jitter and timing dependencies.
- Strategy B Preallocation: Station codec buffers are sized and allocated during initialization (`preallocateBuffers()`), eliminating runtime buffer allocations and overflow checks.

### 4. Signaling Rules & Masts (`SignalMast`, `SignalAspectPolicy.h`)

- **Separation of Concerns**:
  - **Indication**: The operational instruction to train crews (`STOP`, `APPROACH`, `CLEAR`, `DIVERGING_CLEAR`, `RESTRICTING`).
  - **Aspect**: The physical lamp combination (`RED_OVER_RED`, `YELLOW_OVER_GREEN`, etc.).
- Pluggable Rulebooks: Configured per-mast or plant-wide using `AspectResolver` functions:
  - Southern Pacific 1969 (Red over Lunar) and 1985 (Red over Flashing Red with 1 Hz flasher).
  - GCOR Speed, NYC 3-Head Speed, PRR Amber Position Light, B&O Color-Position-Light (`boCpl` with orbital markers), and mechanical Semaphores (`SemaphoreDriver` with servo angles).

### 5. Dynamic Plant Serialization (`PlantSerializer.h`)

- In-place, zero-allocation scanner that serializes plant topology to JSON and deserializes JSON into `ControlPoint` at boot.
- Enables universal microcontroller binaries (`Universal_FieldUnit.ino`) that configure their entire interlocking layout dynamically from flash/LittleFS or MQTT schema distribution.

---

## Domain Nomenclature Conventions

When authoring or modifying code in this codebase:
- Use **Switch**, never "Turnout" (following AAR standard terminology).
- Switches use **odd** numbers (`"1"`, `"3"`, `"5"`); Signals use **even** numbers (`"2"`, `"4"`, `"6"`).
- Suffix **`S`** denotes inbound control demands (`1NWS`, `1RWS`, `2SGS`, `2NGS`, `2HS`, `MC1S`).
- Suffix **`K`** denotes outbound indication truth (`1NWK`, `1RWK`, `1T1K`, `2NGK`, `2TEK`, `MC1K`).
- Parenthesized tokens indicate unasserted/false states (`(1RWK)`, `(2HS)`).
- All changes must adhere to conventional semantic commits and update `docs/CHANGELOG.md`.
