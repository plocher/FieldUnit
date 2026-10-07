# FieldUnit

FieldUnit is a modular C++ toolkit that lets model railroaders build their own field units, CTC machines and tower controls.
The resulting field units run with the same vital safety rules, route locking, and behavioral contracts as the prototype railroad.

---
In 1927, when the first CTC installation went live on the New York Central (Stanley to Berwick, Ohio, built by General Railway Signal), the railroad did not invent a brand new safety theory. In effect, they cut the interlocking machine's lever frame in two: the levers went to the dispatcher's office, and the locking stayed in the field. That first installation was not yet the two-wire coded line of later systems (one account describes a wire to each switch plus a common return). Coded lines that carry many functions over two wires came later; a 1937 article describes a "Union time-code C.T.C. system" on a "single series circuit of two wires". Today the link between the halves is the code line:
```
Historical Mechanical Tower (The Monolithic Safety Engine):
┌──────────────────────────────────────────────────────────────────────────┐
│                            THE TOWER                                     │
│                                                                          │
│   [ Levers ] <────── Mechanical Locking Bed ──────> [ Track & Points ]   │
│ (Human Intent)        (The Locking)                 (Interlocking Plant) │
└──────────────────────────────────────────────────────────────────────────┘

CTC: The Lever Frame Cut in Two Across the Code Line:

┌───────────────────────┐                                ┌───────────────────────┐
│     CTC MACHINE       │                                │     BUNGALOW          │
│                       │        Controls (Intent)       │   at a Control Point  │
│  [ Free Levers ]      │ ─────────────────────────────> │     FIELD UNIT        │
│    (Human Intent)     │ <───────────────────────────── │  (The Locking: WLR)   │
│                       │   Office Indications (Truth)   │           │           │
└───────────────────────┘                                └───────────┼───────────┘
                                                                     ▼
                                                             [ Track & Points ]
                                                           (Interlocking Plant)
```
A dispatcher is simply a person wearing many tower operator hats located miles away from the control points they are managing.
One difference matters: a tower lever that would be unsafe does not move, because the locking bed holds it. A CTC lever moves freely; CODE sends the intent, and only the office indication tells the truth.
The code line, the field station and the field unit together replace the locking bed. They do at a distance, and after the fact, what the locking bed does in the operator's hand.

---
## What FieldUnit Does

On a real railroad, a **control point** is a place where a dispatcher controls train movements by controlled absolute signals, usually with switches between them. Its **interlocking** forces the switch and signal movements to follow each other in a safe sequence.
A dispatcher at the CTC machine decides where trains should go.
However, the dispatcher cannot throw switches or clear signals directly.
The dispatcher sends controls over the code line to the field unit at the control point.

FieldUnit lets you build that field unit.
It includes the physical connection to the devices on your layout:
- **Sensors (Inputs)**: Reads track circuit occupancy detectors (DCCOD, optical sensors) and switch point limit switches.
- **Actuators (Outputs)**: Drives switch motors (Tortoise, servos) and signal lamps (LEDs, searchlights).

The field unit acts as the local safety guard between the dispatcher and your track:
1. **Checks the switches**: Are the switch points locked in position?
2. **Checks the tracks**: Is there a train already occupying the points?
3. **Checks opposing signals**: Are conflicting train routes held at Stop?
4. **Moves the interlocking plant**: If the move is safe, the field unit drives the physical switch motors and displays the signal aspect at trackside.
5. **Reports reality**: The field unit continuously sends office indications (the verified state of the interlocking plant) back to the dispatcher. If it refuses a control, it sends no refusal message; the office indication simply does not change.

FieldUnit brings this exact prototype behavior to model railroad control:
- **Distributed Microcontrollers**: Run autonomous field units on small microcontrollers (ESP32, RP2040, AVR) directly connected to GPIO or I2C port expanders (MCP23017, cpNode-IOX).
- **One Field Processor, Many Field Units**: Run several field units together on one computer, driving remote C/MRI I/O nodes through input and output byte arrays.
- **Interlocking Towers**: Model towers where a tower operator works mechanical or pistol-grip levers under vital safety rules.

