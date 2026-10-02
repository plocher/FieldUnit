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
- FieldUnit-Studio (`~/Dropbox/workspace/FieldUnit-Studio`) - GUI Plant creation, simulation and validation
- FieldUnit-Subdivision (`~/Dropbox/workspace/FieldUnit-Subdivision`) - Dispatcher and cTc machine conglomeration of multiple interlockings into a territory or subdivision. Owns the KiCad → plant JSON tooling and the virtual plant host.
- Railroad (`~/Dropbox/KiCad/Railroad`) - KiCad plant (`SPCoast/CP_<Station>`) and desk (`SPCoast/South-cTc`) schematics that are the design source of truth. Symbol libraries live in `~/Dropbox/KiCad/InterlockingPlant/symbols/` (not in git; expected to move into FieldUnit-Subdivision).
- CMRInet - low level bit/byte oriented distributed I/O system

### Cross-repo derivation chain

```
Railroad/SPCoast/CP_<X>.kicad_sch ──FieldUnit-Subdivision/tools/parse_kicad_plant.py──▶
  FieldUnit-Subdivision/profiles/spcoast_south/cps/generated/CP_<X>.json  (PlantSerializer format)
      ├─▶ virtual plant host (FieldUnit-Subdivision/runtime/plant_host)
      └─▶ station/appliance names, today hand-copied into examples/spcoast_ctc configureDesk()
Railroad/SPCoast/South-cTc.kicad_sch  (desk columns, levers, lamps, MAX7313 bits)
      └─▶ today hand-mirrored in examples/spcoast_ctc/IO-I2C.h; generator planned
```

Today names are hand-copied across `examples/spcoast_ctc`, the Subdivision JSON, the virtual plant and `tools/test_ctc_desk.py`. That duplication is debt; do not extend it. The goal is to generate the desk sketch from the schematics. Treat `spcoast_ctc` as a working template, not a fixed design. Its shortcuts (column-derived expander addresses, fixed per-column bit constants, one layout per backend) are not conventions to follow.

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

- **Run all unit & integration tests** (set `OUT` to a writable scratch dir if `/tmp` is not allowed):
  ```bash
  OUT=${OUT:-/tmp}; for t in tests/*.cpp; do clang++ -std=c++17 -I src "$t" -o "$OUT/$(basename "$t" .cpp)" && "$OUT/$(basename "$t" .cpp)" || exit 1; done
  ```

- Several tests `#include` example sketches directly (`test_corporal_sketch.cpp`, `test_christopher_sketch.cpp`, `test_universal_sketch.cpp`, `test_console.cpp`). Sketches must therefore compile natively, with Arduino-only code behind `#ifdef ARDUINO`. No test covers `spcoast_ctc`.

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
# SPCoast desk (ESP32-C6, esp32 core 3.3.12)
arduino-cli compile --fqbn esp32:esp32:XIAO_ESP32C6 --library . --libraries .. examples/spcoast_ctc/spcoast_ctc.ino
```

### `examples/spcoast_ctc`: SPCoast South Dispatcher cTc Machine

This section describes the current state, including known debt.

- **Hardware:**
  - XIAO ESP32-C6.
  - 14 × MAX7313, one per desk column. The current code computes addresses as `base + (col-1)` (debt; the address belongs in the desk schematic). The base is probed: 0x20, falling back to 0x10.
  - SSD1306 OLED at 0x3C.
  - I2C at 800 kHz, re-set after `I2Cexpander::init`, which drops it to 400 kHz.
- **Backend:** choose it by editing the include at the top of the sketch, `IO-I2C.h` (real) or `IO-CMRI.h` (stub, no transport). Both define `PanelIO : PanelHardware`.
  - Per-column bit roles are fixed constants in `IO-I2C.h`: input mask `0x1EC4`, all I/O active-LOW. They duplicate what the desk schematic already records (debt).
  - **`IO-CMRI.h` has SW_NORMAL/SW_REVERSE swapped (6/7 vs 7/6).**
- **Layout:** `configureDesk()` hard-codes all 7 stations across columns 1–14. CP_Luchessa (columns 5–7) uses KiCad-derived names (783/795/799/784); the other stations still use legacy names.
- **Build quirks:**
  - Keep the hand-written forward declarations near the top; arduino-cli's ctags misses functions under `#ifdef`.
  - `getArduinoLoopTaskStackSize()` is raised to 16 KB for the OLED.
  - `CODELINE_VISUAL_STEPPING` (authentic 15-step US&S pulse display) is off by default because it makes each lever/CODE action take 10–30 s. Enable it for demos, not for development.
