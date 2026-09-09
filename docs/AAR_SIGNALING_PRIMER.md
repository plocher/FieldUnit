# Inside the Bungalow: An Introduction to AAR Signaling for Model Railroaders

## 1. Introduction

A model railroader looks at a junction and sees track, motors, and LEDs.
A prototype signal maintainer looks inside the trackside steel bungalow and sees a collection of standardized **vital relays**.

This document introduces you to the mind of a railroad signal maintainer.
It explains why railroad signaling uses standardized relay names, how those relays work together, and why every switch and signal on the railroad uses the exact same safety circuits.

---

## 2. The Core Rule: Fail-Safe Operation

Prototype signaling has one absolute rule: **Failure must produce the safest possible condition.**

- If a rail breaks $\implies$ The circuit opens $\implies$ The signal shows **Stop** (Red).
- If battery power fails $\implies$ Relays drop by gravity $\implies$ The signal shows **Stop** (Red).
- If a sensor wire disconnects $\implies$ The plant reports **Occupied**.
- If switch points do not fully lock $\implies$ No signal will clear over the points.

In railroad terms, a relay is:
- **Picked Up (Energized)**: Electrical current flows through the coil. Contacts close.
- **Dropped (De-Energized)**: Current stops. Heavy armatures fall by gravity. Contacts open.

---

## 3. The Anatomy of a Switch

On a real railroad, a track switch is not just a motor.
Every switch on the railroad operates through a standardized suite of vital relays:

```
+=============================================================================+
|                      THE 6 RELAYS OF A RAILROAD SWITCH                      |
+-----------+-----------------------------------+-----------------------------+
| Relay     | Full AAR Name                     | What It Does                |
+-----------+-----------------------------------+-----------------------------+
| **`1TR`** | Switch 1 Track Relay              | Detects trains on points    |
| **`1WLR`**| Switch 1 Lock Relay               | Prevents throwing if unsafe |
| **`1WR`** | Switch 1 Control Relay            | Drives the switch motor     |
| **`1NWCR`**| Normal Switch Correspondence     | Verifies points locked Norm |
| **`1RWCR`**| Reverse Switch Correspondence    | Verifies points locked Rev  |
| **`1KR`** | Switch Indication Relay           | Proves points are in line   |
+-----------+-----------------------------------+-----------------------------+
```

### 3.1 Step-by-Step: The Lifecycle of Throwing Switch 1

Here is what happens inside the bungalow when the dispatcher throws Switch 1 from Normal to Reverse:

```
[ Dispatcher sends 1RWS ] ──> [ Check 1WLR (Is switch unlocked?) ]
                                           │
                        ┌──────────────────┴──────────────────┐
                        │                                     │
                 [ 1TR is Occupied ]                   [ 1TR is Vacant ]
                        │                                     │
                        ▼                                     ▼
                Command REJECTED                      1WR drives Motor
              (Points cannot move)                            │
                                                              ▼
                                                     Points travel (In-Flight)
                                                    (1NWCR drops, 1KR drops)
                                                              │
                                                              ▼
                                                     Points lock in Reverse
                                                     (1RWCR picks up, 1KR picks up)
                                                              │
                                                              ▼
                                                     1RWK reports to Dispatcher
```

1. **Dispatcher sends command (`1RWS`)**:
   The dispatcher turns lever 1 to Reverse and presses the code button.
2. **Lock check (`1WLR`)**:
   Power to the motor circuit must pass through a front contact of `1WLR` (Switch Lock Relay).
   If a train occupies the points (`1TR` dropped) or an active route is cleared, `1WLR` is dropped.
   Power cannot reach the motor. The command is rejected.
3. **Motor actuation (`1WR`)**:
   If `1WLR` is picked up, `1WR` energizes and drives the switch machine toward Reverse.
4. **Out of Correspondence**:
   As the points leave the stock rail, the physical contact switch opens.
   `1NWCR` drops immediately.
   `1KR` drops immediately.
   The indication light on the dispatcher's desk goes dark.
