# Tutorial 2: Building Your First Control Point

In **[Tutorial 1: Drawing Your Signaling Track Plan](01_drawing_your_signaling_track_plan.md)**, you learned how to place rail gaps, position signals, and extract data collection tables.
In this tutorial, you will turn that physical data into a complete, working railroad Control Point.

We will use **CP Corporal** (Southern Pacific Coast Line MP 83) as our working example.
By the end of this tutorial, you will have an autonomous field unit that evaluates routes, enforces detector locks, and reports verified indications.

The code in this tutorial matches the verified example in `examples/CP_Corporal/CP_Corporal.ino`.

---

## Step 1: Review the Track Diagram and Data Tables

Here is the signaling diagram for CP Corporal using the standard symbols from Tutorial 1:

```
< Railroad West / North                  MP 83                  Railroad East / South >
  (Toward Gilroy)                                               (Toward Sargent)

                               DERAIL 5                  /─── IND3 ─── (Beet Loader 1)
                                   \ 5T1      o-| 4na   /
                        /───────────\─────── IND1 ─────+───── IND2 ─── (Beet Loader 2)
                    1T1/                 |-o 4sa     SW7 (Hand-throw with 7WLS Lock)
  MT2 <══ 2SAT ═══════][═══════════════+═════════════════+════════][════ 1NAT ══════ 2NAT ══> (<->)
       (Northbound)                    │                 │ 3T1           (Single Track)
                                       │                / [SS]
  MT1 >══ 1SAT ════════════════════════════════════════/ oo-| 2nab
       (Southbound)    |-o 2sa         │                 (Two Heads)
                        (Dwarf)
```

### Review Your Data Collection Tables
From this plan, you have:
1. **Three Switches**: Switch 1 (Industry), Switch 3 (Double-track merge), Switch 5 (Derail).
2. **Seven Track Circuits**: Island blocks `1T1`, `3T1`, `5T1`; approach blocks `1SAT`, `2SAT`, `1NAT`, `2NAT`.
3. **Four Signal Masts**: Mainline 2-head mast `S2NAB`, dwarf `S2SA`, industry signals `S4NA` and `S4SA`.

---

## Step 2: Declare the Appliances

Open your sketch and include the library:

```cpp
#include <FieldUnit.h>

using namespace FieldUnit;

ControlPoint cp("CP_Corporal");
```

### 1. Declare Track Circuits
Every track circuit represents an electrical detection section.
Create the island blocks across the switch points and the approach blocks:

```cpp
// Island blocks over switch points
auto tc1T1 = cp.addTrackCircuit("1T1"); // Switch 1 points
auto tc3T1 = cp.addTrackCircuit("3T1"); // Switch 3 points
auto tc5T1 = cp.addTrackCircuit("5T1"); // Switch 5 derail

// Approach circuits outside the plant
auto tc1SAT = cp.addTrackCircuit("1SAT"); // Southbound approach on MT1
auto tc2SAT = cp.addTrackCircuit("2SAT"); // Northbound exit on MT2
auto tc1NAT = cp.addTrackCircuit("1NAT"); // Single track approach
auto tc2NAT = cp.addTrackCircuit("2NAT"); // Single track advance
```

### 2. Declare Switches and Detector Locks
A switch controls motor movement and reads position contacts.
On a real railroad, you must never throw a switch while a train occupies the points.
Bind each switch to its detector track circuit:

```cpp
auto sw1 = cp.addSwitch("SW1");
auto sw3 = cp.addSwitch("SW3");
auto sw5 = cp.addSwitch("SW5");

// Enforce detector locking
cp.bindDetectorLock(sw1, tc1T1);
cp.bindDetectorLock(sw3, tc3T1);
cp.bindDetectorLock(sw5, tc5T1);
```

FieldUnit now protects these switches.
If a train shunts `tc3T1`, any command to move `sw3` is rejected immediately.

### 3. Declare Signals
Signals divide into two parts:
- **`SignalControl`**: Represents the dispatcher movement authority (`LEFT`, `RIGHT`, `STOP`).
- **`SignalMast`**: Represents the physical wayside mast with its lamps.

