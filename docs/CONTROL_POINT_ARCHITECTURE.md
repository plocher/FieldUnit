# Control Point and Interlocking Appliance Architecture

## 1. Introduction

### 1.1 Project Working Name
Control Point Appliance Architecture (FieldUnit v2 Core).

### 1.2 Author
John Plocher.

### 1.3 Date
2026-09-09.

---

## 2. Project Summary

### 2.1 Project Description
This project defines functional interfaces and behavioral contracts for railroad field appliances. It provides a modular C++ foundation for Control Points and interlockings on model railroads.

### 2.2 Problem Summary
The current FieldUnit library binds hardware pin input/output, network packet wire formats, state machines, and safety logic into monolithic classes. This design prevents reuse, obscures railroad safety rules, and makes configuration difficult. The new architecture separates physical drivers, logical appliances, route control tables, and network protocol codecs.

---

## 3. Proposed Changes

### 3.1 Technical Description
The architecture divides the Control Point into four decoupled tiers.

```
+-------------------------------------------------------------+
|                1. Network Protocol Codec                    |
|      (Unpacks Control Snapshots, Packs Indication Vectors)  |
+-------------------------------------------------------------+
                              |
                              v
+-------------------------------------------------------------+
|               2. Control Point Vital Engine                 |
|      - Evaluates Control Table                              |
|      - Enforces Route Locks and Approach Locks              |
|      - Resolves Operational Indications                     |
+-------------------------------------------------------------+
                              |
                              v
+-------------------------------------------------------------+
|               3. Logical Railroad Appliances                |
|      - Track Circuit (Qualified Occupancy State)            |
|      - Switch (Point Correspondence and Lock State)         |
|      - Signal Mast (Indication to Aspect Mapping)           |
+-------------------------------------------------------------+
                              |
                              v
+-------------------------------------------------------------+
|               4. Physical Hardware Drivers                  |
|      (Drives Tortoise motors, servos, LEDs, reads pins)     |
+-------------------------------------------------------------+
```

#### Snapshot Execution Model
All processing operates on synchronized snapshots.
The engine reads all field inputs at the start of each cycle to create a stable input snapshot.
A Control packet arrives as a complete, plant-wide transaction vector specifying desired state for all appliances.
The engine evaluates the entire transaction against active locks and current occupancy:
- If a switch movement violates locks or occupancy, the engine does not move that switch.
- The engine does not send error packets, NACKs, or conversational text.
- The engine does not queue switch movement requests.
The engine continually emits an Indication vector reporting verified plant reality.
The controlling client detects non-execution by comparing its commanded intent against the reported indications.
Certain signal authorities latch vital memory across cycles:
- **Fleeting**: The signal re-clears automatically after a train clears the route.
- **Call-On**: The engine permits a low-speed move into an occupied block under rulebook authority.
- **Engine Return (Stick Relay Conditioning)**: The engine remembers authority to let an engine return to its train after switching moves.

#### Appliance Contracts and Functional Interfaces

##### A. Track Circuit Appliance (`TrackCircuit`)
The Track Circuit models track block occupancy.
It operates as a pure data source.
It has two attributes:
- `occupancy`: `VACANT` or `OCCUPIED`.
- `quality`: `GOOD`, `LOST_COMMS`, or `DEFECT`.

If an input pin drops or communication fails, quality becomes `LOST_COMMS`.
The vital logic treats both `OCCUPIED` and `LOST_COMMS` as restrictive.
The Indication message reports the distinct quality value to the dispatcher.

##### B. Switch Appliance (`Switch`)
The Switch models track switch points (AAR standard: Switch, not Turnout).
It manages motor movement and position feedback.
It does not manage track fouling circuits directly.
It has three attributes:
- `commandedPosition`: `NORMAL` or `REVERSE`.
- `reportedPosition`: `NORMAL`, `REVERSE`, `MOVING`, or `OUT_OF_CORRESPONDENCE`.
- `lockState`: A bitfield containing `UNLOCKED`, `DETECTOR_LOCKED`, `ROUTE_LOCKED`, or `TIME_LOCKED`.

