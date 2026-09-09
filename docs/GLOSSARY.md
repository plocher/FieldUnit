# Railroad Signaling Nomenclature and Glossary

## 1. Introduction
This document defines standard Association of American Railroads (AAR) signaling terms and relay names used in FieldUnit.
FieldUnit uses prototype relay abbreviations to maintain direct alignment with historical railroad circuit plans.

For a complete conceptual walkthrough of how these relays interact inside a trackside bungalow, see **[Inside the Bungalow: An Introduction to AAR Signaling for Model Railroaders](AAR_SIGNALING_PRIMER.md)**.

### 1.1 The Railroad Architecture Taxonomy
In prototype signaling, terms like "Control Point," "Interlocking," "Plant," and "Bungalow" describe different scopes of the system.
FieldUnit uses each term with one precise meaning:

- **Control Point (CP)**:
  The **logical unit of authority**.
  A named geographic location on the railroad (for example, *CP Christopher, MP 81*) where trains receive movement authority from the dispatcher via wayside signals.
- **Interlocking**:
  The **vital safety arrangement**.
  An arrangement of switches, track circuits, and signals interconnected such that their movements must succeed each other in a strictly safe sequence.
  Every Control Point with switches contains an interlocking.
- **Plant**:
  The **physical collection of trackside hardware**.
  The physical steel rails, switch machines, frog points, and signal masts that make up a junction (for example, *"throwing points in the plant"*).
- **Bungalow**:
  The **physical trackside housing / enclosure**.
  The weather-proof steel shed or instrument case at the junction that houses the relays, batteries, chargers, and field computers.
- **Field Unit**:
  The **local compute and I/O controller**.
  The electronic controller inside the bungalow that executes vital safety logic and connects the plant to the CodeLine. (This library!)
- **The Field**:
  Generic term for **everything trackside** outside the central dispatcher's office.
- **Vital Circuit / Command**:
  Safety-critical circuits, commands, and indications that affect train separation, switch points, and movement authority.
  Failures must result in a fail-safe restrictive condition.
- **Non-Vital Circuit / Command**:
  Auxiliary supervisory circuits and commands (such as Maintainer Call `MC`, snow melters, and power alarms).
  Failures cannot cause a collision or derailment.
  Non-vital commands bypass interlocking locks.

---

## 2. Relay Naming Conventions and Contact Notation

AAR relay names use a structured combination of functional letters:
- **Appliance Identifier**: A number or letter prefix (for example, Switch 1, Signal 2, Track Circuit 1T).
- **Core Function**: The primary operational role (Track, Switch, Signal, Stick).
- **Suffix**: `R` indicates Relay. `P` indicates Repeater Relay.

### 2.1 The Two Golden Rules of Railroad Names
1. **"W" Means sWitch**:
   In railroad telegraphy, the letter **`S`** was already reserved for **`Signal`** (and **`Stick`**).
   To prevent deadly errors, signal engineers used the second letter of switch: **`W`**.
   Therefore: `WR` = s**W**itch **R**elay; `WLR` = s**W**itch **L**ock **R**elay; `NW` = **N**ormal s**W**itch.
2. **Switches Are ALWAYS Odd, Signals Are ALWAYS Even**:
   On physical CTC panels, each control point column pairs an **odd switch lever** on top (1, 3, 5, 777, 781) with an **even signal lever** below it (2, 4, 6, 778, 782) over a common code button.
   Even in complex terminal interlockings, this odd/even discipline is strictly maintained.

### 2.2 Reading Relay Contact Diagrams
In railroad circuit plans, relays operate contacts:
- **Front Contact (`[  ]`)**: Neutral contact. Closed when the relay coil is energized (picked up). In software, evaluates to `true`.
- **Back Contact (`[/]`)**: Closed by gravity or spring when the coil is de-energized (dropped). In software, evaluates to `!true`.
- **Series Contacts**: Wires connect in a line. In software, evaluates to logical AND (`&&`).
- **Parallel Contacts**: Wires branch around each other. In software, evaluates to logical OR (`||`).
- **Stick Circuit**: A front contact wired in parallel with the pickup trigger to latch power to its own coil.