5. **Correspondence Verified (`1RWCR` and `1KR`)**:
   The points make contact on the opposite side.
   A heavy locking bar slides into position.
   Physical circuit controller contacts close.
   `1RWCR` picks up.
   `1KR` picks up.
   The dispatcher's panel lights the Reverse indication lamp (`1RWK`).
   Signals can now clear over the switch.

---

## 4. The Anatomy of a Signal

Just like switches, wayside signals do not light lamps directly from dispatcher commands.
Every signal operates through a standardized chain of vital relays:

```
+=============================================================================+
|                      THE 5 RELAYS OF A RAILROAD SIGNAL                      |
+-----------+-----------------------------------+-----------------------------+
| Relay     | Full AAR Name                     | What It Does                |
+-----------+-----------------------------------+-----------------------------+
| **`2HSR`**| Home Signal Stick Relay           | Latches dispatcher authority|
| **`2HR`** | Home Signal Relay                 | Verifies plant route clear  |
| **`2DR`** | Distant Signal Relay              | Verifies block ahead clear  |
| **`2ASR`**| Approach Stick Relay              | Enforces approach locking   |
| **`2FSR`**| Fleet Stick Relay                 | Auto re-clears for trains   |
+-----------+-----------------------------------+-----------------------------+
```

### 4.1 Step-by-Step: The Lifecycle of Clearing Signal 2

Here is how Signal 2 clears from Stop (Red) to Clear (Green):

```
                       HR Circuit (Home Relay)
 Power ──[ 2HSR Front ]──[ 1KR Front ]──[ 1TR Front ]──[ Opposing ASR ]──( 2HR )
          (Dispatcher)    (Switches)     (Track Clear)   (Opposing Stop)
                                                                │
                                                                ▼
                                                       Signal displays at
                                                       least APPROACH (Yellow)
                                                                │
                       DR Circuit (Distant Relay)               ▼
 Power ──[ 2HR Front ]──[ Next Signal Clear (4HR) ]──────────────( 2DR )
                                                                │
                                                                ▼
                                                       Signal upgrades to
                                                       CLEAR (Green over Red)
```

1. **Dispatcher Authority (`2HSR`)**:
   The dispatcher codes Signal 2 Northward (`2NGS`).
   `2HSR` picks up and sticks energized through its own contact.
2. **Plant Verification (`2HR`)**:
   Before the signal lights green or yellow, current must flow through:
   - `1KR` front contact (proving all route switches are locked in correspondence).
   - `1TR` front contact (proving the route track circuits are unoccupied).
   - Opposing `ASR` front contact (proving conflicting signals are held at Stop).
   When all contacts close, `2HR` picks up.
   The signal drops its red lamp and displays at least `APPROACH` or `RESTRICTING`.
3. **Downstream Block Verification (`2DR`)**:
   If the next signal down the mainline is also displaying a clear aspect, `2DR` picks up.
   This upgrades the signal from `APPROACH` (Yellow) to `CLEAR` (Green).
4. **Signal Knockdown**:
   When the locomotive passes the signal and shunts the entrance track circuit (`1TR` drops):
   - `2HR` immediately de-energizes.
   - The signal immediately drops to `STOP` (Red) behind the train.
   - The stick circuit on `2HSR` breaks.
   - Even after the train leaves the plant, the signal **remains at Stop**.
   - It will not clear again until the dispatcher sends a brand-new code command.

---

## 5. Algorithmic Composition of Names

Notice the naming pattern.
You do not need to memorize hundreds of random acronyms.
AAR relay names are built from four basic elements:

$$\text{Name} = [\text{Appliance Number}] + [\text{Direction / Function}] + [\text{Operational Role}] + [\text{Suffix}]$$

### The "AHA!" Secrets of Railroad Nomenclature

#### Secret 1: "W" means sWitch!
Newcomers often ask: *Why is a switch named with the letter "W"?*
Because in railroad telegraphy and signaling, the letter **`S`** was already reserved for **`Signal`** (and **`Stick`**)!
To avoid deadly confusion, signal engineers used the second letter of switch: **`W`**.
Once you know that `W` = Switch, the acronyms become obvious:
- `WR` = s**W**itch **R**elay.
- `WLR` = s**W**itch **L**ock **R**elay.
- `NW` = **N**ormal s**W**itch.
- `RW` = **R**everse s**W**itch.
- `NWCR` = **N**ormal s**W**itch **C**orrespondence **R**elay.

