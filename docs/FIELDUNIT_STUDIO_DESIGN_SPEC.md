# FieldUnit Studio: Architecture & Design Specification

## 1. Introduction

### 1.1 Project Mission
FieldUnit Studio is a responsive, cross-platform desktop application for macOS, Linux, and Windows. It provides a unified environment for designing, evaluating, simulating and controlling railroad control points.  It can automatically synthesize interlocking control tables, simulate plant behavior against a virtual Centralized Traffic Control (cTc) console, and generate production artifacts.

### 1.2 Dual-Use Operational Model
FieldUnit Studio operates in two primary modes over well defined interfaces:
1. **Design and Simulation Mode**:
   - The user drafts track geometry, verifies safety rules, synthesizes routes, and tests operation using an in-memory vital engine and simulated train progression.
2. **Live Layout Dispatcher Console**:
   - The virtual cTc console connects to simulated, Mocked, and physical layout hardware; the latter can connect over USB/Serial, RS-485, TCP/UDP, or MQTT.
   - The console operates as a Controls source and passive display surface for Indications provided by vital safety logic that executes autonomously in the field (or inside an integrated desktop vital engine driving remote I/O nodes).

---

## 2. Glossary and Terminology

To avoid ambiguous definitions, terms are defined according to prototype Association of American Railroads (AAR) standards and FieldUnit conventions.  See GLOSSARY.md.

---

## 3. Domain Ontology and Taxonomy

FieldUnit Studio structures railroad concepts into a strict hierarchical taxonomy:

```
Territory (Subdivision)
 └── Control Point (CP)
      ├── Interlocking Engine (Vital Rules & Control Table)
      │    ├── Routes (Path, Alignments, Clear Blocks, Ceiling)
      │    └── Stick Logic (Fleeting, Call-On, Engine Return)
      ├── Appliances (Logical Models)
      │    ├── Switch / Crossover (Points, Motors, Sense, WLR)
      │    ├── Track Circuit (Occupancy, Dropout Delay, Quality)
      │    └── Signal Mast (Heads, Governing Direction, Rules)
      ├── Code Line Codec 
      │    ├── Control Vectors (NWS, RWS, SGS, NGS, HS)
      │    └── Indication Vectors (NWK, RWK, TK, SGK, NGK, TEK)
      └── Hardware Profile (Pinout & Driver Mapping)
           ├── Direct GPIO (MCU pins)
           ├── I/O Expanders (MCP23017, PCA9685)
           └── Remote Field Bus (C/MRI, MQTT, RS-485)
```

---

## 4. System Architecture & Tech Stack

```
┌─────────────────────────────────────────────────────────────────────────┐
│                    FieldUnit Studio Desktop Shell                       │
│                        (Tauri v2 / Rust Core)                           │
│  - Native Windows & Menus        - Async Serial / RS-485 (`serialport`) │
│  - Multi-Monitor Window Manager  - TCP / UDP / MQTT Client (`rumqttc`)  │
│  - File System I/O               - CLI Process Bridge (`arduino-cli`)   │
└────────────────────────────────────┬────────────────────────────────────┘
                                     │ IPC Bridge
┌────────────────────────────────────▼────────────────────────────────────┐
│                    Studio Presentation & State Engine                   │
│                     (Svelte 5 / TypeScript / Vite)                      │
│  - Fine-Grained Reactive State   - Layer Manager (Track, Elec, Sig)     │
│  - Semantic Zoom Engine (LOD)    - Project Undo/Redo Transaction Log    │
└──────────────┬─────────────────────┬────────────────────┬───────────────┘
               │                     │                    │
               ▼                     ▼                    ▼
     [ Schematic Canvas ]   [ Route Synthesizer ]   [ Virtual cTc ]
      (HTML5 / Canvas 2D)     (Directed Graph)      (US&S / GRS Desk)
```

