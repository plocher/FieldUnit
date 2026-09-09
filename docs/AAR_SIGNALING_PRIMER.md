# Inside the Bungalow: An Introduction to AAR Signaling for Model Railroaders

---

## Act I: The Threshold

### 1. The View from the Fascia versus the View from the Bungalow
A model railroader stands at the layout fascia and sees track, motors, and LEDs.
When a train approaches, the modeler flips an electrical toggle switch to throw the points and turns a knob to clear a signal.

A prototype railroad signal maintainer stands in a completely different world.
The maintainer steps inside a weather-proof steel bungalow sitting in the gravel next to the junction.
Inside, there are no toggle switches or track power packs.
The maintainer sees rows of glass-cased **vital relays**, banks of storage batteries, heavy wire terminals, and an electronic field controller.

To the maintainer, a junction is not a passive piece of track.
It is an autonomous safety machine that protects human life against error.
This document introduces you to the mind of a railroad signal maintainer.
It explains the standardized relay circuits, operational rules, and communication protocols that keep trains safe.

---

### 2. The Prime Directive: Fail-Safe Operation and Earth's Gravity

At the layout fascia, a modeler flips a toggle switch to `ON` to send electrical current, or to `OFF` to stop it.
If a wire comes loose, the circuit is simply "dead" and nothing moves.

In a railroad bungalow, there are no toggle switches holding circuits on.
Instead, electricity holds a heavy iron armature lifted up against the force of Earth's gravity: **"Picked Up."**
- **Picked Up (Energized)**: Current flows through the relay coil. The magnetic field lifts the heavy armature. Front contacts close.
- **Dropped (De-Energized)**: Current stops. Earth's gravity pulls the heavy armature down. Front contacts open; back contacts close.

This mechanical design enforces the prime directive of railroad signaling: **Any failure must produce the most restrictive condition.**
- If a rail breaks $\implies$ The circuit opens $\implies$ The track relay drops $\implies$ The signal drops to **Stop** (Red).
- If battery power fails $\implies$ Armatures drop by gravity $\implies$ The signal drops to **Stop** (Red).
- If a sensor wire disconnects $\implies$ The armature falls $\implies$ The plant reports **Occupied**.
- If switch points fail to travel and lock $\implies$ Contacts cannot close $\implies$ No signal will clear over the points.

Earth's gravity acts as an unbreakable physical return spring that never fails, never wears out, and never forgets to drop.

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
|    |                   | (Sectional Route)  | once a signal displays Clear. |\n+----+-------------------+--------------------+-------------------------------+
| 3  | **Approach Lock** | `ASR`              | Prevents changing points if   |
|    |                   |                    | a train is approaching Clear. |\n+----+-------------------+--------------------+-------------------------------+
| 4  | **Time Lock**     | `TER` / `TE`       | Enforces a safety countdown   |\n|    |                   |                    | if a Clear signal is revoked. |\n+----+-------------------+--------------------+-------------------------------+
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

## Act V: The Return (CP Corporal in Action)

Now we return from theory to your layout.
Let us examine how all these concepts unite in a real, compilable sketch: **CP Corporal** (Southern Pacific Coast Line MP 83).

### 11.1 The Track Diagram
Double track from the north (`MT1` and `MT2`) converges into single track through Switch 3.
Switch 1 provides access to the Beet Loader spur, protected by an automatic safety derail (`SW5`):

```
< Railroad West/North                    MP 83               Railroad East/South >

                         DERAIL                     /---IND3---DARK Beet Loader 1
                                \ 5T1   [==| 4na]  /
                      /----------\\-------IND1----+---IND2-------DARK Beet Loader 2
                  1T1/                [4sa |==]   [MC]
  MT2 <== 2SAT ══════\\═══════════════\\══════════//════ 1NAT ═══════════ 2NAT ==> (<->)
             [==| 2sa]           3T1/            IRJ                    (Approach)
  MT1 >== 1SAT ════════════════════/  [2nab |==]
          (Approach)                  (Two Heads)
```

### 11.2 The Interlocking Control Table
The route rules read directly in railroad terms:

```cpp
// Route 1: Northbound from Single Track to MT2 right-hand running
cp.route("MT-NB")
  .governedBy(sig2, DirectionAuthority::LEFT)
  .displays(mast2NAB, 0 /* Top Head */, Indication::CLEAR)
  .aligns({ {sw1, SwitchPosition::NORMAL}, 
            {sw3, SwitchPosition::NORMAL} })
  .clears({ tc1T1, tc3T1, tc2SAT });

// Route 2: Northbound from Single Track to MT1 diverging reverse running
cp.route("MT-SB")
  .governedBy(sig2, DirectionAuthority::LEFT)
  .displays(mast2NAB, 1 /* Lower Head */, Indication::DIVERGING_RESTRICTING)
  .aligns({ {sw3, SwitchPosition::REVERSE} })
  .clears({ tc3T1, tc1SAT });

// Route 3: Southbound MT1 through Switch 3 onto Single Track
cp.route("SB-MT")
  .governedBy(sig2, DirectionAuthority::RIGHT)
  .displays(mast2SA, 0, Indication::CLEAR)
  .aligns({ {sw3, SwitchPosition::REVERSE} })
  .clears({ tc3T1, tc1NAT })
  .approaching(tc2NAT);
```

