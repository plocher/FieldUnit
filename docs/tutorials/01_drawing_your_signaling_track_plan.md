# Tutorial 1: Drawing Your Signaling Track Plan

Before writing code or soldering wires, you must create a signaling track diagram.
A signaling track diagram is different from a general track plan.
It shows the electrical boundaries, switch point orientations, and signal governing directions that FieldUnit requires.

This tutorial teaches you how to draw your signaling diagram and identify the required field data.

---

## 1. The Four Essential Markings

Every railroad signaling diagram uses four standard symbols:

```
1. Track Rails:                    ═════════════════════════════════
2. Insulated Rail Joint (IRJ):     ][   (Joint bars bolted across gap)
3. Track Switch Points:            ═══════\═════════  (Normal straight,
                                           \          Reverse diverging)
4. Wayside Signals:
     Standing Below Track:         |-o     (Faces Eastbound trains on engineer's right)
                                   |-oo    (Two Heads, faces Eastbound)
     Standing Above Track:         o-|     (Faces Westbound trains on engineer's right)
                                   oo-|    (Two Heads, faces Westbound)
```

---

## 2. Step-by-Step Diagramming Rules

### Rule 1: Establish Timetable Direction
Prototype railroads operate by timetable direction: **East/South** versus **West/North**.
Always mark the timetable directions at the ends of your diagram:

```
< Railroad West / North                              Railroad East / South >
```

- Movement to the right is **Eastward / Southward**.
- Movement to the left is **Westward / Northward**.

---

### Rule 2: Place Insulated Rail Joints and Optical Fouling Sensors

A common model railroad error is placing rail gaps in the wrong location around a switch.
You must insulate the switch into its own dedicated detection block called the **Island** or **OS Block** (On-Sheet / Occupied Section):

```
       Approach Block 1SA           Island Block 1T1           Exit Block 1NA
   ══════════════════════════][═════════════\══════════════][════════════════
                                             \
                                              \════════════][════════════════
                                              (d)              Exit Block 2NA
                                        Optical Sensor (d)
                                        at Fouling Point
```

#### Why Gaps Must Clear the Fouling Point
If a freight car stands on the switch frog or siding curve, it physically blocks ("fouls") the adjacent track.
The insulated rail joints on both branches must sit far enough past the frog so that a car inside the island block is detected before it collides with a train on the adjacent track.

#### The Model Railroad Reality: Optical Fouling Sensors and Hysteresis
On a real railroad, all wheelsets are steel and conduct track current.
On a model railroad, unpowered freight cars and plastic wheelsets **do not conduct current**.
If an engine sets out three unpowered boxcars over a switch frog, a current-sensing detector (such as a DCCOD) will report the track as `VACANT`!

To prevent throwing switches underneath undetected cars or clearing trains into them, modelers install **Optical Sensors** (infrared reflectance or phototransistors, marked with `(d)`):
1. Place an optical sensor between the rails right at the switch frog and clearance point.
2. The optical sensor detects the physical car body even when wheels do not conduct current.

#### Bridging the Inter-Car Optical Gap (Dropout Delay)
When a train moves over an optical sensor, the gap between cars or couplers causes the infrared reflection to break momentarily.
Without filtering, the sensor flickers to `VACANT` between cars, which would drop vital route locks mid-train.

FieldUnit solves this with a built-in, non-blocking **dropout delay** (hysteresis):
- Shunting to `OCCUPIED` happens **instantly** (vital safety: zero delay when a train arrives).
- Clearing to `VACANT` is delayed by a configured hold-off time (for example, 2000 ms).
- If another car body covers the sensor before 2000 ms expires, the timer resets and the block stays continuously `OCCUPIED`.

In FieldUnit, configure the dropout delay directly when declaring the track circuit:

```cpp
// Add island block with 2000 ms dropout hold-off delay
auto tc1T1 = cp.addTrackCircuit("1T1", /*dropoutDelayMs=*/2000);
```

---

### Rule 3: Place and Name Your Signals
Wayside signals protect the entrances to the interlocking.
Place a signal at every entrance to the island block, outside the insulated rail joints:

```
                                                           Signal 2LA
                                                              o-| (Dwarf)
  <== West ════════][═══════════════════════\══════════════][════════ East ==>
                    1T1 Island               \
                   |-oo                       \════════════][════════ Siding
                Signal 2R                                      o-|
               (Two Heads)                                   Signal 2LB (Dwarf)
```

#### Numbering Conventions: Panel Levers vs. Mileposts
There are two common ways to number switches and signals:

1. **Panel Lever Numbering (Classic US&S / GRS)**:
   Switches and signals are numbered sequentially along the CTC panel:
   - Signals governing movement to the **Right (Eastward)** receive the **`R`** suffix (`2R`).
   - Signals governing movement to the **Left (Westward)** receive the **`L`** suffix (`2LA`, `2LB`).
   - Suffix **`A`** denotes the main track; suffix **`B`** denotes the siding or diverging route.

