# 0010. The train order board: a face is a lever and a mast, outside the interlocking

- Status: proposed (drafted for the owner, 2026-11-10; D1 reworded and the binding keys added 2026-11-12; not yet ratified)
- Date: 2026-11-10
- Builds on: `docs/GLOSSARY.md` entries train order signal, timetable station, Form 19, Form 31; FieldUnit-Subdivision `docs/review/signal-symbols-worklist.md`, "Train order boards" (ruled 2026-11-05, reworked 2026-11-08), items 1 to 6; FieldUnit-Subdivision `tools/layout_model/reference.py` (`TRAIN_ORDER_INPUTS`, `TRAIN_ORDER_CLEAR`); FieldUnit ADR 0003 (D8 a local lever is a local client; D9 heads bind, not masts); FieldUnit ADR 0008 (a mast is an output and nothing reads it); FieldUnit ADR 0009 (the direction a gap fails in).
- Order of authority: as FieldUnit ADR 0007 states it. This ADR adds no relay and no vital logic. Every prototype statement it relies on is in the glossary entry, where it is marked unverified.
- Closes, when implemented: FieldUnit issue #44.

Terms follow `docs/GLOSSARY.md`. The model's word is *train order board*. A *face* is one side of a board: what the trains of one direction see.

## Decisions requested

- D1. **A face is one logic block: a ranked list of inputs, one indication out.** In each scan the face reads all of its inputs together and shows the indication of the highest-ranked input that is asserted. With no input asserted, the face shows its rest indication. The rank is fixed at setup. It is not the order in time in which the inputs became asserted, and the face keeps no state from one scan to the next, so the result depends only on the present inputs. STOP ranked above SLOW is "the more restrictive input wins". A new class, `TrainOrderFace`, holds this rule and nothing else. Default: yes.
- D2. **The words come from the model, not from `src/`.** At setup the generated sketch gives the face its ordered pairs and its rest word: (`STOP` → STOP), (`SLOW` → APPROACH), rest CLEAR. These are the model's `TRAIN_ORDER_INPUTS` and `TRAIN_ORDER_CLEAR`. Default: yes. The alternative is a class with STOP, SLOW and CLEAR fixed in it. It is a few lines smaller, and it states the reference data a second time.
- D3. **The output of a face is a `SignalMast` with one head.** The face sets its indication; nothing else does. The aspect policies and the head drivers that exist then serve a train order head with no change (see Context). Default: yes.
- D4. **A face is outside the interlocking.** It is not declared with a `cp.add…` call. It is not in `tick()`, in `ControlTransaction`, in `IndicationVector`, in the wire schema or in the plant JSON. The sketch evaluates it one time in each loop, beside `executeCycle()`. Default: yes.
- D5. **A board is its faces, and a face takes its identity from the model.** There is no board class. A single board is one face and a double board is two. The model's head and lever records each name their mast and their face (`relations.mast`, `relations.face`), and their binding keys carry both: `SIGNALMAST.Gilroy.E`, `SENSOR.Gilroy.E.STOP`. The generator makes one `TrainOrderFace` for each face and binds it by those keys. FieldUnit adds no naming rule. Default: yes.
- D6. **A face with no asserted input shows CLEAR, including a face whose lever is not wired, is broken or was never sampled.** This is the Subdivision ruling (item 5). It is the opposite of FieldUnit ADR 0009, where a gap fails toward Stop. The reason is that the board is not vital and is not a block signal: a false Clear on it does not admit a train to occupied track. Default: yes, as ruled. It is listed as a decision so that the difference from ADR 0009 is seen one time and agreed.

## Context

```
   fascia lever (Local-TrainOrder)         TrainOrderFace               head (Device-Head-*)
   STOP ──┐                              ┌──────────────────┐
          ├── inputs, ranked ──────────▶ │ highest asserted │ ── Indication ──▶ SignalMast ──▶ head driver
   SLOW ──┘                              │ wins, else rest  │                   (one head)
                                         └──────────────────┘
                 no route, no signal control, no track circuit, no code line
```

What the model gives (read on FieldUnit-Subdivision `origin/main` at da2a765, `schemas/layout-model/examples/spcoast.layout.json`, schema `layout-model/v3.2.0`, 2026-11-12): a board has a Name, a `Station` and a Style `single` or `double`. Each face has one lever with outputs `STOP` and `SLOW`, asserted low, and one head. The Gilroy board is a double with faces `E` and `W`, a three-position upper quadrant head (Style `UQ3`) on each. Its bindings are:

| Face | Head | Lever `STOP` | Lever `SLOW` |
|---|---|---|---|
| `E` | `Gilroy/SIGNALMAST.Gilroy.E/IND` | `Gilroy/SENSOR.Gilroy.E.STOP/S` | `Gilroy/SENSOR.Gilroy.E.SLOW/S` |
| `W` | `Gilroy/SIGNALMAST.Gilroy.W/IND` | `Gilroy/SENSOR.Gilroy.W.STOP/S` | `Gilroy/SENSOR.Gilroy.W.SLOW/S` |

SPCoast has no single board. The key of a single board's one face, which carries no direction, is unverified.

What `src/` has today. Each line was read in the cited file on 2026-11-10.

- `SignalMast::setIndication()` is public and sets the heads through the mast's aspect policy (`src/SignalMast.h:102-109`). A mast need not belong to a plant.
- With one head, the default policy shows CLEAR as green, APPROACH as yellow and STOP as red (`src/SignalAspectPolicy.h:39-55`).
- `SemaphoreDriver` moves an arm to its clear, approach or stop angle from the head's green, yellow or red (`src/drivers/SemaphoreDriver.h:43-64`). `SignalMastDriver` lights a color-light head from the same aspects (`src/drivers/SignalMastDriver.h:40-87`). `MqttApplianceBus::bindSignalMast()` takes one mast (`src/drivers/MqttApplianceBus.h:104`).
- A mast that the plant owns has an index and a place in `IndicationVector::masts` (`src/InterlockingPlant.h:95-96`, `:812-822`), and a route can name it in `displays()`. A train order mast must have neither. That is the reason for D4.

Polarity and contact bounce are not decisions here. "Asserted low" is a property of the input pin (FieldUnit ADR 0003 D6), and the I/O system is presumed to debounce switch inputs (the owner, 2026-11-12). The face receives logical, settled values and does no filtering of its own. A lever that opens one contact before it closes the next can show CLEAR for a moment between two positions. That is accepted: the board is not vital (the owner, 2026-11-12).

The shape the decisions give. It is an illustration; the names can change at review.

```cpp
SignalMast     gilroyE("Gilroy.E", MastType::ONE_HEAD);
TrainOrderFace faceE(&gilroyE, Indication::CLEAR);              // the rest word (D2)

void setup() {
    faceE.input(Indication::STOP).input(Indication::APPROACH);  // the order of the calls is the rank (D1)
}

void loop() {
    executeCycle();                                             // the interlocking, unchanged (D4)
    faceE.set(0, io.readBit(gilroyE_STOP));
    faceE.set(1, io.readBit(gilroyE_SLOW));
    faceE.evaluate();                                           // sets the mast (D3)
    gilroyE_arm.drive(io);
}
```

## Consequences

- One new header, `src/TrainOrderFace.h`, included from `FieldUnit.h`, and one test program. No change to `InterlockingPlant`, the engine, the codecs or `cTcMachine`.
- The face has no allocation and a fixed, small number of inputs. Two are used today.
- A train order mast never reaches the dispatcher. If a layout wants the desk to repeat a board, that is a new indication on the code line and a new decision.
- The glossary entry "train order signal" changes its FieldUnit line from "no appliance yet" to this ADR.
- FieldUnit issue #44 still lists a `CALLON` input that shows RESTRICTING. The 2026-11-08 ruling removed it. The issue text must follow.

## Not decided here

- How a lever input and a head are driven. That is the driver seam (FieldUnit ADR 0003 and 0004, both proposed). The illustration uses the per-appliance drivers that the example sketches use today.
- The time a semaphore arm takes to move. That is the head driver's.

## Record

- 2026-11-12: FieldUnit-Subdivision issue 85 is closed and the compiled SPCoast file carries a distinct key for each face's head and for each lever output. The paragraph that held the bindings open is replaced by the keys, and D5 states where a face's identity comes from. D1 now says "ranked": the owner asked whether "the first asserted input wins" hid a race, and it does not. The owner ruled that the face needs no debounce and that a moment of CLEAR between lever positions is acceptable.
- 2026-11-10: drafted while the layout compiler work was in flight in FieldUnit-Subdivision. No code is written. An earlier suggestion in the session, that a face rest at Stop, is withdrawn in favor of the ruling (D6).