---

## 3. Core Vital Relays and State Flags

### 3.1 Track Circuit Relays
- **`TR` (Track Relay)**:
  Directly monitors the track rails.
  It is energized (picked up) when the track block is `VACANT`.
  It is de-energized (dropped) when train wheels and axles shunt the rails (`OCCUPIED`).
  Broken rails or power failures drop the relay to enforce fail-safe operation.

```
          Positive Rail
   +-------------------------------+
   |                               |
[Battery]                      [TR Coil]  (Energized when block is clear)
   |                               |
   +-------------------------------+
          Negative Rail

  When wheels & axles shunt the rails:
          Positive Rail
   +─────────[Wheels]──────────────+
   |             |                 |
[Battery]     (Shunt)          [TR Coil]  (De-energized / Dropped to RED)
   |             |                 |
   +─────────────┴─────────────────+
          Negative Rail
```

In FieldUnit C++:
```cpp
bool isClear = tc.TR(); // true when picked up (vacant and good quality)
```

- **`TP` / `TPR` (Track Repeater Relay)**:
  Repeats the state of the `TR` to provide additional contacts and software filtering.

---

### 3.2 Switch Control and Correspondence Relays
- **`WR` (Switch Control Relay)**:
  Commands physical switch point movement.
  It operates as `NWSR` (Normal Switch Stick Relay) or `RWSR` (Reverse Switch Stick Relay).

- **`NWCR` (Normal Switch Correspondence Relay)**:
  Energizes only when switch points reach and physically lock in the full Normal position.

- **`RWCR` (Reverse Switch Correspondence Relay)**:
  Energizes only when switch points reach and physically lock in the full Reverse position.

- **`KR` (Switch Indication Relay)**:
  Proves that a switch is in full correspondence (`NWCR` or `RWCR` active).
  Interlocking logic requires active `KR` before clearing any signal over the switch.

```
  Power Source
  ──────┬───────[ Point Switch 1N ]───────[ NWCR Coil ]──── (Normal Correspondence)
        │       (Closed only when points
        │        physically lock Normal)
        │
        └───────[ Point Switch 1R ]───────[ RWCR Coil ]──── (Reverse Correspondence)
                (Closed only when points
                 physically lock Reverse)

  KR (Switch Indication Relay Circuit):
  ──────┬───[ NWCR Front ]───┬────────────[ KR Coil ]────── (Proves switch locked)
        │   (Closed if Norm) │
        │                    │
        └───[ RWCR Front ]───┘
            (Closed if Rev)
```

In FieldUnit C++:
```cpp
bool lockedNormal  = sw.NWCR(); // Normal correspondence
bool lockedReverse = sw.RWCR(); // Reverse correspondence
bool inAlignment   = sw.KR();   // In correspondence in either position
```

- **`LR` / `WLR` (Switch Lock Relay)**:
  Enforces electric and mechanical switch locking.
  It de-energizes when a switch is locked by track occupancy (detector lock) or an active route.
  Switch motors can move if and only if `WLR` is energized.

```
  Power Source
  ──────[ 1TR Front ]──────[ Route Unlocked ]──────[ WLR Coil ]──── (Switch Movable)
        (Detector Lock)    (No Active Route)
```

In FieldUnit C++:
```cpp
bool canThrow = sw.WLR(); // true if unlocked
```

---

### 3.3 Signal Control and Authority Relays
- **`HSR` (Home Signal Stick Relay)**:
  Latches the dispatcher movement authority.
  It picks up when the dispatcher sends a directional control command.
  It drops immediately when a train shunts the entrance track circuit.
  It remains dropped until the dispatcher sends a brand new code command.

