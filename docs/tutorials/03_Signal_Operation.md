# Tutorial 3: Signal Operation: Routes, Signals, Masts, and Heads

In **[Tutorial 2: Building Your First Control Point](02_building_your_first_cp.md)**, you defined appliances and built an interlocking control table.
This tutorial explains how FieldUnit models signals and how an interlocking selects an indication on a multi-head signal that governs multiple routes.

---

## 1. The Four-Tier Signaling Model

FieldUnit separates dispatcher authority, route safety logic, physical structures, and optical lamps into four decoupled abstractions:

```
┌────────────────┐      ┌──────────────────────────┐      ┌────────────┐      ┌────────────┐
│ SignalControl  │ ───> │          Route           │ ───> │ SignalMast │ ───> │ SignalHead │
│  (Authority)   │      │ (Interlocking Logic Row) │      │ (Physical) │      │  (Lamps)   │
└────────────────┘      └──────────────────────────┘      └────────────┘      └────────────┘
```

### A. SignalControl (Movement Authority)
- Represents the logical dispatcher authority or tower signal lever (AAR Home Signal Stick Relay, `HSR`).
- Manages traffic direction requests (`LEFT`, `RIGHT`, or `STOP`).
- Enforces vital stick logic: drops to `STOP` when a train shunts an entrance block (signal knockdown).
- Manages fleeting mode (`FSR`) and approach/time locking countdown timers (`ASR`).
- Operates independently of track geometry, turnout points, or physical lamp hardware.

### B. Route (Vital Interlocking Table Row)
- Represents one safe path through the plant.
- Links an entrance authority (`SignalControl*` and `DirectionAuthority`) to a destination display target (`SignalMast*`, `targetHeadIndex`, and `aspectCeiling`).
- Enforces prerequisite safety conditions:
  - Required switch point alignment and locked correspondence (`aligns(...)`).
  - Required vacant track circuits (`clears(...)`).
  - Downstream advance block conditions for aspect progression (`approaching(...)`).
  - Optional engine-return permissive moves (`engineReturn(...)`).

### C. SignalMast (Physical Wayside Structure)
- Represents a physical signal mast, bridge bracket, or ground dwarf.
- Instantiated with a `MastType` (`ONE_HEAD`, `TWO_HEAD`, `THREE_HEAD`, `DWARF`).
- Holds the active operational rulebook `Indication` (`STOP`, `CLEAR`, `APPROACH`, `DIVERGING_RESTRICTING`, etc.).
- Converts the rulebook indication into physical visual `Aspect` lamp colors across all mounted heads.

### D. Signal Head (Lamps and Appearance)
- Represents an individual searchlight target or color-light lamp cluster on the mast.
- Multi-head masts use 0-indexed positions:
  - `Head 0`: Top Head (Main / normal speed route).
  - `Head 1`: Lower Head (Diverging route or medium/slow speed).
  - `Head 2`: Bottom Head (Third route, yard entrance, or restricting marker).
- When a route drives an indication to a specific head, non-targeted heads display their fail-safe stop marker (`RED` or `DARK`).

---

## 2. Multi-Route Signal Selection: CP Corporal Example

In `examples/CP_Corporal/CP_Corporal.ino`, mast **`2NAB`** governs northbound trains entering double track from single track.

```
                      1T1/                       oo-| 2nab (Two Heads)
   MT2 <══ 2SAT ═══][═══+═══════════════════════+══════════════════][════ 1NAT ══════ 2NAT ══> (<->)
   (Northbound) |-o 4sa SW1                 3T1/ SW3                     (Single Track)
   MT1 >══ 1SAT ═══][═════════════════════════/  [SS]
   (Southbound) |-o 2sa (Dwarf)
```

### Signal and Mast Declaration
```cpp
// 1. Dispatcher authority lever 2
sig2 = cp.addSignalControl("2");

// 2. Northbound entrance mast: two-head wayside signal
mast2NAB = cp.addSignalMast("2NAB", MastType::TWO_HEAD);
```

### Interlocking Control Table Routes
Two routes originate at mast `2NAB`. Both routes require dispatcher authority `sig2` in the `LEFT` (Northbound) direction:

```cpp
// Route 1: Northbound Single Track to MT2 (Right-hand running)
cp.route("MT-NB")
  .governedBy(sig2, DirectionAuthority::LEFT)
  .displays(mast2NAB, 0 /* Top Head */, Indication::CLEAR)
  .aligns({ {sw1, SwitchPosition::NORMAL}, 
            {sw3, SwitchPosition::NORMAL} })
  .clears({ tc3T1, tc1T1, tc2SAT });

// Route 2: Northbound Single Track to MT1 (Reverse running)
cp.route("MT-SB")
  .governedBy(sig2, DirectionAuthority::LEFT)
  .displays(mast2NAB, 1 /* Lower Head */, Indication::DIVERGING_RESTRICTING)
  .aligns({ {sw3, SwitchPosition::REVERSE} })
  .clears({ tc3T1, tc1SAT });
```

---

## 3. How the Engine Evaluates the Signal

During each vital tick (`cp.tick()`), the interlocking engine executes a strict evaluation cycle:

```
[1. Force All Masts to STOP]
               │
               ▼
[2. Evaluate Route "MT-NB"]
  ├── Check Authority: sig2 == LEFT?
  ├── Check Switches: sw1 == NORMAL and sw3 == NORMAL?
  └── Check Blocks: tc3T1, tc1T1, tc2SAT clear?
               │
     Passed? ──┴──> YES: Target Top Head (0) with CLEAR (Green over Red). Apply Locks.
               │
              NO
               │
               ▼
[3. Evaluate Route "MT-SB"]
  ├── Check Authority: sig2 == LEFT?
  ├── Check Switches: sw3 == REVERSE?
  └── Check Blocks: tc3T1, tc1SAT clear?
               │
     Passed? ──┴──> YES: Target Lower Head (1) with DIVERGING_RESTRICTING (Red over Lunar). Apply Locks.
               │
              NO
               │
               ▼
[4. Mast remains at STOP (Red over Red)]
```

### Scenario A: Dispatcher Lines Straight to MT2 (`sw1=N`, `sw3=N`)
1. **Initial Reset**: `mast2NAB->forceStop()` sets Head 0 to `RED` and Head 1 to `RED`.
2. **Evaluate Route `MT-NB`**:
   - `sig2->activeDirection()` matches `DirectionAuthority::LEFT`.
   - `sw1` and `sw3` report full correspondence (`KR`) in `NORMAL`.
   - Track circuits `tc3T1`, `tc1T1`, and `tc2SAT` are `VACANT`.
3. **Display Aspect**:
   - The route assigns `Indication::CLEAR` to **Head 0** (top head).
   - `mast2NAB->setHeadIndication(0, Indication::CLEAR)` sets Head 0 to `GREEN` and leaves Head 1 at `RED`.
   - The physical aspect is **Green over Red** (Clear).
4. **Locking**: `sw1` and `sw3` acquire `SwitchLock::ROUTE_LOCKED`.

### Scenario B: Dispatcher Lines Diverging to MT1 (`sw3=R`)
1. **Initial Reset**: Both heads set to `RED`.
2. **Evaluate Route `MT-NB`**:
   - Switch alignment check fails because `sw3` is not `NORMAL`.
   - Route `MT-NB` is skipped.
3. **Evaluate Route `MT-SB`**:
   - `sig2->activeDirection()` matches `DirectionAuthority::LEFT`.
   - `sw3` reports full correspondence (`KR`) in `REVERSE`.
   - Track circuits `tc3T1` and `tc1SAT` are `VACANT`.
4. **Display Aspect**:
   - The route assigns `Indication::DIVERGING_RESTRICTING` to **Head 1** (lower head).
   - `mast2NAB->setHeadIndication(1, Indication::DIVERGING_RESTRICTING)` sets Head 0 to marker `RED` and Head 1 to `LUNAR`.
   - The physical aspect is **Red over Lunar** (Diverging Restricting).
5. **Locking**: `sw3` acquires `SwitchLock::ROUTE_LOCKED`.

---

## 4. Train Acceptance and Knockdown

When a train accepts the signal and shunts island track circuit `tc3T1`:
1. The engine detects entrance block occupancy (`!tc3T1->isClear()`).
2. The engine immediately invokes `sig2->knockdown()`.
3. Dispatcher authority `sig2` drops to `DirectionAuthority::STOP`.
4. Both signal heads immediately return to **Red over Red**.
5. Standard AAR stick memory ensures the signal stays at `STOP` even after the train vacates `tc3T1`, until the dispatcher clears a new route.

---

## 5. Configuring Rulebook Aspect Policies

Different railroads and historical eras used different physical aspects for the same operational rulebook indication.
FieldUnit provides pluggable **Aspect Policies** on each `SignalMast`:

```cpp
// Pluggable policy signature:
MastAspects myPolicy(Indication ind, uint8_t headCount, bool isDwarf);
```

