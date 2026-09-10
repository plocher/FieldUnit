# FieldUnit

FieldUnit is a modular C++ toolkit that empowers model railroaders to construct their own Control Points and Interlockings.
The resulting control units run with the same vital safety rules, route locking, and behavioral contracts as the prototype railroad.

---

## What FieldUnit Does

On a real railroad, a **Control Point** (or **Interlocking**) is a junction of tracks, switches, and signals.
A dispatcher in an office decides where trains should go.
However, the dispatcher cannot throw switches or clear signals directly.
The dispatcher sends a request to the local Control Point in the field.

FieldUnit lets you build that local field controller.
It includes the physical connection to the devices on your layout:
- **Sensors (Inputs)**: Reads block occupancy detectors (DCCOD, optical sensors) and switch point limit switches.
- **Actuators (Outputs)**: Drives switch motors (Tortoise, servos) and signal lamps (LEDs, searchlights).

The controller acts as the local safety guard between the dispatcher and your track:
1. **Checks the switches**: Are the switch points locked in position?
2. **Checks the tracks**: Is there a train already occupying the points?
3. **Checks opposing signals**: Are conflicting train routes held at Stop?
4. **Moves the plant**: If the move is safe, the Control Point drives the physical switch motors and displays the signal aspect on the rails.
5. **Reports reality**: The Control Point continuously sends verified plant status back to the dispatcher.

FieldUnit brings this exact prototype behavior to model railroad control:
- **Distributed Microcontrollers**: Run autonomous field units on small microcontrollers (ESP32, RP2040, AVR) directly connected to GPIO or I2C port expanders (MCP23017, cpNode-IOX).
- **Centralized Hosts**: Run multiple interlocking plants together on a central computer or processor, driving remote I/O nodes through C/MRI input and output byte arrays.
- **Interlocking Towers**: Model local tower plants where a human leverman operates mechanical or pistol-grip levers under vital safety rules.

```
[ Dispatcher (JMRI / CTC Panel / CodeLine) ]
                     │
          (Seam "A": Supervisory CodeLine)  <-- AAR Snapshots: 1NWS, 2NGS <-> 1NWK, 1T1K
                     │
                     v
       +----------------------------+
       |   Control Point Engine     |  <-- FieldUnit Vital Interlocking Logic
       |  (Evaluates Safety Rules)  |      (Zero Heap Allocation, O(1) Execution)
       +----------------------------+
                     │
          (Seam "B": Appliance I/O Bus)     <-- I2C, C/MRI, GPIO, Servos, MQTT
                     │
                     v
    [ Track Switches, Detectors, Signals, Semaphores ]
```

---

## How FieldUnit Code Looks

FieldUnit replaces complex procedural code with readable, declarative routes:

```cpp
// Siding Route: Diverging move over Crossover 3 into Siding
cp.route("MAIN_TO_SIDING")
  .governedBy("2", DirectionAuthority::RIGHT)
  .displays("2S", 1 /* Lower Head */, Indication::DIVERGING_CLEAR)
  .aligns({ {"1", SwitchPosition::NORMAL}, 
            {"3", SwitchPosition::REVERSE} }) // Crossover 3 aligns both machines
  .clears({ "1T1", "3T1" })
  .entrance("1T1")
  .approaching("2NA");
```

FieldUnit manages the low-level safety details automatically: route evaluation, switch point correspondence, detector locking, approach time locking, aspect derivation, and signal knockdown.
All appliance names resolve once during startup, preserving $O(1)$ raw pointer dereferencing with zero heap allocation during runtime cycles.

---

## Core Features

- **Prototype Accuracy**: Uses standard Association of American Railroads (AAR) relay logic (`TR`, `WR`, `KR`, `HSR`, `ASR`, `ERS`).
- **Vital Interlocking Safety**:
  - Automatically enforces detector locking over switch points.
  - Enforces route locking and opposing move prevention.
  - Enforces approach time locking (`SwitchLock::TIME_LOCKED`) with safe immediate cancellation when approach tracks are vacant.
  - Automatic signal knockdown with standard one-shot stick memory.
- **Pluggable Aspect Policies (`SignalAspectPolicy.h`)**:
  - Configure rulebooks plant-wide or per-mast: Southern Pacific (1969 Lunar and 1985 Flashing Red at 1 Hz), GCOR Speed, NYC 3-Head Speed, PRR Position Lights, and Semaphores.
  - Supports custom rulebook resolver lambdas.
