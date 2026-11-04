# 0007. The interlocking–block seam in the field unit

- Status: accepted (shaped with the owner, 2026-11-02; D1 to D5 agreed by the owner 2026-11-02)
- Date: 2026-11-02
- Builds on: FieldUnit-Subdivision ADR 0013 rev 4 (§13 the functional seam, §14.1 invariants `I1` to `I4`, §14.2 the `blockRules` profiles, §15.4 the control point at the seam, §18 carriers); the handoff note `FieldUnit-Subdivision/docs/review/handoff-fieldunit-line-signals-2026-10-31.md` (rev 4, "Asked back (seam checklist)"); `docs/GLOSSARY.md` sections 10.5 and 10.6.
- Order of authority: the relay model and AAR practice govern, in two documents by scope. Inside an interlocking plant: `docs/GLOSSARY.md` section 10.6. Between interlocking plants: FieldUnit-Subdivision ADR 0013 Part 2. The seam of ADR 0013 §13 is the boundary. Where `src/` differs from either, the code has a defect.
- This is FieldUnit ADR 0007. FieldUnit-Subdivision also has an ADR 0007 (KiCad services ownership split). Cite each with its repository name.

Terms follow `docs/GLOSSARY.md`. The seven seam port names (`requested`, `cleared`, `in_time`, `home_proceed`, `segment_clear`, `next_proceed`, `traffic_free`) are ADR 0013's and are used here unchanged.

## Decisions requested

- D1. `requested` is the control as held by the field unit, `SignalControl::commandedDirection()`, not the `HSR`. The `HSR` (`activeDirection()`) drops at the entrance shunt; the CB&Q `RGZ` that ADR 0013 names is the control. Default: held control. **Agreed 2026-11-02.**
- D2. The inbound ports `segment_clear`, `traffic_free` and `next_proceed` enter the route evaluation as a new kind of route input, a line input, not as virtual track circuits. A track circuit fed from the line is allowed as a stopgap on the Sargent bench only. Default: new input kind. **Agreed 2026-11-02.**
- D3. The automatic signal location (an intermediate mast with its `SR` stick, reading `H` and `D` from ahead and producing them for the location in rear) is a class in this library beside `InterlockingPlant`, not generated code. Default: library class. **Agreed 2026-11-02.**
- D4. The fixed capacity `MAX_APPLIANCES` (16 per kind) becomes a per-plant size before any multi-control-point layout is generated. Default, revised 2026-11-02 (see Record): the sizes come from the data model and the plant allocates its containers once at startup, before the scan loop runs, and never frees them. The architectural rule is no allocation and no free after `setup()`, for deterministic timing; it is not static allocation. The first default, a compile-time parameter per sketch with the type a fixed array, is withdrawn. **Agreed 2026-11-02; default revised and agreed 2026-11-02.**
- D5. ADR numbers collide across the two repositories from 0007 on. Default: keep each repository's own sequence and always cite with the repository name, as this file does. **Agreed 2026-11-02.**

## Context

ADR 0013 rev 4 bounds its Part 2 at a functional seam. At each end of a station-to-station block the interlocking and the block exchange seven facts: four outbound from the interlocking (`requested`, `cleared`, `in_time`, `home_proceed`) and three inbound (`segment_clear` = `H`, `next_proceed` = `D`, `traffic_free` = `F`). How the interlocking evaluates its routes is this library's domain. The handoff asks this repository for one list: for each port, what `src/` exposes today and whether it is available, available but unnamed, or a defect; and the decisions the owner must make to wire the ports while preserving the four invariants.

This ADR is that reply. It also records the triage the owner asked for: for each gap, how hard it is to close.

## The seam checklist

