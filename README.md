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
                     |
         (Desires: "Line Route 1")
                     v
       +----------------------------+
       |   Control Point Engine     |  <-- FieldUnit
       |  (Evaluates Safety Rules)  |
       +----------------------------+
                     |
          (Drives physical pins)
                     v
    [ Track Switches, Detectors, Signals ]
```

---

## How FieldUnit Code Looks

FieldUnit replaces complex procedural code with readable, declarative routes:

```cpp
// Siding Route: Diverging move over Crossover SW3 into Siding
cp.route("MAIN_TO_SIDING")
  .governedBy(sig2, DirectionAuthority::RIGHT)
  .displays(mast2S, 1 /* Lower Head */, Indication::DIVERGING_CLEAR)
  .aligns({ {sw1, SwitchPosition::NORMAL}, 
            {sw3, SwitchPosition::REVERSE}, 
            {sw3B, SwitchPosition::REVERSE} })
  .clears({ tc1T1, tc3T1 })
  .approaching(tc2NA);
```

FieldUnit manages the low-level safety details automatically: route evaluation, switch point correspondence, detector locking, multi-head aspect derivation, and signal knockdown.
This allows you to focus on expressing your railroad's operational routes and rules.

---

## Core Features

- **Prototype Accuracy**: Uses standard Association of American Railroads (AAR) relay logic (`TR`, `WR`, `KR`, `HSR`, `ASR`, `ERS`).
- **Vital Safety**: Automatically prevents throwing switches under standing trains (detector locking) and prevents opposing moves.
- **Zero Heap Allocation After Startup**: Safe for long operating sessions without memory leaks or fragmentation.
- **Deployment Flexibility**:
  - Run **distributed** on small boards (Xiao, ESP32) inside local trackside bungalows.
  - Run **centralized** on a single computer driving remote C/MRI input/output racks.
  - Run **hybrid** layouts mixing local boards with central staging yards.
- **Protocol Independence**: Works with CMRInet serial lines, MQTT brokers, or local memory calls.

---

## Choose Your Path

### 1. New to FieldUnit? Start Here
Learn how to turn a track diagram into working code:
- **[Tutorial 1: Drawing Your Signaling Track Plan](docs/tutorials/01_drawing_your_signaling_track_plan.md)**: Learn where to place rail gaps (island blocks), how to position and name signals, and how to create a data collection sheet.
- **[Tutorial 2: Building Your First Control Point](docs/tutorials/02_building_your_first_cp.md)**: A step-by-step walkthrough turning a plan into working C++ code using CP Corporal as an example.

### 2. Connecting Hardware
Learn how to map physical hardware to FieldUnit appliances:
- **[How-To: Hardware Wiring and AAR Bit Mapping](docs/how-to/01_data_collection_and_aar_bits.md)**: Connect Tortoise motors, DCCOD detectors, searchlight LEDs, and cpNode/IOX boards.
- **[How-To: Dual-Control Switches and Electric Locks](docs/how-to/03_dual_control_and_electric_locks.md)**: Model hand-throw switches and dispatcher switch locks (`WL`).

### 3. Dispatcher and Network Integration
Learn how to connect FieldUnit to your control plane:
- **[How-To: JMRI and CodeLine Integration](docs/how-to/02_jmri_and_cmri_integration.md)**: Connect Control Points to JMRI CTC panels via CMRInet or MQTT.

### 4. Technical Architecture and Domain Reference
Dive into the engineering foundation:
- **[Control Point Architecture Specification](docs/CONTROL_POINT_ARCHITECTURE.md)**: Complete 4-tier architectural specification in ASD-STE100 style.
- **[Signaling Nomenclature and Glossary](docs/GLOSSARY.md)**: Official AAR relay terms and operational definitions.
- **[Example Sketches](examples/)**: Compilable, verified sketches including CP Christopher (MP 81).