The Switch accepts a `throwSwitch(position)` command.
If `lockState` is not `UNLOCKED`, the Switch rejects the command.
The Switch does not energize motor outputs during a rejected command.

##### C. Crossover Appliance (`Crossover`)
A Crossover pairs two physical switches operated by one logical command.
It inherits from `Switch` and can be aligned directly in routes (`Route::aligns({ {xover3, SwitchPosition::REVERSE} })`).
It issues movement commands to both switches in unison.
It reports `NORMAL` only when both switches report `NORMAL`.
It reports `REVERSE` only when both switches report `REVERSE`.
If either switch moves or fails, the Crossover reports `MOVING` or `OUT_OF_CORRESPONDENCE`.

##### D. Signal Mast Appliance (`SignalMast`)
A Signal Mast models a physical wayside signal structure.
A mast contains one, two, or three Signal Heads.
The mast separates the operational rule from the visual aspect:
- **Indication**: The rulebook operational instruction for the train crew (`STOP`, `RESTRICTING`, `APPROACH`, `CLEAR`).
- **Aspect**: The physical appearance of the lamps (`RED_OVER_RED`, `YELLOW_OVER_GREEN`).

The Signal Mast applies an exchangeable rulebook policy.
A Route Signaling policy selects aspects based on the assigned track path.
A Speed Signaling policy selects aspects based on turnout speed categories.

##### E. Approach Circuit Appliance (`RemoteTrackCircuit`)
An Approach Circuit monitors track sections outside the local control point.
It listens to Indication packets broadcast by adjacent control points.
It updates local occupancy without local physical detector wiring.
It includes a freshness timer.
If adjacent station messages stop, the circuit sets quality to `LOST_COMMS`.

##### F. Vital Stick Relays (Fleeting and Engine Return)
The engine models vital stick relays to preserve operational state across train movements:

1. **Standard Signal Stick**:
   A train accepts a cleared signal and shunts the entrance track circuit.
   The signal knocks down immediately to `STOP`.
   The control stick drops and remains dropped.
   The signal does not re-clear when the train departs.
   The signal requires a new dispatcher control command to clear again.

2. **Fleeting Stick (`FS`)**:
   The dispatcher commands fleeting mode active.
   The fleet stick energizes in the local control point.
   A train enters the plant and knocks down the signal.
   When the train vacates the route and downstream blocks clear, the signal re-clears automatically.
   The dispatcher does not need to send new commands for following trains.
   A manual command to `STOP` drops the fleet stick immediately.

3. **Engine Return Stick (`ERS`)**:
   A train pulls into a control point, uncouples, and moves forward into an adjacent block or siding.
   The engine tracks the sequential drop of track circuits:
   $$\text{Interlocking Island Occupied} \longrightarrow \text{Exit Block Occupied}$$
   The Engine Return Stick energizes when the train straddles or exits onto the adjacent block.
   The stick stays energized while the exit block remains continuously occupied.
   The stick proves that the train occupies the block boundary.
   The engine bypasses the standard five-minute approach locking safety timer.
   The engine permits an immediate return move into the plant under a `RESTRICTING` aspect.
   When the engine reverses and clears the exit block, the stick drops to rest.

#### Interlocking Control Table
The Interlocking Control Table defines routes through the plant.
Each row in the table specifies:
1. `RouteID`: Unique name for the route.
2. `GoverningMast`: The entrance signal mast.
3. `RequestedDirection`: Required traffic flow direction (`LEFT` or `RIGHT`).
4. `SwitchAlignments`: Required position for each switch in the path.
5. `BlockCircuits`: Local track circuits that must be `VACANT`.
6. `ApproachCircuits`: Downstream blocks that dictate speed aspects.
7. `AspectCeiling`: Maximum permitted indication for this track geometry.

```cpp
plant.route("MT2-MT1")
  .governedBy(sig2, DirectionAuthority::LEFT)
  .displays(mast2N, 1 /* Lower Head */, Indication::DIVERGING_CLEAR)
  .aligns({ {sw3, SwitchPosition::REVERSE}, {sw3B, SwitchPosition::REVERSE}, {sw1, SwitchPosition::NORMAL} })
  .clears({ tc3BT1, tc3T1, tc1T1 })
  .entrance(tc3BT1)
  .approaching(tc1SA);
```