```
[ CTC machine (JMRI panel / physical machine / simulator) ]
                     │
          (Code Line Interface)             <-- Control transaction / indication vector: 1NWS, 2NGS <-> 1NWK, 1T1K
                     │
                     v
       +----------------------------+
       |   Interlocking Logic       |  <-- `InterlockingPlant`: FieldUnit vital logic
       |  (Evaluates Safety Rules)  |      (Zero Heap Allocation, O(1) Execution)
       +----------------------------+
                     │
             (Device Interface)             <-- Single Boundary to Trackside World
            ┌────────┴────────┐
            ▼                 ▼
   [ Smart Appliances ] [ Hardware Drivers ]
   - MQTT (JMRI topics) - I2C MCP23017, GPIO
   - LCC / OpenLCB      - C/MRI shift registers
   - Fastclock Lighting - A/D, D/A, Servos
```

---

## How FieldUnit Code Looks

FieldUnit replaces complex procedural code with readable, declarative routes:

```cpp
// Siding Route: Diverging move over Crossover 3 into Siding
cp.route("MAIN_TO_SIDING")
  .governedBy("2", DirectionAuthority::RIGHT)
  .displays("2S", Indication::DIVERGING_CLEAR)
  .aligns({ {"1", SwitchPosition::NORMAL}, 
            {"3", SwitchPosition::REVERSE} }) // Crossover 3 aligns both machines
  .clears({ "1T1", "3T1" })
  .entrance("1T1")
  .approaching("2NA");
```

FieldUnit manages the low-level safety details automatically: route evaluation, switch correspondence, detector locking, approach locking and time locking, aspect derivation, and signal knockdown.
All appliance names resolve once during startup, preserving $O(1)$ raw pointer dereferencing with zero heap allocation during runtime cycles.

---

## Core Features

- **Prototype Accuracy**: Uses relay-style logic named with Association of American Railroads (AAR) letters: AAR names such as `TR`, `HSR`, `ASR`, and FieldUnit names built from AAR letters such as `KR`, `WLR`, `FSR` (see the [Glossary](docs/GLOSSARY.md)).
- **Vital Interlocking Safety**:
  - Automatically enforces detector locking over switch points.
  - Enforces route locking and opposing move prevention.
  - Enforces approach locking and time locking (`SwitchLock::TIME_LOCKED`), with immediate release when the approach track circuit is vacant.
  - Automatic signal knockdown with standard one-shot stick memory.
- **Pluggable Aspect Policies (`SignalAspectPolicy.h`)**:
  - Configure rulebooks plant-wide or per-mast: Southern Pacific (1969 Lunar and 1985 Flashing Red at 1 Hz), GCOR Speed, NYC 3-Head Speed, PRR Position Lights, and Semaphores.
  - Supports custom rulebook resolver lambdas.
- **First-Class Crossovers (`Crossover.h`)**:
  - Coordinates dual physical switch machines and limit switches under a single logical crossover appliance.
- **Mechanical Semaphores (`SemaphoreDriver.h`)**:
  - Drives hobby servos (PCA9685 / PWM) to calibrated stop, approach, and clear angles for upper- and lower-quadrant blades.
- **Electric Switch Locks (`WLS` / `WLK`)**:
  - Dispatcher release controls (`WLS`) and lock-released office indications (`WLK`). The field unit releases a lock only when every signal is at stop and no time locking runs.
- **Zero Heap Allocation After Startup**: Safe for operating sessions without memory leaks or fragmentation.
- **Declarative String Wiring**: Eliminates file-scope pointer handles while keeping $O(1)$ runtime execution.
- **Two-Interface Architecture**:
  - **Code Line Interface**: Control transaction and indication vector exchange (`ControlTransaction` $\longleftrightarrow$ `IndicationVector`) via `AarTextCodec`, `BitPackedCodec` (C/MRI), `StreamCodeLine` (Serial/RS-485), and `MqttCodeLine`.
  - **Device Interface**: Unified trackside boundary supporting both:
    - *Low-Level Hardware*: Electrical pins, A/D, D/A, and PWM servos via `IOBus` (onboard GPIO, MCP23017 I2C, C/MRI shift registers).
    - *High-Level Semantic Appliances*: Smart domain messaging via `MqttApplianceBus` (JMRI MQTT topics: `track/sensor/`, `track/turnout/`, `track/signalmast/`).