### Built-in Policies

1. **Standard Default Route (`AspectPolicies::defaultRoute`)**:
   - Traditional North American color-light practice.
   - Restricting / Diverging Restricting = `Aspect::LUNAR` (or `Aspect::RED_OVER_LUNAR`).

2. **Southern Pacific 1969 Rulebook (`AspectPolicies::sp1969`)**:
   - Classic SP lunar era.
   - Rule 290 Restricting = Red over Lunar (`Aspect::RED_OVER_LUNAR`), Dwarf = Lunar (`Aspect::LUNAR`).

3. **Southern Pacific 1985 / Later Rulebook (`AspectPolicies::sp1985`)**:
   - Later SP flashing red era.
   - Rule 290 Restricting = Red over Flashing Red (`Aspect::RED_OVER_FLASHING_RED`), Dwarf = Flashing Red (`Aspect::FLASHING_RED`).
   - The hardware driver (`SignalMastDriver`) automatically pulses the red lamp pin at 1 Hz (500 ms ON / 500 ms OFF).

4. **GCOR Speed Signaling (`AspectPolicies::gcorSpeed`)**:
   - Standard modern speed signaling with multi-head diverging and approach combinations.

5. **New York Central / Eastern Speed Signaling (`AspectPolicies::nycSpeed`)**:
   - Eastern three-head interlocking home signals (High, Medium, and Slow speeds).
   - High Speed Clear: Green over Red over Red (Rule 281).
   - Approach Medium: Yellow over Green over Red (Rule 282).
   - Advance Approach: Yellow over Yellow over Red (Rule 282A).
   - Medium Clear: Red over Green over Red (Rule 283).
   - Slow Clear: Red over Red over Green (Rule 287).
   - Slow Approach: Red over Red over Yellow (Rule 288).
   - Restricting: Red over Red over Yellow (Rule 290).

6. **Pennsylvania Railroad Position Lights (`AspectPolicies::prrPositionLight`)**:
   - PRR amber lamp rows wired to standard color pins (Horizontal=Red, Diagonal=Yellow, Vertical=Green).
   - Clear: Vertical over Dark.
   - Approach: Diagonal over Dark.
   - Medium Clear: Horizontal over Vertical.
   - Stop: Horizontal over Dark.

7. **Upper-Quadrant Semaphore (`AspectPolicies::upperQuadrantSemaphore`)**:
   - Mechanical blades driven by `SemaphoreDriver` to calibrated servo angles.
   - Clear: Top blade 90° vertical.
   - Diverging Clear: Top blade 0° horizontal, lower blade 90° vertical.
   - Approach: Top blade 45° diagonal.
   - Stop: All blades 0° horizontal.

### Configuring Policies in Your Sketch

FieldUnit provides three fluent ways to configure rulebook policies:

#### 1. Plant-Wide Default (Recommended)
On almost every junction, all wayside signals belong to the same railroad. Set the rulebook once on `ControlPoint`, and every mast automatically inherits it:

```cpp
ControlPoint cp("CP_Corporal");

// Set the entire plant to Southern Pacific 1969 practice:
cp.setDefaultAspectPolicy(AspectPolicies::sp1969);

// Masts automatically inherit SP 1969 without extra code:
mast2NAB = cp.addSignalMast("2NAB", MastType::TWO_HEAD);
mast2SA  = cp.addSignalMast("2SA",  MastType::DWARF);
```

#### 2. Fluent Chaining on Declaration
For an individual mast that needs a different policy, chain `setAspectPolicy()` or `withAspectPolicy()`:

```cpp
// Explicitly chain a modern flashing red policy onto one mast:
mast4NA = cp.addSignalMast("4NA", MastType::ONE_HEAD)
            ->setAspectPolicy(AspectPolicies::sp1985);
```

#### 3. Inline Parameter on `addSignalMast`
Pass the policy directly during declaration:

```cpp
mast2NAB = cp.addSignalMast("2NAB", MastType::TWO_HEAD, AspectPolicies::nycSpeed);
```

### Writing a Custom Railroad Policy

You can pass any custom resolver function or stateless lambda:

```cpp
mast->setAspectPolicy([](Indication ind, uint8_t headCount, bool isDwarf) -> MastAspects {
    if (ind == Indication::RESTRICTING) {
        // Model a railroad using Yellow over Red for Restricting:
        return MastAspects(Aspect::YELLOW, Aspect::RED);
    }
    return AspectPolicies::defaultRoute(ind, headCount, isDwarf);
});
```