### 3.2 Operating Regimes and Methods of Operation
The architecture accommodates multiple North American operating regimes:

1. **Centralized Traffic Control (CTC - Rule 261)**:
   Train movements are authorized directly by wayside signal indication under the remote control of a train dispatcher.
   The dispatcher controls the plant through network Control Snapshots across the CodeLine Interface.
   Power switch machines line routes remotely.

2. **Interlocking Tower and Local Station Control**:
   The operator directly controls the plant from local levers (pistol-grip, mechanical, or fascia toggle switches).
   The local control console feeds the Control Point engine directly or via a local CodeLine-analogue connecting the fascia/desk to the trackside bungalow.
   The same Interlocking Control Table evaluates safety rules.
   Inter-station train movements between towers are governed by separate operating rules:
   - **Timetable and Train Order (TT&TO - Rule 251 / Manual Block)**: Authority is conveyed by written train orders and schedules, coordinated between station operators and the dispatcher via telephone or telegraph. Signals serve as train-order boards or manual block signals rather than direct traffic authority.
   - **Track Warrant Control (TWC) / Direct Traffic Control (DTC)**: Radio-transmitted verbal authorities.

3. **Automatic Block Signaling (ABS / APB)**:
   Signals provide block spacing and collision protection.
   Timetable and Train Order (TT&TO) or Track Warrants provide movement authority.
   Directional sticks (`ESR` and `WSR`) prevent opposing moves on single track.
   Switches are hand-operated with electric switch locks (`WLR`).

4. **Dark Territory**:
   No wayside block signals govern movement.
   Track circuits provide occupancy indications to dispatcher screens.
   Electric locks on switches enforce lock-and-block discipline.

### 3.3 Control Point Autonomy and the Two Core Interfaces

FieldUnit defines two independent architectural boundaries: the **CodeLine Interface** connecting to the dispatcher control plane, and the **Device Interface** connecting to the trackside physical world.

```
[ Dispatcher Office / CTC Machine / JMRI Panel ]
                       │
                       │  <=== 1. CodeLine Interface (Transactional Snapshots)
                       │        (Plant-wide atomic vectors: Controls <-> Indications)
                       ▼
┌─────────────────────────────────────────────────────────────┐
│  FieldUnit Vital Engine & Logical Appliances                │
│  (ControlPoint, ControlTable, TrackCircuit, Switch, Mast)   │
└─────────────────────────────────────────────────────────────┘
                       │
                       │  <=== 2. Device Interface (Trackside Boundary)
                       │
        ┌──────────────┴──────────────┐
        ▼                             ▼
[ High-Level Semantic Interface ]  [ Low-Level Hardware Interface ]
(Domain Entities over MQTT / LCC)  (Electrical Pins & Signals via IOBus)
- Turnouts: CLOSED / THROWN        - Physical GPIO & I2C pins
- Sensors: ACTIVE / INACTIVE       - A/D, D/A, Analog threshold sensing
- Signal Masts: Aspect strings     - Timed PWM servo blade angles
- Fastclock / Matter room lights   - C/MRI shift register bit arrays
```

#### 1. The CodeLine Interface (Transactional Snapshot Contract)
The boundary between the Control Plane (Dispatcher / Tower) and the Field Interlocking:
- **Strictly Transactional**: Operates on atomic plant-wide snapshots (`ControlTransaction` ingress, `IndicationVector` egress). It is NOT an RPC, query-response, or piecemeal command protocol.
- Handled by `AarTextCodec` (symbolic text) or `BitPackedCodec` (binary bitstreams).
- Transported by `StreamCodeLine` (RS-485 serial), `MqttCodeLine` (MQTT supervisory topic), or in-memory direct dispatch.
- Control Points are strictly autonomous: they never share pointers or private memory across the CodeLine.

#### 2. The Device Interface (Trackside Appliance Boundary)
The single boundary between logical interlocking rules and the trackside physical world, supporting two switchable implementation types:

