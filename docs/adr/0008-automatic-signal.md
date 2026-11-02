# 0008. The automatic signal: a location that reads the block ahead and produces the HD line to the rear

- Status: proposed (drafted with the owner, 2026-11-02)
- Date: 2026-11-02
- Builds on: FieldUnit ADR 0007 (D2 line inputs, D3 the automatic signal location as a library class); FieldUnit-Subdivision ADR 0013 rev 4 (§14.1 invariants `I1` to `I4`, §15.1 to §15.4 the HD line and the stick, §16 the trace, §18.2 and §18.3 eavesdropping); `docs/GLOSSARY.md` §10.6 (the `HR` and `DR` chains).
- Order of authority: as FieldUnit ADR 0007 states it. Inside the plant, GLOSSARY §10.6; between plants, ADR 0013 Part 2; where `src/` differs, the code has the defect.
- This is FieldUnit ADR 0008. FieldUnit-Subdivision also has an ADR 0008 (the Layout Data Model File). Cite each with its repository name.

Terms follow `docs/GLOSSARY.md`. `H`, `D`, `F`, `SR` and the seam port names are ADR 0013's.

## Decisions requested

- D1. **Shape.** An automatic signal is one object per mast and direction: `AutomaticSignal`. It holds the block it governs into, the block behind it, the HD line it reads from ahead, its directional stick `SR`, and its mast as an output. A location with two masts is two objects. Its display is a control-table `Route` (`InterlockingEngine`, `ControlTable.h`) with no signal control, built by the object and marked automatic; the control table evaluates a route with no signal control only when so marked. Default: as stated.
- D2. **Operands are Facts.** `TrackCircuit` implements `Fact` (`asserted()` is `isClear()`, the `TR`). The blocks of an automatic signal are lists of Facts, so a local location binds track circuits and a shadow of a remote location binds line inputs, with no second class. `MAX_ROUTE_FACTS` rises from 4 to 8. Default: yes.
- D3. **Outputs and names.** Each automatic signal produces two Facts, its HD line to the rear, named for its mast as ADR 0013 §15.1 names lines: `<mast>H` and `<mast>D` (`832SAH`, `832SAD`). `findFact()` resolves them, so a route in rear writes `home("832SAH").proceed("832SAD")` exactly as it writes a line input today. Both are functions of inputs and `SR`; nothing stored, and a re-entrant evaluation (a loop of locations) returns released. Default: yes.
- D4. **The stick and the scan.** `SR` follows ADR 0013 §15.3: it sets when the governed block becomes occupied while `HR` was up on the previous scan (the slow-release `HR`), and holds while `H` from ahead is released. The plant updates every stick before the control table evaluates the routes and records `HR` after it. The §18.3 approach-circuit guard is not implemented. Default: yes.
- D5. **The control point end.** The line a control point produces at the seam (ADR 0013 §15.4: `H` out, `D` out, `F` out, with `tumbleDownTrigger` and `trafficMechanism` as parameters) is a second small class, `ControlPointLine`, built after D1 to D4 under this ADR. Default: in the library, second step.
- D6. **Serialization and conformance.** `PlantSerializer` gains `automaticSignals` mirroring the fluent calls. A test builds the ADR 0013 §14.1 fixture block and asserts the §16 trace and invariants `I1` to `I4` against it. Default: yes.

## Picture

Mast `M` stands at location `L`, governs toward `W`, and its HD line goes back to `U`. ADR 0013 §15.3 names this with location 832 between Corporal and 834: `M` is `832SA`, `U` is Corporal (leaving signals `830SAB`, `828SA`), `W` is 834.

```
              U                        L                         W
 ===T(·→U)=== + ======T(U→L)====== + ======T(L→W)====== + ===T(W→·)===
             |--O                     |--O  M                   |--O

                 ◄──── H, D from L ────   ◄──── H, D from W ────
                 (read by U)              (read by M)

 Corporal                832T        832SA      834T          834SA
```

Each location reads the circuit ahead of it for its own display, and reports on the circuit behind it to the location in rear. The mast is where the result is shown. It is never an input.

## The equations

Transcribed from ADR 0013 §15.2 and §15.3, in the form where every term returns an indication and a route is the least permissive of its terms.

