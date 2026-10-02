# Tutorial 3: Signal Operation: Routes, Signals, Masts, and Heads

In **[Tutorial 2: Building Your First Control Point](02_building_your_first_cp.md)**, you defined appliances and built an interlocking control table.
This tutorial explains how FieldUnit models signals and how an interlocking selects the signal indication for a multi-head signal that governs multiple routes.

---

## 1. The Four-Part Signaling Model

FieldUnit separates the signal lever, route safety logic, physical masts, and lamps into four decoupled parts:

```
┌────────────────┐      ┌──────────────────────────┐      ┌────────────┐      ┌──────────────────────┐
│ SignalControl  │ ───> │          Route           │ ───> │ SignalMast │ ───> │ heads                │
│ (signal lever) │      │ (Interlocking Logic Row) │      │ (Physical) │      │ head1() .. head3()   │
└────────────────┘      └──────────────────────────┘      └────────────┘      └──────────────────────┘
```

### A. SignalControl (the Signal Lever)
- Holds the direction that the dispatcher's signal lever asked for (`LEFT`, `RIGHT`, or `STOP`). The AAR name of the relay that holds it is the home stick relay, `HSR`.
- Enforces vital stick logic: `HSR()` drops, and the direction returns to `STOP`, when a train occupies the entrance track circuit of the route (signal knockdown).
- Manages fleeting mode (`FSR()`) and the approach locking and time locking timer (`ASR()` is false only while the timer runs).
- Operates independently of track geometry, turnout points, or physical lamp hardware.

### B. Route (Vital Interlocking Table Row)
- Represents one safe path through the plant.
- Links a signal control and a direction to a mast and to the indication ceiling for that route: the most favorable signal indication the route permits (the second argument of `displays(...)`).
- Enforces prerequisite safety conditions:
  - Required switch positions, with the switches in switch correspondence (`aligns(...)`).
  - Required vacant track circuits (`clears(...)`).
  - An approach track circuit (`approaching(...)`): when it is occupied, the field unit lowers a Clear signal indication to the matching Approach indication.
  - Optional engine return (`engineReturn(...)`): a route mode that shows Restricting.

### C. SignalMast (Physical Wayside Structure)
- Represents a physical signal mast, bridge bracket, or ground dwarf.
- Instantiated with a `MastType` (`ONE_HEAD`, `TWO_HEAD`, `THREE_HEAD`, `DWARF`).
- Holds the current signal indication (`Indication`: `STOP`, `CLEAR`, `APPROACH`, `DIVERGING_RESTRICTING`, etc.), the meaning of the aspect as the rulebook states it.
- Converts the signal indication into the appearance of every head on the mast, and into the markers.

### D. Heads (Lamps and Appearance)
- A head is one unit of lamps on a mast. It shows one part of an aspect.
- A head is not an object in FieldUnit. The mast holds the appearance of each head: `head1()`, `head2()` and `head3()`.
- The aspect is the appearance of the whole signal, all heads together. `compositeAspect()` returns it, for example `Aspect::GREEN_OVER_RED`.
- Routes never address heads. The mast aspect policy maps the selected signal indication to every head and marker. For example, the default two-head policy maps `CLEAR` to Green over Red and `DIVERGING_RESTRICTING` to Red over Lunar.

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
// 1. Signal control for lever 2
SignalControl* sig2 = cp.addSignalControl("2");

// 2. Northbound entrance mast: two-head wayside signal
SignalMast* mast2NAB = cp.addSignalMast("2NAB", MastType::TWO_HEAD);
```

### Interlocking Control Table Routes
Two routes originate at mast `2NAB`. Both routes require signal control `sig2` in the `LEFT` (Northbound) direction:

```cpp
// Route 1: Northbound Single Track to MT2 (Right-hand running)
cp.route("MT-NB")
  .governedBy(sig2, DirectionAuthority::LEFT)
  .displays(mast2NAB, Indication::CLEAR)
  .aligns({ {sw1, SwitchPosition::NORMAL},
            {sw3, SwitchPosition::NORMAL} })
  .clears({ tc3T1, tc1T1, tc2SAT });

// Route 2: Northbound Single Track to MT1 (Reverse running)
cp.route("MT-NB-REV")
  .governedBy(sig2, DirectionAuthority::LEFT)
  .displays(mast2NAB, Indication::DIVERGING_RESTRICTING)
  .aligns({ {sw3, SwitchPosition::REVERSE} })
  .clears({ tc3T1, tc1SAT });
```

---

## 3. How the Engine Evaluates the Signal

During each vital tick (`cp.tick()`), the interlocking engine executes a strict evaluation cycle:

Each route reduces its own component results with `leastPermissive(...)`.
An unsafe component contributes `STOP`; an approach constraint can contribute a lower permissive signal indication.
The mast reduces all route results with `mostPermissive(...)` before mapping that signal indication to its heads.

```
[1. Force All Masts to STOP]
               │
               ▼
