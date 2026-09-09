# How-To: Data Collection and AAR Bit Mapping

This guide helps layout owners collect the required physical data from their track plan and map it into standard Association of American Railroads (AAR) CodeLine bits.

---

## 1. What Data to Collect

Before writing code, walk your track plan and fill out four collection sheets:

### A. Switches Sheet
For every track switch or crossover:
1. **Name/Number**: e.g. Switch 1, Switch 3.
2. **Motor Pin**: Output pin driving the Tortoise or servo (`T1`).
3. **Normal Sense Pin**: Input pin from points microswitch closed in Normal (`1NW`).
4. **Reverse Sense Pin**: Input pin from points microswitch closed in Reverse (`1RW`).
5. **Detector Island**: The track circuit covering the points (`1T1`).
6. **Frog Number / Speed**: e.g. #10 (Slow), #14 (Medium), #20 (High-Speed).

### B. Track Circuits Sheet
For every detection block:
1. **Name**: Island circuits (`1T1`, `3T1`), Approach circuits (`1SA`, `2SA`), Exit circuits (`1NA`, `2NA`).
2. **Sensor Pin**: Pin on microcontroller or IOX expander.
3. **Polarity**: Active-low (standard for DCCOD open-collector) or active-high.

### C. Signal Masts Sheet
For every signal mast:
1. **Name**: e.g. `2Nab`, `2Sab`.
2. **Type**: One-head, Two-head, Three-head, or Dwarf.
3. **Facing Direction**: Northbound/Leftward or Southbound/Rightward.
4. **Governing Lever**: Which dispatcher signal lever controls authority (`SIG2`).
5. **Head Pins**: Pins for Red, Yellow, Green (and Lunar) LEDs per head.

---

## 2. Standard AAR CodeLine Naming Rules

Real railroads use standard suffix letters to distinguish controls from indications:
- **`S` Suffix = Send / Control**: A command sent from the dispatcher to the field (e.g. `1NWS`, `2SGS`).
- **`K` Suffix = Kontrol / Indication**: Status reported from the field to the dispatcher (e.g. `1NWK`, `2SGK`).

### Common AAR Name Table

| AAR Name | Meaning | Direction | Wire Bit Meaning |
|---|---|---|---|
| **`1NWS`** | Switch 1 Normal Command | Dispatcher $\to$ Field | 1 = Throw Normal |
| **`1RWS`** | Switch 1 Reverse Command | Dispatcher $\to$ Field | 1 = Throw Reverse |
| **`1NWK`** | Switch 1 Normal Correspondence | Field $\to$ Dispatcher | 1 = Points locked Normal |
| **`1RWK`** | Switch 1 Reverse Correspondence | Field $\to$ Dispatcher | 1 = Points locked Reverse |
| **`1T1K`** | Track Circuit 1T1 Occupancy | Field $\to$ Dispatcher | 1 = Block Occupied |
| **`2SGS`** | Signal 2 Southward/Right Authority | Dispatcher $\to$ Field | 1 = Clear Rightward |
| **`2NGS`** | Signal 2 Northward/Left Authority | Dispatcher $\to$ Field | 1 = Clear Leftward |
| **`2HS`**  | Signal 2 Hold / Stop Command | Dispatcher $\to$ Field | 1 = Force Stop |
| **`2SGK`** | Signal 2 Southward Indication | Field $\to$ Dispatcher | 1 = Signal displaying Right |
| **`2NGK`** | Signal 2 Northward Indication | Field $\to$ Dispatcher | 1 = Signal displaying Left |
| **`2TEK`** | Signal 2 Time Element (Time Lock) | Field $\to$ Dispatcher | 1 = Safety timer running |
| **`MC1S`** | Maintainer Call Command | Dispatcher $\to$ Field | 1 = Turn lamp ON |
| **`MC1K`** | Maintainer Call Indication | Field $\to$ Dispatcher | 1 = Lamp is lit |

---

## 3. Creating Your CodeLine Wire Map

Once you assign names, map them to specific byte and bit positions in your `CodeLineCodec`.

Here is the standard 2-byte Control and 4-byte Indication layout:

### Controls (2 Bytes)
```
Byte 0: [ 1NW, 1RW, 3NW, 3RW, 5NW, 5RW, 7NW, 7RW ]
Byte 1: [ 2SG, 2NG,  2H,   - , 4SG, 4NG,  4H, MC1  ]
```

### Indications (4 Bytes)
```
Byte 0: [ 1NWK, 1RWK, 3NWK, 3RWK, 5NWK, 5RWK, 7NWK, 7RWK ]
Byte 1: [ 1T1,  3T1,  5T1,  7T1,  1SA,  2SA,  1NA,  2NA  ]
Byte 2: [ 2SGK, 2NGK, 2TEK,  - , 4SGK, 4NGK, 4TEK, MC1K ]
Byte 3: [ IND1, IND2, IND3,  - ,   - ,   - ,   - ,   -   ]
```

### In Your C++ Sketch

```cpp
CodeLineCodec codec(2 /* control bytes */, 4 /* indication bytes */);

// Map Switch 1 controls on Byte 0
codec.mapSwitchControl(0, /*byte*/ 0, /*normBit*/ 0, /*byte*/ 0, /*revBit*/ 1);

// Map Signal 2 controls on Byte 1
codec.mapSignalControl(0, /*byte*/ 1, /*southBit*/ 0, /*northBit*/ 1, /*stopBit*/ 2);

// Map Switch 1 indications on Byte 0
codec.mapSwitchIndication(0, /*byte*/ 0, /*normBit*/ 0, /*byte*/ 0, /*revBit*/ 1);

// Map Track Circuit 1T1 indication on Byte 1 bit 0
codec.mapTrackIndication(0, /*byte*/ 1, /*bit*/ 0);
```

FieldUnit validates the wire bits automatically:
- If noise sets both `1NW` and `1RW` to 1, the codec marks the packet corrupted and rejects it.
- If multiple signal directions are set to 1, the codec marks the packet corrupted and rejects it.
