# FieldUnit architecture decision records

This file is the index from code to decision. `CLAUDE.md` and `AGENTS.md` point here instead of restating decisions. When an ADR lands, or a decision in one is revised, update this file in the same commit.

Cite every ADR with its repository name (`FieldUnit ADR 0007`, `FieldUnit-Subdivision ADR 0013`): the two sequences overlap from 0007 on (FieldUnit ADR 0007 D5). Numbers that `docs/GLOSSARY.md` cites without a repository name (0008, 0009, 0011, 0013) are FieldUnit-Subdivision's. A status line records the owner's review date; `proposed` means shaped with the owner and not yet ratified; 0005 is a kept, rejected record.

## Code to decision

Read the ADRs in the right-hand column before changing the code in the left.

| Code | Decided in | What it decides |
|---|---|---|
| `src/CodeLine.h`, `src/WireCodec.h` (`AarTextCodec`), the MQTT topic schema, retained indications, `telemetry` and last will; `src/cTcMachine.h` panel provisioning and the code button | FieldUnit ADR 0001 | The code line is MQTT carrying AAR tokens; the panel discovers its schema and aligns from retained indications; a code button sends one transaction. |
| `InterlockingPlant::applyControlTransaction`, `SignalControl`, `vitalValid` | FieldUnit ADR 0002 | A malformed transaction is ignored whole. A valid but unsafe one has every vital control ignored and every non-vital control acted on. No refusal is sent; the office reads the indications. The classes are named vital and non-vital; the code line is not vital. |
| `src/drivers/` (`IOBit`, `IOBus`, the appliance drivers), the `HAND_LOCKED` flag (`src/types.h`, `src/InterlockingPlant.h`), what the generator reads from the field I/O symbols | FieldUnit ADR 0003 (proposed) | One `(Role, Kind)` is one logic block; Kind names are `FAMILY_IMPLEMENTATION_FEEDBACK`; plant switch Kinds say what the interlocking may do, never how; polarity rides on the pin name (`~{X}`); heads bind, not masts. |
| `src/drivers/DriverPolicy.h` and the per-appliance drivers (`SwitchDriver`, `SignalMastDriver`, `TrackCircuitDriver`): the two hardware I/O paths | FieldUnit ADR 0004 (proposed); FieldUnit ADR 0005 (rejected, kept as record) | The driver seam is being redesigned. Check 0004 before extending either path. |
| `src/Switch.h` electric lock and release | FieldUnit ADR 0006 (proposed; not yet in `src/` or in the KiCad symbols) | A dual-control switch is a powered switch plus a release that follows the electric lock exchange of FieldUnit-Subdivision ADR 0004 D1 to D5; while released, both correspondence indications drop and governing signals hold at Stop. |
| `src/LineInput.h`; `Route::home()` and `proceed()` (`src/ControlTable.h`); `SignalControl::commandedDirection()` as the seam's `requested`; the `MAX_*` constants in `src/types.h` and every fixed array they size | FieldUnit ADR 0007 | D1 `requested` is the held control, not the `HSR`. D2 inbound seam facts are line inputs, not virtual track circuits. D3 the automatic signal is a library class. D4 capacity is per plant: sizes from the data model, allocated once at startup, never after `setup()` (default revised 2026-11-02). D5 the citation rule above. |
| `src/AutomaticSignal.h`, `src/ControlPointLine.h`, `src/Fact.h`, `InterlockingEngine::evaluateMastIndication()`, `tests/test_automatic_signal.cpp` | FieldUnit ADR 0008 | A mast is an output and nothing reads it; every operand of `home()`, `proceed()`, `governs()`, `approach()`, `block()` is a `Fact`; a location produces `<mast>H` and `<mast>D`; `SR` is the only stored state; the §16 conformance trace and its three findings (folded into FieldUnit-Subdivision ADR 0013 by its PR 67). |
| `InterlockingPlant::resolveFact`, `unresolved()`, `isComplete()`; `ReleasedFact`; `LineInput::isImplicit`; `markIncomplete()` on `Route`, `AutomaticSignal`, `ControlPointLine`; `PlantSerializer` skipping implicit inputs; `DEFAULT_STALENESS_TIMEOUT_MS` and its three override levels | FieldUnit ADR 0009 | A Fact-typed name with no source becomes an implicit line input; an appliance-typed name puts its owner at Stop; every gap is reported; the library takes no gate; staleness is a carrier property with a compile-time default. |
| A train order board: `src/TrainOrderFace.h`, `tests/test_train_order_face.cpp`, the one-head `SignalMast` it sets, its fascia lever inputs | FieldUnit ADR 0010 | A face is a ranked list of inputs and one indication out, with no state between scans; the words come from the model; its mast is outside the interlocking, the code line and the plant JSON; nothing asserted shows CLEAR. |

Invariants no ADR records yet (declaration order is the wire index, schema order matches on both ends, the one field-unit cycle shape, `FieldUnit.h` as the only include, JSON and the fluent API as one plant) stay in `CLAUDE.md` under "Cross-file invariants".

## Decisions by number

| ADR | Title | Status |
|---|---|---|
| [0001](0001-mqtt-aar-codeline-interface-a.md) | MQTT broker and AAR tokens for the CodeLine interface | accepted (2026-09-12) |
| [0002](0002-control-transaction-classes.md) | How a field unit processes a control transaction | accepted (owner, 2026-10-02) |
| [0003](0003-field-io-symbols.md) | Field I/O symbols: binding plant appliances to hardware channels and operator interfaces | proposed (reviewed with the owner 2026-10-06) |
| [0004](0004-driver-seam.md) | The driver seam: what a device driver may see, and the two contexts | proposed (shaped with the owner 2026-10-07 and 08) |
| [0005](0005-appliance-driver-architecture.md) | Appliance driver architecture: axioms, three layers, primitives × domains | rejected (kept as record) |
| [0006](0006-dual-control-switch.md) | The dual-control switch | proposed (shaped with the owner 2026-10-11) |
| [0007](0007-interlocking-block-seam.md) | The interlocking–block seam in the field unit | accepted (D1 to D5, 2026-11-02; D4 default revised 2026-11-02) |
| [0008](0008-automatic-signal.md) | The automatic signal: a location that reads the block ahead and produces the HD line to the rear | accepted (D1 to D6, 2026-11-02) |
| [0009](0009-unresolved-references.md) | An incomplete plant: what a binding does when its name has no source | accepted (D1 to D6, 2026-11-02; implemented the same day) |
| [0010](0010-train-order-face.md) | The train order board: a face is a lever and a mast, outside the interlocking | accepted (D1 to D6, 2026-11-12; implemented the same day) |
