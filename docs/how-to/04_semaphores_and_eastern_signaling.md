# How-To: Semaphores and Eastern Speed Signaling

This guide explains how to model mechanical semaphore signals with hobby servos and how to configure speed signaling, position-light signals and color-position-light signals in FieldUnit.

Each section says what the library code does ("as implemented in FieldUnit").
The aspect tables in this guide come from the aspect policies in `src/SignalAspectPolicy.h` and from the drivers in `src/drivers/`. They are not rulebook text.

---

## 1. Modeling Semaphores (Servo-Actuated Blades)

On model layouts, semaphores are moved by micro-servos (e.g. SG90) or by multi-channel servo controllers (e.g. PCA9685 I2C boards).

FieldUnit provides a driver for mechanical blades: **`SemaphoreDriver`**.
It sends an angle for each arm through `IOBus::writeAngle(device, channel, angle)`.
`MockIOBus` implements `writeAngle`.
The library has no `IOBus` for a PCA9685. To drive real servos, write an `IOBus` of your own that overrides `writeAngle`.

### Blade Positions (Angles)
Each arm has three angles. `addArm` uses these defaults:
- **Stop (horizontal)**: 0 degrees.
- **Approach (diagonal)**: 45 degrees.
- **Clear (vertical)**: 90 degrees.

The driver reads the appearance of one head for each arm: arm 1 follows `head1()`, arm 2 follows `head2()`, arm 3 follows `head3()`.
A Green head gives the clear angle, a Yellow head the approach angle, and every other appearance the stop angle.

### Configuring a Two-Blade Semaphore Mast

```cpp
#include <FieldUnit.h>

using namespace FieldUnit;

InterlockingPlant cp("CP_Junction");
MockIOBus hardwareBus;   // replace with your own IOBus that overrides writeAngle()

// 1. Declare the two-arm semaphore mast
auto semMast = cp.addSignalMast("SEM_2R", MastType::TWO_HEAD);

// 2. Set the semaphore aspect policy
semMast->setAspectPolicy(AspectPolicies::upperQuadrantSemaphore);

// 3. Configure the semaphore driver with calibrated servo angles
SemaphoreDriver semDriver(semMast);

// Arm 1 (top blade): device 0, channel 0 (Stop=0 deg, Approach=45 deg, Clear=90 deg)
semDriver.addArm(/*device=*/0, /*channel=*/0, /*stop=*/0, /*approach=*/45, /*clear=*/90);

// Arm 2 (lower blade): device 0, channel 1
semDriver.addArm(/*device=*/0, /*channel=*/1, /*stop=*/0, /*approach=*/45, /*clear=*/90);
```

`AspectPolicies::upperQuadrantSemaphore` returns what `AspectPolicies::defaultRoute` returns for every signal indication.
The semaphore behavior comes from `SemaphoreDriver`, not from the policy.

### Driving Servos in `loop()`

```cpp
void loop() {
    uint32_t nowMs = millis();

    // Advance interlocking logic
    cp.tick(nowMs);

    // Output calibrated angles to servos
    semDriver.drive(hardwareBus);
}
```

With the default policy and angles, when a route clears:
- **Mainline route (`CLEAR`)**: Arm 1 moves to 90 degrees; arm 2 stays at 0 degrees.
- **Diverging route (`DIVERGING_CLEAR`)**: Arm 1 stays at 0 degrees; arm 2 moves to 90 degrees.
- **Stop (`STOP`)**: Both arms return to 0 degrees.

---

## 2. Speed Signaling (the `nycSpeed` Policy)

Some railroads use **speed signaling** rather than route signaling.
The signal shows the maximum authorized speed through the interlocking rather than the assigned track route.
The policy `AspectPolicies::nycSpeed` implements a three-head form.
As implemented in FieldUnit, a Green lamp in one head position gives the speed class:
- **Head 1 (top)**: Green means Clear.
- **Head 2 (middle)**: Green means Medium Clear.
- **Head 3 (bottom)**: Green means Slow Clear. Yellow in this head position shows Slow Approach and Restricting.