- **`FSR` (Fleet Stick Relay)**:
  Maintains signal clearance across multiple trains.
  It bypasses the `HSR` stick-down requirement.
  When the first train clears the route, the signal clears automatically for following trains.

```
                     1TR Front
  Dispatcher Code ───[  ]───────┬───────────────────────────( 1HSR )
                                │
         1HSR Front             │
     ┌──────[  ]──────┐         │
     │                ├─────────┘
     │   FSR Front    │
     └──────[  ]──────┘
```

In FieldUnit C++:
```cpp
bool authorityActive = sig.HSR(); // true if authority latched
bool fleetingEnabled = sig.FSR(); // true if fleeting active
```

- **`HR` (Home Relay)**:
  Energizes when route switches correspond and all local plant track circuits are vacant.
  It permits the entrance signal to display at least an `APPROACH` aspect.

- **`DR` (Distant Relay)**:
  Energizes when downstream intermediate blocks and signals are clear.
  It upgrades the signal aspect from `APPROACH` to `CLEAR`.

```
  HR Circuit (Home Relay - Immediate Route Clear):
  ──────[ 1TR Front ]───[ 3TR Front ]───[ 1NWCR Front ]───[ 2ASR Front ]───( 1HR )
        (Plant Clear)   (Points Clear)  (Switch Aligned)  (Opposing Held)

  DR Circuit (Distant Relay - Block Ahead Clear):
  ──────[ 1HR Front ]───[ Next Signal Clear (2HR Front) ]──────────────────( 1DR )
```

In FieldUnit C++:
```cpp
// Evaluated automatically by the Control Table:
// If HR is true and DR is true   ==> Displays CLEAR (Green)
// If HR is true and DR is false  ==> Displays APPROACH (Yellow)
// If HR is false                 ==> Displays STOP (Red)
```

---

### 3.4 Interlocking Safety and Sequence Sticks
- **`ASR` (Approach Stick Relay)**:
  Enforces approach locking on plant entrances.
  It drops when a signal clears to lock all route switches.
  If the dispatcher cancels the signal before a train arrives, `ASR` runs a safety timer.
  Switches remain locked until the timer expires to protect approaching trains.

```
  Signal at STOP
  ──────[ Signal Back Contact ]─────────────────────────────────────────┐
        (Closed only when signal displays STOP)                         │
                                                                        ▼
                                                                   [ ASR Coil ]
  Signal Cleared & Approach Circuit Clear                               ▲
  ──────[ Signal Front Contact ]──────[ Approach Circuit TR Front ]─────┘
        (If train is approaching, contact opens; ASR drops and locks switches)
```

In FieldUnit C++:
```cpp
bool plantFree = sig.ASR(); // true if approach time lock has expired
```

- **`ERS` (Engine Return Stick Relay)**:
  Accommodates switching and direction reversal moves into dark track or spurs.
  It picks up when a train moves forward out of the interlocking into an adjacent block.
  It holds energized while the exit block remains occupied.
  It bypasses the standard five-minute `ASR` approach locking timer.
  It permits an immediate return move into the plant under a `RESTRICTING` aspect.
  It drops when the engine returns and completely vacates the exit block.

```
  Sequential Forward Move Trigger
  ──────[ Island TR Back ]──────[ Exit TR Back ]────────────────────────┐
        (Train on points)       (Train enters exit)                     │
                                                                        ▼
                                                                   [ ERS Coil ]
  Hold-In Path (Stick)                                                  ▲
  ──────[ ERS Front ]───────────[ Exit TR Back ]────────────────────────┘
        (Maintained as long as cars occupy exit track)
```

In FieldUnit C++:
```cpp
// Declared on the route using fluent syntax:
cp.route("SPUR_RETURN")
  .engineReturn(tcAppr /* cars standing */, tcOS /* points */)
  .displays(mast2L, Indication::RESTRICTING);
```

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
