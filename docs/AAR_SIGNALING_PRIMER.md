# Inside the Bungalow: An Introduction to AAR Signaling for Model Railroaders

**Next layer (CTC, subdivisions, structural routes, drawing rules for interlocking plants):**
[CTC, Subdivision, and Plant Design](CTC_SUBDIVISION_AND_PLANT_DESIGN.md).

Terms in this primer follow the [Glossary](GLOSSARY.md). Where the primer and the glossary seem to disagree, the glossary wins.

---

## Act I: The Threshold

### 1. The View from the Fascia versus the View from the Bungalow
A model railroader stands at the layout fascia and sees track, motors, and LEDs.
When a train approaches, the modeler flips an electrical toggle switch to throw the points and turns a knob to clear a signal.

A prototype railroad signal maintainer stands in a completely different world.
The maintainer steps inside a weather-proof steel bungalow sitting in the gravel next to the control point.
Inside, there are no toggle switches or track power packs.
The maintainer sees rows of glass-cased **vital relays**, banks of storage batteries, heavy wire terminals, and, on a modern installation, an electronic field processor.

To the maintainer, a control point is not a passive piece of track.
It is an autonomous safety machine that protects human life against error.
This document introduces you to the mind of a railroad signal maintainer.
It explains the relay circuits, operating rules, and code line communication that keep trains safe.

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
- If a sensor wire disconnects $\implies$ The armature falls $\implies$ The track circuit reports **Occupied**.
- If switch points fail to travel and lock $\implies$ Contacts cannot close $\implies$ No signal will clear over the points.

Earth's gravity acts as a physical return spring that does not need power, does not wear out, and does not forget to drop.

---

## Act II: The Citizens of the Bungalow (The Appliance Cast)

Every device at a control point is an **appliance** with an assigned role and a set of vital relays.

### 3. The Switch: More Than a Motor
A railroad track switch is not just a motor.
A power-operated switch has a heavy electro-mechanical switch machine (such as a US&S M-23 or GRS Model 5) with a point detector circuit controller and a lock.

FieldUnit models each switch with the relays below.
`TR` is an AAR name. `WLR`, `NWCR`, `RWCR` and `KR` are FieldUnit names built from AAR letters; the AAR names for the switch indicators are `NWK` and `RWK` (see [Glossary §10.5](GLOSSARY.md)).

```
+=============================================================================+
|                      THE 6 RELAYS OF A FIELDUNIT SWITCH                     |
+-----------+-----------------------------------+-----------------------------+
| Relay     | Name                              | Operational Role            |
+-----------+-----------------------------------+-----------------------------+
| **`1TR`** | Track relay, OS track circuit 1T1 | Detects trains on points    |
| **`1WLR`**| Switch 1 lock relay               | Cuts motor power if locked  |
| **`1WR`** | Switch 1 relay                    | Drives the switch motor     |
| **`1NWCR`**| Normal, in switch correspondence | Reports Normal = control    |
| **`1RWCR`**| Reverse, in switch correspondence| Reports Reverse = control   |
| **`1KR`** | Switch correspondence relay       | In correspondence, either   |
|           |                                   | position                    |
+-----------+-----------------------------------+-----------------------------+
```

#### Secret 1: "W" Means Switch!
Newcomers often wonder: *Why is a switch named with the letter "W"?*
The answer is short: the AAR letter list assigns **`W`** to switch (it also means west) [AAR56 p. 32].
Do not guess that `S` would have meant "signal": in AAR names, **`S`** means south, stick or storage, and the signal letter is **`G`** (as in `NGS`, `SGK`).
- `WR` = s**W**itch **R**elay.
- `WLR` = s**W**itch **L**ock **R**elay.
- `NW` = **N**ormal s**W**itch.
- `RW` = **R**everse s**W**itch.

#### Secret 2: Odd Switches, Even Signals (a Habit, Not a Rule)
On the SPCoast CTC machine, each panel column pairs an **odd switch lever** on top with an **even signal lever** below it over a common code button.
- Switches: `1`, `3`, `5`, `7` (or milepost switches `777`, `781`).
- Signals: `2`, `4`, `6`, `8` (or milepost signals `778`, `782`).

This is a common convention, older than CTC: it goes back to lever-and-pipe interlocking plants. It is not an AAR rule: the AAR 1956 manual's Fig. 6 numbers its switches with even numbers.

#### Step-by-Step: Throwing Switch 1
```
[ Dispatcher codes 1RWS ] ──> [ Check 1WLR (Is switch unlocked?) ]
                                           │
                        ┌──────────────────┴──────────────────┐
                        │                                     │
                 [ 1TR is Occupied ]                   [ 1TR is Vacant ]
                        │                                     │
                        ▼                                     ▼
              Control NOT acted on                    1WR drives Motor
              (Points cannot move;                            │
               no refusal message)                            ▼
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
Wayside signals do not light lamps directly from the dispatcher's controls.
Every signal works through a chain of vital relays:

```
+=============================================================================+
|                      THE 5 RELAYS OF A RAILROAD SIGNAL                      |
+-----------+-----------------------------------+-----------------------------+
| Relay     | Name                              | Operational Role            |
+-----------+-----------------------------------+-----------------------------+
| **`2HSR`**| Home stick relay (AAR)            | Holds the dispatcher's      |
|           |                                   | direction (LEFT or RIGHT)   |
| **`2HR`** | Home relay (AAR letters)          | Interlocking route clear    |
| **`2DR`** | Proceed-indication relay (AAR     | Block ahead clear           |
|           | letters; D is NOT "distant")      |                             |
| **`2ASR`**| Approach stick relay (AAR)        | Enforces approach locking   |
| **`2FSR`**| Fleet stick relay (FieldUnit name)| Re-clears for following     |
|           |                                   | trains                      |
+-----------+-----------------------------------+-----------------------------+
```

#### Aspect versus Signal Indication
Railroad rulebooks make an exact distinction between what an engineer sees and what it means:
- **Aspect**: What the whole signal *looks like*, all its heads together (e.g. Green over Red, Red over Lunar, Flashing Yellow).
- **Signal indication**: What the rulebook *instructs the crew to do* (e.g. `CLEAR`, `APPROACH`, `DIVERGING_CLEAR`, `RESTRICTING`, `STOP`).

Do not confuse a signal indication with an **office indication** (Act IV): the report that the field sends back to the dispatcher.

#### Step-by-Step: Clearing Signal 2
The circuits below are illustrative word pictures, not prototype circuit plans.
```
                       HR Circuit (Home Relay)
 Power ──[ 2HSR Front ]──[ 1KR Front ]──[ 1TR Front ]──[ Opposing ASR ]──( 2HR )
          (Dispatcher)    (Switches)     (Track Clear)   (Opposing Stop)
                                                                │
                                                                ▼
                                                       Signal displays at
                                                       least APPROACH (Yellow)
                                                                │
                       DR Circuit (Proceed Indication)          ▼
 Power ──[ 2HR Front ]──[ Next Signal Clear (4HR) ]──────────────( 2DR )
                                                                │
                                                                ▼
                                                       Signal upgrades to
                                                       CLEAR (Green over Red)
