# Inside the Bungalow: An Introduction to AAR Signaling for Model Railroaders

---

## Act I: The Threshold

### 1. The View from the Fascia versus the View from the Bungalow
A model railroader stands at the layout fascia and sees track, motors, and LEDs.
When a train approaches, the modeler flips a toggle switch to throw the points and turns a knob to clear a signal.

A prototype railroad signal maintainer stands in a completely different world.
The maintainer steps inside a weather-proof steel bungalow sitting in the gravel next to the junction.
Inside, there are no toggle switches or track power packs.
The maintainer sees rows of glass-cased **vital relays**, banks of storage batteries, heavy wire terminals, and an electronic field controller.

To the maintainer, a junction is not a passive piece of track.
It is an autonomous safety machine that protects human life against error.
This document introduces you to the mind of a railroad signal maintainer.
It explains the standardized relay circuits, operational rules, and communication protocols that keep trains safe.

---

### 2. The Prime Directive: Fail-Safe Operation
Prototype railroad signaling operates under one absolute rule: **Any failure must produce the most restrictive condition.**

On a railroad:
- If a rail breaks $\implies$ The circuit opens $\implies$ The signal drops to **Stop** (Red).
- If battery power fails $\implies$ Armatures drop by gravity $\implies$ The signal drops to **Stop** (Red).
- If a sensor wire disconnects $\implies$ The plant reports **Occupied**.
- If switch points fail to travel and lock $\implies$ No signal will clear over the points.

In railroad terms:
- **Picked Up (Energized)**: Current flows through the relay coil. Front contacts close.
- **Dropped (De-Energized)**: Current stops. Heavy armatures fall by gravity. Front contacts open; back contacts close.

---

## Act II: The Citizens of the Bungalow (The Appliance Cast)

Every device at a control point is an **appliance** with an assigned role and a suite of vital relays.

### 3. The Switch: More Than a Motor
A railroad track switch is not just a motor.
It is a heavy electro-mechanical mechanism (such as a US&S M-23 or GRS Model 5) equipped with a point detector circuit controller and an electric lock.

Every switch operates through a standardized suite of vital relays:

```
+=============================================================================+
|                      THE 6 RELAYS OF A RAILROAD SWITCH                      |
+-----------+-----------------------------------+-----------------------------+
| Relay     | Full AAR Name                     | Operational Role            |
+-----------+-----------------------------------+-----------------------------+
| **`1TR`** | Switch 1 Track Relay              | Detects trains on points    |
| **`1WLR`**| Switch 1 Lock Relay               | Cuts motor power if locked  |
| **`1WR`** | Switch 1 Control Relay            | Drives the switch motor     |
| **`1NWCR`**| Normal Switch Correspondence     | Proves points locked Normal |
| **`1RWCR`**| Reverse Switch Correspondence    | Proves points locked Reverse|
| **`1KR`** | Switch Indication Relay           | Proves points in alignment  |
+-----------+-----------------------------------+-----------------------------+
```

#### Secret 1: "W" Means sWitch!
Newcomers often wonder: *Why is a switch named with the letter "W"?*
In railroad telegraphy, the letter **`S`** was already reserved for **`Signal`** (and **`Stick`**).
To prevent deadly confusion, signal engineers selected the second letter of switch: **`W`**.
- `WR` = s**W**itch **R**elay.
- `WLR` = s**W**itch **L**ock **R**elay.
- `NW` = **N**ormal s**W**itch.
- `RW` = **R**everse s**W**itch.

#### Secret 2: Switches Are Always Odd, Signals Are Always Even!
On physical CTC panels, each control point column pairs an **odd switch lever** on top with an **even signal lever** below it over a common code button.
- Switches: `1`, `3`, `5`, `7` (or milepost switches `777`, `781`).
- Signals: `2`, `4`, `6`, `8` (or milepost signals `778`, `782`).

#### Step-by-Step: Throwing Switch 1
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

---

### 4. The Signal: More Than a Lamp
Wayside signals do not illuminate lamps directly from dispatcher commands.
Every signal operates through a standardized chain of vital relays:

