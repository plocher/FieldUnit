# 0003. Field I/O symbols: binding plant appliances to hardware channels and operator interfaces

- Status: proposed, reviewed with the owner on 2026-10-06 (two drafts); the decisions below record the review's outcome.
- Scope: the `RailroadField` symbol library, the plant switch Kinds, the operator-interface symbols (desk and fascia), and what the generator reads from them.
- Evidence: the Sargent sandbox (Railroad `sandbox/sargent-field-io`, InterlockingPlant `1eed088`), FieldUnit `src/drivers/`.
- Not decided here: the compiler's `(Role, Kind)` table format and the generator's output form (FieldUnit-Subdivision #30 step 3).

## Decisions

- D1. `(Role, Kind)` is 1:1 with one logic block (a FieldUnit driver class, or a row consumed by one). Role is the namespace; a Kind is unique within its Role and may repeat across Roles, and when it repeats it means the same family of thing in each.
- D2. A Kind name has up to three parts, `FAMILY_IMPLEMENTATION_FEEDBACK`: the appliance family it serves (`SWITCH`, `HEAD`, `DETECTOR`, `LAMP`), how it is built (`STALL`, `SERVO`, `3LED`, `OPTICAL`), and for switches how position is known (`SIM`, `SENSED`). Four things never appear in the name because they do not change the logic block: cable packaging (a Turtle and a bare stall motor with contacts are both `SWITCH_STALL_SENSED`; the Turtle's extra `OCC` pin is packaging), polarity (`DETECTOR_OPTICAL` with `OCC=ACTIVE_LOW` is still `DETECTOR_OPTICAL`), bus addresses, and tunables (angles, times). Those are attributes.
- D3. Plant switch Kinds say what the interlocking may do and may know, never how: `SWITCH_REMOTE` (the dispatcher commands it), `SWITCH_LOCK` (the crew operates it under an electric lock), `SWITCH_MANUAL_SENSED` (the crew operates it; the field unit knows its position), `SWITCH_MANUAL` (the crew operates it; position unknown). Manual switches lie outside interlocking limits by definition: routes end at the controlled signals and never include one, so "sensed" has no vital consequence and serves local lamps and devices. A manual switch inside the limits is an error: there it must be `SWITCH_LOCK`. The circuit-controller-without-lock arrangement (a switch that can be thrown while a governing signal is not at STOP) is rejected as unsafe and is not modelled.
- D4. The electric lock is three things, each already drawn: the plant Kind `SWITCH_LOCK`, the lock lever (desk and fascia), and the firmware flag `HAND_LOCKED` in FieldUnit. It is not a device: nothing on the layout is driven to lock. `Device-Switch-ElectricLock` is deleted; its `N`/`R` pins are a `Device-Switch-Sense`, its `REQ`/`LOCK` pins are the fascia lock lever's `WLS`/`WLK`. See the worked example.
- D5. Switch device Kinds, the 95% set: `SWITCH_SENSE`, `SWITCH_STALL_SIM`, `SWITCH_STALL_SENSED`, `SWITCH_SERVO_SIM`, `SWITCH_SERVO_SENSED`. `SWITCH_SERVO_CURRENT` later. Stall motors have no current sense (they stall); an integrated DPDT is `SENSED` like any contact.
- D6. Polarity is one attribute per pin that has one, named for the pin (`OCC=ACTIVE_LOW`), defaulted in the library symbol, overridable on the instance. Direction of travel is `NormalIs=` (`HIGH`/`LOW` for a bit motor, `LOW_ANGLE`/`HIGH_ANGLE` for a servo). The free-text `Polarity` field is removed.
- D7. An unconnected pin on a `DEVICE` or `PANEL` symbol is an error. Variants are separate symbols with separate Kinds, never optional pins.
- D8. One Role, `PANEL`, for every operator interface: the dispatcher's desk, the crew's fascia, the tower operator's machine. Where a lever binds, to a `PanelColumn` of a CTC machine (code line) or on an interlocking's field sheet (local client of the `Switch`), is derived from the netlist and the sheet path, never declared. Fascia symbols are the desk symbols without the `Column` pin: `Local-Switch` (NWS, RWS, NWK, RWK), `Local-Switch-NoLamp` (NWS, RWS), `Local-Lock` (WLS, WLK), and `Local-Lamp` (one pin, `IndicationToken`, `Color`, polarity) in `-Bit`, `-PWM` and `-NeoPixel` packagings, the twin of `PanelLamp-*`. Kinds `SWITCH_LEVER`, `SWITCH_LEVER_NOLAMP`, `LOCK_LEVER`, `LAMP`.
- D9. Heads, not signals: a head device binds to a head name (`836NA`); a mast is its heads. `Lamp` for non-signal lamps (`HBA`, `MC1`).
- D10. Driver symbols: Role `IODRIVER`, Kind = chip (`MCP23017`, `PCA9685`), Value = address; every pin has a channel type (BIT, DUTY, ANGLE) and a device pin may only wire to a driver pin of the same type.
- D11. This ADR lives in FieldUnit `docs/adr/`, numbered after 0002 on `docs/vocabulary-rewrite`.

## Ontology: one appliance, three interfaces

```
                        APPLIANCE  (plant, Role APPLIANCE)
                        the functional thing the railroad controls or monitors; one symbol each
                        SWITCH_REMOTE | SWITCH_LOCK | SWITCH_MANUAL_SENSED | SWITCH_MANUAL | DERAIL |
                        HEAD | TRACK_CIRCUIT | MAINTAINER_CALL | AUXILIARY | VIRTUAL_INDICATION | ...
                              ▲                      ▲                      ▲
             binds by name    │                      │                      │
          ┌───────────────────┘                      │                      └───────────────────┐
   people: PANEL                               hardware: DEVICE                        channels: IODRIVER
   dispatcher (desk column, code line)         actuators, sensors, lamps               expanders, PWM, buses
   crew (fascia, local client)                 one symbol per physical device          one symbol per chip
   tower operator (machine, direct)
```

Appliances are the functional representations of what the railroad controls or monitors: the things a dispatcher, tower operator or crew interact with. A dozen common ones cover the layouts in hand; the long tail (drawbridge, slide fence, car identification) adds rows under `APPLIANCE` and, where needed, an interface symbol. Nothing structural changes.

## Five axes, five homes

| Axis | Question | Home | Values |
|---|---|---|---|
| Authority and knowledge | who may command the points; does the field unit know where they are | plant symbol (Trackplan) | remote / lock / manual sensed / manual |
| Actuation | what the field unit drives | field device Kind | none / stall / servo |
| Feedback | how the field unit knows position | field device Kind | sense / sim / current |
| Operator interface | levers and lamps, and where they are | `PANEL` symbols; location derived | desk / fascia / tower |
| Exposure | what the dispatcher sees or controls | desk sheet, by token | lever / lamp only / nothing |

Nothing on the Trackplan says how points move; nothing on the field sheet says who may command them.

## The chain the generator emits

```
code line → WireCodec → Switch (vital) → driver (Kind) → IOBus (driver symbol) → chip
835RWS     SwitchDemand  commanded/locks   drive()/sample()  writeBit/readBit     MCP23017 0x20 bit 3
fascia NWS ─────────────► (local client) ─┘
```

A device symbol holds the appliance name (Value) and exposes the driver's constructor parameters as pins. The generator emits one constructor call per device symbol; the mapping from demand or aspect to channel values lives in the driver class, written once per Kind. No mapping is ever typed on a symbol. A fascia lever binds to the same `Switch` as a second client, by name.

## Worked example: the electrically locked hand switch 835 at Sargent, every sheet

```
 Trackplan   Switch_Lock 835                     APPLIANCE / SWITCH_LOCK     the appliance

 Desk        PanelLock 835                       PANEL / LOCK_LEVER          dispatcher's lever: WLS out, WLK back, on a column

 Fascia      Local-Lock 835                      PANEL / LOCK_LEVER          WLS ← crew's key or request button
             (field sheet)                                                   WLK → "unlocked" LED
             Local-Lamp  IndicationToken=835WLK  PANEL / LAMP                LAMP=ACTIVE_LOW → "locked" LED (same token, second channel)
             [Local-Switch 835 only if a motor exists; the crew's hand throws the points here]

 Field       Device-Switch-Sense 835             DEVICE / SWITCH_SENSE       N, R ← point contacts
             Device-Detector-Optical 835T1       DEVICE / DETECTOR_OPTICAL   OCC ← the OS circuit
             Driver-I2C-MCP23017 0x20            IODRIVER / MCP23017         the bits above land here

 FieldUnit   Switch 835: HAND_LOCKED released by WLS when every governing signal is at STOP and no time lock runs;
             reported as WLK; locked and sensed reverse → out of correspondence, OS reports occupied.
```

Every symbol above binds by the name `835`; no wire crosses between sheets. The same switch with a crew-operated motor adds `Local-Switch 835` on the fascia and replaces `Device-Switch-Sense` with a `SWITCH_STALL_SENSED` or `SWITCH_SERVO_*` device; nothing else changes.

## Worked example: the powered switch 783 at Luchessa

```
 Trackplan   Switch_Powered 783                  APPLIANCE / SWITCH_REMOTE
 Desk        PanelSwitch 783                     PANEL / SWITCH_LEVER        NWS, RWS out; NWK, RWK back; on column 5
 Fascia      Local-Lamp IndicationToken=783NWK   PANEL / LAMP                optional repeater
 Field       Device-Switch-StallMotor-Sensed-OS  DEVICE / SWITCH_STALL_SENSED  M → motor; N, R ← contacts; OCC ← OS (one cable, the Turtle packaging)
```

## Kind table (field library)

| Symbol | Role / Kind | Pins (channel type) | Attributes (defaults) | Logic block |
|---|---|---|---|---|
| Device-Switch-Sense | DEVICE / SWITCH_SENSE | N, R (BIT) | N=ACTIVE_LOW, R=ACTIVE_LOW | sense-only switch driver |
| Device-Switch-StallMotor-Sim | DEVICE / SWITCH_STALL_SIM | M (BIT) | NormalIs=HIGH, Travel_ms=2000 | `MockSwitchDriver` (simulated feedback) |
| Device-Switch-StallMotor-Sensed | DEVICE / SWITCH_STALL_SENSED | M, N, R (BIT) | NormalIs=HIGH, N=, R= | `SwitchDriver` |
| Device-Switch-StallMotor-Sensed-OS | DEVICE / SWITCH_STALL_SENSED | M, N, R, OCC (BIT) | + OCC=ACTIVE_LOW | `SwitchDriver` + `TrackCircuitDriver` (OS of the switch; valid only when the OS is one circuit) |
| Device-Switch-Servo-Sim | DEVICE / SWITCH_SERVO_SIM | CH (ANGLE) | NormalIs=LOW_ANGLE, Normal_deg, Reverse_deg, Speed | servo switch driver, simulated |
| Device-Switch-Servo-Sensed | DEVICE / SWITCH_SERVO_SENSED | CH (ANGLE), N, R (BIT) | as above + N=, R= | servo switch driver, sensed |
| Device-Detector-Optical | DEVICE / DETECTOR_OPTICAL | OCC (BIT) | OCC=ACTIVE_HIGH | `TrackCircuitDriver` |
| Device-Detector-Current | DEVICE / DETECTOR_CURRENT | OCC (BIT) | OCC=ACTIVE_LOW | `TrackCircuitDriver` |
| Device-Head-3LED | DEVICE / HEAD_3LED | R, Y, G (BIT) | R=,Y=,G=ACTIVE_HIGH | `SignalMastDriver` head |
| Device-Head-3LED-Mux2 | DEVICE / HEAD_3LED_MUX2 | S0, S1 (BIT) | | mux head driver (new) |
| Device-Head-3LED-PWM | DEVICE / HEAD_3LED_PWM | R, Y, G (DUTY) | Fade_ms | PWM head driver (new) |
| Device-Head-Semaphore-Servo | DEVICE / HEAD_SEMAPHORE_SERVO | ARM (ANGLE), LAMP (DUTY) | Stop_deg, Approach_deg, Clear_deg | `SemaphoreDriver` |
| Device-Lamp-Bit | DEVICE / LAMP_BIT | OUT (BIT) | OUT=ACTIVE_HIGH | bit output |
| Device-Lamp-PWM | DEVICE / LAMP_PWM | D (DUTY) | | duty output |
| Device-Lamp-NeoPixel | DEVICE / LAMP_NEOPIXEL | (none) | Chain, Index | NeoPixel lamp (bus) |
| Device-Input-Bit | DEVICE / INPUT_BIT | IN (BIT) | IN=ACTIVE_LOW | bit input |
| Local-Switch | PANEL / SWITCH_LEVER | NWS, RWS (BIT in), NWK, RWK (BIT out) | | local client of the Switch |
| Local-Switch-NoLamp | PANEL / SWITCH_LEVER_NOLAMP | NWS, RWS | | |
| Local-Lock | PANEL / LOCK_LEVER | WLS (in), WLK (out) | | |
| Local-Lamp, -PWM, -NeoPixel | PANEL / LAMP | LAMP (BIT / DUTY / none) | IndicationToken, Color, LAMP=ACTIVE_HIGH; Chain, Index | lamp showing one or more indications |
| Driver-I2C-MCP23017 | IODRIVER / MCP23017 | A1..A8, B1..B8 (BIT) | Address | `I2CexpanderIOBus` |
| Driver-I2C-PCA9685 | IODRIVER / PCA9685 | A1..A8, B1..B8 (DUTY, ANGLE) | Address | PCA9685 bus (new: duty write) |

The desk library's `PanelSwitch`, `PanelLock`, `PanelSignal`, `PanelLamp-*`, `PanelMCall`, `PanelCode` keep their Kinds and take Role `PANEL` (was `APPLIANCE`). Later, outside the 95%: `SWITCH_SERVO_CURRENT` (CH, I), DCC accessory, CMRI bus.

## Rules

1. Value is the plant item's name (switch, circuit, head, auxiliary) or the driver's address. The generator resolves it against the plant of the same interlocking; a head must be a head of a mast there.
2. A pin wires to exactly one driver pin of a matching channel type. Two devices on one driver pin is an error.
3. A polarity attribute must name a pin of the symbol. `NormalIs` is the only direction attribute.
4. `SIM` Kinds make the compiler warn: correspondence is simulated (not prototypical). A `SWITCH_REMOTE` or `SWITCH_LOCK` with no device symbol is an error. A `SWITCH_MANUAL*` inside the interlocking limits (between controlled signals in the signal-graph cut) is an error; outside them it is never a route condition.
5. The `-OS` composite is valid only when the switch's OS (`TC` field or `<switch>T1`) is one circuit; a block of several detectors is a virtual indication (`0<n>T`) that aggregates them.
6. A `PANEL` lever bound on a field sheet is a local client of the `Switch`; the plant's authority Kind says whether its demands are honoured (`SWITCH_MANUAL*` always, `SWITCH_LOCK` when unlocked, `SWITCH_REMOTE` never under CTC). A `PANEL` lever bound to a `PanelColumn` reaches the Switch over the code line.
7. A lamp displays a token; a second lamp for the same token is a second `PANEL` lamp with its own channel and polarity, never a second token.
8. A Role that disagrees with its Kind's table row is an error (the libraries and the table are checked against each other).
9. Nothing from the field library enters the interlocking model. The generator emits a separate hardware-binding output for the same interlocking.

## Naming axioms

1. `(Role, Kind)` ↔ logic block, 1:1. Role is the namespace; Kind is unique within a Role and has affinity across Roles. Diagnostics print both.
2. Kind = `FAMILY_IMPLEMENTATION[_FEEDBACK]`, upper case (D2).
3. Symbol name mirrors the Kind in Title-Case with hyphens, plus an optional packaging suffix (`-OS`, `-PWM`, `-NeoPixel`); a location prefix (`Panel-`, `Local-`) is a human hint, not data.
4. Roles: `APPLIANCE` (plant: the thing), `PANEL` (people), `DEVICE` (hardware), `IODRIVER` (channels); plus `TRACK`, `POLICY`, `STRUCTURE` (plant) and `COLUMN`, `MACHINE`, `CODELINE_*` (machine).
5. Pin names are channel names and equal driver constructor parameters; on `PANEL` symbols they equal token suffixes.
6. Attributes are named for the pin they qualify or carry their unit (`Travel_ms`, `Normal_deg`).
7. References, sheet names, project names and library names carry no meaning.

## Consequences: the mechanical cleanup (one scripted pass each)

- InterlockingPlant, field library: rename symbols and Kinds to the table; add polarity and `NormalIs` attributes with defaults; remove `Polarity`; delete `Device-Switch-ElectricLock`, `Device-Switch-HandThrow`, `Device-Switch-Turtle` (becomes the `-OS` packaging), `Device-Signal-*`, `Device-Digital_*`; `Local-Lock` pins to WLS/WLK; add `Local-Lamp` packagings; Role `PANEL` on the `Local-*` symbols; `IODriver` → `IODRIVER`.
- InterlockingPlant, desk library: Role `APPLIANCE` → `PANEL` on levers, lamps, MC and code buttons. `PanelLock` carries switch-lever pins (NWS/RWS/NWK/RWK) while the code line carries `WLS`/`WLK` for a lock; reconciled in the same pass (O3).
- InterlockingPlant, plant library: `Switch_Lock` Kind `SWITCH_LOCK`; `Switch_Manual`, `Switch_Manual_Sensed` stay.
- Railroad sandbox Sargent: rewrite `lib_id`s to the new names; `ELEC_LOCK 835` → `Device-Switch-Sense 835` + `Local-Lock 835`; `HAND_THROW 1` → `Device-Switch-Sense 1`; remove the `PanelLock`/`PanelSwitch` placeholders; refresh instance Role/Kind.
- Desk sheets (South-cTc): Role refresh to `PANEL`; the desk compiler accepts `PANEL`.
- Compiler/generator (FieldUnit-Subdivision #30 step 3): the `(Role, Kind)` table gains the columns above; the generator POC emits the Sargent binding and compiles it against FieldUnit.
- FieldUnit: new drivers for mux head, PWM head, servo switch; duty write on `IOBus`; local-client demands on `Switch`; unlock-request indication; the locked-and-reversed rule.

## Open

- O1. Token for the crew's unlock request (the dispatcher's control is `WLS`; the field's request has no AAR name yet). FieldUnit-Subdivision ADR 0003 (names) list.
- O2. Which plant switches at Luchessa get `-OS` composites versus separate detectors (drawing choice, per cable).
- O3. `PanelLock` pin set on the desk (see consequences).