```
Read at L, for M's one route
  track term     T(L→W) clear          → CLEAR     else STOP
  home term      H_W asserted          → CLEAR     else STOP
  proceed term   D_W asserted          → CLEAR     else APPROACH     (read only while H_W)
  profile term   opposing HR and SR at L down → CLEAR else STOP      (sameLocationOpposingCut only)
  no authority term, no switch term, no approach-locking term

indication(M) = that route
HR_M          = indication(M) above the Restricting tier  =  T(L→W) ∧ H_W ∧ profile

Produced at L, sent to U
  D to U  =  HR_M
  H to U  =  T(U→L) clear ∧ (H_W ∨ SR_M)
  SR_M    sets:  T(L→W) occupied while HR_M was up on the previous scan
          holds: SR_M ∧ ¬H_W
```

Written as blocks only, with no mast anywhere: `D_W = T(W→X) ∧ H_X` and `H_W = T(L→W) ∧ (H_X ∨ SR_W)`. The relay chain's "next signal `HR` front" (GLOSSARY §10.6, `DR`) is the physical carrier of `D_W`; in a function the carrier disappears.

An automatic route is the controlled route with terms removed, not a different shape. `clears()` is the track term, `home()` the `H` term, `proceed()` the `D` term, all of which FieldUnit ADR 0007 D2 added. A controlled route adds the authority (`HSR`), switch correspondence, opposing-signal and approach-locking terms. One route function serves both; that is why D3 of ADR 0007 put the location in the library.

`SR` enters only the `H` that `M` sends to `U`, never `M`'s own display. It masks the break ahead when a train moving away in `M`'s direction caused it (the tumble-down, §15.3).

## The three sources of a line, and why they are one class

A route reads the HD line from the location ahead. That line reaches the route in one of three ways (ADR 0013 §18):

1. **Wired or topic (variations B, C):** `H` and `D` arrive as line inputs (`LineInput`, ADR 0007 D2).
2. **Local location (variation C, or two locations on one processor):** an `AutomaticSignal` on this processor computes them from its track circuits and its own line from ahead.
3. **Eavesdropping (variations A, D):** nothing arrives as `H` or `D`. The neighbour's codeline indications arrive (`836TK`, `836NGK`, `836TEK`, `836SGK`), and this processor computes the neighbour's `H` and `D` from them (§18.2: `H = ¬836TK ∧ ¬836NGK ∧ ¬836TEK`, `D = 836SGK`). That is the §15.4 control point function evaluated here as a shadow of the far location, with its block term bound to a line input carrying `TK` instead of a track circuit.

The third source is what fixes the shape: the location functions take Facts, and a track circuit is a Fact (D2). The generator then instantiates the same class on each processor and binds local circuits or line inputs as the deployment variation dictates. Staleness of a line input releases it (ADR 0007 D2), so a shadow fails toward Stop as §18.1 requires. The shadow is the `R15` non-vital compression; the model and the drawing carry that mark, the code does not need to.

## What the code becomes

Fluent form, for location 832 of the picture, on the processor that owns `832T` and `834T`:

```cpp
cp.addTrackCircuit("832T");
cp.addTrackCircuit("834T");
cp.addLineInput("834SAH");                 // the line from 834, over a wire or a topic
cp.addLineInput("834SAD");

cp.addAutomaticSignal("832SA", MastType::ONE_HEAD)
  .governs({ "834T" })                     // T(L→W): the block it displays for
  .approach({ "832T" })                    // T(U→L): the block it reports on
  .reads("834SAH", "834SAD");              // H_W, D_W
  // .opposing("832NA")                    // sameLocationOpposingCut, when the profile says so

cp.route("LEAVING-SOUTH")                  // a Corporal leaving signal, in the same plant or not
  .governedBy("830", DirectionAuthority::RIGHT)
  .displays("830SAB", Indication::CLEAR)
  .clears({ "830T1" })
  .home({ "832SAH" })                      // segment_clear
  .proceed("832SAD")                       // next_proceed
  .entrance("830T1");
```

Where `832SA` runs on another processor, the two `addLineInput` calls name `832SAH` and `832SAD` instead, and the route is unchanged. Where Corporal eavesdrops on a neighbour's code line, a `ControlPointLine` (D5) bound to line inputs produces the same two names.