- **Encapsulated Vital Scan Cycle**:
  - `cp.tick(nowMs)` atomically runs Sample Inputs $\longrightarrow$ Vital Safety Rules $\longrightarrow$ Drive Outputs. Every sketch `loop()` collapses down to `cp.tick()`.
- **Driver Policies & Hybrid Mocking**:
  - Configure plant-wide driver policies (`cp.setDefaultDriverPolicy(...)`) and override individual devices (`cp.overrideDriver("3", &mockSw)`) to bench-test uninstalled turnouts before physical wiring.
- **Sectional Route Release**:
  - Trailing switches release progressively as a train clears each switch's detector fouling point, freeing vacated switches for conflicting moves while maintaining route locking ahead of and under the train.
- **Deployment Flexibility**:
  - Run **distributed** on microcontrollers (ESP32, RP2040, AVR) inside local bungalows.
  - Run **centralized**: one field processor (a single computer) runs several field units and drives remote C/MRI racks or cpNodes.
  - Run in automated **CI/CD test runners** with `MockCodeLine` and `MockIOBus`.

---

## Choose Your Path

### 1. New to FieldUnit? Start Here
Learn how to turn a track diagram into working code:
- **[Tutorial 1: Drawing Your Signaling Track Plan](docs/tutorials/01_drawing_your_signaling_track_plan.md)**: Learn where to place rail gaps (OS sections and other track circuits), how to position and name signals, and how to create a data collection sheet.
- **[Tutorial 2: Building Your First Control Point](docs/tutorials/02_building_your_first_cp.md)**: A step-by-step walkthrough turning a plan into working C++ code using CP Corporal as an example.
- **[Tutorial 3: Signal Operation, Masts, and Aspects](docs/tutorials/03_Signal_Operation.md)**: Learn how multi-head signals derive aspects, configure Southern Pacific lunar vs flashing red rulebooks, and write custom aspect policies.

### 2. Connecting Hardware
Learn how to map physical hardware to FieldUnit appliances:
- **[How-To: Hardware Wiring and AAR Bit Mapping](docs/how-to/01_data_collection_and_aar_bits.md)**: Connect Tortoise motors, DCCOD detectors, searchlight LEDs, and cpNode/IOX boards.
- **[How-To: Hand-Throw Switches and Electric Locks](docs/how-to/03_dual_control_and_electric_locks.md)**: Model hand-throw switches with electric locks (`WL`) that the dispatcher releases.
- **[How-To: Semaphores and Eastern Speed Signaling](docs/how-to/04_semaphores_and_eastern_signaling.md)**: Drive mechanical semaphore blades with hobby servos (PCA9685) and configure NYC 3-head speed signaling and PRR position lights.

### 3. Dispatcher and Network Integration
Learn how to connect FieldUnit to your control plane:
- **[How-To: JMRI and Code Line Integration](docs/how-to/02_jmri_and_cmri_integration.md)**: Connect field units to JMRI CTC panels via CMRInet or MQTT.

### 4. Technical Architecture and Domain Reference
Dive into the engineering foundation:
- **[Inside the Bungalow: AAR Signaling Primer](docs/AAR_SIGNALING_PRIMER.md)**: An introduction to the mind of a railroad signal maintainer, explaining why every switch owns 6 relays and how fail-safe circuits work.
- **[Control Point Architecture Specification](docs/CONTROL_POINT_ARCHITECTURE.md)**: Architecture specification of the field unit and its two interfaces, in ASD-STE100 style.
- **[Signaling Nomenclature and Glossary](docs/GLOSSARY.md)**: The source of truth for terms in FieldUnit: prototype, FieldUnit and AAR relay names, with sources.
- **[Example Sketches](examples/)**: Compilable, verified sketches including CP Christopher (MP 81) and CP Corporal (MP 83).