### 4.1 Execution Topologies
1. **Autonomous Field Unit**: Microcontrollers (ESP32, RP2040) run the vital C++ engine trackside. Studio communicates over interface "A" (Serial / MQTT).
2. **Desktop Vital Host with Remote I/O**: Studio's internal Rust vital engine executes safety logic, driving distributed C/MRI or MQTT remote I/O nodes over interface "B".
3. **Pure Desktop Simulation**: The virtual cTc console communicates in-memory with a software mock plant and ghost train testbed.

---

## 5. Workspace Specifications

### 5.1 Workspace 1: Schematic Track Designer (The EDA Capture Board)

#### Schematic Metaphor
Studio treats track design as an Electronic Design Automation (EDA) schematic capture process (similar to KiCad), rather than a tile drawing canvas:
- Appliances (Switches, Crossovers, Signals, Bumpers, CP Boundaries) are components with connection ports.
- Tracks are auto-routed nets connecting ports with orthogonal and 45-degree constraints.
- Moving an appliance rubberbands connected tracks while preserving model board presentation rules.

#### Layer Architecture
A dedicated layer manager enables single-key visibility toggles: (keymappings are suggestions only)
1. **Track Layer (`T`)**: Tangent tracks, switch points, crossovers, bumpers.
2. **Electrical Layer (`E`)**: Insulated rail joints (`][`), track circuit labels (`1T1`, `1SA`), optical sensors (`(d)`), dropout timers.
3. **Signal Layer (`S`)**: Signal masts, heads, governing directions.
4. **Operational Layer (`R`)**: Operational speed ratings, authority badges (`[F]`, `[C]`, `[ERS]`).
5. **Nomenclature Layer (`N`)**: CP names, switch numbers, signal identifiers, mileposts.

#### Semantic Zoom (Level of Detail - LOD)
- **Overview (Corridor View)**: Shows mainline paths, CP names, and occupancy. Electrical pins, IRJs, and sensor nodes fade out.
- **Focus / Isolation Mode**: Double-clicking a CP isolates it, collapsing adjacent CPs into summary capsules.
- **Detail View**: Full engineering graphics appear: IRJs, pin tags, and dropout times.

#### Intelligent Graph Heuristics (90% Auto-Configuration)
When appliances are placed:
- Switch legs terminate in bumpers $\rightarrow$ Auto-tagged **Slow Speed**.
- Parallel track crossovers $\rightarrow$ Auto-tagged **Medium Speed** (or **Limited Speed**).
- Siding passing loops $\rightarrow$ Auto-tagged **Medium Speed**.
- CP Boundary exits to dark track $\rightarrow$ Auto-tagged **Restricting Ceiling**.

#### Visual Milestones & DRC
- Each Control Point displays a subtle background status halo:
  - **Muted Gray**: Unconnected stubs or open ports.
  - **Soft Amber**: Missing optical sensor or unassigned pin.
  - **Soft Green**: Structurally complete, fully insulated, and verified.

---

### 5.2 Workspace 2: Automated Route Synthesizer & Control Table Engine

#### Route Synthesis Algorithm
1. Scans the directed track graph from every entrance signal along all downstream paths to exit boundaries or internal stubs.
2. Synthesizes required switch positions (`Normal` or `Reverse`).
3. Discovers all traversed or fouled track circuits (islands, fouling clearance zones).
4. Identifies conflicting routes (opposing entrance signals, shared switches).
5. Discovers concurrent routes (independent parallel moves that can be cleared simultaneously).
6. Derives the aspect ceiling using the **Rule of the Minimum** across all traversed switches.

#### Operational Speed Classes
Decoupled from physical frog numbers:
- **Normal Speed**: Track speed (`CLEAR`).
- **Limited Speed**: 40–45 mph (`LIMITED_CLEAR`).
- **Medium Speed**: 30 mph (`DIVERGING_CLEAR`).
- **Slow Speed**: 15 mph (`SLOW_CLEAR` or `RESTRICTING`).

#### Interactive Control Table Matrix
Displays all derived routes with interactive bidirectional highlighting. Clicking a route illuminates its path on the schematic in blue, showing aligned points and locked blocks.

