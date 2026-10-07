# Tutorial 2: Building Your First Control Point

In **[Tutorial 1: Drawing Your Signaling Track Plan](01_drawing_your_signaling_track_plan.md)**, you learned how to place rail gaps, position signals, and extract data collection tables.
In this tutorial, you will turn that physical data into working C++ for one interlocking plant.

We will use **CP Corporal** (Southern Pacific Coast Line MP 83) as our working example.
By the end of this tutorial, you will have an autonomous field unit that evaluates routes, enforces detector locks, and reports verified office indications.

The code in this tutorial follows the example in `examples/CP_Corporal/CP_Corporal.ino`, with two differences.
This tutorial passes appliance pointers to the route builder where the sketch passes names, and it declares switch 5 with `addDerail`.

The C++ class that holds the logic is `InterlockingPlant`.
The appliances and routes you declare on it are the interlocking model.
Together they are the interlocking application, and that application running on a field processor is a field unit.
The variable in these samples is called `cp`.

---

## Step 1: Review the Track Diagram and Data Tables

Here is the signaling diagram for CP Corporal using the standard symbols from Tutorial 1:

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

### Review Your Data Collection Tables
From this plan, you have:
1. **Two Switches and One Derail**: Switch 1 (Industry), Switch 3 (Double-track merge), Derail 5.
2. **Seven Track Circuits**: OS track circuits `1T1`, `3T1`, `5T1`; and track circuits `1SAT`, `2SAT`, `1NAT`, `2NAT` outside the OS sections.
3. **Four Signal Masts**: Mainline 2-head mast `2NAB`, dwarf `2SA`, industry signals `4NA` and `4SA`.

---

## Step 2: Declare the Appliances

Open your sketch and include the library:

```cpp
#include <FieldUnit.h>

using namespace FieldUnit;

InterlockingPlant cp("CP_Corporal");
```

### 1. Declare Track Circuits
Every track circuit represents an electrical detection section.
Create the OS track circuits over the switch points and the other track circuits:

```cpp
// OS track circuits over switch points
auto tc1T1 = cp.addTrackCircuit("1T1"); // Switch 1 points
auto tc3T1 = cp.addTrackCircuit("3T1"); // Switch 3 points
auto tc5T1 = cp.addTrackCircuit("5T1"); // Derail 5

// Track circuits outside the OS sections
auto tc1SAT = cp.addTrackCircuit("1SAT"); // MT1, southbound approach
auto tc2SAT = cp.addTrackCircuit("2SAT"); // MT2
auto tc1NAT = cp.addTrackCircuit("1NAT"); // Single track, approach
auto tc2NAT = cp.addTrackCircuit("2NAT"); // Single track, beyond 1NAT
```

### 2. Declare Switches and Detector Locks
A switch drives a motor and reads position contacts.
On a real railroad, you must never throw a switch while a train occupies the points.
Bind each switch to its OS track circuit:

```cpp
auto sw1 = cp.addSwitch("1");
auto sw3 = cp.addSwitch("3");
auto sw5 = cp.addDerail("5"); // a derail is a switch-shaped appliance

// Enforce detector locking
cp.bindDetectorLock(sw1, tc1T1);
cp.bindDetectorLock(sw3, tc3T1);
cp.bindDetectorLock(sw5, tc5T1);
```

FieldUnit now protects these appliances.
If a train shunts `tc3T1`, the field unit does not act on a control to move `sw3`.
It sends no refusal.
The office learns that the switch did not move because the office indications do not agree with the lever.

For a derail, NORMAL is the derailing position and REVERSE is the clear position.
A route through derail 5 therefore asks for `SwitchPosition::REVERSE`.

You can declare the OS track circuit and the detector lock in one call: `cp.addSwitch("1", "1T1")` and `cp.addDerail("5", "5T1")` create the track circuit if it does not exist and bind it.
See **[How-To: Derails and OS Binding](../how-to/05_derails_and_os_binding.md)**.

### 3. Declare Signals
A signal has two parts in FieldUnit:
- **`SignalControl`**: Holds the direction (`LEFT`, `RIGHT`, `STOP`) that the dispatcher's signal lever asked for.
- **`SignalMast`**: Represents the physical wayside mast with its heads and lamps.

```cpp
// Signal controls, one for each signal lever
auto sig2 = cp.addSignalControl("2");
auto sig4 = cp.addSignalControl("4");

// Wayside signal masts
auto mast2NAB = cp.addSignalMast("2NAB", MastType::TWO_HEAD); // 2-Head Northbound
auto mast2SA  = cp.addSignalMast("2SA",  MastType::DWARF);    // Southbound Dwarf on MT1
auto mast4NA  = cp.addSignalMast("4NA",  MastType::ONE_HEAD); // Industry exit
auto mast4SA  = cp.addSignalMast("4SA",  MastType::ONE_HEAD); // Industry entrance
```

---

## Step 3: Write the Interlocking Control Table

The Interlocking Control Table defines the valid routes through your plant.
Each route connects five facts:
1. Which signal lever gives the direction?
2. Which direction must traffic flow?
3. Which mast shows the signal, and what is the most favorable signal indication it may show (the indication ceiling)?
4. What switch positions must be locked?
5. What track circuits must be clear?