[2. Evaluate Route "MT-NB"]
  ├── Check Direction: sig2 == LEFT?
  ├── Check Switches: sw1 == NORMAL and sw3 == NORMAL, in switch correspondence?
  └── Check Track Circuits: tc3T1, tc1T1, tc2SAT clear?
               │
     Passed? ──┴──> YES: Contribute CLEAR for mast 2NAB. Apply Locks.
               │
              NO
               │
               ▼
[3. Evaluate Route "MT-NB-REV"]
  ├── Check Direction: sig2 == LEFT?
  ├── Check Switches: sw3 == REVERSE, in switch correspondence?
  └── Check Track Circuits: tc3T1, tc1SAT clear?
               │
     Passed? ──┴──> YES: Contribute DIVERGING_RESTRICTING for mast 2NAB. Apply Locks.
               │
              NO
               │
               ▼
[4. Reduce route results with mostPermissive for mast 2NAB]
[5. Mast policy maps the resulting signal indication to all heads]
```

### Scenario A: Dispatcher Lines Straight to MT2 (`sw1=N`, `sw3=N`)
1. **Initial Reset**: `mast2NAB->forceStop()` sets `head1()` and `head2()` to `RED`.
2. **Evaluate Route `MT-NB`**:
   - `sig2->activeDirection()` matches `DirectionAuthority::LEFT`.
   - `sw1` and `sw3` are in switch correspondence (`KR()`) in `NORMAL`.
   - Track circuits `tc3T1`, `tc1T1`, and `tc2SAT` are `VACANT`.
3. **Display**:
   - The route contributes `Indication::CLEAR` for the complete mast.
   - `mast2NAB->setIndication(Indication::CLEAR)` sets `head1()` to `GREEN` and `head2()` to `RED`.
   - The aspect is **Green over Red** (`Aspect::GREEN_OVER_RED`), the Clear signal indication.
4. **Locking**: `sw1` and `sw3` acquire `SwitchLock::ROUTE_LOCKED`.

### Scenario B: Dispatcher Lines Diverging to MT1 (`sw3=R`)
1. **Initial Reset**: `head1()` and `head2()` are set to `RED`.
2. **Evaluate Route `MT-NB`**:
   - The switch check fails because `sw3` is not `NORMAL`.
   - Route `MT-NB` contributes `STOP`.
3. **Evaluate Route `MT-NB-REV`**:
   - `sig2->activeDirection()` matches `DirectionAuthority::LEFT`.
   - `sw3` is in switch correspondence (`KR()`) in `REVERSE`.
   - Track circuits `tc3T1` and `tc1SAT` are `VACANT`.
4. **Display**:
   - The route contributes `Indication::DIVERGING_RESTRICTING` for the complete mast.
   - `mast2NAB->setIndication(Indication::DIVERGING_RESTRICTING)` sets `head1()` to `RED` and `head2()` to `LUNAR`.
   - The aspect is **Red over Lunar** (`Aspect::RED_OVER_LUNAR`), the Diverging Restricting signal indication.
5. **Locking**: `sw3` acquires `SwitchLock::ROUTE_LOCKED`.

---

## 4. Train Acceptance and Knockdown

When a train accepts the signal and shunts the entrance track circuit `tc3T1`:
1. The engine detects occupancy of the entrance track circuit (`!tc3T1->isClear()`).
2. The engine immediately invokes `sig2->knockdown()`.
3. Signal control `sig2` drops to `DirectionAuthority::STOP`.
4. The signal immediately returns to **Red over Red**.
5. Stick memory keeps the signal at `STOP` even after the train vacates `tc3T1`, until the dispatcher sends a new control.

---

## 5. Configuring Aspect Policies

Different railroads and eras show different aspects for the same signal indication.
FieldUnit provides pluggable **aspect policies** on each `SignalMast`.
A policy is a plain function. It takes the signal indication, the head count and a dwarf flag, and returns the appearance of each head and the markers:

```cpp
// Pluggable policy signature (the pointer type is AspectResolver):
MastAspects myPolicy(Indication ind, uint8_t headCount, bool isDwarf);
```

### Built-in Policies

The lists below describe what the code in `SignalAspectPolicy.h` returns.
They name no rulebook rule numbers.

1. **`AspectPolicies::defaultRoute`**:
   - Restricting and Diverging Restricting: `Aspect::LUNAR` on a one-head mast or dwarf, and `Aspect::RED` over `Aspect::LUNAR` on a two-head or three-head mast.

2. **`AspectPolicies::sp1969`**:
   - Returns what `defaultRoute` returns for every signal indication.

3. **`AspectPolicies::sp1985`**:
   - The same as `defaultRoute`, except Restricting and Diverging Restricting use flashing red: `Aspect::FLASHING_RED` on a one-head mast or dwarf, and `Aspect::RED_OVER_FLASHING_RED` on a two-head or three-head mast.
   - The hardware driver (`SignalMastDriver`) automatically pulses the red lamp pin at 1 Hz (500 ms ON / 500 ms OFF).

4. **`AspectPolicies::gcorSpeed`**:
   - Returns what `sp1985` returns for every signal indication.

5. **`AspectPolicies::nycSpeed`**:
   - Three-head masts (head 1 over head 2 over head 3):
     - Clear: Green over Red over Red.
     - Approach Medium: Yellow over Green over Red.
     - Advance Approach: Yellow over Yellow over Red.
     - Medium Clear: Red over Green over Red.
     - Approach Slow: Yellow over Red over Green.
     - Approach: Yellow over Red over Red.
     - Medium Approach: Red over Yellow over Red.
     - Slow Clear: Red over Red over Green.
     - Slow Approach: Red over Red over Yellow.
     - Restricting: Red over Red over Yellow.
     - Stop: Red over Red over Red.
   - **Slow Approach and Restricting have the same aspect in this policy.** The code returns Red over Red over Yellow for both, so a signal that shows it cannot tell the two signal indications apart. If you need them to differ, write a custom policy (below).
   - Two-head masts and dwarfs have their own, shorter mappings in the same function.

6. **`AspectPolicies::prrPositionLight`**:
   - The policy returns colors. You wire the rows of a position-light head to the color pins: the horizontal row to the red pin, the diagonal row to the yellow pin, the vertical row to the green pin.
   - Clear: Vertical over Dark (`GREEN`, `DARK`).
   - Approach: Diagonal over Dark (`YELLOW`, `DARK`).
   - Medium Clear: Horizontal over Vertical (`RED`, `GREEN`).
   - Medium Approach and Restricting: Horizontal over Diagonal (`RED`, `YELLOW`).
   - Stop: Horizontal over Dark (`RED`, `DARK`).

7. **`AspectPolicies::upperQuadrantSemaphore`**:
   - Returns what `defaultRoute` returns for every signal indication. The semaphore behavior is in `SemaphoreDriver`, which turns the appearance of each head into a servo angle for the matching arm: Green gives the clear angle, Yellow the approach angle, and every other appearance the stop angle.
   - With the default arm angles (0, 45 and 90 degrees), a two-arm mast shows these positions. Clear: top arm 90 degrees, lower arm 0. Diverging Clear: top arm 0, lower arm 90. Approach: top arm 45. Stop: both arms 0.
   - See **[How-To: Semaphores and Eastern Signaling](../how-to/04_semaphores_and_eastern_signaling.md)**.

### Configuring Policies in Your Sketch

FieldUnit provides three fluent ways to configure aspect policies:

#### 1. Plant-Wide Default (Recommended)
On almost every junction, all wayside signals belong to the same railroad. Set the policy once on `InterlockingPlant` before you declare masts, and every mast declared afterwards inherits it:

```cpp
InterlockingPlant cp("CP_Corporal");

