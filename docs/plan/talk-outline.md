# Talk Outline: From the Switch Tender to Your Layout

Draft 1, 2026-11-03. Deliverable 1 of `docs/plan/user-docs-plan.md`.
Audience: NMRA Layout Design and Operations SIGs, in person, early 2027. One hour.
Tone: approachable, interesting, fun. Pure story. Optional demo hooks are marked **[demo, optional]**.

How to read this outline:

- One section for each spine segment of the plan, about 5 minutes each.
- Each section has the same fields: the problem, the invention, the terms (3 to 6, spelled as in `docs/GLOSSARY.md`),
  the one figure, the relay layer, the East/West sidebar, the closing line, and sources.
- **Sources** use the keys of `docs/GLOSSARY.md` section 12 where a key exists.
  A claim with no opened source is marked **[cite?]**. Do not present a [cite?] claim as fact on a slide until it has a source.
- **Figures** are placeholders. Each one names the KiCad symbols it needs, in the symbol language of the Railroad library.
  A symbol that is not known to exist is marked **(symbol needed)**. The figures are drawn in deliverable 3.
- "What FieldUnit does" lines cite `docs/GLOSSARY.md` or `src/`. Where FieldUnit does not do it yet, the line says so.
- The term assignment for every glossary entry, including the ones the talk does not say aloud, is in
  `docs/plan/glossary-segments.md`.

## Running order and timing

| # | Segment | Min | Clock |
|---|---|---|---|
| 0 | Cold open | 2 | 0:00 |
| 1 | The switch tender | 5 | 0:02 |
| 2 | Fixed signals | 5 | 0:07 |
| 3 | The tower and the lever frame | 5 | 0:12 |
| 4 | Track circuits | 5 | 0:17 |
| 5 | Power interlocking | 5 | 0:22 |
| 6 | Automatic block (ABS/APB) | 5 | 0:27 |
| 7 | Train control | 4 | 0:32 |
| 8 | CTC, the lever frame cut in two | 6 | 0:36 |
| 9 | The dispatcher's desk | 5 | 0:42 |
| 10 | Digital code lines to PTC | 4 | 0:47 |
| 11 | Your layout as a railroad | 6 | 0:51 |
| | Questions | 3+ | 0:57 |

Segments 7 and 10 are short on purpose: they are history the project does not implement (plan principle 7).
Segment 8 gets the extra minute because it is the hinge of the talk.

---

## 0. Cold open (2 min)

- Open on one picture: a CTC machine (`docs/images/SPCoast-cTc-panel.png`) next to a pile of glass-cased relays (photo needed).
- The hook: "Every lever and every lamp on this desk is the answer to a wreck, a lawsuit, or a payroll.
  In the next hour we walk a hundred years of railroad history, and we pick up the vocabulary one era at a time.
  By the end, the desk will read like a sentence."
- State the two axes once: **era** explains why a word exists; **region** (East and West, for now) explains why
  railroads did it differently. Traffic density and money drove most of the differences.
- State the promise: no electronics, no code. One drawing language, used from the first slide to the last.

---

## 1. The switch tender (5 min)

**Era.** From the first railroads to the late 1800s; on light lines, much longer [cite?].

**The problem.** One track, trains in both directions, and switches that a person must line by hand.
A switch left in the wrong position puts a train into a siding at speed. Two trains on one track meet head on.

**The invention.** Two things that are not hardware:

- The switch stand with a target, thrown by hand, and a person responsible for it [cite?].
- The timetable and the train order: authority to occupy the track comes from paper, by telegraph [cite?].
  The first telegraphic train order is often credited to Charles Minot on the Erie, 1851 [cite?].
- The derail, which sacrifices a car to save the main line.

**Terms.** switch, manual switch, derail, fouling point, yard limits.

**Figure.** The end of a siding, hand-operated: a switch drawn with Style `manual`, a derail on the siding,
the fouling point marked, and a `Rule93-Yard Limits` sign at the edge. A switch stand target (symbol needed).