```
+=============================================================================+
|                      THE 5 RELAYS OF A RAILROAD SIGNAL                      |
+-----------+-----------------------------------+-----------------------------+
| Relay     | Full AAR Name                     | Operational Role            |
+-----------+-----------------------------------+-----------------------------+
| **`2HSR`**| Home Signal Stick Relay           | Latches dispatcher authority|
| **`2HR`** | Home Signal Relay                 | Verifies plant route clear  |
| **`2DR`** | Distant Signal Relay              | Verifies block ahead clear  |
| **`2ASR`**| Approach Stick Relay              | Enforces approach locking   |
| **`2FSR`**| Fleet Stick Relay                 | Auto re-clears for trains   |
+-----------+-----------------------------------+-----------------------------+
```

#### Aspect versus Indication
Railroad rulebooks make an exact distinction between what an engineer sees and what it means:
- **Aspect**: What the signal *looks like* (e.g. Green over Red, Red over Lunar, Flashing Yellow).
- **Indication**: What the rulebook *instructs the crew to do* (e.g. `CLEAR`, `APPROACH`, `DIVERGING_CLEAR`, `RESTRICTING`, `STOP`).

#### Step-by-Step: Clearing Signal 2
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

---

### 5. The Extended Appliance Cast
Beyond standard switches and signals, an authentic interlocking plant includes specialized appliances:

#### 1. Crossovers (`Crossover`)
A crossover connects two parallel main tracks using two paired switches (`SW3` and `SW3B`).
In prototype signaling, both switch machines are commanded in unison.
Both points must lock in correspondence before any route clears.

#### 2. Derails (`Derail`)
A derail sits on a siding or industrial spur.
It physically derails a rolling car before it can foul the mainline.
Power-operated derails are slaved inversely to the main switch:
- When the main switch is Normal $\implies$ Derail is ON RAIL (derailing position).
- When the main switch is Reverse $\implies$ Derail is OFF RAIL (clear position).

#### 3. Electric Switch Locks (`ESL` / `WL`)
In CTC territory, hand-throw switches feature an electric padlock housing.
The train crew cannot lift the hand-throw lever until the dispatcher sends an unlock command (`WLS`), releasing the internal solenoid latch.

#### 4. Axle Counters and Optical Sensors
Where track circuits suffer from poor ballast resistance or unpowered model train wheelsets:
- **Axle Counters** count wheels in and out of a block.
- **Optical Sensors (`d`)** detect physical car bodies at clearance and fouling points.

#### 5. Trackside Defect Detectors
- **Hot Box Detectors (HBD)**: Sense overheated wheel bearings via infrared.
- **Dragging Equipment Detectors (DED)**: Impact plates that detect dragging chains or air hoses.

#### 6. Switch Heaters (`SNOW`)
Gas burners or electric calrod elements along the rails that melt snow and ice to keep points moving during winter storms.

#### 7. Bungalow Utilities
- **Maintainer Call (`MC`)**: Outside white lamp summoning personnel.
- **Power Off Relay (`POR`)**: Senses commercial AC utility failure; warns dispatcher that the plant is running on emergency batteries.
- **Intrusion Sensor (`DOOR`)**: Detects unauthorized entry into the bungalow.

---

### 6. Algorithmic Composition of Names
AAR relay names follow a standard formula:

$$\text{Name} = [\text{Appliance Number}] + [\text{Direction / Function}] + [\text{Operational Role}] + [\text{Suffix}]$$

- `1` + `NW` + `CR` $\implies$ **`1NWCR`**: Switch 1, Normal, Correspondence Relay.
- `1` + `RW` + `CR` $\implies$ **`1RWCR`**: Switch 1, Reverse, Correspondence Relay.
- `1` + `W`  + `LR` $\implies$ **`1WLR`**: Switch 1, Switch Lock Relay.
- `2` + `H`  + `SR` $\implies$ **`2HSR`**: Signal 2, Home, Stick Relay.
- `2` + `A`  + `SR` $\implies$ **`2ASR`**: Signal 2, Approach, Stick Relay.