```

---

### 5. The Extended Appliance Cast
Beyond switches and signals, a real interlocking plant includes other appliances:

#### 1. Crossovers (`Crossover`)
A crossover connects two parallel main tracks using two switches that work together from one lever (for example `3A` and `3B`).
In prototype signaling, both switch machines receive the same control.
Both must be in switch correspondence before any route over them clears.

#### 2. Derails (`Derail`)
A derail sits on a siding or industrial spur.
It physically derails a rolling car before it can foul the main track.
As on the prototype, derail **NORMAL** means the derailing position (on the rail) and **REVERSE** means clear (a train may pass). A route through a derail requires REVERSE.
An **independent derail** has its own lever, its own number and its own tokens on the code line.
A **dependent derail** is named `<switchId>D` (for example `1D`). It has no lever and no tokens of its own, and it moves with its switch, in the same position:
- When the main switch is Normal $\implies$ Derail is NORMAL (derailing).
- When the main switch is Reverse $\implies$ Derail is REVERSE (clear).

The switch's `NWK` and `RWK` office indications require both machines in switch correspondence.
Crossover machine ends use `A`/`B`/`C` suffixes; **`D` is reserved for derails**.

#### 3. Electric Switch Locks (`WL`)
In CTC limits, a hand-throw switch can have an electric lock (AAR `WL`).
The train crew cannot lift the hand-throw lever until the dispatcher sends a release control (`WLS`), which releases the lock.
An electric lock on a hand-throw switch is not a dual-control switch: a dual-control switch is a power switch that can also be thrown by hand.

#### 4. Axle Counters and Optical Sensors
Where track circuits suffer from poor ballast resistance or unpowered model train wheelsets:
- **Axle Counters** count wheels into and out of a track section.
- **Optical Sensors (`d`)** detect physical car bodies at clearance and fouling points.

#### 5. Trackside Defect Detectors
- **Hot Box Detectors (HBD)**: Sense overheated wheel bearings via infrared.
- **Dragging Equipment Detectors (DED)**: Impact plates that detect dragging chains or air hoses.

#### 6. Switch Heaters (`SNOW`)
Gas burners or electric calrod elements along the rails that melt snow and ice to keep points moving during winter storms.

#### 7. Bungalow Utilities
- **Maintainer Call (`MC`)**: A control and lamp that call the signal maintainer to the location.
- **Power-off indication**: Shows the dispatcher that a location has lost power. A 1959 CTC machine had a power-off indication lamp for each location.
- **Door contact (`DOOR`)**: A modeling idea, not a sourced prototype appliance. It reports that the bungalow door is open.

---

### 6. Algorithmic Composition of Names
An AAR name has a number prefix and letters [AAR56 p. 31]:

$$\text{Name} = [\text{Number of the lever, signal or track circuit}] + [\text{Descriptive letters}] + [\text{Letter for the kind of unit}]$$

The last letter gives the general kind of unit (`R` for relay). The letters before it describe the unit. Example from the AAR manual: `10HR` is signal 10, home, relay.

- `2` + `HS` + `R` $\implies$ **`2HSR`**: Signal 2, Home Stick, Relay (AAR).
- `2` + `AS` + `R` $\implies$ **`2ASR`**: Signal 2, Approach Stick, Relay.
- `1` + `NWC` + `R` $\implies$ **`1NWCR`**: Switch 1, Normal sWitch Correspondence, Relay (FieldUnit name).
- `1` + `RWC` + `R` $\implies$ **`1RWCR`**: Switch 1, Reverse sWitch Correspondence, Relay (FieldUnit name).
- `1` + `WL` + `R` $\implies$ **`1WLR`**: Switch 1, sWitch Lock, Relay (FieldUnit name).

One letter has several meanings; its position in the name decides which one is meant.

---

## Act III: The Laws of Safety (The Interlocking)

---

### 7. The 4 Interlocking Locking Regimes
An interlocking enforces four distinct locking regimes to protect against physical hazards:

```
+=============================================================================+
|                      THE 4 PROTOTYPE LOCKING REGIMES                        |
+----+-------------------+--------------------+-------------------------------+
| #  | Locking Regime    | Relay / FieldUnit  | What It Protects              |
+----+-------------------+--------------------+-------------------------------+
| 1  | **Detector Lock** | `TR` (OS track     | Prevents throwing points      |
|    |                   | circuit)           | directly underneath a train.  |
+----+-------------------+--------------------+-------------------------------+
| 2  | **Route Lock**    | `ROUTE_LOCKED`     | Holds the points along a      |
|    |                   | (sectional release)| route once a signal clears.   |
+----+-------------------+--------------------+-------------------------------+
| 3  | **Approach Lock** | `ASR`              | Holds the points if a cleared |
|    |                   |                    | signal is cancelled while a   |
|    |                   |                    | train is approaching.         |
+----+-------------------+--------------------+-------------------------------+
| 4  | **Time Lock**     | `TIME_LOCKED`,     | Holds the points for a set    |
|    |                   | office ind. `TEK`  | time after a signal is        |
|    |                   |                    | restored to stop.             |
+----+-------------------+--------------------+-------------------------------+
```

On the prototype, approach locking and time locking are two kinds of locking. FieldUnit uses **one timer** for both, as described below.

#### 7.1 Detector Locking (Points Protection)
- **The Hazard**: Throwing points while a car sits on them splits the train and causes a derailment.
- **The Rule**: Power to the switch motor passes through `1TR` front contacts. As long as wheels shunt `1TR`, the switch is detector-locked and cannot move.

#### 7.2 Route Locking (Path Reservation)
- **The Hazard**: Throwing a trailing-point switch ahead of a train that has entered the interlocking.
- **The Rule**: When a signal clears, every switch on the route gets `ROUTE_LOCKED`. Switches release progressively as the train clears each switch (sectional route release).

#### 7.3 Approach Locking (Signal Cancellation Protection)
- **The Hazard**: The dispatcher clears a Green signal. A train approaches at speed. The dispatcher cancels the signal and lines a switch against it. The train cannot stop in time.
- **Safe Cancellation (Immediate Release)**: If the approach track circuit `1SA` is `VACANT`, there is no train. The field unit releases the switches at once. It does the same if the route declares no approach track circuit.
- **Hazardous Cancellation (Timed Release)**: If the approach track circuit `1SA` is `OCCUPIED`, the field unit starts the time-locking timer. `ASR()` goes false and the route switches get `TIME_LOCKED`. The points stay held until the timer expires: 30 s by default in FieldUnit (`SignalControl::setTimeLockDuration`).
- `ASR()` does not drop just because a signal clears. While the signal is clear, route locking holds the switches. `ASR()` is false only while the timer runs.

#### 7.4 Time Locking and the CTC Machine
While the time-locking timer runs:
- The signal is at Stop (`2NGK` and `2SGK` dropped).
- The office indication `2TEK` (time locking runs) is asserted.
- On the CTC machine, the red **time element lamp** of signal 2 flashes while time locking runs. (Planned in FieldUnit: `cTcMachine` has no `TEK` lamp output today.)
- If the dispatcher moves the Switch 1 lever and codes it while `2TEK` is asserted, the switch does not move. The switch lamp keeps showing the old position, so the lever and the lamp disagree: the switch is **out of office correspondence**. That disagreement is the only refusal the dispatcher gets. A CTC machine can also sound an alarm for it. (Planned in FieldUnit: no alarm output exists today.)
- When the timer reaches zero, `2TEK` drops and the points are free.

---

### 8. The Complete Interlocking Logic Chains

In prototype signaling, vital logic is not an abstract mathematical equation.
It is an electrical circuit where every contact protects against a specific, life-threatening hazard.

This section dissects the core logic chains that FieldUnit evaluates during each scan of the interlocking plant.
For each chain, we show a relay contact word picture, then its plain-English logical equivalent, and why each contact is there.
The word pictures are illustrative. They are not prototype circuit plans.

---

#### 8.1 Switch Lock Relay (`WLR`) — The Four Padlocks on the Motor
A switch machine (like a US&S M-23 or a model Tortoise) exerts tremendous mechanical force against the points.
If the motor runs while a train is moving over the points, it will split the switch, derail the cars, and tear up the track.
Power to the motor circuit must pass through `1WLR`.
If any contact opens, `1WLR` drops, instantly cutting all electrical power to the motor:

```
  Power ───[ 1TR Front ]───[ RouteLock Back ]───[ 2ASR Front ]───[ ElectricLock Released ]───( 1WLR )
          (Points Clear)     (Route Free)     (No Time Locking)    (WLS accepted, if any)