### Configuring a 3-Head Speed Signal

```cpp
// 1. Declare a three-head interlocking home signal with the nycSpeed policy
auto mastNYC = cp.addSignalMast("NYC_HOME", MastType::THREE_HEAD, AspectPolicies::nycSpeed);

// 2. Connect the hardware driver (red, yellow, green pins for each head)
SignalMastDriver mastDriver(mastNYC);
mastDriver.addHead(OutputBit(1, 0, 0), OutputBit(1, 0, 1), OutputBit(1, 0, 2)); // head 1
mastDriver.addHead(OutputBit(1, 0, 3), OutputBit(1, 0, 4), OutputBit(1, 0, 5)); // head 2
mastDriver.addHead(OutputBit(1, 0, 6), OutputBit(1, 0, 7), OutputBit(1, 1, 0)); // head 3
```

### How Routes Drive Speed Signal Indications

In your Interlocking Control Table, name the signal indication that each route may show:

```cpp
// High speed mainline: CLEAR -> Green over Red over Red
cp.route("MAIN_HIGH")
  .governedBy(sig2, DirectionAuthority::RIGHT)
  .displays(mastNYC, Indication::CLEAR)
  .aligns({ {sw1, SwitchPosition::NORMAL} })
  .clears({ tc1T1 });

// Medium speed turnout: MEDIUM_CLEAR -> Red over Green over Red
cp.route("TURNOUT_MEDIUM")
  .governedBy(sig2, DirectionAuthority::RIGHT)
  .displays(mastNYC, Indication::MEDIUM_CLEAR)
  .aligns({ {sw1, SwitchPosition::REVERSE} })
  .clears({ tc1T1, tc2T1 });

// Slow speed track: SLOW_CLEAR -> Red over Red over Green
cp.route("YARD_SLOW")
  .governedBy(sig2, DirectionAuthority::RIGHT)
  .displays(mastNYC, Indication::SLOW_CLEAR)
  .aligns({ {sw3, SwitchPosition::REVERSE} })
  .clears({ tc3T1 });
```

In this policy, Slow Approach and Restricting have the same aspect (Red over Red over Yellow). See Tutorial 3.

---

## 3. Position-Light Signals (the `prrPositionLight` Policy)

A position-light head shows its aspect with the position of its lamps.
FieldUnit's policy `AspectPolicies::prrPositionLight` returns colors, so you wire each lamp row to a color pin of `SignalMastDriver`.
As implemented in FieldUnit:
- The horizontal lamp row goes to the red pin.
- The diagonal lamp row goes to the yellow pin.
- The vertical lamp row goes to the green pin.

### Configuring the Policy

```cpp
auto prrMast = cp.addSignalMast("PRR_HOME", MastType::TWO_HEAD);
prrMast->setAspectPolicy(AspectPolicies::prrPositionLight);
```

The policy returns these head appearances for a two-head mast:
- **Clear**: Vertical over Dark (`Aspect::GREEN`, `Aspect::DARK`).
- **Approach**: Diagonal over Dark (`Aspect::YELLOW`, `Aspect::DARK`).
- **Medium Clear**: Horizontal over Vertical (`Aspect::RED`, `Aspect::GREEN`).
- **Medium Approach** and **Restricting**: Horizontal over Diagonal (`Aspect::RED`, `Aspect::YELLOW`).
- **Stop**: Horizontal over Dark (`Aspect::RED`, `Aspect::DARK`).

---

## 4. Color-Position-Light (CPL) Signals

### What the sources say about B&O CPL markers

A 1925 proposal for B&O color-position-light signals (Railway Signaling, July 1925) describes markers that sit above or below the two red lights:
- "White marker light above two red lights in horizontal line, stop, then proceed; main route."
- "White marker light below two red lights in horizontal line, stop, then proceed; restricted route."