##### Type A: Low-Level Hardware Interface (Electrical Pins & Signals)
Connects appliance drivers to "dumb" electrical hardware requiring voltage, current, and timing management:
- Abstracted by `IOBus`, `IOBit`, and dedicated drivers (`SwitchDriver`, `TrackCircuitDriver`, `SignalMastDriver`, `SemaphoreDriver`).
- Decomposes appliances into electrical coordinates: `(device, offset, bitIndex, polarity)`, ADC thresholds, and PWM servo angles.
- Supported backends:
  1. **Direct Microcontroller GPIO**: `digitalRead` / `digitalWrite`.
  2. **Local I2C Port Expanders**: MCP23017, cpNode-IOX via `I2CexpanderIOBus`.
  3. **Classic C/MRI Serial Polling**: Remote shift register nodes via `CmriIOBus` (`IB[]`/`OB[]`).
  4. **C/MRI over Network Sockets**: Ethernet/WiFi remote nodes.
  5. **Automated Desktop Simulation**: `MockIOBus`.

##### Type B: High-Level Semantic Interface (Smart Domain Appliances)
Connects logical appliances directly to autonomous, networked smart devices communicating via domain entities rather than pin bit-fields:
- Abstracted by `MqttApplianceBus`.
- Operates on named domain appliance topics and payloads:
  - `TrackCircuit` $\longleftrightarrow$ `track/sensor/<name>` (`ACTIVE` / `INACTIVE` or `OCCUPIED` / `VACANT`)
  - `Switch` $\longleftrightarrow$ `track/turnout/<name>` (`CLOSED` / `THROWN`) and feedback `track/turnout/<name>/state`
  - `SignalMast` $\longleftrightarrow$ `track/signalmast/<name>` (Aspect strings: `"Clear"`, `"Approach"`, `"Stop"`, etc.)
  - `Light` $\longleftrightarrow$ `track/light/<name>` (`ON` / `OFF`)
- Implements symmetric **`onPack` / `onUnpack`** callback vocabulary matching MQTT subscription ingress and publication egress.
- Enables future smart integrations without altering interlocking code, such as Matter / HomeAutomation bridges for fastclock-driven layout room ambient lighting.

#### The Deployment Matrix
Because the vital engine is insulated across these interfaces, the exact same plant definition runs across multiple deployment topologies:

| Deployment Topology | CodeLine Interface | Low-Level Electrical I/O | High-Level Device Bus |
| :--- | :--- | :--- | :--- |
| **1. Smart Bungalow (Distributed)** | **Across layout wire**<br>(RS-485 serial or CodeLine MQTT) | **Local inside bungalow**<br>(GPIO or I2C expanders) | — |
| **2. Central Host (Classic C/MRI)** | **Local in host memory**<br>(Internal C++ function call) | **Across layout wire**<br>(`CmriIOBus` serial or TCP) | — |
| **3. IoT Smart Layout (MQTT Devices)** | **Supervisory MQTT topic**<br>(`railroad/cp_corporal/control`) | — | **Device MQTT topics**<br>(`MqttApplianceBus`: `track/sensor/1T1`) |
| **4. Hybrid Layout** | **CodeLine Serial or MQTT** | **Local I2C detectors** | **WiFi/MQTT turnouts & masts** |
| **5. Desktop Test Bench / Simulation** | **`MockCodeLine`**<br>(Injected text test vectors) | **`MockIOBus`** | — |

### 3.4 Interlocking Tower Control as a Hybrid Model
An Interlocking Tower combines elements of both a Control Plane and a Control Point:
- **External View (To Dispatcher)**:
  The Tower is a field plant.
  The dispatcher does not directly throw tower switches.
  The dispatcher communicates movement requests or grants directional traffic authority to the tower.
- **Internal View (To Tower Operator)**:
  The Tower acts as a local human Control Plane.
  The operator pulls local mechanical, electric, or fascia levers.
  The levers feed directly into the Control Point engine.
- **Cooperative Locking (The Slot)**:
  Signals connecting tower territory to dispatcher territory require a cooperative handshake.
  The dispatcher grants authority.
  The tower operator lines the local route.
  The signal clears only when both the dispatcher and the operator agree.

