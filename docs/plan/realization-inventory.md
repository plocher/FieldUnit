# Realization on a Layout: Inventory and Gaps

Draft 1, 2026-11-04. Seed for the first distillation in "Course correction" (`docs/plan/user-docs-plan.md`).
A gap analysis, not documentation: what exists, where, how far it goes, and what is missing.
Every "exists" cites a file that was opened. Status words: **exists** (code and a test or a working example),
**partial** (code without a transport, a test or an example, or behind an open ADR), **planned** (in an ADR or the
plan only), **gap** (nowhere).

## 1. The frame: roles, seams, implementations

This frame is the headline of the work (owner, 2026-11-04): composability at several levels, and a cosmopolitan
attitude toward alternative instantiations, ours or another project's. The system is built so that each role can be real or virtual on its own, and so that each seam can be crossed in
several ways. The combinations are the product. FieldUnit ADR 0004 ("Three seams, three deployments") states it:

| Seam | Joins | Crossed by |
|---|---|---|
| SEAM-0, the code line | office unit (CTC machine logic) and field unit | a code line encoding on a transport (`CodeLine`, `AarTextCodec`, `BitPackedCodec`) |
| SEAM-1, hardware I/O | logic and the devices | an `IOBus` (`MockIOBus`, `I2CexpanderIOBus`, `CmriIOBus`) or a semantic bus (`MqttApplianceBus`) |
| SEAM-2, remote I/O link | a host application and a remote I/O processor | CMRInet (IB/OB), MQTT, ... |

Each unit can run in three deployments (ADR 0004): **A** virtual (simulated devices), **B** firmware on a processor
near the devices, **C** a host application with remote I/O. The panel side of the office has the same seam:
`PanelHardware` (`src/cTcMachine.h:100`) with interchangeable backends.

Naming debt: ADR 0004 numbers the seams 0, 1, 2 (code line, hardware I/O, remote I/O); the glossary entry **seam**
follows the rejected ADR 0005 (Seam 1 the code line, Seam 2 hardware I/O). One numbering has to win.

## 2. Inventory by seam

### SEAM-0: code line (office ↔ field)

| Item | Status | Where |
|---|---|---|
| Transport interface `CodeLine` | exists | `src/CodeLine.h:13` |
| MQTT transport | exists | `MqttCodeLine`, `src/CodeLine.h:90`; FieldUnit ADR 0001 (topic schema) |
| Serial / stream transport | exists | `StreamCodeLine`, `src/CodeLine.h:147` |
| Test transport | exists | `MockCodeLine`, `src/CodeLine.h:27`; the sketch tests |
| AAR text token encoding | exists | `AarTextCodec`, `src/WireCodec.h:415` |
| Bit-packed encoding (C/MRI-style byte arrays) | exists | `BitPackedCodec`, `src/WireCodec.h:1470`; how-to 2 |
| JMRI as the office, over C/MRI | partial | `docs/how-to/02_jmri_and_cmri_integration.md`; no end-to-end test against JMRI in the repo |
| US&S 506-style step encoding | planned | a code line type in the glossary (section 6); visual stepping only on the desk display |
| LocoNet, LCC, DCC-EX as code line transports | gap | LocoNet is named in the glossary entry **transport**; LCC and DCC-EX appear nowhere |

### SEAM-1: hardware I/O (logic ↔ devices)

| Item | Status | Where |
|---|---|---|
| Bus interface with typed directions and polarity | exists | `IOBus` (`src/drivers/IOBus.h:9`), `IOBit`/`InputBit`/`OutputBit` (`src/drivers/IOBit.h`) |
| Simulated devices | exists | `MockIOBus` (`src/drivers/IOBus.h:19`) |
| I2C expanders (MCP23017, cpNode-IOX, MAX7313 on the desk) | exists | `I2CexpanderIOBus` (`src/drivers/I2CexpanderIOBus.h:13`); `examples/CP_Corporal`; `examples/spcoast_ctc/IO-I2C.h` |
| C/MRI byte buffers as a bus | partial | `CmriIOBus` (`src/drivers/CmriIOBus.h:11`), "centralized interlocking compute with remote I/O nodes"; `tests/test_centralized_chubb.cpp` runs a centralized plant on it; the serial polling that fills IB/OB is not in `src/` |
| Semantic appliance bus: JMRI MQTT topics, IoT devices | exists in unit tests | `MqttApplianceBus` (`src/drivers/MqttApplianceBus.h:34`), `MqttDriverPolicy` (`src/drivers/DriverPolicy.h:138`); `tests/test_mqtt_appliance_bus.cpp` (auto-binding, sensor and turnout topics, mast aspects, end-to-end route clearing over MQTT) and `tests/test_driver_policy.cpp`; no test against a real broker or JMRI |
| PWM servo controller (PCA9685) | gap | `IOBus::writeAngle` exists and `MockIOBus` implements it; no PCA9685 `IOBus` (how-to 4) |