- **Secrets:** copy `secrets.h.example` to `secrets.h` (gitignored) for WiFi and MQTT. OTA hostname is `spcoast-ctc`.
- **MQTT:**
  - Client id `ctc-desk-south`.
  - Subscribes to `ctc/SPCoast/codeline/+/indications`; publishes to `ctc/SPCoast/codeline/<station>/controls`.
  - Last will: `ctc/SPCoast/telemetry` = `OFFLINE`.
- **Other files:** `historical/` is reference-only legacy XML. Ignore `examples/spcoast_ctc_bench`, a one-off electrical connectivity test.

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

`tools/test_ctc_desk.py` needs `paho-mqtt` and a broker on localhost:1883. Its `STATIONS["CP_Luchessa"]` still uses legacy names (1/3/5/2); update it whenever desk names change.

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
│   (InterlockingPlant, ControlTable, Route Locking, Approach,│
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
  - Symmetrical Office/Field Design: Both field bungalows (`InterlockingPlant`) and office consoles (`cTcMachine`) use the same codec layer and data structures without duplicated logic.

- **Device Interface (Trackside Plane)**:
  Decouples logical appliance safety models from physical actuation and telemetry.
  - Low-Level Electrical: `IOBus`, `InputBit`, and `OutputBit` handle pin numbers, port offsets, and active-high vs active-low polarity across GPIO, MCP23017 I2C expanders, and shift registers.
  - High-Level Semantic: `MqttApplianceBus` maps appliances directly to discrete MQTT topics (e.g. JMRI MQTT schemas: `track/sensor/`, `track/turnout/`, `track/signalmast/`).
  - Driver Policies: `InterlockingPlant::setDefaultDriverPolicy()` runs atomic `sampleAll(nowMs)` and `driveAll(nowMs)` during scan ticks. `mockSwitch()` and `overrideDriver()` allow hybrid bench-testing of individual appliances before physical track installation.

### 2. Vital Interlocking Engine (`InterlockingPlant`, `ControlTable`)

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

- In-place, zero-allocation scanner that serializes plant topology to JSON and deserializes JSON into `InterlockingPlant` at boot.
- Enables universal microcontroller binaries (`Universal_FieldUnit.ino`) that configure their entire interlocking layout dynamically from flash/LittleFS or MQTT schema distribution.

---

## Domain Nomenclature Conventions

When authoring or modifying code in this codebase:
- Use **Switch**, never "Turnout" (following AAR standard terminology).
- **Interlocking vs controlled point:** an interlocking (`Luchessa`) contains one or more controlled points (`CP Luchessa`, `CP Gilroy`, `CP Carnadero`), and one desk column is one CP. A `CtcStation` and its MQTT topic key are the interlocking; its control and indication messages carry the tokens of all of its CPs. Never prefix an interlocking with `CP`. Names are case-preserved when produced; `cTcMachine` station lookups compare case-insensitively and fold only case. Legacy stations still named `CP_<X>` are renamed as each is cut over.
- Switches use **odd** numbers (`"1"`, `"783"`); Signals use **even** numbers (`"2"`, `"784"`). KiCad-derived plants use prototype numbers (switch 783, signal 784, masts like `784EAB`, OS circuit `783T1`).
- Dependent derails are named `<switch>D` (e.g. `795D`), paired with their switch (same position) and hidden from the CodeLine. As on the prototype, derail NORMAL means derailing (on the rail) and REVERSE means clear. See `docs/how-to/05_derails_and_os_binding.md`. `addSwitch(name, os)` and the JSON `"os"` key bind a switch to its OS track circuit, which provides the detector lock.
- Suffix **`S`** denotes inbound control demands (`1NWS`, `1RWS`, `2SGS`, `2NGS`, `2HS`, `MC1S`).
- Suffix **`K`** denotes outbound indication truth (`1NWK`, `1RWK`, `1T1K`, `2NGK`, `2TEK`, `MC1K`).
- Parenthesized tokens indicate unasserted/false states (`(1RWK)`, `(2HS)`).
- All changes must adhere to conventional semantic commits and update `docs/CHANGELOG.md`.