---

## Act III: The Laws of Safety (The Interlocking Plant)

---

### 7. The 4 Interlocking Locking Regimes
An interlocking enforces four distinct locking regimes to protect against physical hazards:

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

#### 7.1 Detector Locking (Points Protection)
- **The Hazard**: Throwing points while a car sits on them splits the train and causes a derailment.
- **The Rule**: Power to the switch motor passes through `1TR` front contacts. As long as wheels shunt `1TR`, the switch is detector-locked and cannot move.

#### 7.2 Route Locking (Path Reservation)
- **The Hazard**: Throwing a trailing-point switch ahead of a train that has entered the plant.
- **The Rule**: When a signal clears, every switch on the path asserts `ROUTE_LOCKED`. Switches release progressively as the train clears each switch (sectional route release).

#### 7.3 Approach Locking (Signal Revocation Protection)
- **The Hazard**: The dispatcher clears a Green signal. A train approaches at speed. The dispatcher cancels the signal and lines an opposing switch. The train cannot stop in time and derails.
- **Safe Cancellation (Immediate Release)**: If approach track `1SA` is `VACANT`, there is no train. The plant releases immediately with zero delay.
- **Hazardous Cancellation (Timed Release)**: If approach track `1SA` is `OCCUPIED`, `ASR` drops immediately. The plant engages **Time Locking (`TER`)**. Points stay frozen until a countdown timer expires (30-60s model, 3-5m proto).

#### 7.4 Time Locking (`TER`) and CTC Panel Indications
While the time lock timer counts down:
- The signal drops to Stop immediately (`2NGK = 0, 2SGK = 0`).
- The Time Element indication asserts (`2TEK = 1`).
- On the dispatcher's panel, the red **Time Element lamp (`TE`) flashes**.
- If the dispatcher tries to throw Switch 1 while `2TEK` is active, the switch does not move, the correspondence lamps go dark, and the panel sounds the **Out-of-Correspondence (OOC) / Transit Alarm**.
- When the timer reaches zero, `2TEK` drops, the alarm silences, and points are freed.

---

### 8. The Complete Interlocking Logic Chains

FieldUnit evaluates these exact vital logic chains during each plant cycle:

#### 8.1 Switch Lock Relay (`WLR`) — Can the Switch Move?
```
  Power ───[ 1TR Front ]───[ RouteLock Back ]───[ 2ASR Front ]───[ SwitchLock Front ]───( 1WLR )
```
`WLR = (1TR is VACANT) AND (NOT RouteLocked) AND (2ASR is UNLOCKED) AND (HandSwitchLocked)`

#### 8.2 Switch Correspondence Relay (`KR`) — Are Points Locked in Line?
```
  Power ───┬───[ Normal Commanded ]───[ 1NWCR Front ]───┬───( 1KR )
           │                                            │
           └───[ Reverse Commanded ]──[ 1RWCR Front ]───┘
```
`KR = (Normal Commanded AND 1NWCR) OR (Reverse Commanded AND 1RWCR)`

#### 8.3 Crossover Proving Relay (`3KR`) — Are Both Crossover Switches Aligned?
```
  Power ───[ SW3 KR Front ]───[ SW3B KR Front ]───( 3KR )
```
`3KR = (SW3 is KR) AND (SW3B is KR)`

#### 8.4 Home Relay (`HR`) — Can the Signal Clear?
```
  Power ───[ 2HSR Front ]───[ Route KR Fronts ]───[ Route TR Fronts ]───[ Opposing ASR Fronts ]───( 2HR )
```
`HR = (2HSR active) AND (All Route Switches in KR) AND (All Route Blocks VACANT) AND (Opposing Signals in ASR)`

#### 8.5 Distant Relay (`DR`) — Can the Signal Upgrade to Clear?
```
  Power ───[ 2HR Front ]───[ Advance Block TR Front ]───[ Next Signal HR Front ]───( 2DR )
```
`DR = (2HR picked up) AND (Advance Block VACANT) AND (Next Downstream Signal Permissive)`

