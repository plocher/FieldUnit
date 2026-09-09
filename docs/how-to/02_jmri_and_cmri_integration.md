# How-To: JMRI and C/MRI Integration

This guide explains how to connect FieldUnit Control Points to JMRI (Java Model Railroad Interface) using standard C/MRI serial or network drivers.

---

## 1. The JMRI Architecture Model

JMRI acts as the dispatcher's Control Plane.
FieldUnit acts as the local Control Point in the field.

```
+--------------------------------------------------------------+
|                         JMRI (Host)                          |
|   - CTC Panel (Levers, Code Buttons, Model Board Display)    |
|   - Sends Control Packets (T Packets)                        |
|   - Polls for Indication Packets (P / R Packets)             |
+--------------------------------------------------------------+
                               |
                   C/MRI Serial or TCP Link
                               |
                               v
+--------------------------------------------------------------+
|                   FieldUnit Control Point                    |
|   - CodeLineCodec decodes T packets into ControlTransactions |
|   - Interlocking Control Table evaluates plant safety        |
|   - CodeLineCodec encodes IndicationVectors into R packets   |
+--------------------------------------------------------------+
```

---

## 2. Understanding JMRI C/MRI Address Numbering

In JMRI, C/MRI I/O points use a standard numbering convention:
- **`CT` (C/MRI Turnout / Output)**: Represents Control bits sent from JMRI to FieldUnit.
- **`CS` (C/MRI Sensor / Input)**: Represents Indication bits received by JMRI from FieldUnit.

The numbering formula is:
$$\text{Address} = (\text{Node Address} \times 1000) + (\text{Byte Index} \times 8) + \text{Bit Index} + 1$$

For Node 1 (`UA = 1`):
- `Byte 0, Bit 0` $\implies$ Address `1001`.
- `Byte 0, Bit 7` $\implies$ Address `1008`.
- `Byte 1, Bit 0` $\implies$ Address `1009`.

### JMRI Mapping Table for CP Christopher

| Hardware Role | AAR Name | Packet Type | Wire Location | JMRI System Name |
|---|---|---|---|---|
| Switch 1 Normal Command | `1NWS` | Control (T) | Byte 0, Bit 0 | `CT1001` |
| Switch 1 Reverse Command | `1RWS` | Control (T) | Byte 0, Bit 1 | `CT1002` |
| Switch 3 Normal Command | `3NWS` | Control (T) | Byte 0, Bit 2 | `CT1003` |
| Switch 3 Reverse Command | `3RWS` | Control (T) | Byte 0, Bit 3 | `CT1004` |
| Signal 2 Southward (Right) | `2SGS` | Control (T) | Byte 1, Bit 0 | `CT1009` |
| Signal 2 Northward (Left) | `2NGS` | Control (T) | Byte 1, Bit 1 | `CT1010` |
| Signal 2 Hold (Stop) | `2HS` | Control (T) | Byte 1, Bit 2 | `CT1011` |
| Switch 1 Normal Indication | `1NWK` | Indication (R) | Byte 0, Bit 0 | `CS1001` |
| Switch 1 Reverse Indication | `1RWK` | Indication (R) | Byte 0, Bit 1 | `CS1002` |
| Track Circuit 1T1 Occupancy | `1T1` | Indication (R) | Byte 1, Bit 0 | `CS1009` |
| Signal 2 Northward Displayed | `2NGK` | Indication (R) | Byte 2, Bit 1 | `CS1018` |

---

## 3. Configuring the FieldUnit Codec

In your Arduino sketch, configure `CodeLineCodec` to match the exact byte layout you configured in JMRI:

```cpp
CodeLineCodec codec(2 /* control bytes */, 4 /* indication bytes */);

// Byte 0: Switch controls
codec.mapSwitchControl(0, 0, 0, 0, 1); // SW1: b0=1NW (CT1001), b1=1RW (CT1002)
codec.mapSwitchControl(1, 0, 2, 0, 3); // SW3: b2=3NW (CT1003), b3=3RW (CT1004)

// Byte 1: Signal controls
codec.mapSignalControl(0, 1, 0, 1, 2); // SIG2: b0=2SG (CT1009), b1=2NG (CT1010), b2=2H (CT1011)

// Indications:
codec.mapSwitchIndication(0, 0, 0, 0, 1); // SW1: b0=1NWK (CS1001), b1=1RWK (CS1002)
codec.mapTrackIndication(0, 1, 0);        // 1T1:  b0=1T1K  (CS1009)
codec.mapSignalIndication(0, 2, 0, 1, 2); // SIG2: b0=2SGK, b1=2NGK (CS1018), b2=2TEK
```

---

## 4. Setting Up JMRI CTC Panel

1. **Add C/MRI Connection**:
   In JMRI Preferences $\to$ Connections, add a **C/MRI** connection (Serial or Network Driver).
2. **Configure Node**:
   Add Node 1 with 2 output bytes (16 bits) and 4 input bytes (32 bits).
3. **Bind Levers in Layout Editor / PanelPro**:
   - Create a Turnout lever bound to `CT1001` / `CT1002`.
   - Create a Signal lever bound to `CT1009` / `CT1010` / `CT1011`.
   - Create an occupancy sensor on your track diagram bound to `CS1009`.
4. **Operate**:
   - Turn the signal lever to Left.
   - Press the CTC Code Button.
   - JMRI transmits the 2-byte Control packet.
   - FieldUnit evaluates the route, checks switch locks, and displays the aspect.
   - On the next poll, FieldUnit returns the 4-byte Indication packet.
   - JMRI lights up the track diagram occupancy lamp and signal indication light.