```

`WLR = (1TR is VACANT) AND (NOT RouteLocked) AND (2ASR picked up: no time locking) AND (NOT ElectricLocked)`

In FieldUnit, `Switch::WLR()` is true when the switch has no active lock: detector, route, time or electric lock. A switch moves only when `WLR()` is true.

Why each contact is vital:
1. **`1TR Front` (Detector Locking)**:
   Proves no train is physically standing on or straddling the points.
   If wheels shunt the rails, `1TR` drops and physically breaks the motor circuit.
2. **`RouteLock Back` (Route Locking)**:
   Proves no cleared route has reserved this switch.
   Even if the track is empty right now, if a signal is Green for an approaching train, the route lock contact opens to hold the points.
3. **`2ASR Front` (Approach Locking)**:
   Proves no train is bearing down on the interlocking under a cancelled signal whose time-locking timer is still running.
4. **`ElectricLock Released` (Electric Switch Lock)**:
   On a hand-throw switch with an electric lock, `WLR` stays dropped until the dispatcher releases the lock with `WLS`.
   The field unit releases the lock only when every signal is at stop and no time locking runs.

---

#### 8.2 Switch Correspondence Relay (`KR`) — Proving the Points Truly Made
The railroad never trusts a motor alone.
A motor can hum, an electrical wire can corrode, or ballast gravel can jam between the rail and the point.
The dispatcher may code Normal, but the points might be gapped open by 1/2 inch—enough for a wheel flange to pick the point and derail the train.

The `KR` relay is the **only proof the interlocking trusts** that a switch is where its control says it should be:

```
  Power ───┬───[ Normal Controlled ]───[ 1NWCR Front ]───┬───( 1KR )
           │   (Control: Normal)       (Points Normal)   │
           │                                             │
           └───[ Reverse Controlled ]──[ 1RWCR Front ]───┘
               (Control: Reverse)      (Points Reverse)
```

`KR = (Control Normal AND 1NWCR) OR (Control Reverse AND 1RWCR)`

Why this dual check is vital:
- `1NWCR` picks up only when the switch reports Normal and its control is Normal: the switch is in switch correspondence, Normal.
- `1RWCR` picks up only when the switch reports Reverse and its control is Reverse.
- If the points are in transit (`MOVING`), gapped by ballast, or disagree with the control, both `NWCR` and `RWCR` drop. After the travel timeout (5000 ms by default), a moving switch reports `OUT_OF_CORRESPONDENCE`.
- `1KR` drops, making it electrically impossible to clear any signal over the switch.

---

#### 8.3 Crossover Proving Relay (`3KR`) — Preventing the Half-Thrown Nightmare
A crossover connects two main tracks via two physical switch machines (`3A` on Track 1, `3B` on Track 2).
If `3A` throws to Reverse, but `3B` jams in Normal, a train entering the crossover would be steered across the gap directly into the side of a train on the adjacent track!

To prevent this catastrophe, the interlocking evaluates both machines in series:

```
  Power Source ───[ 3A KR Front ]───[ 3B KR Front ]───( 3KR )
                  (MT1 Points)      (MT2 Points)
```

`3KR = (3A is KR) AND (3B is KR)`

Both switch machines must travel together, lock together, and prove switch correspondence together.
If either switch binds or lags, `3KR` drops, and no route over the crossover can clear.

---

#### 8.4 Home Relay (`HR`) — The Guardian of the Entrance
What must be true before an engineer sees anything other than a solid red Stop aspect?
Current to the signal mechanism must pass through four independent safety gates:

```
  Power ───[ 2HSR Front ]───[ Route KR Fronts ]───[ Route TR Fronts ]───[ Opposing ASR Fronts ]───( 2HR )
     (Dispatcher)     (Switches in Line)     (Track Circuits Clear)    (Opposing Held Stop)
```

`HR = (2HSR active) AND (All Route Switches in KR) AND (All Route Track Circuits VACANT) AND (Opposing Signals in ASR)`

Why each contact is vital:
1. **`2HSR Front` (Direction)**: The dispatcher coded a control for this direction, and the field unit accepted it.
2. **`Route KR Fronts` (Alignment)**: Every switch on the route is proven in switch correspondence, in position.
3. **`Route TR Fronts` (Occupancy)**: Every track circuit on the route is proven empty and un-shunted.
4. **`Opposing ASR Fronts` (Collision Protection)**: Conflicting and opposing signals on the same track are held at Stop, with their approach locking intact.

When all contacts close, `2HR` picks up. The signal drops its red aspect and displays at least `APPROACH` or `RESTRICTING`.

FieldUnit has no `HR` object. `InterlockingEngine::evaluateIndication` (`ControlTable.h`) takes the least favorable of the route's indication ceiling and these same checks.

---

#### 8.5 Proceed-Indication Relay (`DR`) — Looking Down the Line
The `HR` relay proves it is safe to enter *this* interlocking.
But how fast may the train travel?
The `DR` relay looks down the track to the next signal.
(`D` here is the AAR letter for "proceed indication". It does not mean "distant".)

```
  Power ───[ 2HR Front ]───[ Block Ahead TR Front ]───[ Next Signal HR Front ]───( 2DR )
           (Route Clear)    (Block Ahead Clear)       (Next Signal Permissive)
```

`DR = (2HR picked up) AND (Block Ahead VACANT) AND (Next Signal Permissive)`

- If the block ahead is occupied, `DR` stays dropped.
  The signal displays **`APPROACH`** (Yellow).
  The engineer knows to slow down and prepare to stop at the next signal.
- If the block ahead is clear AND the next signal is also displaying a permissive aspect, `DR` picks up.
  This upgrades the aspect from `APPROACH` (Yellow) to **`CLEAR`** (Green).

FieldUnit has no `DR` object. It lowers CLEAR to APPROACH when the track circuit named in the route's `approaching()` is occupied.

---

#### 8.6 Signal Knockdown and Stick Relay (`HSR`) — The One-Shot Rule
Why must a signal never stay green behind a train?
If it did, a following train could enter the occupied block, and a rear-end collision could occur.
The signal must drop to Stop the instant the locomotive passes the mast:

```
                     Entrance Track Circuit TR Front
  Dispatcher Code ──────[       ]───────┬───────────────────────────( 2HSR )
                                        │
         2HSR Front                     │
     ┌──[    ]──────┐                   │
     │              ├───────────────────┘
     │  2FSR Front  │
     └──[    ]──────┘