#### 8.6 Signal Knockdown and Stick Relay (`HSR`)
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
`HSR_next = (Dispatcher Code) OR ((2HSR picked up) AND (Entrance Island VACANT)) OR (2FSR active)`

#### 8.7 Engine Return Stick Relay (`ERS`)
```
  Forward Exit Move Trigger
  ───[ Route Active Front ]───[ Island TR Back ]───[ Exit Track TR Back ]───┐
                                                                            ▼
                                                                        [ 2ERS Coil ]
  Hold-In Path (Stick)                                                      ▲
  ───[ 2ERS Front ]───────────[ Exit Track TR Back ]────────────────────────┘
```
`ERS_pickup = (Forward Route Active) AND (Island TR is OCCUPIED) AND (Exit Track is OCCUPIED)`
`ERS_hold   = (2ERS picked up) AND (Exit Track remains OCCUPIED)`

---

## Act IV: The Outside World (Communication Across Distance)

---

### 9. The CodeLine: Asynchronous Truth versus Remote Procedure Calls

Modern software developers often think in Remote Procedure Calls (RPC):
`response = client.call("throwSwitch", 1, REVERSE);`
They expect immediate return codes, error messages, or NACKs.

Railroad signaling operates on a completely different, asynchronous foundation.

#### 9.1 The Control Packet is an Atomic Plant Transaction
A dispatcher does not send isolated commands to individual devices.
The dispatcher lines all levers for a plant on their desk:
- Switch 1 lever to Normal.
- Switch 3 lever to Reverse.
- Switch 5 lever to Normal.
- Signal 2 lever to Right.
- Maintainer Call toggle to Off.

The dispatcher presses the **Code Button**.
The office transmits **one complete, atomic snapshot of desired reality** across the CodeLine:
- **Unparenthesized token (`3RWS`)**: The function is **asserted** (active command to throw or clear).
- **Parenthesized token (`(1NWS)`)**: The function is **unasserted** (confirmation that this device should not change).

```
[ (1NWS), (1RWS), (3NWS), 3RWS, (5NWS), (5RWS), (2SGS), (2NGS), (2HS), (MC1S) ]
```

Why must unasserted items be present?
- `(1NWS)` explicitly confirms: *"Leave Switch 1 alone in its existing position."*
- If tokens were missing, the field unit could not tell whether Switch 1 was supposed to be untouched or if the packet was truncated by line noise.
- **The Truncation Rule**: If any expected token is missing or truncated, the entire vital control transaction is rejected. Zero switches move and zero signals clear.

#### 9.2 The Indication Stream Reports Ground Truth
The Control Point in the field does not send "error packets," "NACKs," or conversational replies.
The Control Point simply reports verified physical reality.

- If the dispatcher commands Switch 1 to Reverse while a train sits on the points:
  - The Control Point does not move the motor.
  - The Control Point does not send an error text message.
  - The Control Point continues reporting its true Indication Vector:
    `[ 1NWK=1, 1RWK=0, 1T1K=1 ]` (Points remain in Normal correspondence; detector track is occupied).

#### 9.3 Correspondence: The State of Mind
The dispatcher's machine detects non-execution by comparing its commanded intent against the reported indications:

$$\text{Commanded Intent} \stackrel{?}{=} \text{Observed Ground Truth}$$

- When points are in motion, the indication light is **dark** (out of correspondence).
- When points lock in the commanded position, the indication lamp lights up.
- If a switch is locked or obstructed, the indication lamp **remains dark** or sounds a transit alarm.
- The dispatcher sees: *"The plant did not move."*

---

### 10. The CodeLine Taxonomy: Controls versus Indications

Every token on the CodeLine falls into one of two safety classes:
- **Vital (Safety-Critical)**: Affects train separation, switch points, and movement authority. Requires strict validation, correspondence checking, and fail-safe defaults.
- **Non-Vital (Operational / Supervisory / Environmental)**: Auxiliary commands and alerts (Maintainer Call, snow melters, power loss). Cannot cause a collision or derailment. Bypasses interlocking locks.