#### Secret 2: Switches are ALWAYS Odd, Signals are ALWAYS Even!
Look at any CTC panel or interlocking diagram:
- **Switch numbers are ALWAYS ODD**: Switch 1, Switch 3, Switch 5 (or 777, 781).
- **Signal numbers are ALWAYS EVEN**: Signal 2, Signal 4, Signal 6 (or 778, 782).

On a physical CTC machine panel, each control point column pairs an odd switch lever on top with an even signal lever below it over a common code button.
Even when a plant has complex crossovers and ladders, this odd/even discipline is strictly maintained.

---

### Examples for Switch 1 (`"1"`):
- `1` + `NW` + `CR` $\implies$ **`1NWCR`**: Switch 1, Normal, Correspondence Relay.
- `1` + `RW` + `CR` $\implies$ **`1RWCR`**: Switch 1, Reverse, Correspondence Relay.
- `1` + `W`  + `LR` $\implies$ **`1WLR`**: Switch 1, Switch Lock Relay.
- `1` + `NW` + `S`  $\implies$ **`1NWS`**: Switch 1, Normal, Send (Control command).
- `1` + `NW` + `K`  $\implies$ **`1NWK`**: Switch 1, Normal, Kontrol (Indication report).

### Examples for Signal 2 (`"2"`):
- `2` + `H`  + `SR` $\implies$ **`2HSR`**: Signal 2, Home, Stick Relay.
- `2` + `A`  + `SR` $\implies$ **`2ASR`**: Signal 2, Approach, Stick Relay.
- `2` + `NG` + `S`  $\implies$ **`2NGS`**: Signal 2, Northward, Send (Control command).
- `2` + `SG` + `K`  $\implies$ **`2SGK`**: Signal 2, Southward, Kontrol (Indication report).
- `2` + `TE` + `K`  $\implies$ **`2TEK`**: Signal 2, Time Element, Kontrol (Time lock running).

---

## 6. How FieldUnit Translates Relays to Software

FieldUnit encapsulates these vital relay circuits under the hood.
You do not need to be an expert in AAR wiring diagrams to build a working plant; FieldUnit evaluates these contracts automatically whenever you write a simple route.
However, if you are a signal historian or want to verify the exact circuit logic, FieldUnit exposes every one of these vital relay states as direct query methods:

```cpp
// Check if Track Relay is picked up (unoccupied and good quality)
if (tc1T1->TR()) { ... }

// Check if Switch 1 is in Normal correspondence
if (sw1->NWCR()) { ... }

// Check if Switch 1 is free to move (Lock relay picked up)
if (sw1->WLR()) { ... }

// Check if Signal 2 authority is actively latched
if (sig2->HSR()) { ... }

// Check if approach locking is clear (ASR picked up)
if (sig2->ASR()) { ... }
```

By using the authentic names, your C++ code matches historical railroad circuit plans directly.
You can read a real railroad plan from 1950, find the contact chain, and verify that your FieldUnit plant behaves with identical safety.

---

## 7. The Complete Interlocking Logic Chains

This section enumerates the exact vital logic equations FieldUnit evaluates during each plant cycle.
Knowledgeable signal modelers can use this section to verify the rigor of the engine, while newcomers can see the complete safety checks operating behind every route.

### 7.1 Switch Lock Relay (`WLR`) — Can the Switch Move?
Before a switch motor can energize, FieldUnit evaluates `sw->WLR()`:

$$\text{WLR} = \text{TR}_{\text{island}} \land \neg \text{RouteLocked} \land \text{ASR}_{\text{approaching signals}} \land \text{HandSwitchLocked}$$

- `TR` front contact: Proves no train occupies the points (Detector Locking).
- `RouteLocked` back contact: Proves no active cleared route reserves this switch.
- `ASR` front contacts: Proves no approaching train has been cleared toward this switch whose timer is still running down.
- `HandSwitchLocked`: Proves the local electric switch lock is locked and secure.
- **Rule**: If any condition fails, $\text{WLR} == \text{false}$. Power to the switch motor is cut off and the throw command is rejected.

