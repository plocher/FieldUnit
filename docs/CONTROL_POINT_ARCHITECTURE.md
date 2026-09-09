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
|      - Turnout (Point Correspondence and Lock State)        |
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
A Control packet arrives as an atomic snapshot of requested plant changes.
The engine evaluates the request against active locks and current occupancy.
The engine immediately evaluates all turnout motion commands.
If a turnout command violates locks or occupancy, the engine rejects it immediately.
The engine does not queue turnout movement requests.
Certain signal authorities latch vital memory across cycles:
- **Fleeting**: The signal re-clears automatically after a train clears the route.
- **Call-On**: The engine permits a low-speed move into an occupied block under rulebook authority.
- **Engine Return (Stick Relay Conditioning)**: The engine remembers authority to let an engine return to its train after switching moves.
The engine emits an Indication snapshot when plant state changes or when a heartbeat timer expires.

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

##### B. Turnout Appliance (`Turnout`)
The Turnout models track switch points.
It manages motor movement and position feedback.
It does not manage track fouling circuits directly.
It has three attributes:
- `commandedPosition`: `NORMAL` or `REVERSE`.
- `reportedPosition`: `NORMAL`, `REVERSE`, `MOVING`, or `OUT_OF_CORRESPONDENCE`.
- `lockState`: A bitfield containing `UNLOCKED`, `DETECTOR_LOCKED`, `ROUTE_LOCKED`, or `TIME_LOCKED`.

The Turnout accepts a `throw(position)` command.
If `lockState` is not `UNLOCKED`, the Turnout rejects the command.
The Turnout does not energize motor outputs during a rejected command.

##### C. Crossover Appliance (`Crossover`)
A Crossover pairs two physical turnouts operated by one logical command.
It issues movement commands to both turnouts in unison.
It reports `NORMAL` only when both turnouts report `NORMAL`.
It reports `REVERSE` only when both turnouts report `REVERSE`.
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

#### Interlocking Control Table
The Interlocking Control Table defines routes through the plant.
Each row in the table specifies:
1. `RouteID`: Unique name for the route.
2. `GoverningMast`: The entrance signal mast.
3. `RequestedDirection`: Required traffic flow direction (`LEFT` or `RIGHT`).
4. `SwitchAlignments`: Required position for each turnout in the path.
5. `BlockCircuits`: Local track circuits that must be `VACANT`.
6. `ApproachCircuits`: Downstream blocks that dictate speed aspects.
7. `AspectCeiling`: Maximum permitted indication for this track geometry.

```cpp
plant.addRoute({
    .name              = "MT2-MT1",
    .mast              = mast2N,
    .direction         = Direction::LEFT,
    .aspectCeiling     = Indication::DIVERGING_CLEAR,
    .switches          = { {xover3, Turnout::REVERSE}, {sw1, Turnout::NORMAL} },
    .blockCircuits     = { tc3BT1, tc3T1, tc1T1 },
    .approachCircuits  = { tc1SA }
});
```

---

### 3.2 Interfaces

#### Ingress Interface (Control Message)
The Control Point ingests a structured snapshot from the network codec:
- Turnout command map: List of `(TurnoutId, CommandedPosition)`.
- Signal authority map: List of `(SignalId, DirectionAuthority)`.
- Auxiliary commands: List of `(AuxId, BooleanState)`.

#### Egress Interface (Indication Message)
The Control Point exports a structured snapshot to the network codec:
- Turnout correspondence: Position and lock bitmask per switch.
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