#### 10.1 CodeLine Control Taxonomy (Office to Field)

| Domain | Control Mnemonic | Prototype Function | Safety Class | Precondition / Safety Checks |
|---|---|---|---|---|
| **Switch** | `1NWS` | Command Switch 1 Normal | **VITAL** | Must satisfy `WLR` (detector lock `1TR` vacant, route free, no active approach time-lock). |
| **Switch** | `1RWS` | Command Switch 1 Reverse | **VITAL** | Must satisfy `WLR` (detector lock `1TR` vacant, route free, no active approach time-lock). |
| **Signal** | `2NGS` / `2L` | Clear Signal 2 North/Left | **VITAL** | Must satisfy `HR` (switches in `KR`, blocks `TR` clear, opposing held in `ASR`). |
| **Signal** | `2SGS` / `2R` | Clear Signal 2 South/Right | **VITAL** | Must satisfy `HR` (switches in `KR`, blocks `TR` clear, opposing held in `ASR`). |
| **Signal** | `2HS` | Force Signal 2 Stop / Cancel | **VITAL** | Always accepted; trips `ASR` approach locking if train is approaching. |
| **Electric Lock**| `7WLS` | Release Electric Switch Lock 7 | **VITAL** | Mainline signals over switch 7 must be at Stop and approach timer expired. |
| **Fleeting** | `2FS` | Toggle Fleeting on Signal 2 | **NON-VITAL** | Informational; conditions `FSR` stick bypass. |
| **Call-On** | `2COS` | Authorize Low-Speed Call-On | **VITAL** | Requires explicit dispatcher button; allows `RESTRICTING` into occupied block. |
| **Maintainer** | `MC1S` | Maintainer Call Lamp ON/OFF | **NON-VITAL** | Always accepted immediately; zero safety interlocks. |
| **Auxiliary** | `SNOWS` | Switch Heater / Snow Melter | **NON-VITAL** | Always accepted; environmental auxiliary. |
| **Auxiliary** | `GENS` | Backup Generator Start/Stop | **NON-VITAL** | Always accepted; environmental auxiliary. |

#### 10.2 CodeLine Indication Taxonomy (Field to Office)

| Domain | Indication Mnemonic | Prototype Meaning | Safety Class | Source of Truth |
|---|---|---|---|---|
| **Switch** | `1NWK` | Switch 1 Locked in Normal | **VITAL** | `1NWCR` circuit controller contact closed. |
| **Switch** | `1RWK` | Switch 1 Locked in Reverse | **VITAL** | `1RWCR` circuit controller contact closed. |
| **Switch** | `1OOK` / Transit | Switch 1 Out of Correspondence | **NON-VITAL** | Derived: both `NWK` and `RWK` are 0 (in motion or failed). |
| **Track** | `1T1K` | Track Circuit 1T1 Occupied | **VITAL** | `1TR` track relay dropped (wheels shunting rails). |
| **Track** | `1SAK` | Approach Block 1SA Occupied | **VITAL** | `1SATR` track relay dropped. |
| **Signal** | `2NGK` | Signal 2 Northward Permissive | **VITAL** | Signal lamp current sensor verifies green/yellow lit. |
| **Signal** | `2SGK` | Signal 2 Southward Permissive | **VITAL** | Signal lamp current sensor verifies green/yellow lit. |
| **Signal** | `2TEK` | Signal 2 Time Lock Running | **NON-VITAL** | `2TER` timer relay picked up (drives panel blinking light). |
| **Electric Lock**| `7WLK` | Electric Lock 7 Unlocked | **VITAL** | Proves lock solenoid is energized and points can move. |
| **Maintainer** | `MC1K` | Maintainer Call Lamp Lit | **NON-VITAL** | Current sensor on maintainer call lamp fixture. |
| **Power** | `PORK` | Power Off (Commercial AC Loss) | **NON-VITAL** | Commercial AC power loss relay (`POR` dropped, running on battery). |
| **Security** | `DOORK` | Bungalow Door Opened | **NON-VITAL** | Door intrusion contact switch. |