```

`HSR_next = (Dispatcher Code) OR ((2HSR picked up) AND (Entrance Track Circuit VACANT)) OR (2FSR active)`

1. **The Knockdown**: As the locomotive's front wheels pass the signal and bridge the insulated joint, the entrance track relay drops.
   This opens the contact and breaks the stick circuit on `2HSR`.
   The signal drops to Stop (Red) behind the engine.
2. **The Stick Break**: Because `2HSR` dropped, its own front contact opens.
   Even after the entire train leaves the interlocking and the track relay picks back up, `2HSR` stays dropped.
   The signal will not clear again until the dispatcher codes a new control.
3. **The Fleeting Bypass (`2FSR`)**: If the dispatcher turns fleeting on (`2FSR` picked up), the fleet contact bypasses the broken `2HSR` contact.
   As soon as the train vacates the route and the blocks ahead clear, power feeds back to the signal, clearing it automatically for a following train.

---

#### 8.7 Engine Return Stick Relay (`ERS`) — Switching Fluidity Without Stalls
During switching operations, an engine pulls past a signal into an adjacent track or siding, uncouples from its cars, and needs to immediately reverse direction to couple back up.

Normally, revoking signal authority or attempting a reverse movement trips approach locking (`ASR`), starting the time-locking timer.
The Engine Return circuit eliminates this delay safely:

```
  Forward Exit Move Trigger
  ───[ Route Active Front ]───[ OS TR Back ]───[ Exit Track TR Back ]───┐
                                                                            ▼
                                                                        [ 2ERS Coil ]
  Hold-In Path (Stick)                                                      ▲
  ───[ 2ERS Front ]───────────[ Exit Track TR Back ]────────────────────────┘
```

`ERS_pickup = (Forward Route Active) AND (OS TR is OCCUPIED) AND (Exit Track is OCCUPIED)`
`ERS_hold   = (2ERS picked up) AND (Exit Track remains OCCUPIED)`

1. **The Sequence Trigger**:
   The circuit monitors the physical progression of the locomotive:
   $$\text{Route Active} \longrightarrow \text{OS section shunted} \longrightarrow \text{Exit track shunted}$$
   When the engine straddles the boundary onto the exit track, `2ERS` picks up.
2. **The Hold-In Path**:
   `2ERS` stays energized through its own front contact as long as the train's cars continue to stand on the exit track (`Exit TR` remains dropped).
3. **The Benefit**:
   Because `2ERS` proves the train is sitting right at the boundary at switching speed, it **does not wait for the time-locking timer**.
   The return dwarf signal immediately displays **`RESTRICTING`** (Lunar or Yellow), authorizing the engineer to back up at low speed to couple onto the cars.
4. **The Fail-Safe Reset**:
   If another train pulls those cars away and the exit track circuit becomes vacant, the circuit breaks.
   `2ERS` drops immediately, and the return signal drops to Stop fail-safe.

**In FieldUnit today** (`Route::engineReturn(standingCars, os)`): the code implements a reduced form of this circuit.
It shows `RESTRICTING` when the route switches are in position, the OS track circuit is clear and the standing-cars track circuit is occupied.
It has no stick: it does not remember the forward move through the OS section, and it does not check the signal control or time locking.
The relay circuit above is the design principle. The difference is recorded as a defect against the code, not as a change to the principle.
The name "engine return stick" is the project's; the AAR list has `TSR` (track stick relay), and railroads also used directional stick relays.

---

## Act IV: The Outside World (Communication Across Distance)

---

### 9. The Code Line: Asynchronous Truth versus Remote Procedure Calls

Modern software developers often think in Remote Procedure Calls (RPC):
`response = client.call("throwSwitch", 1, REVERSE);`
They expect immediate return codes, error messages, or NACKs.

Railroad signaling operates on a completely different, asynchronous foundation.

#### 9.0 The Free Lever: Tower Lever versus CTC Lever
In a tower, the operator pulls a lever on the interlocking machine.
If the move is unsafe, the lever does not move: the locking bed holds it. The refusal is immediate, in the operator's hand.
Because the lever cannot be in an unsafe position, the lever itself shows the state of the interlocking plant.

On a CTC machine, the lever moves freely. Nothing at the office holds it.
The dispatcher moves the lever, presses **CODE**, and the intent goes out on the code line.
Only the office indication that comes back tells the truth. If the field unit refuses, no message says so: the office indication simply does not change to agree with the lever.

So the code line, the field station and the field unit together replace the locking bed. They do at a distance, and after the fact, what the locking bed does in the operator's hand.
That is why CTC needs two words where the tower needed one: **control** (the dispatcher's intent) and **office indication** (the state of the interlocking plant).

#### 9.1 One Code, One Control Transaction
A dispatcher does not send one control at a time to one device.
The dispatcher lines all levers for a control point on the CTC machine (here, CP Corporal):
- Switch 1 lever to Normal.
- Switch 3 lever to Reverse.
- Derail 5 lever to Normal (derailing).
- Signal 2 lever to Right.
- Signal 4 lever to Stop (center).
- Maintainer Call toggle to Off.

The dispatcher presses the **code button (CODE)**.
The office sends **one complete control transaction** across the code line:
- **Unparenthesized token (`3RWS`)**: The function is **asserted** (the lever is in that position).
- **Parenthesized token (`(3NWS)`)**: The function is **dropped** (the lever is not in that position).

```
[ 1NWS, (1RWS), (3NWS), 3RWS, 5NWS, (5RWS), 2SGS, (2NGS), (2HS), (4SGS), (4NGS), 4HS, (MC1S) ]
```

Why must dropped tokens be present?
- Every function the field station carries is sent on every code, asserted or dropped. The field unit can then tell "lever not in this position" from "token lost on the line".
- A switch lever with neither contact closed sends both switch tokens dropped, `(1NWS), (1RWS)`. FieldUnit reads that as "no switch control": leave Switch 1 where it is.
- **The Truncation Rule**: If any expected token is missing, out of order, or unknown, `AarTextCodec` rejects the whole packet. Zero switches move and zero signals clear.

#### 9.2 Office Indications Report the Truth
The field unit does not send "error packets," "NACKs," or conversational replies.
The field unit simply reports the state of the interlocking plant.

- If the dispatcher codes Switch 1 to Reverse while a train sits on the points:
  - The field unit does not move the motor.
  - The field unit does not send an error message.
  - The field unit continues to send its true indication vector:
    `1NWK, (1RWK), 1T1K` (Points remain Normal and in switch correspondence; the OS track circuit is occupied).

#### 9.3 Office Correspondence: The State of Mind
The CTC machine detects that the field unit did not act by comparing the lever with the office indications:

$$\text{Lever (Intent)} \stackrel{?}{=} \text{Office Indication (Truth)}$$

- When the points are in motion, both switch lamps are **dark** (out of switch correspondence).
- When the points lock in the controlled position, the matching lamp lights.
- If the switch is locked or obstructed, the lamp for the old position **stays lit**, and the lever disagrees with it: the switch is out of office correspondence.
- The dispatcher sees: *"The switch did not move."*

#### 9.4 CTC Machine Behavior
- **Levers are intent, office indications are truth**: Office levers state the dispatcher's intent; the field unit reports the state of the interlocking plant.
- **No controls at power-up**: A CTC machine does not send controls when it powers up. A lever moved while the machine was off holds intent that nobody confirmed. Sending it could throw a switch the dispatcher no longer expects to move.
- **Cold start (FieldUnit)**: On power-up, the SPCoast desk applies the retained office indications from the MQTT code line. The panel lamps show the state of the interlocking plant. If a switch lever disagrees with its lamp, the switch is out of office correspondence. The dispatcher must either move the lever to match the lamp, or line the lever and deliberately press CODE.
- **Requirement**: A CTC machine must let the dispatcher state an unsafe intent. The field unit refuses it. A CTC machine that blocks a lever from its own copy of the state moves the locking to the office, which is not vital.

#### 9.5 Three Roles and One Seam: Interlocking Plant, Field Station, and Panel Column

The word "control point" is often used loosely, for a place, for an address on the code line, and for a part of the CTC machine. FieldUnit separates them.
Every CTC layout has **three roles**: the **CTC machine** (office), the **code line**, and the **field unit**.
Between them runs **one seam**: controls and office indications, as named functions (tokens such as `1NWS` and `1NWK`).
Everything else (steps, code cycle, addresses, field stations, capacity, encoding, timing) belongs to the **code line type**: US&S 506, MQTT, or another.

```
┌─────────────────────────────────────────────────────────────┐
│ CTC machine (office)                                        │
│    - Panel columns: levers, lamps, code button.             │
│    - States intent. Is not vital.                           │
└──────────────────────────────┬──────────────────────────────┘
                               │  code line (type: US&S 506, MQTT, ...)
                               │  controls ↓     ↑ office indications
                               ▼