2. **Milepost Numbering (Southern Pacific / Western Roads)**:
   Switches and signals use the physical timetable milepost location.
   On the SP Coast Line, switches at CP Christopher (MP 77.8) are numbered `SW777` and `SW781`.
   Signals carry the milepost prefix with directional letters (for example, `782R` and `782L`).

FieldUnit supports both conventions because names are user-defined strings.

---

### Rule 4: Multi-Head Masts and Signaling Regimes

When a signal governs a facing-point switch, a train can proceed along two different paths.
The mast requires multiple heads:

```
  |-OO  Signal 2R (Two-Head Mast):
    O   Top Head (A): Governs the straight mainline path.
    O   Lower Head (B): Governs the diverging siding path.
```

The meaning of multiple heads depends on your railroad's signaling regime:
- **Route Signaling**: Heads display the assigned track path.
  The top head displays aspects for the main route; the lower head displays aspects for the diverging route.
- **Speed Signaling**: Heads display allowable speed zones through the plant.
  The top head governs maximum track speed; the lower head governs medium or slow speed through turnouts.

Trailing-point entrances (where two tracks merge into one) only need to convey a subset of indications (leaving the track).
Therefore, trailing entrances typically use a single-head mast or dwarf signal (`2LA`, `2LB`).

---

## 3. The Complete Working Example: CP End-of-Siding

Combining these rules gives the complete signaling diagram for a single-track passing siding:

```
< Railroad West (North)                              Railroad East (South) >

                                                      Signal 2LA
                                                         o-| (Dwarf)
  Main Track <==== 1SA ══════════][═════════════════\══════════════][════ 1NA ====>
                  (Approach)          1T1 Island     \                  (Exit Block)
                                 |-oo                 \
                              Signal 2R                \
                             (Two Heads)                \
  Siding Track <═════════════════════════════════════════\═════════][════ 2NA ====>
                                                         (d)             o-|
                                                       Optical        Signal 2LB
                                                       Sensor          (Dwarf)
```

---

## 4. Extracting Your Data Collection Sheet

From this completed diagram, write down the three tables that FieldUnit requires:

### 1. Switches Table
| Switch ID | Normal Path | Reverse Path | Island Track Circuit | Speed Limit |
|---|---|---|---|---|
| `SW1` | Straight to Main | Diverging to Siding | `1T1` | #10 Turnout (Slow / 15 mph) |

### 2. Track Circuits Table
| Track Circuit ID | Role | Physical Location | Dropout Delay |
|---|---|---|---|
| `1T1` | Island / OS Block | Over Switch 1 points and frog | 2000 ms (Optical + DCCOD) |
| `1SA` | Approach Circuit | Mainline west of Signal 2R | 0 ms (Current detection) |
| `1NA` | Exit / Advance Block | Mainline east of Switch 1 | 0 ms (Current detection) |
| `2NA` | Exit Block | Siding east of Switch 1 | 0 ms (Current detection) |

### 3. Signal Masts Table
| Mast ID | Type | Facing Direction | Governing Authority | Heads |
|---|---|---|---|---|
| `2R` | Two-Head Mast | Eastward (Right) | `SIG2` (Direction `RIGHT`) | Top: Main, Lower: Siding |
| `2LA` | Dwarf Mast | Westward (Left) | `SIG2` (Direction `LEFT`) | Single Head: Main to Single |
| `2LB` | Dwarf Mast | Westward (Left) | `SIG2` (Direction `LEFT`) | Single Head: Siding to Single |

---

## 5. You Are Ready to Build

With this data collection sheet completed:
1. You know every track circuit and dropout delay to instantiate.
2. You know which switch is detector-locked by which island block.
3. You know the exact routes to write in your Interlocking Control Table.

Proceed to **[Tutorial 2: Building Your First Control Point](02_building_your_first_cp.md)** to turn this plan into working C++ code.

---

## 2. Step-by-Step Diagramming Rules

### Rule 1: Establish Timetable Direction
Prototype railroads operate by timetable direction: **East/South** versus **West/North**.
Always mark the timetable directions at the ends of your diagram:

```
< Railroad West / North                              Railroad East / South >
```

- Movement to the right is **Eastward / Southward**.
- Movement to the left is **Westward / Northward**.

---

### Rule 2: Place Insulated Rail Joints and Optical Fouling Sensors (The Island Block)
A common model railroad error is placing rail gaps in the wrong location around a switch.
You must insulate the switch into its own dedicated detection block called the **Island** or **OS Block** (On-Sheet / Occupied Section):

```
       Approach Block                                    Exit Block
   ======================||=====+======================||==========
                          ^      \                      ^
                          |       \===||=============== |
                    Gaps Before    d   ^           Gaps After
                    Switch Points  |   |           Fouling Point
                                   | Gaps Past
                                   | Fouling Point
                                   v
                     Optical Detector ('d') at Clearance Point
```

#### Why Gaps Must Clear the Fouling Point
If a freight car stands on the switch frog or siding curve, it physically blocks ("fouls") the adjacent track.
The insulated rail joints on both branches must sit far enough past the frog so that a car inside the island block is detected before it collides with a train on the adjacent track.

