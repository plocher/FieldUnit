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

```
  Power Source
  ───[ 1TR Front ]───[ RouteLock Back ]───[ 2ASR Front ]───[ SwitchLock Front ]───( 1WLR )
     (Points Clear)    (Route Free)       (No Approach)    (Local Lock Safe)
```

$$\text{WLR} = (\text{1TR is VACANT}) \text{ AND } (\text{NOT RouteLocked}) \text{ AND } (\text{2ASR is UNLOCKED}) \text{ AND } (\text{HandSwitchLocked})$$

- `1TR` front contact: Proves no train occupies the points (Detector Locking).
- `RouteLock` back contact: Proves no active cleared route reserves this switch.
- `2ASR` front contact: Proves no approaching train has been cleared toward this switch whose timer is still running down.
- `HandSwitchLocked`: Proves the local electric switch lock is locked and secure.
- **Rule**: If any contact opens, `1WLR` drops. Power to the switch motor is cut off and the throw command is rejected.

### 7.2 Switch Correspondence Relay (`KR`) — Are Points Locked in Line?
Before any signal can clear over a switch, FieldUnit evaluates `sw->KR()`:

```
  Power Source
  ───┬───[ Normal Commanded ]───[ 1NWCR Front ]───┬───( 1KR )
     │   (Dispatcher Demand)    (Points Normal)   │
     │                                            │
     └───[ Reverse Commanded ]──[ 1RWCR Front ]───┘
         (Dispatcher Demand)    (Points Reverse)
```

$$\text{KR} = (\text{Normal Commanded AND 1NWCR}) \text{ OR } (\text{Reverse Commanded AND 1RWCR})$$

- Proves that the points physically made contact AND that they agree with what the plant commanded.
- If points gap, bounce, or fail to travel within the motion timeout, `1KR` drops.
- No signal can display a permissive aspect over points when `1KR` is dropped.

### 7.3 Crossover Proving Relay (`3KR`) — Are Both Crossover Switches Aligned?
A crossover connects two main tracks via two physical switch machines (`SW3` and `SW3B`):

```
  Power Source ───[ SW3 KR Front ]───[ SW3B KR Front ]───( 3KR )
                  (MT1 Points)       (MT2 Points)
```

$$\text{3KR} = (\text{SW3 is KR}) \text{ AND } (\text{SW3B is KR})$$

- Proves that *both* the MT1 points and the MT2 points have thrown and locked in the same position.
- If either switch machine is lagging or gapped, `3KR` drops.
- Prevents sending a train across a half-thrown crossover.

### 7.4 Home Relay (`HR`) — Can the Signal Clear?
The Home Relay evaluates whether the immediate plant route is safe for train movement:

```
  Power Source
  ───[ 2HSR Front ]───[ All Route KR Fronts ]───[ All Route TR Fronts ]───[ Opposing ASR Fronts ]───( 2HR )
     (Dispatcher)     (Switches in Line)        (Track Blocks Clear)      (Opposing Held Stop)
```

$$\text{HR} = (\text{2HSR active}) \text{ AND } (\text{All Route Switches in KR}) \text{ AND } (\text{All Route Blocks VACANT}) \text{ AND } (\text{Opposing Signals in ASR})$$

- Dispatcher movement authority is active (`2HSR` picked up).
- All switches along the path report correspondence (`KR` picked up).
- All track circuits on the path are vacant and healthy (`TR` picked up).
- All opposing / conflicting signals are locked at Stop (`ASR` picked up).
- When `2HR` picks up, the entrance signal drops its red aspect and displays at least `APPROACH` or `RESTRICTING`.

### 7.5 Distant Relay (`DR`) — Can the Signal Upgrade to Clear?
The Distant Relay evaluates downstream block spacing (Automatic Block Signaling logic):

```
  Power Source ───[ 2HR Front ]───[ Advance Block TR Front ]───[ Next Signal HR Front ]───( 2DR )
                  (Plant Clear)   (Block Ahead Clear)         (Next Signal Permissive)
```

$$\text{DR} = (\text{2HR picked up}) \text{ AND } (\text{Advance Block VACANT}) \text{ AND } (\text{Next Downstream Signal Permissive})$$

- When the block ahead is clear and the next signal is also permissive, `2DR` picks up.
- Upgrades `APPROACH` (Yellow) to `CLEAR` (Green).
- If the block ahead is occupied, `2DR` drops, holding the aspect at `APPROACH` (Yellow) to warn the engineer to stop at the next signal.

### 7.6 Signal Knockdown and Stick Relay (`HSR`)
The stick circuit ensures a signal protects the train that accepted it:

```
                        Entrance Island TR Front
  Dispatcher Code ──────[       ]───────┬───────────────────────────( 2HSR )
                                        │
         2HSR Front                     │
     ┌──[    ]──────┐                   │
     │              ├───────────────────┘
     │  2FSR Front  │
     └──[    ]──────┘
```