### 3.5 AAR Relay Contact Logic Equivalence
FieldUnit maps Association of American Railroads (AAR) relay circuits directly to C++ code:
- Series contacts map to logical AND (`&&`).
- Parallel contacts map to logical OR (`||`).
- Front contacts (neutral closed when energized) map to `true`.
- Back contacts (closed when de-energized) map to `!true`.
- Stick circuits map to self-holding boolean state variables.

```cpp
// AAR Home Signal Relay (1HR) circuit equivalence:
// 1HR = 1TR && 2TR && 1NWCR && 2ASR
bool HR = tr1.TR() && tr2.TR() && sw1.NWCR() && sig2.ASR();
```

---

### 3.6 Interfaces

#### Ingress Interface (Control Message)
The Control Point ingests a structured snapshot from the network codec:
- Switch command map: List of `(SwitchId, CommandedPosition)`.
- Signal control map: List of `(SignalId, DirectionAuthority)`.
- Auxiliary commands: List of `(AuxId, BooleanState)`.

#### Egress Interface (Indication Message)
The Control Point exports a structured snapshot to the network codec:
- Switch correspondence: Position and lock bitmask per switch.
- Track occupancy: Occupancy state and quality flag per circuit.
- Signal status: Displayed aspect and active time-lock status per mast.
- System health: Local maintainer mode and communication status.

#### Driver Interface (Hardware Layer)
Appliances access pins through abstract driver interfaces:
- `SwitchDriver`: `setMotor(position)`, `readPoints()`.
- `SignalLampDriver`: `setLamps(headIndex, colorMask)`.
- `SensorDriver`: `readState()`.

---

## 4. Technical Considerations

### 4.1 Security and Safety Dependencies
- All memory allocations occur during `setup()`.
- The system prohibits dynamic heap allocation after `setup()` completes.
- Incomplete or corrupted network messages cause immediate command rejection.
- Lost sensor communication forces the associated block to a restrictive state.

### 4.2 Compatibility
- The architecture is protocol-neutral.
- It supports CMRInet serial, MQTT, and bespoke local buses.
- Hardware drivers support direct micro-controller GPIO, MCP23017 I2C expanders, and PCA9685 servo drivers.

### 4.3 Design and Architectural Principles
- **Immediate Turnout Evaluation**: The engine executes or rejects turnout movements immediately. It does not queue switch requests.
- **Vital Latching States**: The engine supports stick relay logic for Fleeting, Call-On, and Engine Return.
- **Fail-Safe Operation**: Broken circuits and communication failures produce restrictive indications.
- **Single Source of Truth**: The Control Table defines all interlocking relationships.
- **Separation of Presentation**: The Control Point does not model dispatcher levers or physical panel desks.

---

## 5. Scope

### 5.1 In Scope
- Core object models for Track Circuits, Turnouts, Crossovers, Signal Heads, and Signal Masts.
- Interlocking Control Table compiler and execution engine.
- Atomic Control and Indication snapshot managers.
- Local and remote approach block monitors.
- Exchangeable Speed Signaling and Route Signaling policy tables.
- Hardware port adapters for standard cpNode and IOX hardware.

### 5.2 Out of Scope
- Dispatcher panel hardware interfaces (handled by separate cTc panel software).
- Network physical transport implementations (uses existing CMRInet and MQTT libraries).
- Non-railroad IoT communication protocols (Matter, Zigbee, Home Assistant native).
- Configuration file parsers (initial versions use pure C++ setup code).

### 5.3 Dependencies
- Arduino core environment for ESP32 and AVR architectures.
- Verified physical pin driver libraries for I2C port expanders.
- Reliable CodeLine transport layer for remote indication listening.

### 5.4 Definition of Done
1. All core appliance classes pass unit tests with simulated hardware inputs.
2. The Control Table engine correctly reproduces CP Christopher routes from C++ definitions.
3. The vital logic proves zero heap allocation calls after `setup()` completes.
4. An unsafe turnout command received during block occupancy triggers immediate rejection without motor movement.
5. Communication timeout on a remote approach circuit causes the entrance signal to downgrade its aspect.