#### The Model Railroad Reality: Optical Fouling Sensors
On a real railroad, all wheelsets are steel and conduct electricity.
On a model railroad, unpowered freight cars and plastic wheelsets **do not conduct track current**.
If an engine sets out three unpowered boxcars over a switch frog, a current-sensing detector (such as a DCCOD) will show the track as `VACANT`!

To prevent throwing switches underneath undetected cars or clearing trains into them, modelers install **Optical Sensors** (infrared reflectance or phototransistors, marked with `d`):
- Place an optical detector between the rails right at the switch frog and clearance points.
- The optical sensor detects the physical car body even when wheels do not conduct current.
- FieldUnit combines the current detector and the optical sensors into a single vital detection block:
  $$\text{Island Occupied} = \text{CurrentDetector} \lor \text{OpticalSensor}_1 \lor \text{OpticalSensor}_2$$

Label this island detection block with the switch number plus `T`:
- For Switch 1: the island is **`1T`** (or **`1T1`**).
- For Switch 3: the island is **`3T`** (or **`3T1`**).

---

### Rule 3: Place and Name Your Signals
Wayside signals protect the entrances to the interlocking.
Place a signal at every entrance to the island block, outside the insulated rail joints:

```
               Signal 2R                               Signal 2LA
                 |-OO                                     O-|
  <== West ======+===||====================+========||====+====== East ==>
                 ^   1T1 (Island Circuit)  ^        1NA (Exit)
                 |                         |
                 |                         \========||====+====== Siding
                 |                                  2SA   O-|
            Gaps at Entrance                            Signal 2LB
```

#### The Prototype Lever Naming Rule
In CTC territory, signals are numbered by their dispatcher control lever:
- Signals governing movements to the **Right (Eastward)** receive the **`R`** suffix (for example, `2R`).
- Signals governing movements to the **Left (Westward)** receive the **`L`** suffix (for example, `2L`).
- When multiple tracks enter from the same side:
  - The main track signal is **`A`** (for example, `2LA`).
  - The siding or diverging signal is **`B`** (for example, `2LB`).

---

### Rule 4: Multi-Head Masts for Diverging Routes
When a signal governs a facing-point switch (a switch where a train can split into two paths), the mast requires multiple heads:

```
  |-OO  Signal 2R (Two-Head Mast):
    O   Top Head (A): Governs the straight mainline route (Green over Red = Clear).
    O   Lower Head (B): Governs the diverging siding route (Red over Green = Diverging Clear).
```

Trailing-point entrances (where two tracks merge into one) only need single-head masts or dwarfs because there is only one path through the switch.

---

## 3. The Complete Working Example: CP End-of-Siding

Combining these rules gives the complete signaling diagram for a single-track passing siding:

```
< Railroad West (North)                              Railroad East (South) >

                             Signal 2R
                               |-OO (Two Heads)
  Main Track <==== 1SA ========+===||====================+========||==== 1NA ====>
                  (Approach)   ^   1T1 (Island Block)    \        ^   (Exit Block)
                               |                          \       |
                               |    Switch 1 (Normal: ----)\      |
                               |             (Reverse: \  ) \     |
                               |                             \    |
                               |                  Signal 2LA  \   |
                               |                     O-|       \  |
                               |                     (Dwarf)    \ |
  Siding Track <=================================================+=||==== 2NA ====>
                                                                 ^
                                                            Signal 2LB (Dwarf)
```

---

## 4. Extracting Your Data Collection Sheet

From this completed diagram, write down the three tables that FieldUnit requires:

### 1. Switches Table
| Switch ID | Normal Path | Reverse Path | Island Track Circuit | Speed Limit |
|---|---|---|---|---|
| `SW1` | Straight to Main | Diverging to Siding | `1T1` | #10 Turnout (Slow / 15 mph) |

### 2. Track Circuits Table
| Track Circuit ID | Role | Physical Location | Polarity |
|---|---|---|---|
| `1T1` | Island / OS Block | Over Switch 1 points and frog | Active-Low (DCCOD) |
| `1SA` | Approach Circuit | Mainline west of Signal 2R | Active-Low (DCCOD) |
| `1NA` | Exit / Advance Block | Mainline east of Switch 1 | Active-Low (DCCOD) |
| `2NA` | Exit Block | Siding east of Switch 1 | Active-Low (DCCOD) |

### 3. Signal Masts Table
| Mast ID | Type | Facing Direction | Governing Authority | Heads |
|---|---|---|---|---|
| `2R` | Two-Head Mast | Eastward (Right) | `SIG2` (Direction `RIGHT`) | Top: Main, Lower: Siding |
| `2LA` | Dwarf Mast | Westward (Left) | `SIG2` (Direction `LEFT`) | Single Head: Main to Single |
| `2LB` | Dwarf Mast | Westward (Left) | `SIG2` (Direction `LEFT`) | Single Head: Siding to Single |

---

## 5. You Are Ready to Build

With this data collection sheet completed:
1. You know every track circuit to instantiate.
2. You know which switch is detector-locked by which island block.
3. You know the exact routes to write in your Interlocking Control Table.

Proceed to **[Tutorial 1: Building Your First Control Point](01_building_your_first_cp.md)** to turn this plan into working C++ code.