```cpp
// Dispatcher movement authorities
auto sig2 = cp.addSignalControl("SIG2");
auto sig4 = cp.addSignalControl("SIG4");

// Wayside signal masts
auto mast2NAB = cp.addSignalMast("S2NAB", MastType::TWO_HEAD); // 2-Head Northbound
auto mast2SA  = cp.addSignalMast("S2SA",  MastType::DWARF);    // Southbound Dwarf on MT1
auto mast4NA  = cp.addSignalMast("S4NA",  MastType::ONE_HEAD); // Industry exit
auto mast4SA  = cp.addSignalMast("S4SA",  MastType::ONE_HEAD); // Industry entrance
```

---

## Step 3: Write the Interlocking Control Table

The Interlocking Control Table defines the valid routes through your plant.
Each route connects five facts:
1. Which signal lever grants authority?
2. Which direction must traffic flow?
3. Which physical mast displays the aspect?
4. What switch positions must be locked?
5. What track circuits must be clear?

Use FieldUnit's fluent builder to define your routes:

```cpp
// Route 1: Northbound from Single Track to MT2 (Right-Hand Running)
cp.route("MT-NB")
  .governedBy(sig2, DirectionAuthority::LEFT)
  .displays(mast2NAB, 0 /* Top Head */, Indication::CLEAR)
  .aligns({ {sw1, SwitchPosition::NORMAL}, 
            {sw3, SwitchPosition::NORMAL} })
  .clears({ tc1T1, tc3T1, tc2SAT });

// Route 2: Northbound from Single Track to MT1 (Diverging Reverse Running)
cp.route("MT-SB")
  .governedBy(sig2, DirectionAuthority::LEFT)
  .displays(mast2NAB, 1 /* Lower Head */, Indication::DIVERGING_RESTRICTING)
  .aligns({ {sw3, SwitchPosition::REVERSE} })
  .clears({ tc3T1, tc1SAT });

// Route 3: Southbound from MT1 through Switch 3 onto Single Track
cp.route("SB-MT")
  .governedBy(sig2, DirectionAuthority::RIGHT)
  .displays(mast2SA, 0, Indication::CLEAR)
  .aligns({ {sw3, SwitchPosition::REVERSE} })
  .clears({ tc3T1, tc1NAT })
  .approaching(tc2NAT);
```

Notice what you did not write:
- You did not write nested `if` statements.
- You did not calculate binary masks.
- You wrote the route once in plain railroad language.

---

## Step 4: Run the Vital Cycle

In your sketch `loop()`, advance the plant using non-blocking time:

```cpp
void loop() {
    uint32_t nowMs = millis();

    // 1. Process incoming dispatcher commands
    cp.applyControlTransaction(ctl, nowMs);

    // 2. Evaluate plant safety and route logic
    cp.tick(nowMs);

    // 3. Export verified indications back to dispatcher
    IndicationVector ind;
    cp.exportIndicationVector(ind);
}
```

---

## Step 5: What FieldUnit Does Automatically

Because you defined the plant using FieldUnit:

1. **Automatic Signal Knockdown**:
   When a train accepts `mast2NAB` and enters `tc3T1`, the signal drops to Stop immediately.

2. **Standard Stick Memory**:
   When the train departs, the signal stays at Stop.
   The signal will not clear again until the dispatcher sends a new command.

3. **Detector Lock Protection**:
   If the dispatcher tries to throw Switch 3 while a train is on `tc3T1`, the command is rejected.
   The switch motor does not move.

4. **Multi-Head Aspect Derivation**:
   - For Route 1 (straight), the top head shows Green and the lower head shows Red (Clear).
   - For Route 2 (diverging), the top head shows Red and the lower head shows Lunar (Diverging Restricting).

---

## Next Steps

- **[How-To: Hardware Wiring and AAR Bit Mapping](../how-to/01_data_collection_and_aar_bits.md)**: Connect real Tortoise motors and DCCOD detectors.
- **[How-To: JMRI Integration](../how-to/02_jmri_and_cmri_integration.md)**: Control CP Corporal from a JMRI CTC panel.