### 7.2 Switch Correspondence Relay (`KR`) — Are Points Locked in Line?
Before any signal can clear over a switch, FieldUnit evaluates `sw->KR()`:

$$\text{KR} = (\text{NormalCommanded} \land \text{NWCR}) \lor (\text{ReverseCommanded} \land \text{RWCR})$$

- Proves that the points physically made contact AND that they agree with what the plant commanded.
- If points gap, bounce, or fail to travel within the motion timeout, $\text{KR} == \text{false}$.
- No signal can display a permissive aspect over points when $\text{KR} == \text{false}$.

### 7.3 Crossover Proving Relay (`3KR`) — Are Both Crossover Switches Aligned?
A crossover connects two main tracks via two physical switch machines (`SW3` and `SW3B`):

$$\text{3KR} = \text{KR}_{\text{Switch 3}} \land \text{KR}_{\text{Switch 3B}}$$

- Proves that *both* the MT1 points and the MT2 points have thrown and locked in the same position.
- If either switch machine is lagging or gapped, $\text{3KR} == \text{false}$.
- Prevents sending a train across a half-thrown crossover.

### 7.4 Home Relay (`HR`) — Can the Signal Clear?
The Home Relay evaluates whether the immediate plant route is safe for train movement:

$$\text{HR} = \text{HSR} \land \bigwedge \text{KR}_{\text{route switches}} \land \bigwedge \text{TR}_{\text{route blocks}} \land \bigwedge \text{ASR}_{\text{opposing signals}}$$

- Dispatcher movement authority is active (`HSR` picked up).
- All switches along the path report correspondence (`KR` picked up).
- All track circuits on the path are vacant and healthy (`TR` picked up).
- All opposing / conflicting signals are locked at Stop (`ASR` picked up).
- When $\text{HR} == \text{true}$, the entrance signal drops its red aspect and displays at least `APPROACH` or `RESTRICTING`.

### 7.5 Distant Relay (`DR`) — Can the Signal Upgrade to Clear?
The Distant Relay evaluates downstream block spacing (Automatic Block Signaling logic):

$$\text{DR} = \text{HR} \land \text{TR}_{\text{advance block ahead}} \land \text{HR}_{\text{next downstream signal}}$$

- When the block ahead is clear and the next signal is also permissive, $\text{DR} == \text{true}$.
- Upgrades `APPROACH` (Yellow) to `CLEAR` (Green).
- If the block ahead is occupied, $\text{DR} == \text{false}$, holding the aspect at `APPROACH` (Yellow) to warn the engineer to stop at the next signal.

### 7.6 Signal Knockdown and Stick Relay (`HSR`)
The stick circuit ensures a signal protects the train that accepted it:

$$\text{HSR}_{\text{next}} = \text{DispatcherCommand} \lor (\text{HSR} \land \text{TR}_{\text{entrance island}}) \lor \text{FSR}$$

- When the train shunts the entrance track circuit (`TR` drops), the stick path breaks.
- `HSR` drops immediately to Stop.
- If Fleeting (`FSR`) is off, `HSR` remains dropped even after the train leaves the plant.
- The signal cannot clear again until the dispatcher sends a new command.

### 7.7 Engine Return Stick Relay (`ERS`)
The Engine Return circuit allows an engine to reverse direction back onto its train without tripping safety timers:

$$\text{ERS}_{\text{pickup}} = \text{ForwardRouteActive} \land \text{TR}_{\text{island}} \text{ (dropped)} \land \text{TR}_{\text{exit track}} \text{ (dropped)}$$
$$\text{ERS}_{\text{hold}} = \text{ERS} \land (\text{TR}_{\text{exit track}} == \text{OCCUPIED})$$

- Tracks the forward progression of the locomotive uncoupling and pulling past the points.
- Energizes when the engine occupies the exit track.
- Holds energized while the cars continue to stand on the exit track.
- Bypasses the 5-minute approach locking timer (`ASR`).
- Automatically displays a `RESTRICTING` aspect on the return signal.
- Drops fail-safe to Stop if the cars on the exit track depart.
