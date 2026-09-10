# How-To: Semaphores and Eastern Speed Signaling

This guide explains how to model mechanical semaphore signals with hobby servos and how to configure Eastern Railroad speed signaling and position-light systems in FieldUnit.

---

## 1. Modeling Semaphores (Servo-Actuated Blades)

On a prototype railroad, semaphores use mechanical blades with colored spectacle glasses moving in front of an oil or electric lamp.
On model layouts, semaphores are actuated by micro-servos (e.g. SG90) or multi-channel servo controllers (e.g. PCA9685 I2C boards).

FieldUnit provides a dedicated hardware driver for mechanical blades: **`SemaphoreDriver`**.

### Blade Positions (Angles)
- **Stop (Horizontal)**: 0 degrees (Red spectacle).
- **Approach / Caution (45 deg diagonal)**: 45 degrees (Yellow spectacle).
- **Clear / Proceed (90 deg vertical)**: 90 degrees (Green spectacle).

### Configuring a Two-Blade Semaphore Mast

```cpp
#include <FieldUnit.h>

using namespace FieldUnit;

ControlPoint cp("CP_Junction");

// 1. Declare the two-arm semaphore mast
auto semMast = cp.addSignalMast("SEM_2R", MastType::TWO_HEAD);

// 2. Set the semaphore rulebook policy
semMast->setAspectPolicy(AspectPolicies::upperQuadrantSemaphore);

// 3. Configure the Semaphore Driver with calibrated servo angles
SemaphoreDriver semDriver(semMast);

// Arm 0 (Top blade): Device 0 (PCA9685 board), Channel 0 (Stop=0 deg, Approach=45 deg, Clear=90 deg)
semDriver.addArm(/*device=*/0, /*channel=*/0, /*stop=*/0, /*approach=*/45, /*clear=*/90);

// Arm 1 (Lower blade): Device 0, Channel 1
semDriver.addArm(/*device=*/0, /*channel=*/1, /*stop=*/0, /*approach=*/45, /*clear=*/90);
```

### Driving Servos in `loop()`

```cpp
void loop() {
    uint32_t nowMs = millis();

    // Advance interlocking logic
    cp.tick(nowMs);

    // Output calibrated PWM angles to servos
    semDriver.drive(hardwareBus);
}
```

When a route clears:
- **Mainline Route (`CLEAR`)**: Top blade moves to 90 degrees; lower blade stays at 0 degrees.
- **Diverging Route (`DIVERGING_CLEAR`)**: Top blade stays at 0 degrees; lower blade moves to 90 degrees.
- **Stop (`STOP`)**: Both blades return to 0 degrees horizontal.

---

## 2. Eastern Speed Signaling (New York Central 3-Head Masts)

Eastern railroads (New York Central, DL&W, Reading, Erie, Conrail, and NORAC) used **Speed Signaling** rather than Western Route Signaling.
Signals inform the engineer of the maximum authorized speed through the interlocking rather than the assigned track route:
- **Head 0 (Top)**: High / Normal Speed.
- **Head 1 (Middle)**: Medium Speed (typically 30 mph through turnouts).
- **Head 2 (Bottom)**: Slow Speed (typically 15 mph) or Restricting.

### Configuring a 3-Head NYC Speed Signal

```cpp
// 1. Declare three-head interlocking home signal
auto mastNYC = cp.addSignalMast("NYC_HOME", MastType::THREE_HEAD);

// 2. Assign the NYC Speed Signaling policy
mastNYC->setAspectPolicy(AspectPolicies::nycSpeed);

// 3. Connect hardware driver (pins for all 3 heads)
SignalMastDriver mastDriver(mastNYC);
mastDriver.addHead(h0Red, h0Yellow, h0Green);
mastDriver.addHead(h1Red, h1Yellow, h1Green);
mastDriver.addHead(h2Red, h2Yellow, h2Green);
```

### How Routes Drive Eastern Speed Indications

In your Interlocking Control Table, specify the prototype speed indication:

```cpp
// High Speed Mainline: CLEAR -> Green over Red over Red (Rule 281)
cp.route("MAIN_HIGH")
  .governedBy(sig2, DirectionAuthority::RIGHT)
  .displays(mastNYC, Indication::CLEAR)
  .aligns({ {sw1, SwitchPosition::NORMAL} })
  .clears({ tc1T1 });

// Medium Speed Turnout (#15 Turnout): MEDIUM_CLEAR -> Red over Green over Red (Rule 283)
cp.route("TURNOUT_MEDIUM")
  .governedBy(sig2, DirectionAuthority::RIGHT)
  .displays(mastNYC, Indication::MEDIUM_CLEAR)
  .aligns({ {sw1, SwitchPosition::REVERSE} })
  .clears({ tc1T1, tc2T1 });

// Slow Speed Track (#8 Turnout): SLOW_CLEAR -> Red over Red over Green (Rule 287)
cp.route("YARD_SLOW")
  .governedBy(sig2, DirectionAuthority::RIGHT)
  .displays(mastNYC, Indication::SLOW_CLEAR)
  .aligns({ {sw3, SwitchPosition::REVERSE} })
  .clears({ tc3T1 });
```

---

## 3. Pennsylvania Railroad (PRR) Position-Light Signals

PRR signals communicate indications using geometry (angles of amber light rows) instead of colors:
- **Horizontal**: Stop.
- **45° Diagonal Right**: Approach / Caution.
- **Vertical**: Clear.

### Pin Mapping Strategy
Wire each row pair to the standard color pins:
- Horizontal lamp pair $\implies$ `redPin`
- Diagonal lamp pair $\implies$ `yellowPin`
- Vertical lamp pair $\implies$ `greenPin`

### Configuring PRR Rulebook Policy

```cpp
auto prrMast = cp.addSignalMast("PRR_HOME", MastType::TWO_HEAD);
prrMast->setAspectPolicy(AspectPolicies::prrPositionLight);
```

Aspect resolution:
- **Clear**: Vertical over Dark (`Aspect::GREEN`, `Aspect::DARK`).
- **Approach**: Diagonal over Dark (`Aspect::YELLOW`, `Aspect::DARK`).
- **Medium Clear**: Horizontal over Vertical (`Aspect::RED`, `Aspect::GREEN`).
- **Stop**: Horizontal over Dark (`Aspect::RED`, `Aspect::DARK`).