#### 10.3 The Non-Vital Execution Boundary Rule
When a `ControlTransaction` arrives at a FieldUnit:
1. **Vital commands** pass through the interlocking safety checks. If a switch is locked, its movement command is rejected.
2. **Non-Vital commands** (such as Maintainer Call `MC1S`) execute immediately regardless of interlocking lock state.
   This guarantees that a dispatcher can always summon a maintainer even if a derailment or broken rail has locked down the vital interlocking logic.

---

## Act V: The Return (Building Your Own Plant)

---

### 11. Translating Relay Logic to Modern C++ (FieldUnit)

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
- Switch 1 lever to Normal (unchanged).
- Switch 3 lever to Reverse (commanded to throw).
- Switch 5 lever to Normal (unchanged).
- Signal 2 lever to Right (unchanged / Stop).
- Maintainer Call toggle to Off.

The dispatcher then presses the **Code Button**.
The office transmits **one complete, atomic snapshot of desired reality** across the CodeLine.
In railroad notation:
- **Unparenthesized token (`3RWS`)**: The function is **asserted** (active command to throw or clear).
- **Parenthesized token (`(1NWS)`)**: The function is **unasserted** (confirmation that this device should not change).

If the dispatcher wants to throw *only* Switch 3 to Reverse, the office does not send a lone `3RWS` command.
It transmits the **entire plant vector containing both asserted and unasserted states**:

```
[ (1NWS), (1RWS), (3NWS), 3RWS, (5NWS), (5RWS), (2SGS), (2NGS), (2HS), (MC1S) ]
```

Why must the unasserted items be present?
- `(1NWS)` and `(1RWS)` explicitly confirm: *"Leave Switch 1 alone in its existing position."*
- If those tokens were missing, the field unit could not tell whether Switch 1 was supposed to be untouched or if the packet was truncated by line noise.
- **The Truncation Rule**: If any expected token is missing or truncated, the entire vital control transaction is rejected as invalid. Zero switches move and zero signals clear.
- Transmitting the whole of everything is what proves the transaction is complete, authentic, and safe to evaluate.

#### The Special Case: Maintainer Call (`MC`)
Notice the one exception in the transaction: `MC1S`.
Maintainer Call is an auxiliary, **non-vital circuit**:
- Lighting the maintainer lamp on the outside of the bungalow cannot cause a collision or derailment.
- In fact, the Maintainer Call exists primarily to summon a maintainer to the bungalow **when the plant is broken, failed, or locked down**!
- Therefore, even if vital switch or signal commands in a packet are rejected due to active locks or track occupancy, valid non-vital commands like `MC` are still executed. It is always safe to call the maintainer.

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

---

## 10. The Complete Command and Indication Taxonomy (Vital vs. Non-Vital)

In railroad safety engineering, every command sent across the CodeLine and every indication reported back falls into one of two safety classifications:
- **Vital (Safety-Critical)**:
  Commands and indications that affect train separation, switch points, and movement authority.
  If a vital circuit fails, it could cause a collision or derailment.
  Vital commands require strict validation, correspondence checking, and fail-safe defaults.
- **Non-Vital (Operational / Supervisory / Environmental)**:
  Auxiliary commands and indications that provide convenience or maintenance alerts.
  If a non-vital circuit fails, it causes operational delays, but **cannot cause a collision or derailment**.
  Non-vital commands bypass interlocking locks and do not require safety redundancy.

---

### 10.1 Ingress Command Taxonomy (Dispatcher / Office to Field)

