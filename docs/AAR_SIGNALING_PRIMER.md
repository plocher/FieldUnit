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

In FieldUnit, you do not solder physical relays or write complex circuit diagrams.
FieldUnit provides these exact AAR relay contracts directly as member methods on its appliances:

```cpp
// Check if Track Relay is picked up (unoccupied and good quality)
if (tc1T1->TR()) { ... }

// Check if Switch 1 is in Normal correspondence
if (sw1->NWCR()) { ... }

// Check if Switch 1 is free to move (Lock relay picked up)
if (sw1->WLR()) { ... }

// Check if Signal 2 authority is actively latched
if (sig2->HSR()) { ... }

// Check if approach locking is clear
if (sig2->ASR()) { ... }
```

By using the authentic names, your C++ code matches historical railroad circuit plans directly.
You can read a real railroad plan from 1950, find the contact chain, and verify that your FieldUnit plant behaves with identical safety.
