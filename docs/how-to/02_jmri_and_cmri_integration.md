# How-To: JMRI and C/MRI Integration

This guide explains how to connect FieldUnit field units to JMRI (Java Model Railroad Interface) using standard C/MRI serial or network drivers.

---

## 1. The JMRI Architecture Model

JMRI plays the role of the CTC machine, where the dispatcher states intent.
FieldUnit acts as the field unit, which decides whether to act on that intent.

```
+--------------------------------------------------------------+
|                    JMRI (office, CTC machine)                |
|   - CTC machine (Levers, Code Buttons, Lamps)                |
|   - Sends control packets (C/MRI T packets)                  |
|   - Polls for office indication packets (C/MRI P / R)        |
+--------------------------------------------------------------+
                               |
                   C/MRI Serial or TCP Link
                               |
                               v
+--------------------------------------------------------------+
|                   FieldUnit field unit                       |
|   - CodeLineCodec unpacks control bytes into a               |
|     ControlTransaction                                       |
|   - Interlocking control table evaluates plant safety        |
|   - CodeLineCodec packs the IndicationVector into bytes      |
+--------------------------------------------------------------+
```

---

## 2. Understanding JMRI C/MRI Address Numbering

In JMRI, C/MRI I/O points use a standard numbering convention:
- **`CT` (C/MRI Turnout / Output)**: Represents controls sent from JMRI to FieldUnit.
- **`CS` (C/MRI Sensor / Input)**: Represents office indications received by JMRI from FieldUnit.

The numbering formula is:
$$\text{Address} = (\text{Node Address} \times 1000) + (\text{Byte Index} \times 8) + \text{Bit Index} + 1$$

For Node 1 (`UA = 1`):
- `Byte 0, Bit 0` $\implies$ Address `1001`.
- `Byte 0, Bit 7` $\implies$ Address `1008`.
- `Byte 1, Bit 0` $\implies$ Address `1009`.

### JMRI Mapping Table for CP Christopher

This table covers the switches and the signal of `examples/CP_Christopher/CP_Christopher.ino`. The header of that sketch lists the whole wire schema.

| Function | Token | Packet Type | Wire Location | JMRI System Name |
|---|---|---|---|---|
| Switch 1 Normal control | `1NWS` | Control (T) | Byte 0, Bit 0 | `CT1001` |
| Switch 1 Reverse control | `1RWS` | Control (T) | Byte 0, Bit 1 | `CT1002` |
| Switch 3 Normal control | `3NWS` | Control (T) | Byte 0, Bit 2 | `CT1003` |
| Switch 3 Reverse control | `3RWS` | Control (T) | Byte 0, Bit 3 | `CT1004` |
| Signal 2 control: RIGHT | `2SGS` | Control (T) | Byte 1, Bit 0 | `CT1009` |
| Signal 2 control: LEFT | `2NGS` | Control (T) | Byte 1, Bit 1 | `CT1010` |
| Signal 2 control: stop | `2HS` | Control (T) | Byte 1, Bit 2 | `CT1011` |
| Switch 1 office indication: Normal | `1NWK` | Office indication (R) | Byte 0, Bit 0 | `CS1001` |
| Switch 1 office indication: Reverse | `1RWK` | Office indication (R) | Byte 0, Bit 1 | `CS1002` |
| Track circuit 1T1 office indication (occupied) | `1T1K` | Office indication (R) | Byte 1, Bit 0 | `CS1009` |
| Signal 2 office indication: LEFT | `2NGK` | Office indication (R) | Byte 2, Bit 1 | `CS1018` |

---

## 3. Configuring the FieldUnit Codec

In your Arduino sketch, configure `CodeLineCodec` to match the exact byte layout you configured in JMRI.
The first argument of each `map...` call is the index of the appliance in the order you declared it on the `InterlockingPlant`.

```cpp
CodeLineCodec codec(2 /* control bytes */, 4 /* indication bytes */);

// Byte 0: Switch controls
codec.mapSwitchControl(0, 0, 0, 0, 1); // SW1: b0=1NWS (CT1001), b1=1RWS (CT1002)
codec.mapSwitchControl(1, 0, 2, 0, 3); // SW3: b2=3NWS (CT1003), b3=3RWS (CT1004)

// Byte 1: Signal controls
codec.mapSignalControl(0, 1, 0, 1, 2); // SIG2: b0=2SGS (CT1009), b1=2NGS (CT1010), b2=2HS (CT1011)

// Office indications:
codec.mapSwitchIndication(0, 0, 0, 0, 1); // SW1: b0=1NWK (CS1001), b1=1RWK (CS1002)
codec.mapTrackIndication(0, 1, 0);        // 1T1:  b0=1T1K  (CS1009)
codec.mapSignalIndication(0, 2, 0, 1, 2); // SIG2: b0=2SGK, b1=2NGK (CS1018), b2=2TEK
```

When a packet arrives, unpack it and apply it. After the vital cycle, pack the office indications:

```cpp
ControlTransaction ctl;
if (codec.unpackControls(rxBytes, rxLength, ctl)) {
    cp.applyControlTransaction(ctl, nowMs);
}
cp.tick(nowMs);

IndicationVector ind;
cp.exportIndicationVector(ind);
codec.packIndications(ind, txBytes, sizeof(txBytes));
```

---

## 4. Setting Up the JMRI CTC Machine

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
   - JMRI transmits the 2-byte control packet.
   - The field unit acts on the control only if it is safe. It checks the locks and the route, and then shows the signal.
   - On the next poll, the field unit returns the 4-byte office indication packet.
   - JMRI lights the track circuit occupancy lamp and the signal's office indication.
   - If the office indications do not agree with the lever, the field unit did not act on the control.
