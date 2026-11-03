# 0009. An incomplete plant: what a binding does when its name has no source

- Status: accepted (drafted with the owner, 2026-11-02; D1 to D6 ratified by the owner 2026-11-02; implemented the same day)
- Date: 2026-11-02
- Builds on: FieldUnit ADR 0007 (D2 line inputs, D4 per-plant capacity); FieldUnit ADR 0008 (D2 operands are Facts, D3 produced Facts, the "names bind at the call" rule); FieldUnit-Subdivision ADR 0008 §4.7 (absence is information; gaps are stubbed by the emulator's mocks) and §4.9 (the drawing is the deployment; the Day 0 view that elides hardware is a consumer's to build); FieldUnit-Subdivision ADR 0011 decision 5 and §6 (incomplete is not in error; each consumer decides, as a capability gate, whether a file holds enough for its own work; the code generator's stance is out of scope there); FieldUnit-Subdivision ADR 0009 §3.2 and §3.3 (a virtual processor hosts emulated interlockings).
- Order of authority: as FieldUnit ADR 0007 states it. This ADR adds no relay. It decides what the plant does when the model gives it less than a relay chain would have.
- This is FieldUnit ADR 0009. FieldUnit-Subdivision also has an ADR 0009 (explicit seams). Cite each with its repository name.

Terms follow `docs/GLOSSARY.md`. *Fact*, *line input*, *source* and *bind* are the §10.5 entries. *Ghost* is the Studio design specification's word for a simulated train or plant (`docs/FIELDUNIT_STUDIO_DESIGN_SPEC.md:75`, `:157`); here it is anything outside `src/` that drives a fact the plant has no source for.

## The owner's statement (2026-11-02)

> Note that it is explicit for the data model to be incomplete. If it is too incomplete, of course, nothing can be processed or even emulated. But a design with no hardware can be emulated; with some "error-correcting heuristics" it could even be operated virtually. There will be step functions here: start with emulation-only, add some hardware I/O, parts of a field unit could be deployed. We'll need a bunch of TBD stuff to do this well.

The Layout Data Model side of this is decided: a file may be incomplete, absence is information, and each consumer decides what it can do (FieldUnit-Subdivision ADR 0011 §4 item 3, §6). FieldUnit is such a consumer. What it does short of a complete plant is this ADR.

## Decisions requested

- D1. **A Fact-typed name with no source becomes an implicit line input.** When `home()`, `proceed()`, `governs()`, `approach()`, `block()` or `reads()` names something `findFact()` cannot resolve, the plant declares a `LineInput` of that name and binds to it. It reads released until something drives it and goes stale like any wire (ADR 0007 D2). When the line-input array is full, the binding falls back to one shared, permanently released Fact and the reference is reported (D3). Either way the term fails toward Stop. Default: yes.
- D2. **An appliance-typed name with no source makes its owner incomplete, and an incomplete owner is at Stop.** `clears()`, `entrance()`, `approaching()`, `governedBy()`, `displays()`, `aligns()`, `leaving()`, `opposing()` and a control point line's `home()` name appliances, which have a code-line slot, an index and a driver. The plant creates none of them. The route or location whose binding failed is marked incomplete: an incomplete route evaluates to Stop, an incomplete automatic signal's `H` and `D` read released, an incomplete control point line's `H`, `D` and `F` read released. Default: yes.
- D3. **The plant reports its gaps; the engine never refuses.** `InterlockingPlant` keeps a fixed list of unresolved references, each with the name, the call that made it and the owner (route or location), and answers `isComplete()`. A conflict (a source added after an implicit fact of its name already exists, D1's limit) is reported in the same list. Nothing in `tick()` consults the list: the vital answer to a gap is the released term, already given by D1 and D2. Default: yes.
- D4. **Implicit facts are derived, not given.** `PlantSerializer` writes only the line inputs the author declared; an implicit one is re-derived on read, so the file does not grow a declaration the author never wrote (FieldUnit-Subdivision ADR 0011 decision 1: `given` as stated, `derived` recomputed). A ghost finds what to drive through `lineInputCount()` / `lineInput(i)` and a flag `LineInput::isImplicit()`. Default: yes.
- D5. **The ghost drives an implicit fact through the interface a wire uses, and its heuristics live outside `src/`.** `update(value, nowMs)` and `touch(nowMs)` are the whole interface. "Assume clear", "follow the simulated train", "mirror the neighbour's indication" are policies of the emulator, Studio or a test, never of the plant. Default: yes.
- D6. **No stage in the model, no gate in the library.** FieldUnit adds no "emulated / partial / deployed" field to the plant or the JSON. A stage is the set of facts a ghost drives, and that set is a consumer's choice (FieldUnit-Subdivision ADR 0008 §4.9). Whether a sketch refuses to drive hardware outputs while `isComplete()` is false is the sketch's policy. Default: the library offers the query and takes no stance; `Universal_FieldUnit.ino` logs the report at boot.

## Context: what the plant does today with a name that has no source

Every claim below was read in the cited file on 2026-11-02.

| Call | Today, when the name does not resolve | Direction |
|---|---|---|
| `Route::homeFact(const char*)` → `homeFact(const Fact*)` | the null pointer is skipped; the route has one `H` term fewer (`src/ControlTable.h:254-259`, `src/InterlockingPlant.h:889-892`) | permissive |
| `Route::proceed(const char*)` | `proceedFact_` is null; no Approach cap (`src/ControlTable.h:266-271`, `src/InterlockingPlant.h:894-897`) | permissive |
| `Route::clears({...})` | the null circuit is skipped; the track term covers fewer circuits (`src/InterlockingPlant.h:848-856`) | permissive |
| `Route::entrance()`, `approaching()` | null; no entrance or approach circuit (`src/InterlockingPlant.h:871-879`) | permissive |
| `Route::governedBy()` | null authority; the route is dark unless automatic (`src/InterlockingPlant.h:797-801`, `src/ControlTable.h:425`) | restrictive |
| `Route::displays()` | null mast; the route is skipped (`src/InterlockingPlant.h:803-807`, `src/ControlTable.h:380`) | silent |
| `AutomaticSignal::governs()`, `approach()` | null skipped; with no governed circuit the track term is trivially clear (`src/AutomaticSignal.h:85-100`) | permissive |
| `AutomaticSignal::reads(h, d)` | a null `H` is an absent term, which reads asserted (`src/AutomaticSignal.h:204-207`); a null `D` removes the Approach cap (`:104-112`) | permissive |
| `AutomaticSignal::opposing()` | null; no cut (`src/AutomaticSignal.h:118-122`) | permissive |
| `ControlPointLine::governs()`, `block()`, `leaving()` | null skipped (`src/ControlPointLine.h:104-123`); with no leaving mast, `requested`, `cleared` and `in_time` are all false, so `H` out is asserted | permissive |
| `PlantSerializer` on any of the above | the same calls, the same result; nothing reported (`src/PlantSerializer.h:1005` passes names straight through) | silent |

Two things are wrong with this and one is right. Wrong: the failure direction is permissive almost everywhere, which is the opposite of a vital circuit's (an open wire is a released relay). Wrong: nothing records that it happened, so a plant built from an incomplete model looks like a complete plant with a shorter equation. Right: the plant still runs, which is what the owner's step function needs. D1 to D3 keep the third and fix the first two.

The primitive already exists. A `LineInput` is "released until the first `update()`; released again when neither `update()` nor `touch()` arrives within the staleness timeout" (`src/LineInput.h:22-27`, `:53-55`). That is the exact semantics of a fact with no source yet: released, and alive only while something keeps it alive. ADR 0008 chose Facts as the one operand kind precisely so that a local circuit and a remote value bind the same way; a value with no source yet is the third case of the same kind.

## The step function

Reading the owner's three stages against this design. The plant, its routes and its locations are the same object in every row; only who drives each implicit fact changes.

| Stage | Hardware | Who drives the facts | What `isComplete()` says |
|---|---|---|---|
| Emulation only | none | a ghost drives every track circuit and every implicit fact (today `tools/test_ctc_desk.py --loopback` is a ghost of this kind for the desk) | false, until the model declares every source |
| Some hardware I/O | a few circuits and masts wired | drivers sample and drive the wired appliances; the ghost drives the rest | false |
| Part of a field unit deployed | this processor's appliances wired; the neighbour's values on the wire or eavesdropped (ADR 0008, the three sources) | line inputs from the carrier; no ghost | true, when every name has a source |
| Complete | all | drivers and carriers | true |

Nothing in the plant knows which row it is in (D6). The emulator decides what to drive; a sketch decides whether a false `isComplete()` means "do not drive outputs" or "log and run". The vital rule is the same in every row: a fact nobody drives reads released, and a released term is Stop.

The limit the owner named, "too incomplete to process", is D2's line. A fact can be stood in for, because a released boolean is a complete answer. An appliance cannot: a track circuit the model does not declare has no index, no code-line token and no driver, and inventing one would change the wire schema silently (the "appliance index is declaration order" invariant, `CLAUDE.md`). So a route that names a circuit the model lacks is at Stop until the model declares it, and the report says which name.

### Forward references and conflicts (D1's limit)

`findFact()` searches line inputs first (`src/InterlockingPlant.h:377-392`). If a route names `832T` before `addTrackCircuit("832T")` runs, D1 declares an implicit line input `832T`, and the later circuit is shadowed: the route reads the implicit input forever and the circuit is never consulted. Rebinding is not possible without indirection, and indirection costs every read in the scan.

Default: the "add every source before a binding names it" rule (ADR 0008, `CLAUDE.md`) stands, and `addTrackCircuit()`, `addLineInput()`, `addAutomaticSignal()` and `addControlPointLine()` check for an implicit fact of the new name and report a conflict (D3). A conflict is FieldUnit-Subdivision ADR 0011 §6's "in error", not "incomplete": the model said two things about one name. The serializer already writes sources before references and reads automatic signals in two passes, so a generated file never conflicts; the fluent API can.

## What the code becomes

- `src/LineInput.h`: a flag `implicit_`, set by the plant, read by `isImplicit()`. Nothing else.
- `src/Fact.h`: `ReleasedFact`, one static instance, `asserted()` false, name `"(unsourced)"`. The D1 overflow fallback.
- `src/InterlockingPlant.h`: `resolveFact(name, call, owner)` wraps `findFact()`; on a miss it calls `addLineInput(name)` with the default timeout, marks it implicit and records the reference; on overflow it records the reference and returns the released fact. The name-taking `Route`, `AutomaticSignal` and `ControlPointLine` calls at the bottom of the file use it for their Fact-typed arguments. `resolveAppliance(...)` for the appliance-typed ones records the miss and marks the owner incomplete. A fixed list `unresolved_[MAX_UNRESOLVED]` of `{name, call, owner, kind}` with `kind` one of `implicit`, `overflow`, `missing`, `conflict`; `unresolvedCount()`, `unresolved(i)`, `isComplete()`. `MAX_UNRESOLVED` 16; a seventeenth is counted, not stored.
- `src/ControlTable.h`: `Route::incomplete_`; `evaluateRouteIndication()` returns Stop for an incomplete route before any term is read.
- `src/AutomaticSignal.h`, `src/ControlPointLine.h`: `incomplete_`; the outputs read released while set.
- `src/PlantSerializer.h`: `lineInputs` skips implicit inputs on write. On read, nothing new: the bindings re-derive them.
- `examples/Universal_FieldUnit/`: after `deserialize()`, log each unresolved reference and `isComplete()`. The example takes no gate.

Zero heap: the implicit input is a slot of the existing `lineInputs_[MAX_APPLIANCES]` array; the report is a fixed array. Nothing new is allocated.

Rejected: a resolution policy enum (strict / lenient) on the plant. A plant that refuses an incomplete model is a plant that cannot emulate, which is the owner's first stage; a plant that accepts it silently is today. One behaviour, always reported, is simpler than a switch, and the gate belongs to the consumer (D6).

Rejected: inventing a track circuit for an unknown `clears()` name. See "too incomplete to process" above.

## What the tests prove

- A route whose `home()` names nothing readable is at Stop, the plant reports one `implicit` reference, and `isComplete()` is false. A ghost calls `update(true)` on the implicit input and the route clears. The ghost stops touching it and the route drops to Stop after the staleness timeout.
- The same for `proceed()` (Approach cap holds while released), `governs()`, `approach()`, `block()` and `reads()`.
- A route whose `clears()` names a circuit the plant lacks is at Stop whatever its other terms say, and the report says `missing` with the circuit's name.
- Seventeen distinct unknown fact names: the seventeenth binds to the released fact, reads released, and is reported `overflow`.
- `addTrackCircuit("832T")` after a route named `832T` is reported `conflict`, and the route still reads the implicit input (the documented limit).
- The serializer round trip of a plant with implicit inputs writes no `lineInputs` entry for them and re-derives them on read, with the same report.
- Every existing program passes unchanged: a complete plant has an empty report and the same behaviour as before.

## Consequences

- `docs/GLOSSARY.md` §10.5 `fact` gains "A name with no source binds to an implicit line input (FieldUnit ADR 0009 D1)". A new entry `implicit fact` [FieldUnit]. `line input` says that a line input may be declared by the plant.
- `CLAUDE.md`'s "names bind at the call" invariant is reworded: a name that does not exist yet binds to an implicit line input and is reported, not "binds to nothing, silently".
- ADR 0007 D4 (per-plant capacity) gains a reason: implicit facts spend line-input slots, so a large incomplete model hits the overflow path sooner than a complete one. The overflow path is fail-safe, so capacity remains structural, not vital.
- FieldUnit-Subdivision ADR 0011 §4 item 3 left the code generator's stance out of scope. This ADR is FieldUnit's stance as a consumer; the handoff note to Subdivision should point here, so that the generator's "missing" report and the plant's `unresolved` report use the same words for the same gap.
- The desk (`cTcMachine`) is not covered. A desk column naming a station or token that no plant produces is a different gap with a different consumer; it waits for the desk generator.

## Open questions for the owner (the "TBD stuff")

1. D2 puts an incomplete route at Stop. The alternative is "evaluate what is there", which is today's behaviour and is permissive. Is there a prototype case where a drawn-but-unwired circuit should read clear rather than occupied? If so it is a ghost policy (D5), not a plant one; confirm.
2. D6 takes no gate in the library. Should `driveOutputs()` under the `DriverPolicy` path refuse while `isComplete()` is false, as a second line behind D1 and D2? That is a policy on hardware outputs, which ADR 0004 (proposed) is redesigning.
3. The implicit input's staleness timeout: the wire default (5000 ms), or 0 (never stale, an in-memory fact)? Default here is the wire's, so a ghost that stops is noticed. A test or an emulator can set 0 per input.
4. Whether the report should name a stage (a count of ghost-driven facts, for the emulator's display). Default: no; the emulator counts implicit inputs it drives.
5. What Studio shows for a plant in this state is Studio's (FieldUnit-Subdivision ADR 0011 §6, "its diagnostics are its own display").

### The owner's answers (2026-11-02)

1. No prototype case wants a drawn-but-unwired circuit to read clear. If one appears, an `unimplemented:override` kind of attribute on the model is the shape to add; that is a ghost policy under D5 and a Layout Data Model question, not a plant one. Not built.
2. No gate. "If a device is connected to a driver, the driver should drive." Incompleteness shows as a lamp that never lights or a signal that always shows Stop, which D1 and D2 already give. `driveOutputs()` is unchanged.
3. The staleness timeout is a per-layout setting, overridable per interlocking; finer than that is not known to be needed, and a line input may still state its own. The library default of 5000 ms is a starting point only, and on the owner's figures it is on the fast side: modern railroads (the last twenty years) connected code lines over 1200 baud modems; CTC-era machines took 10 to 30 seconds to send one complete station control message; C/MRI hosts poll between one and five times a second depending on many things (the owner, 2026-11-02). A timeout shorter than the carrier's full cycle releases a healthy line, so the value belongs to the carrier, not to the plant: the generator sets the per-interlocking value from the code line's kind (reference data), and a plant on a US&S 506 line or a slow modem gets tens of seconds, a plant on MQTT or a fast C/MRI poll gets a few. Configurable at three levels: compile time (`-DFIELDUNIT_DEFAULT_STALENESS_TIMEOUT_MS=<ms>`, or the same `#define` before including `FieldUnit.h`, sets `DEFAULT_STALENESS_TIMEOUT_MS` for every plant in the binary), per interlocking (`InterlockingPlant::setDefaultStalenessTimeout()` and the JSON key `defaultStalenessTimeoutMs`, applied to line inputs and track circuits that state none and to every implicit fact), and per fact (`LineInput::setStalenessTimeout()`, `TrackCircuit::setStalenessTimeout()`, the JSON `stalenessTimeoutMs` on a line input). The per-layout value is the Layout Data Model's (`settings`, FieldUnit-Subdivision ADR 0011), from which the generator writes the per-interlocking key.

4. and 5. Owned by whatever implements the emulator and Studio, which the current code generation work does not include. Later.

## Record

- 2026-11-02: D1 to D6 ratified and implemented the same day (`src/Fact.h` `ReleasedFact`, `src/LineInput.h` `isImplicit()`, `src/InterlockingPlant.h` `resolveFact()` / `noteMissing()` / the report / `setDefaultStalenessTimeout()`, `Route`, `AutomaticSignal` and `ControlPointLine` `markIncomplete()`, `src/PlantSerializer.h`, `examples/Universal_FieldUnit`, `tests/test_unresolved_references.cpp`). All 19 programs pass; `CP_Corporal` and `Universal_FieldUnit` build for ESP32. One addition to the draft: an incomplete `AutomaticSignal` also marks its own route incomplete, so its mast is at Stop and not only its `H` and `D` released.
- 2026-11-02: drafted with the owner after ADR 0008 landed, from the owner's statement that the data model is explicitly allowed to be incomplete and that emulation, partial hardware and partial deployment are stages of one design. The table of today's behaviour came from reading the bindings in `src/` the same day. Nothing is ratified.