---

### 5.3 Workspace 3: The Virtual cTc Machine (Operator Console)

#### Physical Console Ergonomics
- **Aesthetic**: Authentic matte enamel finish, engraved brass/silver nameplates, US&S / GRS styling.
- **Switch Levers (Odd Numbers)**: 60° angular detents (`Normal` up, `Reverse` down).
- **Signal Levers (Even Numbers)**: 45° angular detents (`Left`, `STOP`, `Right`).
- **Out of Correspondence (OOC)**:
  - **GRS Style**: Illuminated lever barrel.
  - **US&S Style**: Flashing jewel lamps above the lever.
  - Changing a lever position creates an immediate visual disagreement indication before the code button is punched.
- **Approach Time-Lock Indication (`2TEK`)**: The active direction jewel flashes during the timer countdown (e.g. 180s) when a cleared signal is restored to stop.
- **Mechanical Blocking Dogs**: Right-click any lever to clamp a **Red Dog** (Track Out of Service) or **Blue Dog** (Blue Signal Protection). Clamped levers cannot be thrown.
- **Code Buttons**: Silver or brass buttons. Moving levers establishes intent; pressing the button transmits the interface "A" snapshot.

#### Acoustic Engine
- **Relay Stepper Clatter**: Synthesizes the authentic rhythmic cadence of 504/506 line coding relays based on station address and data vectors.
- **OS Chimes**: Authentic single-stroke brass bell on island circuit shunts.

#### Ghost Train Simulation Layer
- Interactive train avatar dragged and dropped onto approach tracks.
- Era-appropriate prime mover audio (EMD 567/645 diesel rumble or steam chuff with brake air release).
- Autonomous response to signals: halts at red signals; moves automatically when a route clears, knocking down signals and tripping detector locks in sequence.

---

### 5.4 Workspace 4: Artifact, Code, and Documentation Generator

#### 1. C++ Sketch Generation
Generates production-ready, declarative `configurePlant()` code conforming to FieldUnit v2 standards.

#### 2. Direct Build & Flash (`arduino-cli`)
Studio connects to `arduino-cli` to compile sketches and flash connected microcontrollers (ESP32, RP2040, Arduino Mega) over USB with a single click.

#### 3. Serialized Plant JSON (FieldUnit Issue #1)
Exports portable, versioned JSON configuration files for dynamic runtime loading from microcontroller flash memory (SPIFFS / LittleFS / SD).

#### 4. Shop Wiring Sheets
Generates shop documentation in two formats:
- **Markdown (`WIRING.md`)**: Version-controlled, tablet-browsable documentation.
- **Printable High-Resolution PDF**: Formatted for 8.5" $\times$ 11" landscape printing with terminal block pinouts and physical solder check-off boxes `[ ]`.

#### 5. AAR / AREMA "Packet-Lite" Documentation
Generates a complete engineering package for the interlocking:
- Geographic overview and track chart.
- Complete Interlocking Control Table matrix.
- Circuit logic descriptions and locking justification.
- interface "A" CodeLine bit allocation ledger.
- optional, over-the-top: Relay ladder logic charts and bungalow terminal block wiring tags.

---

## 6. Project Persistence Format

Projects persist as a unified JSON bundle (`*.fieldunit`):
```json
{
  "$schema": "https://fieldunit.org/schema/v1/project.json",
  "version": "1.0.0",
  "metadata": {
    "name": "Coast Division",
    "subdivision": "Santa Cruz Branch",
    "rulebook": "GCOR",
    "created": "2026-09-11T06:15:00Z"
  },
  "controlPoints": [ ... ],
  "topology": {
    "appliances": [ ... ],
    "nets": [ ... ]
  },
  "routes": [ ... ],
  "codeLineMap": { ... }
}
```
Studio supports bi-directional round-tripping: importing an existing `plant.json` or `configurePlant()` sketch reconstructs the schematic topology.