### 11.3 The CodeLine Wire Mapping
The AAR control and indication bits are defined using sequential token streaming:

```cpp
codec.decodeControls({
    decodeSwitch(sw1),   // 1NW, 1RW
    decodeSwitch(sw3),   // 3NW, 3RW
    decodeSwitch(sw5),   // 5NW, 5RW
    decodeSignal(sig2),  // 2SG, 2NG, 2H
    decodeSignal(sig4),  // 4SG, 4NG, 4H
    decodeMaintainer("MC1")
});

codec.encodeIndications({
    encodeSwitch(sw1),   // 1NWK, 1RWK
    encodeSwitch(sw3),   // 3NWK, 3RWK
    encodeSwitch(sw5),   // 5NWK, 5RWK
    encodeTrack(tc1T1),  // 1T1
    encodeTrack(tc3T1),  // 3T1
    encodeTrack(tc5T1),  // 5T1
    encodeTrack(tc1NAT), // 1NAT
    encodeTrack(tc2NAT), // 2NAT
    encodeTrack(tc1SAT), // 1SAT
    encodeTrack(tc2SAT), // 2SAT
    encodeSignal(sig2),  // 2SGK, 2NGK, 2TEK
    encodeSignal(sig4)   // 4SGK, 4NGK, 4TEK
});
```

See the complete, working sketch in `examples/CP_Corporal/CP_Corporal.ino`.

---

## Act VI: Beyond the Basics (Modeling the Extended Cast on Your Layout)

Bruce Chubb's classic *Railroader's Application Handbook* showed how adding small realistic operating appliances turns an ordinary layout into a living railroad.

### 12. Switch Heaters (`SNOW` with Orange LEDs)
In snow territory, switch points freeze solid without heaters.
- **On the Model**: Mount two miniature flickering orange or amber LEDs under the ties along the stock rails of main track switches.
- **In FieldUnit**: Bind an `OutputBit` to the switch heater relay.
- When winter operating sessions begin, the dispatcher asserts `SNOWS`.
- The ties glow with realistic gas fire!

### 13. Electric Switch Locks (`WL` / `ESL`) on the Fascia
For industrial spurs or hand-operated crossovers:
- Mount a miniature toggle switch and a bi-color LED (Red/Green) on the layout fascia.
- The train crew cannot throw the switch stand until they radio the dispatcher for a release.
- The dispatcher transmits `7WLS = UNLOCK`.
- The fascia LED turns green, and FieldUnit energizes the virtual `WLR` relay.
- The crew flips the fascia toggle to throw the switch, and mainline signals drop to Stop immediately.

### 14. Maintainer Call (`MC`) and Wayside Party-Line Telephones
On pre-radio or secondary mainlines, trains had no radios in locomotive cabs:
- The dispatcher turned on the **Maintainer Call lamp (`MC1S`)** at the next control point to summon the train crew.
- **The Operating Rule**: *"When a white light is displayed at a control point, trains must stop and a member of the crew must immediately call the train dispatcher."*
- Mount a tiny white 0402 SMD LED on the peak of the bungalow roof or on the signal mast.
- Mount a working telephone handset (or magneto phone) on the fascia.
- When the white light flashes, the train crew stops their train, picks up the handset, and calls the dispatcher for orders!

### 15. Bungalow Diagnostic Telemetry (`POR` and `DOOR`)
Modern microprocessor field units monitor their own health:
- If a local I2C expander chip fails to acknowledge on the bus, or if a switch motor runs past its motion timeout, FieldUnit sets the `DOOR` or `POR` alarm bit on the CodeLine.
- The dispatcher console immediately sounds an equipment trouble alarm.
- The dispatcher calls out the signal maintainer to investigate the bungalow.

### 16. Defect Detectors (Hot Box and Dragging Equipment)
Mainline railroads install automated defect detectors every 15 to 25 miles:
- Place two optical sensors between the ties along a straight stretch of track.
- An Arduino or audio module (e.g. DFPlayer) counts axles as the train rolls overhead.
- Once the caboose passes, the module broadcasts an automated radio voice message through a layout speaker:
  *"SP Detector, Milepost 81.2. No defects. Total axles: 48. Temperature: 68 degrees. Detector out."*

---

## Summary
You now hold the keys to the bungalow.
You understand the 6 relays of a switch, the 5 relays of a signal, the 4 locking regimes, and the asynchronous ground truth of the CodeLine.

Proceed to **[Tutorial 1: Drawing Your Signaling Track Plan](tutorials/01_drawing_your_signaling_track_plan.md)** to begin constructing your first plant.