Triage scale: **trivial** (an accessor over state that exists), **small** (one new term with tests, under a day), **moderate** (a new class or engine contribution, a few days), **structural** (changes a fixed-size type or the plant's shape), **impossible**. Nothing lands at impossible.

| Seam port | Direction | What `src/` has today | Gap | Triage |
|---|---|---|---|---|
| `requested` | out | `SignalControl::commandedDirection()` (`SignalControl.h:36`), the control as held; and `HSR()` (`:62`), which drops at the entrance shunt (`ControlTable.h:376`). Both are exported in `IndicationVector::signals` (`InterlockingPlant.h:565`). | None in code. D1 says which one the port means. | trivial |
| `cleared` | out | `SignalMast::currentIndication()` (`SignalMast.h:42`), set by the route evaluation (`ControlTable.h:312`). | None in code. The model states which masts govern into the block (ADR 0013 §20, routes library). `cleared` is "indication more favorable than Stop, Stop and Proceed or Restricting". | trivial |
| `in_time` | out | `SignalControl::isTimeLocked()` (`:39`), the `ASR` back contact, exported as `timeLocked`. | None. FieldUnit's `ASR` differs from the prototype elsewhere (GLOSSARY 10.5: it does not drop while a signal is clear), but "timer running" is exactly this port. | trivial |
| `home_proceed` | out | `currentIndication()` of the home mast facing the block. | None. Which mast is the home facing the block is a model fact. | trivial |
| `segment_clear` (`H` in) | in | Nothing by this name. A route clears when its authority has the direction, every route switch is aligned, every `clears()` circuit is vacant, and the `approaching()` circuit lowers Clear to Approach (`ControlTable.h:474`). | One new route term, a line input, ANDed into the `HR` condition (the `clearanceContribution`). Stopgap: feed `H` into a `TrackCircuit` listed under `clears()`; `TrackCircuit` already takes remote updates and flips to `LOST_COMMS` after a staleness timeout (`TrackCircuit.h:96`), and `isClear()` requires `GOOD` (`:36`), so the stopgap fails safe. | small |
| `traffic_free` (`F` in) | in | Nothing. | The same line-input term as `H` in. Under the `SP_1942` profile it is the same input as `segment_clear`. | small |
| `next_proceed` (`D` in) | in | No `DR`. GLOSSARY 10.6 "Code today": the evaluation has the `HR` conditions and no `DR`; it does not read the next signal and lowers Clear to Approach from the `approaching()` circuit (`ControlTable.h:497`). Issue #19 records that `approaching()` does two jobs. | A second trigger for the existing reduction (`reduceForOccupiedApproach`, `ControlTable.h:504`): when `D` in is released, cap the route at Approach. This is the `DR` the glossary already calls a defect; the seam work and the defect fix are one change. | moderate |

## What the checklist does not cover

Three items outside the seven ports decide whether the seam can be wired at all.

1. **The automatic signal location has no home (D3).** A mast in this library lights only through a route with a `SignalControl` authority. An intermediate signal has no authority, has an `SR` stick, reads `H` and `D` from the location ahead, and produces `H` and `D` for the location in rear (ADR 0013 §15.3). That is a new class with roughly five boolean equations and one stick, composing with the plant through the same line inputs. `SR` picks up when the governed circuit is occupied while the mast shows proceed, which is the knockdown test the engine already makes (`ControlTable.h:376`). Triage: **moderate**. The class is vital logic, and the Gherkin invariants need something fixed to run against, which is why D3 puts it in the library.

2. **Scan order is a safety matter only without `F`.** Under a profile with `trafficMechanism: "hd_chain"` and `tumbleDownTrigger: "cleared"` (`SP_1942`), a tick that both clears the last circuit of a block and evaluates a pending opposing control can strand the receding `SR` if the route engine runs before the signal locations. The fix is to evaluate the signal locations in `evaluateVitalRules` (`InterlockingPlant.h:518`) before `engine_.evaluate()` (`:561`), where `SignalControl::evaluate` already runs. Triage: **small**. With a `traffic_free` term that no stick bypasses (`CBQ_1946`, `ERIE_NW_1942`), the order is a latency matter only (ADR 0013 §18.3 item 4, invariant `I3`).

3. **Capacity is the one structural item (D4).** Every line input that rides the track-circuit stopgap spends one of 16 circuit slots per plant, and the SPCoast layout's Watsonville already has 13 switches and 45 circuits. A proper line-input kind spends no circuit slot, but the 16 limit still has to become a per-plant size before any multi-control-point layout generates. Triage: **structural**, and independent of the seam.

## Carriers (ADR 0013 §18)

| Variation | What this library needs | Triage |
|---|---|---|
| B, wired `H`/`D`(`/F`) | One `InputBit` per line on the existing `IOBus`, read into a line input. | trivial |
| C, one processor | The line inputs are plain booleans in memory. | trivial |
| A, codeline indication eavesdropping | A field unit decodes a neighbouring station's indication message: a second `AarTextCodec` instance built from the neighbour's schema (the codec is move-only and preallocates). The staleness fail-safe exists. The gate is the model carrying the neighbour's schema. Marked `non_vital` by ADR 0013. | moderate |
| D, US&S 506 | No 506 codec exists in this repository. Outside the seam. | not triaged |

## The invariants (ADR 0013 §14.1)

`I1` opposing exclusion and `I2` traffic retention hold in this library once `segment_clear` and `traffic_free` are required terms of the leaving signal's `HR` condition and the outbound ports are exported as the checklist says. `I3` receding stick release holds under item 2 above. `I4` following separation is the route's `clears()` term today plus `next_proceed` once the `DR` exists. The scenarios run against a declared fixture block, not SPCoast names (ADR 0013 §14.1 Background).

## Consequences

- Issues to file in this repository, one each: the line-input route kind (D2); the `DR` as a second reduction trigger (the glossary's open defect; relate to #19); the automatic signal location class (D3); the evaluation order of locations before the route engine; the per-plant capacity (D4). The `WireCodec` defect that writes only `NGS`/`SGS` and `NGK`/`SGK` direction letters is related (leaving signals on an E/W railroad) but not a seam port.
- `docs/GLOSSARY.md` section 10.6 gains one line mapping the seam ports onto its relays (`requested` = control held, `cleared` = `HR` front, `in_time` = `ASR` back, `home_proceed` = `HR` front of the home signal, `segment_clear` = `H` into `HR`, `next_proceed` = `D` into `DR`, `traffic_free` = `F`), and section 10.5, which names the directional stick relays `ESR` and `WSR` (likewise north and south), gains a note that ADR 0013 writes the automatic signal location's stick as `SR`.
- The handoff's "asked back" list is answered by this file. FieldUnit-Subdivision's handoff note should point here.
- No decision is open.

## Record

- 2026-11-02: D4's default revised with the owner, after ADR 0009 landed. The drawings now state each plant's counts, so the sizes come from the data model and are allocated once at startup, as the Strategy B codec buffers already are (`src/cTcMachine.h:310`, `:564`); nothing allocates or frees after `setup()`. The fit against the target's RAM and the code line type's capacity (AAR tokens have none; a US&S 506 field station carries 7 controls and 7 indications, `docs/GLOSSARY.md` section 6) is the FieldUnit code generator's check, as the data model's consumer, repeated at boot: a plant that does not fit fails at boot and says why. Office and field agree on the counts because both are generated from one model, and the wire codec already rejects a mismatch at decode. This needs no ADR of its own: which constants become per-plant, whether counts are per kind, and what the boot report prints are visible in the code when it lands.
- 2026-11-02: cross-repo review of ADR 0013 rev 3 found §16.1 assumed the plant rejects a signal control whose route is occupied; the plant stores it (`InterlockingPlant.h:422`, `SignalControl::updateCommand`). Rev 4 bounded Part 2 at the seam and added the invariants and profiles. The owner agreed D1 to D3 the same day, then D4 and D5 after the pull request was opened. This file is the reply to the rev 4 handoff.
