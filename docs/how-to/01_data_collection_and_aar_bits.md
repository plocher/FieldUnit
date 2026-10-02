# How-To: Data Collection and Code Line Functions (AAR Names)

This guide helps layout owners collect the required physical data from their track plan and map it into the names and wire positions of the code line functions.
The names follow the Association of American Railroads (AAR) style.

---

## 1. What Data to Collect

Before writing code, walk your track plan and fill out four collection sheets:

### A. Switches Sheet
For every track switch or crossover:
1. **Name/Number**: e.g. Switch 1, Switch 3.
2. **Motor Pin**: Output pin driving the Tortoise or servo (`T1`).
3. **Normal Sense Pin**: Input pin from points microswitch closed in Normal (`1NW`).
4. **Reverse Sense Pin**: Input pin from points microswitch closed in Reverse (`1RW`).
5. **OS Track Circuit**: The track circuit covering the points (`1T1`).
6. **Frog Number / Speed**: e.g. #10 (Slow), #14 (Medium), #20 (High-Speed).

### B. Track Circuits Sheet
For every track circuit:
1. **Name**: OS track circuits (`1T1`, `3T1`), approach track circuits (`1SA`, `2SA`), and other track circuits (`1NA`, `2NA`).
2. **Sensor Pin**: Pin on microcontroller or IOX expander.
3. **Polarity**: Active-low (standard for DCCOD open-collector) or active-high.

### C. Signal Masts Sheet
For every signal mast:
1. **Name**: e.g. `2Nab`, `2Sab`.
2. **Type**: One-head, Two-head, Three-head, or Dwarf.
3. **Facing Direction**: Northbound/Leftward or Southbound/Rightward.
4. **Signal Control**: Which signal lever gives the direction (`2`).
5. **Head Pins**: Pins for Red, Yellow, Green (and Lunar) LEDs per head.

---

## 2. Names of Code Line Functions

Each function on the code line has a name, called a token.
A token is the appliance name plus letters that say what the function does.
FieldUnit uses these final letters to tell controls from office indications:
- **Final `S` = control**: a function sent from the office to the field (e.g. `1NWS`, `2SGS`).
  This is a FieldUnit convention. The AAR list reads `S` as south, stick or storage.
- **Final `K` = office indication**: a function sent from the field to the office (e.g. `1NWK`, `2SGK`).
  The AAR list gives `K` as "indicator".

The letters in front of the final letter also carry meaning. The AAR letters `NW` and `RW` mean switch normal and switch reverse.
In `NGS`, `SGS`, `NGK` and `SGK`, FieldUnit uses `N` for LEFT and `S` for RIGHT.
See section 10.3 of the [Glossary](../GLOSSARY.md) for the full table and the origin of each letter.

### Common Token Table

| Token | Meaning | Direction | Value on the wire |
|---|---|---|---|
| **`1NWS`** | Switch 1 Normal control | Dispatcher $\to$ Field | 1 = throw Normal |
| **`1RWS`** | Switch 1 Reverse control | Dispatcher $\to$ Field | 1 = throw Reverse |
| **`1NWK`** | Switch 1 office indication: Normal, in switch correspondence | Field $\to$ Dispatcher | 1 = points detected Normal and in switch correspondence |
| **`1RWK`** | Switch 1 office indication: Reverse, in switch correspondence | Field $\to$ Dispatcher | 1 = points detected Reverse and in switch correspondence |
| **`1T1K`** | Track circuit 1T1 office indication | Field $\to$ Dispatcher | 1 = occupied |
| **`2SGS`** | Signal 2 control: RIGHT | Dispatcher $\to$ Field | 1 = RIGHT |
| **`2NGS`** | Signal 2 control: LEFT | Dispatcher $\to$ Field | 1 = LEFT |
| **`2HS`**  | Signal 2 control: put the signal at stop | Dispatcher $\to$ Field | 1 = stop |
| **`2SGK`** | Signal 2 office indication: RIGHT | Field $\to$ Dispatcher | 1 = RIGHT is the active direction |
| **`2NGK`** | Signal 2 office indication: LEFT | Field $\to$ Dispatcher | 1 = LEFT is the active direction |
| **`2TEK`** | Signal 2 office indication: time locking runs | Field $\to$ Dispatcher | 1 = timer running |
| **`MC1S`** | Maintainer call control | Dispatcher $\to$ Field | 1 = call the maintainer |
| **`MC1K`** | Maintainer call office indication | Field $\to$ Dispatcher | 1 = call lamp lit |

---

## 3. Creating Your Code Line Wire Map

Once you assign names, map them to specific byte and bit positions in your `CodeLineCodec`.

Here is the standard 2-byte control and 4-byte office indication layout:

### Controls (2 Bytes)
```
Byte 0: [ 1NW, 1RW, 3NW, 3RW, 5NW, 5RW, 7NW, 7RW ]
Byte 1: [ 2SG, 2NG,  2H,   - , 4SG, 4NG,  4H, MC1  ]
```

### Office Indications (4 Bytes)
```
Byte 0: [ 1NWK, 1RWK, 3NWK, 3RWK, 5NWK, 5RWK, 7NWK, 7RWK ]
Byte 1: [ 1T1,  3T1,  5T1,  7T1,  1SA,  2SA,  1NA,  2NA  ]
Byte 2: [ 2SGK, 2NGK, 2TEK,  - , 4SGK, 4NGK, 4TEK, MC1K ]
Byte 3: [ IND1, IND2, IND3,  - ,   - ,   - ,   - ,   -   ]
```

### In Your C++ Sketch

The first argument of every `map...` call is the index of the appliance in the order you declared it on the `InterlockingPlant` (switch 0 is the first switch you declared).

```cpp
CodeLineCodec codec(2 /* control bytes */, 4 /* indication bytes */);

// mapSwitchControl(switchIndex, normalByte, normalBit, reverseByte, reverseBit)
// Switch 1 controls on byte 0
codec.mapSwitchControl(0, 0, 0, 0, 1);

// mapSignalControl(signalIndex, byte, rightBit, leftBit, stopBit)
// Signal 2 controls on byte 1
codec.mapSignalControl(0, 1, 0, 1, 2);

// mapSwitchIndication(switchIndex, normalByte, normalBit, reverseByte, reverseBit)
// Switch 1 office indications on byte 0
codec.mapSwitchIndication(0, 0, 0, 0, 1);

// mapTrackIndication(trackIndex, byte, bit)
// Track circuit 1T1 office indication on byte 1, bit 0
codec.mapTrackIndication(0, 1, 0);
```

Use the codec to turn the wire bytes into a control transaction, and an indication vector into wire bytes:

```cpp
ControlTransaction ctl;
if (codec.unpackControls(rxBytes, rxLength, ctl)) {
    cp.applyControlTransaction(ctl, nowMs);
}

IndicationVector ind;
cp.exportIndicationVector(ind);
codec.packIndications(ind, txBytes, sizeof(txBytes));
```

FieldUnit validates the wire bits automatically:
- If noise sets both `1NW` and `1RW` to 1, `unpackControls` rejects the packet and marks the control transaction not valid (`vitalValid` is false).
- If more than one of the signal control bits (`2SG`, `2NG`, `2H`) is 1, `unpackControls` rejects the packet in the same way.