Inside `src/`:

- `src/AutomaticSignal.h` (new): the class, its two output Facts, `SR`, the previous-scan `HR` memory, and a re-entrancy guard on the `H` output. `FieldUnit.h` includes it.
- `src/TrackCircuit.h`: `class TrackCircuit : public Fact`, `asserted()` returns `isClear()` (`TrackCircuit.h:36`). `name()` already exists.
- `src/ControlTable.h`: `MAX_ROUTE_FACTS` 8 (`:18`); a route flag `automatic`; `evaluate()` no longer skips a marked route for a null signal control (`:413`), and `authorityContribution()` returns the ceiling for a marked route (`:556`). Everything else in the route walk, the mast accumulation and the time locks is unchanged. The mark matters because one other route is declared today without `governedBy()`: the engine-return route in `tests/test_tracer_bullet.cpp`. On the prototype a return movement to equipment left on the main track is authorized by the train dispatcher or control operator (GCOR Rule 6.20), so that route's missing signal control is the defect GLOSSARY §10.6 already records for `engineReturn()`, not a second kind of authority-less route; the mark leaves it as it is until that defect is fixed.
- `src/InterlockingPlant.h`: `automaticSignals_[MAX_APPLIANCES]`, `addAutomaticSignal()`, `findFact()` searches line inputs, then automatic signal outputs; `evaluateVitalRules()` updates every stick after the staleness ticks and before the control table's `evaluate()`, and latches `HR` after it. That is the evaluation-order item of ADR 0007's consequences, closed here.
- `src/PlantSerializer.h`: `automaticSignals` with `name`, `type`, `aspectPolicy`, `governs`, `approach`, `reads` (`h`, `d`), `opposing`.

Rejected shape: `SignalMast` or a per-mast relay object as the Fact the route in rear reads. A mast is an output; the route in rear depends on the block ahead, and reading a mast made one mast depend on another mast, which is the wrong shape (the owner, 2026-11-02).

## What the tests prove

- Unit: an `AutomaticSignal` with its governed block, its line from ahead and its stick, through Stop, Approach and Clear; `H` out masked by `SR` behind a receding train; released on a stale line input; released on a loop of locations.
- Conformance: the ADR 0013 §14.1 fixture, control points West and East, circuits `1T` and `2T`, location 2 with masts `2E` and `2W`. The control point ends are driven by hand as line inputs until D5 lands. The §16 trace runs row by row, and `I1` to `I4` are asserted at each row.
- The existing 17 programs still pass; `CP_Corporal` still compiles for ESP32.

## Consequences

- FieldUnit ADR 0007's consequences list five issues to file. This ADR closes three of them without issues: the `DR` trigger for a local next signal (it is `proceed()` reading a produced `D`), the automatic signal location class (D1), and the evaluation order (D4). The line-input kind is done (ADR 0007 D2, implemented 2026-11-02). Capacity (ADR 0007 D4) stays its own issue.
- Item 2 of the ADR 0007 follow-ups, "the `DR` term reads a local next mast", is withdrawn. Nothing reads a mast.
- GLOSSARY §10.6 `DR` "Code today" is rewritten when D1 to D4 land: the next-signal condition is the `D` produced by the location ahead, local or remote. The `HR`, `DR` entry in §10.5 keeps "FieldUnit has no `HR` or `DR` object": they are evaluated, not stored.
- `SR` holding while `H` from ahead is stale is the relay's behaviour on a broken line (ADR 0013 §15.3, [CBQ1946 pp. 615–616]). The conformance test records what the trace does with it; if `I3` fails under a stale line, that is a profile question for ADR 0013, not a code change here.
- The `approaching()` double duty (issue #19) is untouched. An automatic signal names its blocks `governs()` and `approach()` in the prototype's sense, in advance and in rear of the signal.
- The AAR signaling primer §8.5 gains the picture and the block-only equations with D1 to D4.

## Record

- 2026-11-02: drafted with the owner after the item 2 discussion. The owner's rulings that shaped it: the mast is an output and never an input to another mast's equation; the number plate changes the aspect the crew reads, not the indication; eavesdropping is the location function run on remote operands.