Device drivers on SEAM-1:

| Device | Status | Where |
|---|---|---|
| Stall-motor switch machine (Tortoise): 1 output, 2 point-detection inputs | exists | `SwitchDriver` (`src/drivers/SwitchDriver.h:12`) |
| Servo switch machine | gap | `docs/how-to/01_data_collection_and_aar_bits.md` names servos; no driver |
| Solenoid (twin-coil) switch machine | gap | nowhere |
| Track circuit from any detector, polarity, dropout delay | exists | `TrackCircuitDriver` (`src/drivers/TrackCircuitDriver.h:9`); `TrackCircuit` |
| Color light mast heads | exists | `SignalMastDriver` (`src/drivers/SignalMastDriver.h:17`) |
| Semaphore arms on servos | exists | `SemaphoreDriver` (`src/drivers/SemaphoreDriver.h:19`); how-to 4 |
| B&O color position light with markers | exists | `CplMastDriver` (`src/drivers/CplMastDriver.h:36`); `tests/test_cpl_signaling.cpp` |
| Electric lock, dual control, fascia key and lamps | partial | logic and tokens (how-to 3; FieldUnit ADR 0006 proposed); no fascia device driver |
| Driver seam redesign | planned | FieldUnit ADR 0004 (proposed); ADR 0005 rejected; the two I/O paths (per-appliance drivers from `loop()`, `DriverPolicy` inside `tick()`) coexist until it lands |

### SEAM-2: remote I/O link (host ↔ remote I/O processor)

| Item | Status | Where |
|---|---|---|
| Field unit as a C/MRI host for CMRInet nodes | partial | `CmriIOBus` holds the buffers; no CMRInet polling transport in `src/` |
| Field unit reaching devices over MQTT | partial | `MqttApplianceBus`, as above |
| Field unit on a host, devices on a cpNode | planned | ADR 0004 deployment C |

### Between control points: line circuits (logic, after SEAM-0)

| Item | Status | Where |
|---|---|---|
| Automatic signal location, producing the HD line to the rear | exists | `AutomaticSignal` (`src/AutomaticSignal.h:48`), FieldUnit ADR 0008 (accepted); `tests/test_automatic_signal.cpp` |
| What a control point produces at the seam | exists | `ControlPointLine` (`src/ControlPointLine.h:68`), ADR 0008 D5 |
| Line inputs and route `home()` / `proceed()` terms; an incomplete plant fails safe | exists | `LineInput`, `Fact` (`src/LineInput.h`), ADR 0007 D2, ADR 0009; `tests/test_line_input.cpp` |
| Transport of line circuits between field units | gap | the classes produce and consume the HD line; how it crosses between field units on a layout is not in this inventory yet |

### Field processors and configuration

| Item | Status | Where |
|---|---|---|
| Fixed sketch per interlocking (ESP32, RP2040, AVR) | exists | `examples/CP_Corporal`, `examples/CP_Christopher`; zero heap after startup |
| One firmware, plant loaded from JSON in flash | exists | `examples/Universal_FieldUnit`; `src/PlantSerializer.h` |
| Bench jig: plant loaded over USB, code line on the same link | exists | `examples/FieldUnit_Tracer` |
| Hybrid bench: some appliances real, some mocked | exists | `mockSwitch`, `overrideDriver` (`src/drivers/DriverPolicy.h`) |
| Interlocking model from KiCad drawings | exists in FieldUnit-Subdivision | glossary sections 2.4 and 2.6; not in this repository |
| Virtual field units on a host (seven at once) | exists in FieldUnit-Subdivision | `spcoast_virtual_plant` (glossary sections 2.4, 2.5) |