$$\text{HSR}_{\text{next}} = (\text{Dispatcher Code}) \text{ OR } ((\text{2HSR picked up}) \text{ AND } (\text{Entrance Island VACANT})) \text{ OR } (\text{2FSR active})$$

- When the train shunts the entrance track circuit (`TR` drops), the stick path breaks.
- `2HSR` drops immediately to Stop.
- If Fleeting (`2FSR`) is off, `2HSR` remains dropped even after the train leaves the plant.
- The signal cannot clear again until the dispatcher sends a new command.

### 7.7 Engine Return Stick Relay (`ERS`)
The Engine Return circuit allows an engine to reverse direction back onto its train without tripping safety timers:

```
  Forward Exit Move Trigger
  ───[ Route Active Front ]───[ Island TR Back ]───[ Exit Track TR Back ]───┐
                                (On Points)        (Enters Exit)            │
                                                                            ▼
                                                                        [ 2ERS Coil ]
  Hold-In Path (Stick)                                                      ▲
  ───[ 2ERS Front ]───────────[ Exit Track TR Back ]────────────────────────┘
                              (Held while cars remain)
```

$$\text{ERS}_{\text{pickup}} = (\text{Forward Route Active}) \text{ AND } (\text{Island TR is OCCUPIED}) \text{ AND } (\text{Exit Track is OCCUPIED})$$
$$\text{ERS}_{\text{hold}} = (\text{2ERS picked up}) \text{ AND } (\text{Exit Track remains OCCUPIED})$$

- Tracks the forward progression of the locomotive uncoupling and pulling past the points.
- Energizes when the engine occupies the exit track.
- Holds energized while the cars continue to stand on the exit track.
- Bypasses the 5-minute approach locking timer (`ASR`).
- Automatically displays a `RESTRICTING` aspect on the return signal.
- Drops fail-safe to Stop if the cars on the exit track depart.

---

## 8. The 4 Interlocking Locking Regimes

In railroad safety engineering, an interlocking does not use a single generic lock.
It enforces four distinct, hierarchical **Locking Regimes**.
Each regime protects against a specific physical hazard:

```
+=============================================================================+
|                      THE 4 PROTOTYPE LOCKING REGIMES                        |
+----+-------------------+--------------------+-------------------------------+
| #  | Locking Regime    | AAR Relay Circuit  | What It Protects              |
+----+-------------------+--------------------+-------------------------------+
| 1  | **Detector Lock** | `TR` (Island / OS) | Prevents throwing points      |
|    |                   |                    | directly underneath a train.  |
+----+-------------------+--------------------+-------------------------------+
| 2  | **Route Lock**    | `RSR` / `LR`       | Freezes points along a path   |
|    |                   | (Sectional Route)  | once a signal displays Clear. |
+----+-------------------+--------------------+-------------------------------+
| 3  | **Approach Lock** | `ASR`              | Prevents changing points if   |
|    |                   |                    | a train is approaching Clear. |
+----+-------------------+--------------------+-------------------------------+
| 4  | **Time Lock**     | `TER` / `TE`       | Enforces a safety countdown   |
|    |                   |                    | if a Clear signal is revoked. |
+----+-------------------+--------------------+-------------------------------+
```

### 8.1 Detector Locking (Points Protection)
- **The Physical Hazard**: Throwing switch points while a locomotive or car sits on top of them splits the train and causes an immediate derailment.
- **The Vital Circuit**: Power to the switch motor passes through a front contact of the island track relay (`1TR`).
- **The Rule**: As long as wheels shunt `1TR`, the switch is detector-locked.
  The switch cannot move under any command.
  When the last car clears the island block, the detector lock releases immediately.

### 8.2 Route Locking (Path Reservation)
- **The Physical Hazard**: Moving a trailing-point switch ahead of an oncoming train that has already entered the plant.
- **The Vital Circuit**: When a signal displays a permissive aspect, every switch along that path asserts `ROUTE_LOCKED`.
- **The Rule**: Route locking remains active until the train traverses the route.
  In modern plants, switches release progressively as the train clears each individual fouling section (sectional route release).

### 8.3 Approach Locking (The Hazardous Signal Revocation Hazard)
- **The Physical Hazard**: The dispatcher clears a high-speed Green signal.
  A heavy freight train approaches at 50 mph.
  The dispatcher suddenly cancels the route and throws the switch for an opposing movement.
  The heavy train cannot stop in time and derails over the moving points!

To prevent this tragedy, the railroad uses **Approach Locking (`ASR`)**:

```
                              Approach Locking Circuit
  Signal at STOP
  ──────[ Signal Back Contact ]─────────────────────────────────────────┐
        (Closed only when signal displays STOP)                         │
                                                                        ▼
                                                                   [ ASR Coil ]
  Signal Cleared & Approach Track Clear                                 ▲
  ──────[ Signal Front Contact ]──────[ Approach Track TR Front ]───────┘
        (If train is approaching, contact opens; ASR drops and locks plant)
```