| Domain | Command Mnemonic | Prototype Function | Safety Class | Precondition / Safety Checks |
|---|---|---|---|---|
| **Switch** | `1NWS` | Command Switch 1 Normal | **VITAL** | Must satisfy `WLR` (detector lock `1TR` vacant, route free, no active approach time-lock). |
| **Switch** | `1RWS` | Command Switch 1 Reverse | **VITAL** | Must satisfy `WLR` (detector lock `1TR` vacant, route free, no active approach time-lock). |
| **Signal** | `2NGS` / `2L` | Clear Signal 2 North/Left | **VITAL** | Must satisfy `HR` (switches in `KR`, blocks `TR` clear, opposing held in `ASR`). |
| **Signal** | `2SGS` / `2R` | Clear Signal 2 South/Right | **VITAL** | Must satisfy `HR` (switches in `KR`, blocks `TR` clear, opposing held in `ASR`). |
| **Signal** | `2HS` | Force Signal 2 Stop / Cancel | **VITAL** | Always accepted; trips `ASR` approach locking if train is approaching. |
| **Electric Lock**| `7WLS` | Release Electric Switch Lock 7 | **VITAL** | Mainline signals over switch 7 must be at Stop and approach timer expired. |
| **Fleeting** | `2FS` | Toggle Fleeting on Signal 2 | **NON-VITAL** | Informational; conditions `FSR` stick bypass. |
| **Call-On** | `2COS` | Authorize Low-Speed Call-On | **VITAL** | Requires explicit dispatcher button; allows `RESTRICTING` into occupied block. |
| **Maintainer** | `MC1S` | Maintainer Call Lamp ON/OFF | **NON-VITAL** | Always accepted immediately; zero safety interlocks. |
| **Auxiliary** | `SNOWS` | Switch Heater / Snow Melter | **NON-VITAL** | Always accepted; environmental auxiliary. |
| **Auxiliary** | `GENS` | Backup Generator Start/Stop | **NON-VITAL** | Always accepted; environmental auxiliary. |

---

### 10.2 Egress Indication Taxonomy (Field to Dispatcher / Office)

| Domain | Indication Mnemonic | Prototype Meaning | Safety Class | Source of Truth |
|---|---|---|---|---|
| **Switch** | `1NWK` | Switch 1 Locked in Normal | **VITAL** | `1NWCR` circuit controller contact closed. |
| **Switch** | `1RWK` | Switch 1 Locked in Reverse | **VITAL** | `1RWCR` circuit controller contact closed. |
| **Switch** | `1OOK` / Transit | Switch 1 Out of Correspondence | **NON-VITAL** | Derived: both `NWK` and `RWK` are 0 (in motion or failed). |
| **Track** | `1T1K` | Track Circuit 1T1 Occupied | **VITAL** | `1TR` track relay dropped (wheels shunting rails). |
| **Track** | `1SAK` | Approach Block 1SA Occupied | **VITAL** | `1SATR` track relay dropped. |
| **Signal** | `2NGK` | Signal 2 Northward Permissive | **VITAL** | Signal lamp current sensor verifies green/yellow lit. |
| **Signal** | `2SGK` | Signal 2 Southward Permissive | **VITAL** | Signal lamp current sensor verifies green/yellow lit. |
| **Signal** | `2TEK` | Signal 2 Time Lock Running | **NON-VITAL** | `2TER` timer relay picked up (drives panel blinking light). |
| **Electric Lock**| `7WLK` | Electric Lock 7 Unlocked | **VITAL** | Proves lock solenoid is energized and points can move. |
| **Maintainer** | `MC1K` | Maintainer Call Lamp Lit | **NON-VITAL** | Current sensor on maintainer call lamp fixture. |
| **Power** | `PORK` | Power Off (Commercial AC Loss) | **NON-VITAL** | Commercial AC power loss relay (`POR` dropped, running on battery). |
| **Security** | `DOORK` | Bungalow Door Opened | **NON-VITAL** | Door intrusion contact switch. |

---

### 10.3 The Execution Boundary Rule
When a `ControlTransaction` arrives at a FieldUnit:
1. **Vital Switch and Signal commands** pass through the interlocking safety checks.
   If a switch is locked, its movement command is rejected.
2. **Non-Vital commands** (such as Maintainer Call `MC1S`) execute immediately regardless of interlocking lock state.
   This guarantees that a dispatcher can always summon a maintainer even if a derailment or broken rail has locked down the vital interlocking logic.