// Set the whole plant to the sp1969 policy:
cp.setDefaultAspectPolicy(AspectPolicies::sp1969);

// Masts declared from here on inherit sp1969 without extra code:
mast2NAB = cp.addSignalMast("2NAB", MastType::TWO_HEAD);
mast2SA  = cp.addSignalMast("2SA",  MastType::DWARF);
```

You can also pass the policy to the constructor: `InterlockingPlant cp("CP_Corporal", AspectPolicies::sp1969);`.

#### 2. Fluent Chaining on Declaration
For an individual mast that needs a different policy, chain `setAspectPolicy()` or `withAspectPolicy()`:

```cpp
// Explicitly chain the sp1985 policy onto one mast:
mast4NA = cp.addSignalMast("4NA", MastType::ONE_HEAD)
            ->setAspectPolicy(AspectPolicies::sp1985);
```

#### 3. Inline Parameter on `addSignalMast`
Pass the policy directly during declaration:

```cpp
mast2NAB = cp.addSignalMast("2NAB", MastType::TWO_HEAD, AspectPolicies::nycSpeed);
```

### Writing a Custom Policy

You can pass any plain function or stateless lambda:

```cpp
mast->setAspectPolicy([](Indication ind, uint8_t headCount, bool isDwarf) -> MastAspects {
    if (ind == Indication::RESTRICTING) {
        // Show Yellow over Red for Restricting:
        return MastAspects(Aspect::YELLOW, Aspect::RED);
    }
    return AspectPolicies::defaultRoute(ind, headCount, isDwarf);
});
```