#### The Two Scenarios of Approach Locking:
1. **Safe Cancellation (Immediate Release)**:
   The dispatcher cancels a Clear signal.
   The approach track circuit (`1SA`) is **`VACANT`**.
   There is no train approaching.
   The plant releases **immediately**!
   The dispatcher can throw switches right away with zero delay.
2. **Hazardous Cancellation (Timed Release)**:
   The dispatcher cancels a Clear signal.
   The approach track circuit (`1SA`) is **`OCCUPIED`** (a train is bearing down on the signal).
   The approach contact opens.
   `ASR` drops immediately!
   The plant engages **Time Locking (`TER`)**.
   Switches remain frozen until a safety countdown expires (30–60 seconds on model layouts, 3–5 minutes on prototype railroads).
   This guarantees the train has either come to a complete stop or passed safely through the plant before any points can move.

### 8.4 Time Locking (`TER` - Time Element Relay) and CTC Panel Indications
Time locking is the vital countdown timer that runs whenever Approach Locking is tripped.

#### What Happens in the Field:
- The signal drops to Stop immediately (`2NGK = 0, 2SGK = 0`).
- The Time Element relay energizes and begins timing (`2TEK = 1`).
- While the timer counts down, all switches in the cancelled route remain locked.
- Opposing signals remain locked at Stop.
- When the timer reaches zero, `ASR` energizes (picks back up), freeing the switches for new movements.

#### What the Dispatcher Sees on the CTC Machine Panel:
1. **The Timer Lamp Flashes**:
   Above the signal lever, the red Stop lamp illuminates, and the **Time Element lamp (`TE`) flashes or burns solid red**.
   This visual indicator tells the dispatcher: *"Approach locking is in effect. Safety time is running down."*
2. **Out-of-Correspondence (OOC) on the Switches**:
   If the dispatcher attempts to throw Switch 1 while `2TEK` is active:
   - The switch lever on the panel points to `Reverse`.
   - The switch points in the field stay locked in `Normal`.
   - The Normal indication lamp on the panel goes dark, but the Reverse lamp fails to light.
   - The panel lights the **Transit / Out-of-Correspondence (OOC) alarm** (or sounds the panel chime), warning the dispatcher that the switch did not follow the lever.
3. **Timer Expiry and Plant Release**:
   - Once the timer reaches zero in the field, `2TEK` drops to 0.
   - The flashing timer light goes dark.
   - If the dispatcher sends a fresh code button transmission, the switch is now free to move, the points travel, and the correspondence lamp illuminates.

---

## 9. The CodeLine: Asynchronous Truth vs. Remote Procedure Calls

Modern software developers often think in Remote Procedure Calls (RPC) or REST APIs:
`response = client.call("throwSwitch", 1, REVERSE);`
They expect immediate return codes, error messages, or NACKs.

Railroad signaling operates on a completely different, asynchronous foundation.

### 9.1 The Control Packet is an Atomic Plant Transaction
A dispatcher does not send isolated commands to individual devices.
The dispatcher lines all the levers for a plant on their desk:
- Switch 1 lever to Reverse.
- Switch 3 lever to Normal.
- Signal 2 lever to Right.
- Maintainer Call toggle to Off.

The dispatcher then presses the **Code Button**.
The office transmits **one complete, atomic snapshot of desired reality** across the CodeLine:
`[ 1RWS, 3NWS, 2RGS, MC1S ]`

Safety cannot be evaluated on an isolated command.
The field unit evaluates the entire desired plant state vector together as a single atomic transaction.

### 9.2 The Indication Stream Reports Ground Truth
The Control Point in the field does not send "error packets," "NACKs," or conversational replies.
The Control Point simply reports verified physical reality.

- If the dispatcher commands Switch 1 to Reverse while a train sits on the points:
  - The Control Point does not move the motor.
  - The Control Point does not send an error text message.
  - The Control Point continues reporting its true Indication Vector:
    `[ 1NWK=1, 1RWK=0, 1T1K=1 ]` (Points remain in Normal correspondence; detector track is occupied).

### 9.3 Correspondence: The State of Mind
The dispatcher's machine detects non-execution by comparing its commanded intent against the reported indications:

$$\text{Commanded Intent} \stackrel{?}{=} \text{Observed Ground Truth}$$

- When points are in motion, the indication light is **dark** (out of correspondence).
- When points lock in the commanded position, the indication lamp lights up.
- If a switch is locked or obstructed, the indication lamp **remains dark** or lights an transit alarm.
- The dispatcher sees: *"The plant did not move."*

This separation guarantees that the dispatcher's panel can never hallucinate a safe plant state.
The field reports truth; the office observes agreement.