### Office side

| Item | Status | Where |
|---|---|---|
| CTC machine logic | exists | `cTcMachine` (`src/cTcMachine.h:528`), `tests/test_ctc_machine.cpp` |
| Physical desk on I2C (14 × MAX7313) | exists | `examples/spcoast_ctc` with `IO-I2C.h` |
| Desk panel over C/MRI | partial | `IO-CMRI.h`: a stub with no transport, and SW_NORMAL/SW_REVERSE swapped (AGENTS.md) |
| Ghost plant and lamp walk for the desk | exists | `tools/test_ctc_desk.py --loopback`, `--walk`, `--monitor` |
| CTC machine on a screen (virtual desk) | planned | glossary section 2.1 ("a screen display ... is a CTC machine"); no code |
| JMRI panels or CATS as the office | partial | how-to 2 (JMRI C/MRI); CATS untested |

## 3. Combinations

The combinations the seams allow, as a layout would meet them. Status is the weakest link in the chain.

| # | Office | Code line | Field unit | Devices | Status | Weakest link |
|---|---|---|---|---|---|---|
| 1 | virtual or physical desk | MQTT | virtual, on a host | simulated | exists | none; this is SPCoast today |
| 2 | physical desk | MQTT | firmware under the benchwork | I2C expanders, Tortoises, LED masts | exists | the driver seam is in flux (ADR 0004) |
| 3 | JMRI / CATS on an existing CMRInet layout | C/MRI (bit-packed) | firmware | local I/O | partial | no end-to-end test with JMRI or CATS |
| 4 | FieldUnit desk or screen | MQTT | FieldUnit as host of an existing CMRInet layout | the layout's existing C/MRI nodes | partial | CMRInet polling transport |
| 5 | any | MQTT | virtual or firmware | IoT appliances on MQTT (JMRI MQTT topics, ESP32 devices) | partial | `MqttApplianceBus` tested end to end, but only against a mock broker |
| 6 | any | any | firmware or host | LCC, DCC-EX accessories, LocoNet | gap | no bus, no transport |
| 7 | screen CTC machine | MQTT | any | any | planned | no virtual desk |
| 8 | none (bench) | USB | `FieldUnit_Tracer` | real and mocked mixed | exists | none |

## 4. Gaps to close, and who or what can close them

| Gap | Kind | Who or what |
|---|---|---|
| CMRInet polling transport (field unit as C/MRI host) | software | CMRInet repository; Chubb's protocol docs |
| End-to-end JMRI and CATS tests (combination 3) | test | a JMRI install; SIG members who run CATS |
| `MqttApplianceBus` against real devices (combination 5) | test | a JMRI MQTT setup; one ESP32 device |
| LCC, DCC-EX, LocoNet | design, then software | owner's priority call; their specs |
| Servo and solenoid switch machines; PCA9685 bus | software | the driver seam (ADR 0004) first |
| Fascia devices for electric lock and dual control | software and hardware | ADR 0006; the SPCoast fascia |
| Virtual (screen) CTC machine | software | `PanelHardware` backend |
| Seam numbering (ADR 0004 vs glossary) | docs | owner's decision |
| Physical numbers: room for masts, track centers, wire counts, cost | measurement | hands-on path 2; SIG members |
| Detection and DCC power districts; detector kinds and wiring | knowledge | SIG members; detector vendors' docs |

## 5. Open questions for the owner

1. Is "roles × seams × deployments" the right frame for the realization distillation, with combinations as the
   product the SIG sees?
2. Which combinations matter most to this audience? A guess: 3 (keep JMRI/CATS, add field units) and 4 (new office,
   existing CMRInet layout) are the ones people with running layouts would try first.
3. Which interoperability targets come first: LCC, DCC-EX, LocoNet?
4. Seam numbering: ADR 0004's SEAM-0/1/2, or the glossary's Seam 1/2?