That text is an early proposal, and later practice changed.
A later web summary of CPL practice (read through a fetch tool, not checked against a primary source) says that markers above are high speed, markers below are medium speed, and no marker is slow speed.
No source we opened gives marker meanings by clock position.
No source we opened gives a "limited speed" marker or a "cab speed" marker.
The marker meanings below are therefore FieldUnit's own. They are not B&O practice.

### As implemented in FieldUnit

A CPL mast in FieldUnit has a cluster of four lamp pairs, wired to four pins: red, yellow, green and lunar.
It can also have up to six marker lamps. `CplMarker` in `types.h` names them by clock position:

| `CplMarker` | Position | Code comment in `types.h` |
|---|---|---|
| `TOP_12` | 12 o'clock | Normal speed route |
| `UPPER_R_2` | 2 o'clock | Medium speed route |
| `LOWER_R_4` | 4 o'clock | Limited speed route |
| `BOTTOM_6` | 6 o'clock | Slow speed route |
| `LOWER_L_8` | 8 o'clock | Auxiliary or restricting |
| `UPPER_L_10` | 10 o'clock | Cab speed or advance |

The policy `AspectPolicies::boCpl` never sets `LOWER_R_4` or `LOWER_L_8`.

### Configuring a CPL Signal Mast

```cpp
#include <FieldUnit.h>

using namespace FieldUnit;

InterlockingPlant cp("CP_HarpersFerry");
MockIOBus hardwareBus;   // replace with the IOBus of your hardware

// 1. Declare the signal mast (ONE_HEAD or DWARF)
auto cplMast = cp.addSignalMast("2LA", MastType::ONE_HEAD);

// 2. Assign the CPL aspect policy
cplMast->setAspectPolicy(AspectPolicies::boCpl);

// 3. Connect the CPL hardware driver
CplMastDriver cplDriver(cplMast);

// Cluster lamp pair pins: red, yellow, green, lunar
cplDriver.setDiskPins(
    /*red=*/   OutputBit(1, 0, 0),
    /*yellow=*/OutputBit(1, 0, 1),
    /*green=*/ OutputBit(1, 0, 2),
    /*lunar=*/ OutputBit(1, 0, 3)
);

// Marker pins, in the order top12, upperR2, lowerR4, bottom6 (then optionally upperL10, lowerL8)
cplDriver.setMarkerPins(
    /*top12=*/    OutputBit(1, 0, 4),
    /*upperR2=*/  OutputBit(1, 0, 5),
    /*lowerR4=*/  OutputBit(1, 0, 6),
    /*bottom6=*/  OutputBit(1, 0, 7)
);
```

### Driving CPL Hardware in `loop()`

```cpp
void loop() {
    uint32_t nowMs = millis();

    // Advance interlocking logic
    cp.tick(nowMs);

    // Drive the lamp pairs, the flashing lamps (1 Hz) and the markers
    cplDriver.drive(hardwareBus, nowMs);
}
```

### What `boCpl` Returns (as implemented in FieldUnit)

| Signal indication | Cluster lamp | Marker |
| :--- | :--- | :--- |
| `CLEAR` | Green | `TOP_12` |
| `APPROACH` | Yellow | `TOP_12` |
| `ADVANCE_APPROACH` | Flashing Yellow (1 Hz) | `TOP_12` |
| `MEDIUM_CLEAR`, `DIVERGING_CLEAR` | Green | `UPPER_R_2` |
| `MEDIUM_APPROACH`, `APPROACH_MEDIUM`, `DIVERGING_APPROACH`, `APPROACH_DIVERGING` | Yellow | `UPPER_R_2` |
| `SLOW_CLEAR` | Green | `BOTTOM_6` |
| `SLOW_APPROACH`, `APPROACH_SLOW` | Yellow | `BOTTOM_6` |
| `RESTRICTING`, `APPROACH_RESTRICTING` | Lunar | none |
| `DIVERGING_RESTRICTING` | Lunar | `BOTTOM_6` |
| `STOP` | Red | none |
| `CAB_SPEED` | Green | `UPPER_L_10` |

A dwarf mast has no markers. It shows only the cluster lamp.