┌─────────────────────────────────────────────────────────────┐
│ Field station(s): addresses on the code line                │
│    - 506 line: 7 controls + 7 office indications each.      │
│    - MQTT line: one per field unit, no capacity limit.      │
│    - Has no behavior.                                       │
└──────────────────────────────┬──────────────────────────────┘
                               ▼
┌─────────────────────────────────────────────────────────────┐
│ Field unit = interlocking application on a field processor  │
│    - Enforces the locking (WLR, KR, ASR, HSR).              │
│    - Solves one interlocking plant.                         │
└──────────────────────────────┬──────────────────────────────┘
                               ▼
              Interlocking plant: track, switches, derails,
              signals and track circuits
```

1. **Interlocking Plant**: The real track, switches, derails, signals and track circuits that one interlocking controls. It can contain more than one control point. One field unit solves one interlocking plant. The bungalow (or a tower's ground floor) is only the enclosure for its equipment.
2. **Field Station**: One address on a code line, and the set of controls and office indications carried under that address. It has no behavior. The code line type assigns field stations. On a US&S 506 line, one field station carries 7 controls and 7 office indications, so a large interlocking plant answers at several field stations, with one field unit behind all of them. Luchessa answers at three field stations on a 506 line; it answers at one on an MQTT line.
3. **Panel Column**: One vertical part of the CTC machine face, with levers, lamps and a code button. In FieldUnit, a `CtcStation` (the office-side record of one field station) can span up to four panel columns, which share one CODE button.

##### 9.5.1 The SPCoast CTC Machine's Display Rhythm (a Model Design)

Everything in this subsection is a design of the SPCoast CTC machine (US&S style). **It is not a prototype code.** The US&S 506 code has 16 steps and selects a field station with 7 station selection steps (§9.6). This display has 15 steps and a 4-step panel-column address, because it only has to animate lamps on one machine.

The MQTT code line carries each complete control transaction at once. When the display is turned on (`CODELINE_VISUAL_STEPPING` in `examples/spcoast_ctc`; it is off by default), the desk plays one display cycle per `PanelColumn` before it acts. For a field station that spans several columns, a press of the shared CODE button plays one cycle per column, and the desk publishes the complete `ControlTransaction` only after the last column's completion pulse. A received indication vector likewise plays one cycle per column before the desk applies it.

The two repurposed traffic lamps on Column 3 show the direction of the code:
- **S lamp / Southbound traffic lamp**: Office-to-field control code.
- **N lamp / Northbound traffic lamp**: Field-to-office indication code.

Each display cycle has the following fixed step allocation:

| Step | Function |
| :--- | :--- |
| 1 | Line clear / synchronization |
| 2–5 | Four-bit panel-column address |
| 6–7 | Switch Normal and Reverse |
| 8–10 | Signal Left, Stop/time locking, and Right |
| 11–13 | Up to three track indication lamps (indication cycles only) |
| 14 | Maintainer call |
| 15 | Completion |

An asserted function produces a **long 350 ms pulse**. A dropped function produces a **short 110 ms pulse**. A 25 ms lamp-off interval separates pulses. Operators can read the address and each function from the cadence, instead of watching an arbitrary blink pattern. The desk keeps the full set of AAR tokens internally; the display only makes the fast MQTT code line look and feel like a stepping code line.

#### 9.6 Machines, Eras, and Code Lines

Railroad **signal towers** housed mechanical, electro-mechanical, and electric interlocking machines and communication gear used to direct train traffic, switches and signals.

- **Key Equipment Found in Railroad Towers**:
  - **Interlocking Machines**: Lever frames with mechanical or electric locking that forced operators to line switches in a safe sequence before clearing a signal.
  - **Mechanical Levers ("Armstrong" Plants)**: Tall levers connected through pipes, bell cranks, and rods directly to switches and derails.
  - **Pistol-Grip and Miniature Lever Machines**: Compact electric or electro-pneumatic interlocking machines built by General Railway Signal (GRS) and Union Switch & Signal (US&S).
  - **Model Boards / Track Diagrams**: Track maps with small lamps that show track circuit occupancy.
  - **Relays and Storage Batteries**: Electrical equipment, often on the ground floor, that powers track circuits, signals, and switch motors.

CTC machines, built from relays, let one dispatcher control many distant control points over a code line.

##### Code line types
Each code line type defines its own steps, addressing and capacity. Do not carry the numbers of one type to another.

- **US&S 506 time code** (the code line type of the SPCoast CTC machine):
  - One code has **16 steps**: 1 conditioning step, 7 station selection steps, 7 function steps, and 1 delivery step.
  - A field station on 506 or 506A carries **7 controls and 7 office indications**.
  - One 506 code line serves up to **35 field stations**.
  - 506C carries 7 controls and 35 office indications for each field station.
- **US&S 514**: described as similar to the 506, with 35 field locations.
- **Earlier Union time code**: a 1937 article describes a "Union time-code C.T.C. system" on a "single series circuit of two wires". A 1937 description of an earlier Union time code says that a control code had 14 impulses. Step counts changed from system to system.
- **GRS systems**: GRS named its coded systems by letter (Type F, H, J and K) and sold them under names such as SyncroStep and SyncroScan. Its consoles included the NX and the Traffic Master. Their steps and formats differ from the US&S codes. (Do not confuse these with the GRS Model 5 family, 5A to 5H: those are electric switch machines, the appliances that a GRS Type K system controls.)
- **NX (Entrance-Exit) machines**: A GRS route-setting design. The operator sets a complete route by pushing an entrance button and an exit button, and the machine lines the switches between them.
- **MQTT (FieldUnit)**: one topic per field unit, so one field station; no capacity limit.

#### 9.7 Switch Machine Physics: Lock Dog Dominance vs. Point Detection

In a power switch machine (for example a US&S M-23 or GRS Model 5):
$$\text{Switch correspondence } (\text{KR}) = \text{Points Closed (Detector Rod)} \ \mathbf{AND} \ \text{Lock Dog Seated (Lock Rod)}$$
- For vital safety, the lock contacts dominate point detection: an unlocked switch **cannot** be in switch correspondence, even if the point rail is still touching the stock rail.
- As soon as the machine starts to unlock, the lock dog withdraws from the lock rod notch.
- The circuit controller contacts open immediately, dropping `NWCR`/`RWCR` and `KR`.
- On the CTC machine, the switch lamp goes **DARK** (out of switch correspondence).

##### Independent derail: 5

This is a derail with its own lever and its own tokens:

- numbered like a switch;
- receives a control from the dispatcher;
- a route through it requires REVERSE (clear);
- has its own office indications (`5NWK`, `5RWK`).

##### Dependent derail: 3D

This is a derail that moves with its switch:

- tied to its switch, such as 3, and always in the same position;
- has no lever and no tokens of its own;
- the switch's `3NWK` and `3RWK` require both machines in switch correspondence.

#### 9.8 A Natural Timeline for Fast-Clock Model Railroad Operations

On the prototype, a code took seconds to step out, the switch took seconds to move, and the office indication took seconds to step back.
On a model railroad with fast clocks and compressed distances, that full wait feels sluggish. Instantaneous network delivery, however, feels synthetic.

A suggested cadence for fast-clock layout operations (a model design):

| Phase | Duration | What Happens Physically | What You See on the CTC Machine |
| :--- | :--- | :--- | :--- |
| **1. Outbound Control Code** | **1.2s to 1.8s** | Office steps out the address and function pulses. | **Old switch lamp stays lit.** Code button released. Stepper relays clatter. |
| **2. Field Reception & Lock Dog Pull** | **~150ms** | Field unit accepts the control. Motor withdraws the lock dog. | Lock contacts open $\implies$ **Old switch lamp goes DARK**. |
| **3. Point Travel** | **2.0s to 2.8s** | Motor drives switch points across the ties. | Both lamps **DARK** (out of switch correspondence / MOVING). |
| **4. Seating & Inbound Office Indication** | **1.2s to 1.6s** | Points seat; the lock dog seats in the notch. The field sends the office indication code back. | Points locked $\implies$ return code steps $\implies$ **New switch lamp lights**. |

Total elapsed time from button press to new lamp: **~4.5 to 5.5 seconds**.
This timeline gives the mechanical and electrical hesitation of prototype signaling while staying responsive for layout operating sessions.

---

### 10. The Code Line Taxonomy: Controls versus Office Indications

First, one fact about safety: **vital logic is in the field.** The code line and the CTC machine are not vital. The words "vital" and "non-vital" describe how the field unit processes a control, not the wire that carries it. Each control is in one of two classes:
- **Vital control**: Can affect a safety protection (switches, signals, electric locks). The interlocking logic checks it before the field unit acts.
- **Non-vital control**: Cannot affect a safety protection (maintainer call, snow melters). The interlocking logic does not check it against the locking.

Office indications carry the same class column in the tables below. It names the class of the function they report.

FieldUnit's `AarTextCodec` carries switch, signal, electric lock and maintainer call controls, and switch, track circuit, signal, electric lock and maintainer call office indications. Rows marked † are prototype examples with no FieldUnit token today.

#### 10.1 Control Taxonomy (Office to Field)

| Domain | Control Token | Prototype Function | Safety Class | Precondition in the Field Unit |
|---|---|---|---|---|
| **Switch** | `1NWS` | Control Switch 1 Normal | Vital | Must satisfy `WLR` (OS track circuit `1T1` vacant, no route lock, no time locking, no electric lock). |
| **Switch** | `1RWS` | Control Switch 1 Reverse | Vital | Must satisfy `WLR` (OS track circuit `1T1` vacant, no route lock, no time locking, no electric lock). |
| **Signal** | `2NGS` / lever `2L` | Clear Signal 2 LEFT | Vital | Signal clears only when the route checks pass (switches in `KR`, route track circuits clear, opposing signals held). |
| **Signal** | `2SGS` / lever `2R` | Clear Signal 2 RIGHT | Vital | Signal clears only when the route checks pass (switches in `KR`, route track circuits clear, opposing signals held). |
| **Signal** | `2HS` | Put Signal 2 at Stop | Vital | Always accepted. If the approach track circuit is occupied, time locking runs. |
| **Electric Lock**| `7WLS` | Release Electric Lock 7 | Vital | Every signal must be at Stop and no time locking may run. |
| **Fleeting** † | `2FS` | Turn Fleeting On for Signal 2 | Vital | Conditions the `FSR` stick bypass. |
| **Call-On** † | `2COS` | Call-On (Restricting) | Vital | Allows `RESTRICTING` into an occupied track circuit. |
| **Maintainer** | `MC1S` | Maintainer Call ON/OFF | Non-vital | Applied at once; no locking checks. |
| **Auxiliary** † | `SNOWS` | Switch Heater / Snow Melter | Non-vital | Applied at once. |

#### 10.2 Office Indication Taxonomy (Field to Office)

| Domain | Office Indication Token | Prototype Meaning | Safety Class | Source in the Field Unit |
|---|---|---|---|---|
| **Switch** | `1NWK` | Switch 1 Normal, in switch correspondence | Vital | `1NWCR` picked up. |
| **Switch** | `1RWK` | Switch 1 Reverse, in switch correspondence | Vital | `1RWCR` picked up. |
| **Switch** | (no token) | Switch 1 out of switch correspondence | — | Both `1NWK` and `1RWK` dropped (in motion or failed). |
| **Track** | `1T1K` | OS Track Circuit 1T1 Occupied | Vital | `1TR` track relay dropped (wheels shunting rails). |
| **Track** | `1SAK` | Approach Track Circuit 1SA Occupied | Vital | `1SATR` track relay dropped. |
| **Signal** | `2NGK` | Signal 2 Cleared LEFT | Vital | `2HSR` holds LEFT. |
| **Signal** | `2SGK` | Signal 2 Cleared RIGHT | Vital | `2HSR` holds RIGHT. |
| **Signal** | `2TEK` | Time Locking Runs at Signal 2 | Vital | The time-locking timer runs (`ASR()` of signal 2 is false). |
| **Electric Lock**| `7WLK` | Electric Lock 7 Released | Vital | The field unit released the lock. |
| **Maintainer** | `MC1K` | Maintainer Call On | Non-vital | The field unit's maintainer call state. |
| **Power** † | `PORK` | Power Off (Commercial AC Loss) | Non-vital | A power-off relay dropped; running on battery. |
| **Security** † | `DOORK` | Bungalow Door Open (modeling idea) | Non-vital | Door contact. |

#### 10.3 The Two Classes of Control
When a `ControlTransaction` arrives at a field unit, two rules apply:
1. **A malformed transaction is ignored as a whole.** A transaction is malformed when it is incomplete, contains unknown or corrupted data, or fails a structural check of its code line. The field unit ignores the controls of both classes, updates its error counters, and sends office indications of its current, unchanged state. A UDP packet with a bad checksum behaves the same way.
2. **A valid transaction that is unsafe has every vital control ignored together.** If acting on the transaction would violate a safety protection, the field unit ignores every vital control in it. It does not act on the safe ones and skip the unsafe one. If a switch is locked, no vital control in that transaction acts, including the free switches. The field unit acts on every non-vital control (such as Maintainer Call `MC1S`). It sends no refusal; the office learns the result from the office indications.

This lets a dispatcher call a maintainer even when a derailment or broken rail has locked down the interlocking.

The current code differs: `InterlockingPlant::applyControlTransaction()` still applies the maintainer call when it marks the transaction invalid, and skips only the unsafe control. See FieldUnit `docs/adr/0002-control-transaction-classes.md`.

---

## Act V: The Return (CP Corporal in Action)

Now we return from theory to your layout.
Let us examine how all these concepts unite in a real, compilable sketch: **CP Corporal** (Southern Pacific Coast Line MP 83).

### 11.1 The Track Diagram
Rule 251 double track from the north (`MT1` and `MT2`) converges into single track through Switch 3.
Switch 3 is operated as a **Spring Switch (`[SS]`)**: Southbound trains on `MT1` make a trailing-point move through the spring points onto single track without needing motor alignment.
Northbound trains on single track `1NAT` face signal `2nab` at Switch 3:
- Moving straight onto `MT2` follows the current of traffic (right-hand running).
- Diverging onto `MT1` enters the track **against the current of traffic**, which limits the aspect to `DIVERGING_RESTRICTING`.

Switch 1 provides access to the Beet Loader spur, protected by derail 5. In the sketch, derail 5 follows switch 1 in the same position: switch 1 Normal gives derail 5 Normal (derailing); switch 1 Reverse gives derail 5 Reverse (clear).

Every track circuit boundary is an **Insulated Rail Joint (IRJ)** (`][`), and signals face the approaching engineer on the engineer's right-hand side using standard schematic symbols:
- **`|-o`** (or **`|-oo`**): Below track, lamps face left (governs Eastward / Southbound moves).
- **`o-|`** (or **`oo-|`**): Above track, lamps face right (governs Westward / Northbound moves).

```
< Railroad West / North                  MP 83                  Railroad East / South >
  (Toward Gilroy)                                               (Toward Sargent)

                               DERAIL 5                  /─── IND3 ─── (Beet Loader 1)
                                   \ 5T1      o-| 4na   /
                          /─────────+─────── IND1 ─────+───── IND2 ─── (Beet Loader 2)
                         /                           SW7 (Hand-throw with 7WLS Lock)
                     1T1/                       oo-| 2nab (Two Heads)
  MT2 <══ 2SAT ═══][═══+═══════════════════════+══════════════════][════ 1NAT ══════ 2NAT ══> (<->)
  (Northbound) |-o 4sa SW1                 3T1/ SW3                     (Single Track)
  MT1 >══ 1SAT ═══][═════════════════════════/  [SS]
  (Southbound) |-o 2sa (Dwarf)
```

### 11.2 The Interlocking Control Table
The route rules read directly in railroad terms (from `examples/CP_Corporal/CP_Corporal.ino`):

```cpp
// Route 1: Northbound Single Track to MT2 right-hand running (SW1=N, SW3=N)
cp.route("MT-NB")
  .governedBy("2", DirectionAuthority::LEFT)
  .displays("2NAB", Indication::CLEAR)
  .aligns({ {"1", SwitchPosition::NORMAL},
            {"3", SwitchPosition::NORMAL} })
  .clears({ "3T1", "1T1", "2SAT" })
  .entrance("3T1");

// Route 2: Northbound Single Track to MT1 reverse running (SW3=R)
cp.route("MT-NB-REV")
  .governedBy("2", DirectionAuthority::LEFT)
  .displays("2NAB", Indication::DIVERGING_RESTRICTING)
  .aligns({ {"3", SwitchPosition::REVERSE} })
  .clears({ "3T1", "1SAT" })
  .entrance("3T1");

// Route 3: Southbound MT1 through switch 3 onto single track (SW3=R)
cp.route("SB-MT")
  .governedBy("2", DirectionAuthority::RIGHT)
  .displays("2SA", Indication::CLEAR)
  .aligns({ {"3", SwitchPosition::REVERSE} })
  .clears({ "3T1", "1NAT" })
  .entrance("1SAT")
  .approaching("2NAT");
```

### 11.3 The Code Line Token Mapping
The AAR controls and office indications are declared in the order they travel on the code line:

```cpp
codec.decodeControls({
    decodeSwitch(cp.findSwitch("1")),          // 1NWS, 1RWS
    decodeSwitch(cp.findSwitch("3")),          // 3NWS, 3RWS
    decodeSwitch(cp.findSwitch("5")),          // 5NWS, 5RWS
    decodeSignal(cp.findSignalControl("2")),   // 2SGS, 2NGS, 2HS
    decodeSignal(cp.findSignalControl("4")),   // 4SGS, 4NGS, 4HS
    decodeMaintainer(0)                        // MC1S
});

codec.encodeIndications({
    encodeSwitch(cp.findSwitch("1")),          // 1NWK, 1RWK
    encodeSwitch(cp.findSwitch("3")),          // 3NWK, 3RWK
    encodeSwitch(cp.findSwitch("5")),          // 5NWK, 5RWK
    encodeTrack(cp.findTrackCircuit("1T1")),   // 1T1K
    encodeTrack(cp.findTrackCircuit("3T1")),   // 3T1K
    encodeTrack(cp.findTrackCircuit("5T1")),   // 5T1K
    encodeTrack(cp.findTrackCircuit("1NAT")),  // 1NATK
    encodeTrack(cp.findTrackCircuit("2NAT")),  // 2NATK
    encodeTrack(cp.findTrackCircuit("1SAT")),  // 1SATK
    encodeTrack(cp.findTrackCircuit("2SAT")),  // 2SATK
    encodeSignal(cp.findSignalControl("2")),   // 2SGK, 2NGK, 2TEK
    encodeSignal(cp.findSignalControl("4")),   // 4SGK, 4NGK, 4TEK
    encodeMaintainer(0)                        // MC1K
});
```

See the complete, working sketch in `examples/CP_Corporal/CP_Corporal.ino`.

---

## Act VI: Beyond the Basics (Modeling the Extended Cast on Your Layout)

Bruce Chubb's *Railroader's C/MRI Application Handbook* (Volume 1, Enhancements; Volume 2, Signaling) is a good companion for this part: small operating appliances turn an ordinary layout into a living railroad.

### 12. Switch Heaters (`SNOW` with Orange LEDs)
In snow country, switch points freeze solid without heaters.
- **On the Model**: Mount two miniature flickering orange or amber LEDs under the ties along the stock rails of main track switches.
- **In FieldUnit**: FieldUnit has no snow-melter token today. Drive the LEDs from an `OutputBit` in your sketch.
- When winter operating sessions begin, the dispatcher turns the heaters on.
- The ties glow with realistic gas fire!

### 13. Electric Switch Locks (`WL`) on the Fascia
For industrial spurs or hand-operated crossovers:
- Mount a miniature toggle switch and a bi-color LED (Red/Green) on the layout fascia.
- The train crew cannot throw the switch stand until they radio the dispatcher for a release.
- The crew turns the fascia key: the field unit raises the request indication `7WLQK`, and the dispatcher's lock lever plate starts to blink.
- The dispatcher moves the lock lever to R and codes `7WLS` (release).
- The field unit releases the lock only when every signal is at Stop and no time locking runs. It then asserts `7WLK` (released), and `7WLR` can pick up.
- The fascia's green locked lamp goes dark, the red request lamp stops blinking and stays lit, and the crew throws the switch stand.
- The whole exchange, lamp by lamp, is in section 18.

### 14. Maintainer Call (`MC`) and Wayside Telephones
On the prototype, the maintainer call did what its name says: it called the signal maintainer to a location. A 1959 CTC machine had a "maintainer's call" control, with a maintainers' call lamp at each location.
- The dispatcher codes **`MC1S`** to light the Maintainer Call lamp at the location.
- Mount a tiny white 0402 SMD LED on the peak of the bungalow roof or on the signal mast.

**A modeling idea (not a prototype rule):** on a layout with no radios, you can use the same lamp to call an operator to a fascia telephone.
- Mount a working telephone handset (or magneto phone) on the fascia.
- When the white light shows, the operator picks up the handset and calls the dispatcher.

### 15. Bungalow Telemetry (Power-Off and Door)
A real CTC machine could show a power-off indication for each location.
FieldUnit has no power-off or door token today. If you add one:
- Make it a non-vital function, with its own token.
- A power-off indication means a loss of power. A door indication means an open door. Do not reuse either one for other faults, such as an I2C expander that stops answering or a switch motor that runs past its travel timeout; give each fault its own name.
- The dispatcher then calls out the signal maintainer to investigate the bungalow.

### 16. Defect Detectors (Hot Box and Dragging Equipment)
Main line railroads install automated defect detectors at intervals along the line:
- Place two optical sensors between the ties along a straight stretch of track.
- An Arduino or audio module (e.g. DFPlayer) counts axles as the train rolls overhead.
- Once the train passes, the module plays an automated radio voice message through a layout speaker:
  *"SP Detector, Milepost 81.2. No defects. Total axles: 48. Temperature: 68 degrees. Detector out."*

### 17. Semaphores, Quadrants and Eras (What a Route Can Show)
A semaphore arm says what the signal means, and which arm you have depends on the year.
- **Lower-quadrant semaphores** are the original design: the arm drops from horizontal. Two positions. Horizontal is Stop; angled down (usually 45 or 60 degrees) is Clear.
- **Upper-quadrant semaphores** arrived around 1903 and became the North American standard: the arm rises from horizontal. Three positions. Horizontal is Stop; diagonal (about 45 degrees) is Approach; vertical is Clear.
- **The timeline**: before 1903 only lower quadrants existed, so a route could show Stop or Clear and nothing else; there was no Approach. From 1903 to about 1908 both were in use. Through the WWI years upper quadrants replaced lowers on busy routes. By the 1940s colour-light and colour-position-light signals were replacing semaphores on main lines.
- **In FieldUnit**: the era decides the aspect set, so the era's indication tables (the route indication tables) carry it, with this history beside them so the limit is obvious. A pre-1903 route shows Stop and Clear, never Approach. A two-position head on a route whose era table asks for Approach is an era error, not a wiring error: either the head is the wrong quadrant for the year, or the table is the wrong year for the head.
- **On the Model**: a servo moves the arm. Declare the head's quadrant with the head (two positions or three) so the generator can hold it against the era table, and give each position its own angle so a lower quadrant drops and an upper quadrant rises.

### 18. The Electric Lock, Step by Step (Crew, Dispatcher, Field)
Section 13 shows the fascia. This is the whole exchange, with every lamp.

**The three tokens.** `WLS` is the control (dispatcher to field). `WLK` is the lock indication (field to dispatcher): asserted means the electric lock has energized, withdrawn its plunger, and the switch is released; deasserted means the switch is locked and secured for track speed. `WLQK` is the crew's unlock request (field to dispatcher), non-vital; the name is FieldUnit's, not AAR's.

**Why released is the asserted state: fail-safe.** The lock is gravity-based: when power fails, the lock bar drops and the switch is locked. The lock relay is fail-safe too: it drops when power fails, into the locked state. So the relay is energized to unlock, and the indication follows the relay. That is why some references say "locked when 0" and others "unlocked when active": both describe the same relay. FieldUnit says only asserted and deasserted, and names what the asserted state means: `WLK` asserted is released.

**The lever plate.** The dispatcher's lock lever has two lamps: a green locked lamp (lit while `WLK` is deasserted) and a red request lamp (driven by `WLQK`). The fascia plate has the same two lamps and the crew's key.

**The exchange.** Start: `WLK` deasserted (locked), `WLQK` deasserted (no request); green lit, red dark.
1. The crew turns the fascia key to ask for a release. The field unit asserts `WLQK`.
2. At the office, the red lamp goes from dark to blinking: the dispatcher sees a request.
3. The dispatcher moves the lock lever from N to R (released) and codes it. The office sends `WLS`.
4. While `WLS` is out and `WLK` has not yet arrived, the office extinguishes the green lamp and keeps the red one blinking: the release is pending.
5. The field unit checks that every signal is at Stop and no time locking runs (a time-element relay may run a countdown first), then energizes the lock relay, releases the lock and asserts `WLK`.
6. The office stops the blink and leaves the red lamp lit: unlocked, crew at work.
7. The crew throws the switch as needed. No switch position goes to the dispatcher; it is not needed.
8. When the crew is done, they use the key to re-lock. The field unit deasserts `WLQK`.
9. The office extinguishes the red lamp.
10. The field unit normalizes the switch, re-locks (a simulated lock on the model), drops the lock relay and deasserts `WLK`.
11. The office lights the green lamp. Back to the start.

**AAR definitions, for the record.**
- Control `WLS` (switch lock, `WL`): sent from the dispatching office to the field. It starts the unlock sequence. Depending on local track occupancy it either drops the locking circuit at once (short time) or starts a time-element relay (`TE`) that counts down a safety timer (long time) before the switch is released.
- Indication `WLK` (switch lock indication): deasserted, the switch is physically locked and secured for main-line track speed, the fail-safe state; asserted, the electric lock has energized, released its plunger, and the switch is unlocked.

---

## Summary
You now hold the keys to the bungalow.
You understand the 6 relays of a switch, the 5 relays of a signal, the 4 locking regimes, the free lever of a CTC machine, and the asynchronous truth of the code line.

Proceed to **[Tutorial 1: Drawing Your Signaling Track Plan](tutorials/01_drawing_your_signaling_track_plan.md)** to begin constructing your first interlocking plant.