Use FieldUnit's fluent builder to define your routes:

```cpp
// Route 1: Northbound from Single Track to MT2 (Right-Hand Running)
cp.route("MT-NB")
  .governedBy(sig2, DirectionAuthority::LEFT)
  .displays(mast2NAB, Indication::CLEAR)
  .aligns({ {sw1, SwitchPosition::NORMAL},
            {sw3, SwitchPosition::NORMAL} })
  .clears({ tc3T1, tc1T1, tc2SAT })
  .entrance(tc3T1);

// Route 2: Northbound from Single Track to MT1 (Diverging Reverse Running)
cp.route("MT-NB-REV")
  .governedBy(sig2, DirectionAuthority::LEFT)
  .displays(mast2NAB, Indication::DIVERGING_RESTRICTING)
  .aligns({ {sw3, SwitchPosition::REVERSE} })
  .clears({ tc3T1, tc1SAT })
  .entrance(tc3T1);

// Route 3: Southbound from MT1 through Switch 3 onto Single Track
cp.route("SB-MT")
  .governedBy(sig2, DirectionAuthority::RIGHT)
  .displays(mast2SA, Indication::CLEAR)
  .aligns({ {sw3, SwitchPosition::REVERSE} })
  .clears({ tc3T1, tc1NAT })
  .entrance(tc1SAT)
  .approaching(tc2NAT);
```

Notice what you did not write:
- You did not write nested `if` statements.
- You did not calculate binary masks.
- You wrote the route once in plain railroad language.

---

## Step 4: Describe the Code Line Functions

The code line carries controls to the field unit and office indications back to the office.
An `AarTextCodec` writes each function as a token, such as `3NWS` or `1T1K`.
Tell the codec which appliances to carry, in order:

```cpp
AarTextCodec codec;

codec.decodeControls({
    decodeSwitch(sw1), decodeSwitch(sw3), decodeSwitch(sw5),
    decodeSignal(sig2), decodeSignal(sig4),
    decodeMaintainer(0)
});

codec.encodeIndications({
    encodeSwitch(sw1), encodeSwitch(sw3), encodeSwitch(sw5),
    encodeTrack(tc1T1), encodeTrack(tc3T1), encodeTrack(tc5T1),
    encodeSignal(sig2), encodeSignal(sig4),
    encodeMaintainer(0)
});
```

---

## Step 5: Run the Vital Cycle

Advance the plant once for each pass of `loop()` using non-blocking time:

```cpp
void executeCycle(CodeLine& line, uint32_t nowMs) {
    // 1. Ingress: read a control transaction from the code line
    char rxBuffer[256];
    size_t bytesRead = 0;
    if (line.receiveControlPacket(reinterpret_cast<uint8_t*>(rxBuffer), sizeof(rxBuffer) - 1, bytesRead)) {
        rxBuffer[bytesRead] = '\0';
        ControlTransaction ctl;
        if (codec.decodeControls(rxBuffer, ctl)) {
            cp.applyControlTransaction(ctl, nowMs);
        }
    }

    // 2. Evaluate locks, switch correspondence and route logic
    cp.tick(nowMs);

    // 3. Egress: report the office indications
    IndicationVector ind;
    cp.exportIndicationVector(ind);
    char txBuffer[256];
    size_t txLen = 0;
    if (codec.encodeIndications(ind, txBuffer, sizeof(txBuffer), txLen)) {
        line.transmitIndicationPacket(reinterpret_cast<const uint8_t*>(txBuffer), txLen);
    }
}
```

The `CodeLine` is the transport (for example `MockCodeLine` on your desktop, or `MqttCodeLine`).
A sketch with hardware samples its input drivers before `cp.tick()` and runs its output drivers after it.
`examples/CP_Corporal/CP_Corporal.ino` shows how.

---

## Step 6: What FieldUnit Does Automatically

Because you defined the plant using FieldUnit:

1. **Automatic Signal Knockdown**:
   When a train accepts the signal at `mast2NAB` and enters `tc3T1`, the signal drops to Stop immediately.

2. **Standard Stick Memory**:
   When the train departs, the signal stays at Stop.
   The signal will not clear again until the dispatcher sends a new control.

3. **Detector Lock Protection**:
   If the dispatcher tries to throw Switch 3 while a train is on `tc3T1`, the field unit does not act on the control.
   The switch motor does not move.

4. **Multi-Head Display**:
   - For Route 1 (straight), `head1()` of the mast is Green and `head2()` is Red. The signal shows Green over Red (Clear).
   - For Route 2 (diverging), `head1()` is Red and `head2()` is Lunar. The signal shows Red over Lunar (Diverging Restricting).

---

## Next Steps

- **[How-To: Data Collection and Code Line Functions](../how-to/01_data_collection_and_aar_bits.md)**: Connect real Tortoise motors and DCCOD detectors.
- **[How-To: JMRI Integration](../how-to/02_jmri_and_cmri_integration.md)**: Control CP Corporal from a JMRI CTC machine.