**Relay layer.** None yet. The protection is a person and a rulebook: Standard Code Rule 93 (yard limits) [PRR56]
and SP Rule 105 (entering a siding) [owner's transcription, GLOSSARY section 9]. Say this aloud: everything after this
segment replaces a person's attention with a circuit.

**East/West.** East: double track came early on dense lines, and with it the rule to keep right (Rule D-151 [PRR56]).
West: long single track with sidings, where timetable and train order lasted for decades [cite?].
FieldUnit setting: the switch Style (`manual`, `manual_sensed`, `powered`, `dual`) [GLOSSARY section 7].

**What FieldUnit does.** A manual switch is part of the model; a derail rests in NORMAL, the derailing position
[GLOSSARY section 7, derail].

**Closing line.** "On a model railroad, this is a ground throw, a car card and a crew that reads the timetable."

**Sources.** [PRR56] for Rules 93 and D-151; [CFR218] and [GCOR6] for the derail position. [cite?] for the switch stand,
the train order history and the Minot date.

---

## 2. Fixed signals (5 min)

**Era.** Mid-1800s to the 1920s: balls and boards, then semaphores, then color lights [cite?].

**The problem.** A train at speed cannot stop to ask. The engineer needs an instruction at a fixed place, from far away,
day and night.

**The invention.** A fixed signal: a mast with one or more heads whose appearance (the aspect) has one meaning in the
rulebook (the signal indication). The semaphore arm by day, a lamp behind a colored roundel by night.
Lower-quadrant arms had two positions; upper-quadrant arms, from 1903, added Approach [GLOSSARY section 7, semaphore
signal; AAR_SIGNALING_PRIMER section 17; date needs a primary source: cite?].

**Terms.** signal, mast, head, aspect, signal indication, semaphore signal.

**Figure.** Three masts side by side, each showing Approach: a `UQ` semaphore, a `CL` searchlight head and a `CPL`
three-lamp head. One two-head mast named `784EAB`, with the aspect (red over yellow) labeled on one side and the
signal indication on the other. A lower-quadrant `LQ` arm with a note "no Approach before 1903".

**Relay layer.** The equation of the segment: aspect = rulebook(signal indication, mast). The indication is what the
interlocking decides; the aspect is how one railroad, in one era, shows it [GLOSSARY section 8, aspect and signal
indication].

**East/West.** Route signaling and speed signaling. Western roads such as the SP showed which route was lined;
eastern roads such as the NYC and PRR showed what speed was allowed, at the cost of more heads [cite?]. Each road also
picked its own hardware: PRR position light, B&O position-color light, searchlights on many western roads [cite?].
FieldUnit setting: the aspect policy of each mast, `sp1969`, `sp1985`, `gcorSpeed`, `nycSpeed`, `prrPositionLight`,
`boCpl`, `upperQuadrantSemaphore` (`src/SignalAspectPolicy.h:95`, `:108`, `:134`, `:152`, `:242`, `:326`, `:287`).

**What FieldUnit does.** The interlocking logic computes an `Indication`; the mast's `AspectResolver`
(`src/SignalAspectPolicy.h:24`) turns it into lamps. A two-position head on a route whose era asks for Approach is an
era error, not a wiring error [GLOSSARY section 7, semaphore signal].

**Closing line.** "On a model railroad, this is an LED or a servo on a mast, and a table that says what it means."

**Sources.** [AAR56] for aspects and indications. [cite?] for the 1903 date, route versus speed signaling by region,
and which roads used which hardware.

**[demo, optional]** One mast switching aspect policy live, SP 1969 to NYC speed, same indication.

---

## 3. The tower and the lever frame (5 min)

**Era.** From the 1850s in Britain and the 1870s in North America [cite?] to the end of the mechanical plant.

**The problem.** At a junction or a crossing at grade, several switches and signals must agree.
A tower operator in a hurry can clear a signal over a switch that is wrong.

**The invention.** The interlocking machine: a frame of levers, connected by pipe and wire to the switches and signals,
with a locking bed under the levers. A lever that must not move does not move. The tower operator feels the refusal
in the hand [GLOSSARY section 2.3].

**Terms.** interlocking, interlocking machine, tower, tower operator, lever, interlocking plant.

**Figure.** A small junction: a crossover `815`/`815A`, two home signals, and the interlocking limits shaded between
them. Under it, the lever frame (symbol needed) with numbered levers and one line of a locking sheet:
"lever 2 reversed locks lever 815 normal". The lever positions follow AAR56: N and R for a switch, L, N, R for a signal
[GLOSSARY section 5, lever].

**Relay layer.** The locking sheet is a truth table before there were relays. "2R locks 815N" reads as: signal 2 may
clear only if switch 815 is normal. This is the route, written as locking (route is not yet a glossary entry; see the
gap list in `glossary-segments.md`).

**East/West.** The cost of a tower was its operators, three shifts a day. East: dense traffic paid for towers every
few miles on multi-track main lines [cite?]. West: towers mostly at junctions and crossings at grade [cite?].
FieldUnit setting: none directly. The tower operator role is not yet in the model [GLOSSARY section 4, tower operator].

**What FieldUnit does.** Routes in the interlocking model (`aligns`, `clears`, `governedBy`, see CLAUDE.md "JSON and
the fluent API") are the locking sheet. FieldUnit has no tower lever frame yet; the interlocking machine is in the
library's scope statement (AGENTS.md, "Interlocking Towers") but not in the glossary as a FieldUnit term.

**Closing line.** "On a model railroad, this is the control panel at a yard throat, one that refuses to line a
conflicting route."

**Sources.** [AAR56] for lever positions. [cite?] for the first interlocking dates and tower spacing.
Odd switch levers and even signal levers: a habit, not a rule [GLOSSARY section 10.2, unverified].

---

## 4. Track circuits (5 min)

**Era.** The closed track circuit is usually credited to William Robinson, 1872 [cite?].

**The problem.** The tower operator cannot see under the train. A switch thrown under a moving car derails it.
A signal cannot know that the block ahead holds a train, or that a rail is broken.

**The invention.** A battery at one end of a section of rail, a relay at the other, insulated joints at both ends.
Wheels and axles shunt the current and the track relay drops. A broken rail or a dead battery also drops it.
The design fails to "occupied", which is the safe answer [GLOSSARY section 10.5, `TR`; primer section 2].

**Terms.** track circuit, `TR`, OS section, detector locking, approach track circuit.

**Figure.** One track circuit between two insulated joints (`IRJ-Signal` symbols at the ends), the battery and the
`TR` coil, drawn twice: vacant (relay up) and shunted by a wheelset (relay down). Use the ASCII sketch of GLOSSARY
section 10.5 as the brief. A second panel: the OS section over switch 783, named `783T1`.

**Relay layer.** `TR` up means vacant. Detector locking is one contact in the switch lock circuit:
`1WLR = 1TR front · no route lock · no time lock` [GLOSSARY section 10.5, `WLR`].

**East/West.** Not a regional story. Use the slot for the modeler's sidebar instead: current sensing, optical sensors
and axle counters are all one `TR` in the model; sensors on one input are one track circuit
[GLOSSARY section 7, aggregation].

**What FieldUnit does.** A track circuit starts OCCUPIED; a remote circuit with no update for 5000 ms is `LOST_COMMS`
and not clear; `addSwitch(id, os)` binds the detector lock; a dropout delay bridges gaps between cars for optical
sensors [GLOSSARY section 7, track circuit; section 8, detector locking].

**Closing line.** "On a model railroad, this is your block detector, and it must say 'occupied' when it fails."

**Sources.** [AAR56] p. 31 for track circuit naming. [BNSF] for "OS" as "on sheet". [cite?] for Robinson and 1872.

---

## 5. Power interlocking (5 min)

**Era.** Electro-pneumatic plants from the late 1800s; all-relay interlocking by the 1930s [cite?].

**The problem.** Pipe and wire limit how far a lever can reach and how many levers one person can pull.
Big terminals need more. Now that the track circuit exists, the locking should use it.

**The invention.** Power switch machines and relays. The locking moves from steel bars to relay contacts, and gains
new rules that a mechanical bed could not hold: route locking, approach locking and time locking.
Relays are vital: gravity drops the armature when power fails, so every failure falls to Stop [primer section 2].

**Terms.** powered switch, switch correspondence, route locking, approach locking, time locking, vital.

**Figure.** A powered switch with its points detected, feeding `1NWCR`, `1RWCR` and `1KR` in the written circuit
form of GLOSSARY section 10.5. Beside it, the `2HR` and `2ASR` circuits of section 10.6. Relay coil and contact
symbols in the KiCad library (symbol needed, unless the Railroad library already has them).

**Relay layer.** This is the segment where the equations live:

- `2HR = 2HSR · route KR fronts · route TR fronts · opposing ASR fronts` [GLOSSARY section 10.6].
- `2ASR` up while the signal is at stop and the approach is clear, or the train accepted the signal,
  or the time element has run [GLOSSARY section 10.6].
- Front contact is `x`, back contact is `!x`, series is `&&`, parallel is `||` [GLOSSARY section 10.4].

**East/West.** Thin. Plant size, not region, decided power interlocking: terminals and big junctions first,
on any road that could pay for them [cite?]. Proposal: drop the sidebar here and say so.

**What FieldUnit does.** `InterlockingEngine::evaluateIndication` has the `HR` conditions; it has no `DR` and lowers
Clear to Approach from the `approaching()` circuit; `ASR()` is false only while the timer runs (default 30 s)
[GLOSSARY sections 8 and 10.6, "Code today"]. Say plainly on the slide: where the code differs from the circuit,
the code has the defect (FieldUnit ADR 0007).

**Closing line.** "On a model railroad, this is the logic behind your panel: not a diode matrix, but rules that fail
to Stop."

**Sources.** [AAR56] p. 37 for `HSR`. [AARNOM46] for written-circuit form. [cite?] for electro-pneumatic and relay
interlocking dates.

---

## 6. Automatic block, ABS and APB (5 min)

**Era.** Automatic block signals from the 1870s; absolute permissive block (APB) on single track from the 1910s [cite?].

**The problem.** Between stations, trains still run on timetable and train order. A following train runs into the
one ahead; on single track, two trains meet head on.

**The invention.** Signals with no person behind them. Track circuits drive the signals between control points.
On single track, APB adds a direction: once a train enters at a head block, the opposing signals are held at Stop all
the way to the next siding.

**Terms.** block, automatic signal, signal location, cut section, traffic, direction of traffic, current of traffic.

**Figure.** Single track between two sidings. Head-block masts with `Kind` `ABSOLUTE` at each end, back-to-back
intermediate masts with `Kind` `PERMISSIVE` and number plates (`IRJ-Signals`), and the `HD` line drawn as arrows
running from each signal location back to the one before it. A `RULE_261` sign on the single track; a `RULE_251` sign
on a double-track inset.

**Relay layer.** The `HD` line: `H` feeds the receiving signal's `HR` when the track ahead is clear; `D` feeds its
`DR` when the next signal shows Approach or better [GLOSSARY section 7, HD line]. The directional stick relay `SR`
lets a following train close up behind a receding one [GLOSSARY section 10.5, `SR`].

**East/West.** East: double track with a current of traffic, Rule 251, signals for following moves only [PRR56].
West: single track, where APB was the cheap way to get signals in both directions without CTC [cite?].
Absolute head block: most restrictive indication Stop (Rule 292); permissive with number plate: Stop and Proceed
(Rule 291) [AAR56 pp. 51, 53–54]. FieldUnit setting: mast `Kind`, `Plate-G` and `Plate-R` markers, `RULE_251` and
`RULE_261` signs [GLOSSARY sections 7 and 9].

**What FieldUnit does.** Claims stop here. The model has no block or `TP` entity yet (FieldUnit-Subdivision issue
#24); FieldUnit has no `SR` object; FieldUnit ADR 0007 (accepted 2026-11-02) puts `SR` in the automatic signal
location class. The `HD` line is the design of FieldUnit-Subdivision ADR 0013 Part 2 [GLOSSARY sections 7 and 10.5].

**Closing line.** "On a model railroad, these are the signals between your sidings, the ones that run themselves."

**Sources.** [AAR56], [PRR56], [CBQ1946], [RS1947] p. 92 for line circuits, [CFR236] §236.769 for traffic.
[cite?] for ABS and APB dates and for APB as the western single-track answer.

---

## 7. Train control: ATS, cab signals, ATC (4 min)

**Era.** The ICC orders of the 1920s; the 1947 ICC order that set the 79 mph limit [cite?].

**The problem.** Signals work only if the engineer sees and obeys them. Fog, fatigue and a missed signal still
wreck trains.

**The invention.** Put the signal in the cab and let it act on the brakes:
automatic train stop (intermittent inductors at the signal), continuous cab signals carried by coded track circuits,
and automatic train control that enforces speed [cite?].

**Terms.** None in the glossary. This segment is a gap: ATS, cab signal, ATC, coded track circuit and train control
have no entries (see `glossary-segments.md`). The only trace is the directional coded-track naming form under
track circuit [GLOSSARY section 7; RS1944].

**Figure.** One block with a track inductor at the signal and a cab signal display in the locomotive cab
(symbols needed; no Railroad library symbols exist for train-borne equipment, and none should, because the project
does not model it). A photo may serve better than a drawing here.

**Relay layer.** Coded track circuit: the rate at which the track current is pulsed carries the aspect to the cab
[cite?]. One line only; no equation.

**East/West.** This is the plan's "79 mph limit as the West's answer". After the 1947 order, roads with dense,
fast passenger service (PRR, NYC and others) installed cab signals or train stop; many western roads kept speeds
under 80 mph instead of paying for it [cite?, all of it]. Money and density, stated as plainly as anywhere in the talk.
FieldUnit setting: none. Train speed belongs to the throttle.

**What FieldUnit does.** Nothing, by design. FieldUnit does not control train speed (plan principle 7).

**Closing line.** "On a model railroad, this is the engineer. That is you. FieldUnit stops at the signal; the throttle
is yours."

**Sources.** [RS1944] for coded track circuits with CTC and cab signaling. [cite?] for every date, order and
regional claim in this segment.

---

## 8. CTC, the lever frame cut in two (6 min)

**Era.** 1927: the first CTC on the New York Central, Stanley to Berwick, Ohio, built by GRS [EKEVING, one secondary
source]. Two-wire coded lines by the 1930s [RS1937].

**The problem.** Single track is full. Train orders are slow, towers cost three shifts each, and double track costs
more than either.

**The invention.** Cut the interlocking machine in two. The levers go to the dispatcher's office; the locking stays
in the field. A code line carries the dispatcher's intent out and the truth back. The CTC lever is free: it can be
moved to an unsafe position, and the field refuses by not acting. The office learns of the refusal later, by absence
[GLOSSARY section 2.3].

**Terms.** CTC, code line, control, office indication, office correspondence, field station.

**Figure.** The README's two-panel drawing, redrawn in KiCad symbols: on the left, a tower with levers, locking bed and
track; on the right, a `PanelColumn` with lever and lamps, the code line, and a `MAIN HOUSE` / `House` at the control
point with switch 783 and signal 784. Tokens on the wire: `783NWS` going out, `783NWK` coming back.

**Relay layer.** No new relays. The same `HR` and `WLR` circuits now take their lever input from a received control.
The one sentence: the code line, the field station and the field unit together replace the locking bed; they do at a
distance, and after the fact, what the locking bed does in the operator's hand [GLOSSARY section 2.3].

**East/West.** West: CTC paid for itself on long single track and put off double track; the SP installed it through
the 1940s [RS1942, RS1945, RS1947]. East: CTC also let roads run fewer main tracks in both directions [cite?].
FieldUnit setting: the code line type, US&S 506 style or AAR tokens over MQTT [GLOSSARY section 3, code line type].

**What FieldUnit does.** A control transaction is applied whole or not at all; an unsafe one has every vital control
ignored together, and no refusal is sent (FieldUnit ADR 0002) [GLOSSARY section 5, control transaction].
`AarTextCodec` writes the tokens.

**Closing line.** "On a model railroad, this is the dispatcher's panel at the end of the aisle, and a box under the
benchwork at each siding."

**Sources.** [EKEVING], [RS1937], [RS1942], [RS1945], [RS1947], [USS1949] for "C.T.C. control machine",
[CFR236] for "traffic control system". [cite?] for the eastern track-reduction claim.

**[demo, optional]** The SPCoast CTC machine with `tools/test_ctc_desk.py --loopback` as a ghost plant: throw a
switch, press CODE, watch the lamp go dark and come back. With `CODELINE_VISUAL_STEPPING` on, the lamps step out the
code at about prototype speed (AGENTS.md; slow, so use it for one move only).

---

## 9. The dispatcher's desk (5 min)

**Era.** 1930s to 1980s: the US&S and GRS CTC machines; a pushbutton console by 1959 [RS1959].

**The problem.** One person, many control points, a whole subdivision. The dispatcher must see where every train is,
keep traffic moving, and handle the exceptions: a maintainer on the line, a crew at a hand-operated switch.

**The invention.** The CTC machine: one panel column per control point, with switch and signal levers, a code button,
track indication lamps on a model board, and the extras that make it a working tool: fleeting, maintainer call,
electric locks for outlying switches, dual-control switches. The dispatcher is many tower operators at once.

**Terms.** control point, subdivision, CTC machine, dispatcher, panel column, code button.

**Figure.** One panel column face in the KiCad `PanelColumn` symbol: switch lever N/R with its lamps, signal lever
L/N/R with its lamps, the CODE button, and track lamps above. Beside it, a strip of the subdivision: seven
interlockings on one line, Luchessa in the middle with its three control points.

**Relay layer.** The electric lock as the one example: `WLK` is asserted when released, because the lock relay is
energized to unlock and drops to locked when power fails [GLOSSARY section 3, electric lock; section 10.3].
Otherwise this is an office segment, and the office is not vital.

**East/West.** West: long territories under one dispatcher, on one machine [RS1942, RS1945; scale needs cite?].
East: towers kept at busy junctions under the dispatcher [cite?; `docs/reference/README.md` discusses control
operators but cites only web pages, so it is a lead, not a source].
FieldUnit setting: which stations the desk carries, and the number of panel columns per station
(`MAX_COLUMNS_PER_STATION`) [GLOSSARY section 3, panel column].

**What FieldUnit does.** `cTcMachine` sends a station's controls when CODE is released (`OneShot`); it does not send
controls at power-up; it must let the operator state an unsafe intent [GLOSSARY sections 2.3 and 5].

**Closing line.** "On a model railroad, this is the dispatcher's job in your op session: one person, one panel,
the whole main line."

**Sources.** [AAR56] p. 34 for `TK`; [RS1959] for fleeting and maintainer call on a 1959 machine; [USS1949].

**[demo, optional]** `tools/test_ctc_desk.py --walk` lights every station's lamps on the real desk.

---

## 10. Digital code lines to PTC (4 min)

**Era.** Solid-state and microprocessor code systems, processor-based interlockings, data radio; PTC after the
Rail Safety Improvement Act of 2008 [cite?].

**The problem.** Relay code lines are slow and carry a few functions per station; pole lines are costly to keep.
And trains still pass signals at Stop.

**The invention.** The code line becomes data. The interlocking logic becomes software that is configured per site
(the industry's "application" [ARTC23]). PTC adds what segment 7 started: the train itself enforces its authority
and speed limits [cite?].

**Terms.** code line type, encoding, transport, token, control transaction, indication vector.

**Figure.** One code line drawn three ways over the same field station: US&S 506 steps (16 slots), AAR tokens over
MQTT, and an I/O encoding over CMRInet. A `transport` symbol (symbol needed) under each.

**Relay layer.** The relays did not go away; they became the specification. The software is correct when it behaves
as the circuit does [GLOSSARY section 10.6].

**East/West.** Short and optional: roads that already had cab signals overlaid PTC on them; others built it new
[cite?]. FieldUnit setting: none. PTC is history only.

**What FieldUnit does.** A code line type is one encoding on one transport; FieldUnit sends AAR tokens over MQTT; a
token is not a step [GLOSSARY sections 3 and 6]. PTC is out of scope: it needs train speed control, which this
project does not do (plan, segment 10). Correct switches, signals and CTC are the foundation PTC is built on.

**Closing line.** "On a model railroad, this is your layout network, MQTT, CMRInet or LocoNet, carrying the same
controls and indications the 506 did."

**Sources.** [RRS506] for the 16 steps; [ARTC23] for "application". [cite?] for digital code line history and PTC.

**[demo, optional]** `tools/test_ctc_desk.py --monitor`: the tokens on the wire, as text.

---

## 11. Your layout as a railroad (6 min)

**The problem.** The modeler wants all of this, for one era and one region, without becoming a signal engineer.

**The answer.** Draw the railroad in the symbols the audience has now seen ten times. The drawing is the interlocking
model. The logic is the same for every railroad: the interlocking logic. Put them together on a small computer under
the benchwork and you have a field unit; put a CTC machine at the end of the aisle and you have a subdivision.

**Terms.** interlocking model, interlocking logic, interlocking application, field processor, field unit.

**Figure.** The full Luchessa drawing from `Railroad/SPCoast/`, the one the reader will make (plan principle 4),
with call-outs pointing back to each segment: the switch (1), masts (2), interlocking limits (3), `783T1` (4),
the route locking (5), the `HD` line toward the next control point (6), the `PanelColumn` (9), the code line (8, 10).

**Relay layer.** One equation, from GLOSSARY section 2.4:
field unit = (interlocking logic + interlocking model) on a field processor.

**East/West.** The recap of every druther the talk named, as one table the audience can take home:
switch Style, mast aspect policy, semaphore quadrant, mast `Kind` and plates, `RULE_251` or `RULE_261`, code line type.
"Your givens and druthers fit."

**What FieldUnit does.** Point to the hands-on paths of the plan: a virtual layout; one end of a siding as a control
point (1 switch, 1 signal with 3 masts, OS detector) on a 16-I/O field processor; an existing JMRI layout.
Say which are ready on the day of the talk and which are not.

**Closing line.** "On a model railroad, this is your railroad."

**Sources.** `docs/GLOSSARY.md` sections 2.4 and 2.5; `docs/plan/user-docs-plan.md`, hands-on paths.

**[demo, optional]** The virtual SPCoast subdivision (FieldUnit-Subdivision `spcoast_virtual_plant`) behind the desk:
line a route through Luchessa and watch the signal clear.

---

## Open questions for John

1. **Segment 7 has no glossary terms.** Either add entries (ATS, cab signal, ATC, coded track circuit, train control)
   as [Prototype] history, or keep it as a story-only segment with no terms. Everything in it is [cite?] today.
2. **Segment 5 has no regional story.** Plant size, not region, drove power interlocking (as far as I know; unsourced).
   Drop the sidebar, or find a regional angle (for example, electro-pneumatic plants on eastern terminals)?
3. **Segment 3 needs "route".** The glossary has no entry for route, though the code (`Route`) and the talk lean on it.
   Also missing: locking bed (named inside interlocking machine), home signal, absolute signal, pipe and wire.
4. **B&O naming.** The glossary calls the B&O system "position-color light signal" (`PCL`) and uses
   "color-position light signal" (`CPL`) for a three-lamp head; the code calls the B&O policy `boCpl`, and common
   usage calls the B&O system "color position light". An audience of peers will notice. Confirm or reconcile.
5. **Dates and regional claims.** Every era date is [cite?] except CTC 1927 [EKEVING], the 1937 time code [RS1937],
   the SP CTC articles and the 1959 console. The 1903 upper-quadrant date is in the glossary and primer without a key.
   Which of Chubb, the AAR texts in `docs/reference/` or jonroma.net articles cover these?
6. **Figures that need new symbols:** switch stand target, lever frame, relay coil and contacts, transport,
   train-borne equipment (or use photos for segment 7).
7. **Demo readiness.** Which demo hooks will be ready by early 2027: the ghost plant (`--loopback`) works today against
   the desk; the virtual SPCoast plant lives in FieldUnit-Subdivision.