- **First-Class Crossovers (`Crossover.h`)**:
  - Coordinates dual physical switch machines and limit switches under a single logical crossover appliance.
- **Mechanical Semaphores (`SemaphoreDriver.h`)**:
  - Drives hobby servos (PCA9685 / PWM) to calibrated stop, approach, and clear angles for upper- and lower-quadrant blades.
- **Electric Switch Locks (`WLS` / `WLK`)**:
  - Dispatcher unlock commands (`WLS`) and verified field unlock indications (`WLK`) with automatic signal-safety interlocks.
- **Zero Heap Allocation After Startup**: Safe for operating sessions without memory leaks or fragmentation.
- **Declarative String Wiring**: Eliminates file-scope pointer handles while keeping $O(1)$ runtime execution.
- **Two-Seam Architecture**:
  - **Seam "A" (Supervisory)**: Protocol independence via `AarTextCodec`, `BitPackedCodec` (C/MRI), `StreamCodeLine` (Serial/RS-485), and `MqttCodeLine`.
  - **Seam "B" (Appliance I/O)**: Hardware independence via `IOBus` (onboard GPIO, MCP23017 I2C expanders, C/MRI byte arrays, and servos).
- **Deployment Flexibility**:
  - Run **distributed** on microcontrollers (ESP32, RP2040, AVR) inside local bungalows.
  - Run **centralized** on a single computer driving remote C/MRI racks or cpNodes.
  - Run in automated **CI/CD test runners** with `MockCodeLine` and `MockIOBus`.

---

## Choose Your Path

### 1. New to FieldUnit? Start Here
Learn how to turn a track diagram into working code:
- **[Tutorial 1: Drawing Your Signaling Track Plan](docs/tutorials/01_drawing_your_signaling_track_plan.md)**: Learn where to place rail gaps (island blocks), how to position and name signals, and how to create a data collection sheet.
- **[Tutorial 2: Building Your First Control Point](docs/tutorials/02_building_your_first_cp.md)**: A step-by-step walkthrough turning a plan into working C++ code using CP Corporal as an example.
- **[Tutorial 3: Signal Operation, Masts, and Aspects](docs/tutorials/03_Signal_Operation.md)**: Learn how multi-head signals derive aspects, configure Southern Pacific lunar vs flashing red rulebooks, and write custom aspect policies.

### 2. Connecting Hardware
Learn how to map physical hardware to FieldUnit appliances:
- **[How-To: Hardware Wiring and AAR Bit Mapping](docs/how-to/01_data_collection_and_aar_bits.md)**: Connect Tortoise motors, DCCOD detectors, searchlight LEDs, and cpNode/IOX boards.
- **[How-To: Dual-Control Switches and Electric Locks](docs/how-to/03_dual_control_and_electric_locks.md)**: Model hand-throw switches and dispatcher switch locks (`WL`).
- **[How-To: Semaphores and Eastern Speed Signaling](docs/how-to/04_semaphores_and_eastern_signaling.md)**: Drive mechanical semaphore blades with hobby servos (PCA9685) and configure NYC 3-head speed signaling and PRR position lights.

### 3. Dispatcher and Network Integration
Learn how to connect FieldUnit to your control plane:
- **[How-To: JMRI and CodeLine Integration](docs/how-to/02_jmri_and_cmri_integration.md)**: Connect Control Points to JMRI CTC panels via CMRInet or MQTT.

### 4. Technical Architecture and Domain Reference
Dive into the engineering foundation:
- **[Inside the Bungalow: AAR Signaling Primer](docs/AAR_SIGNALING_PRIMER.md)**: An introduction to the mind of a railroad signal maintainer, explaining why every switch owns 6 relays and how fail-safe circuits work.
- **[Control Point Architecture Specification](docs/CONTROL_POINT_ARCHITECTURE.md)**: Complete 4-tier architectural specification in ASD-STE100 style.
- **[Signaling Nomenclature and Glossary](docs/GLOSSARY.md)**: Official AAR relay terms and operational definitions.
- **[Example Sketches](examples/)**: Compilable, verified sketches including CP Christopher (MP 81) and CP Corporal (MP 83).
