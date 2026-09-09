# Railroad Signaling Nomenclature and Glossary

## 1. Introduction
This document defines standard Association of American Railroads (AAR) signaling terms and relay names used in FieldUnit.
FieldUnit uses prototype relay abbreviations to maintain direct alignment with historical railroad circuit plans.

---

## 2. Relay Naming Conventions

AAR relay names use a structured combination of functional letters:
- **Appliance Identifier**: A number or letter prefix (for example, Switch 1, Signal 2, Track Circuit 1T).
- **Core Function**: The primary operational role (Track, Switch, Signal, Stick).
- **Suffix**: `R` indicates Relay. `P` indicates Repeater Relay.

---

## 3. Core Vital Relays and State Flags

### 3.1 Track Circuit Relays
- **`TR` (Track Relay)**:
  Directly monitors the track circuit rails.
  It is energized (picked up) when the track block is `VACANT`.
  It is de-energized (dropped) when train wheels and axles shunt the rails (`OCCUPIED`).
  Broken rails or power failures drop the relay to enforce fail-safe operation.
- **`TP` / **`TPR`** (Track Repeater Relay)**:
  Repeats the state of the `TR` to provide additional contacts and software filtering.

### 3.2 Switch Control and Correspondence Relays
- **`WR` (Switch Control Relay)**:
  Commands physical switch point movement.
  It operates as `NWSR` (Normal Switch Stick Relay) or `RWSR` (Reverse Switch Stick Relay).
- **`NWCR` (Normal Switch Correspondence Relay)**:
  Energizes only when switch points reach and lock in the full Normal position.
- **`RWCR` (Reverse Switch Correspondence Relay)**:
  Energizes only when switch points reach and lock in the full Reverse position.
- **`KR` (Switch Indication Relay)**:
  Proves that a switch is in full correspondence (`NWCR` or `RWCR` active).
  Interlocking logic requires active `KR` before clearing any signal over the switch.
- **`LR` / `WLR` (Switch Lock Relay)**:
  Enforces electric and mechanical switch locking.
  It de-energizes when a switch is locked by track occupancy (detector lock) or active route.
  Switch motors can move if and only if `LR` is energized.

### 3.3 Signal Control and Authority Relays
- **`HR` (Home Relay)**:
  Energizes when route switches correspond and all local plant track circuits are vacant.
  It permits the entrance signal to display at least a `RESTRICTING` or `APPROACH` aspect.
- **`DR` (Distant Relay)**:
  Energizes when downstream intermediate blocks and signals are clear.
  It upgrades the signal aspect from `APPROACH` to `CLEAR`.
- **`HSR` (Home Signal Stick Relay)**:
  Latches the dispatcher movement authority.
  It picks up when the dispatcher sends a directional control command.
  It drops immediately when a train shunts the entrance track circuit.
  It remains dropped until the dispatcher sends a brand new code command.
- **`FSR` (Fleet Stick Relay)**:
  Maintains signal clearance across multiple trains.
  It bypasses the `HSR` stick-down requirement.
  When the first train clears the route, the signal clears automatically for following trains.

### 3.4 Interlocking Safety and Sequence Sticks
- **`ASR` (Approach Stick Relay)**:
  Enforces approach locking on plant entrances.
  It drops when a signal clears to lock all route switches.
  If the dispatcher cancels the signal before a train arrives, `ASR` runs a safety timer.
  Switches remain locked until the timer expires to protect approaching trains.
- **`ERS` (Engine Return Stick Relay)**:
  Accommodates switching and direction reversal moves.
  It picks up when a train moves forward out of the interlocking into an adjacent block.
  It holds energized while the exit block remains occupied.
  It bypasses the standard five-minute `ASR` approach locking timer.
  It permits an immediate return move into the plant under a `RESTRICTING` aspect.
  It drops when the engine returns and completely vacates the exit block.
- **`ESR` / `WSR` (Directional Stick Relays)**:
  Track train travel direction in single-track Absolute Permissive Block (APB) territory.
  They energize in sequential block drop order (A to B to C).
  They prevent opposing signals while allowing following trains on permissive aspects.

---

## 4. Operational Concepts

- **Aspect**:
  The physical appearance of a signal (lights, colors, positions, flash patterns).
- **Indication**:
  The rulebook instruction delivered to the train crew (such as Stop, Proceed, Restricting).
- **Control Snapshot**:
  A synchronized group of desired appliance states transmitted by the dispatcher.
- **Indication Snapshot**:
  A synchronized report of verified plant state transmitted to the dispatcher.
- **Detector Locking**:
  Electrical locking preventing switch movement while a train occupies the turnout points.
- **Route Locking**:
  Electrical locking preventing switch movement along an active cleared route path.
- **Approach Locking**:
  Electrical locking preventing route changes after a train passes the approach signal.
- **Time Locking**:
  A safety timer running down after an operator cancels a permissive signal.
- **Slotting (The Slot)**:
  A cooperative electrical circuit between a dispatcher and a tower operator.
  Signals connecting tower territory to CTC territory require both entities to grant authority.
  The signal clears only when both contacts close in series.
- **Interlocking Tower**:
  A local control facility where a leverman lines switches and clears signals manually.
  Operates as an autonomous plant using the same vital safety rules as remote CTC.
