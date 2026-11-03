# Talk Outline: From the Switch Tender to Your Layout

Draft 6, 2026-11-03. Deliverable 1 of `docs/plan/user-docs-plan.md`.
Audience: NMRA Layout Design and Operations SIGs, in person, early 2027. One hour.
Tone: approachable, interesting, fun. Pure story. Optional demo hooks are marked **[demo, optional]**.

Changes in draft 3 (owner review, 2026-11-03): each segment speaks only the vocabulary of its era; rules that came later
are named in the future tense; CTC is split into the idea and the code line (8), the office (9) and the field (10);
digital code lines (11) and PTC (12) are separate segments; the last segment looks forward.
The spine in `user-docs-plan.md` is updated to match.
Draft 4: the 22 terms that draft 3 marked in italics now have `docs/GLOSSARY.md` entries; segment 8 says where the
dispatcher's authority ends and carries the East/West story of CTC; segment 10's sidebar is about dual control.
Draft 6: draft 4 content kept, with a story line added to each segment and gathered under "The narrative"; signal type
names corrected (color light, B&O color position light, PRR position light). Draft 5, which replaced the content with
spoken prose, is withdrawn.

How to read this outline:

- One section per spine segment, 2 to 6 minutes each.
- **Story line** comes first: two or three sentences that carry the segment's key points. Read in order, the story
  lines tell the whole narrative (see "The narrative" below); the speaker ad-libs between them, and the slide takes the
  audience through the details and terms for the rest of the segment.
- Each section has the same fields: the era, the problem, the invention, the terms (spelled as in `docs/GLOSSARY.md`;
  every term has a glossary entry), the one figure, the layer under the picture, the East/West sidebar,
  what FieldUnit does, the closing line, and sources.
- **The layer under the picture** is the rulebook or the locking sheet in segments 1 to 3, before there were relays,
  and the relay circuit from segment 4 on.
- **[lead: KEY]** marks a source found by web search, listed under "Source leads" at the end, and not yet opened.
  **[cite?]** marks a claim with no lead. Well-known history has no marker.
- **Figures** are placeholders, named in the KiCad symbol language of the Railroad library.
  **(symbol needed)** marks a symbol not known to exist. The figures are drawn in deliverable 3.
- "What FieldUnit does" says, in the user's terms, what the software does for that segment, or that it does nothing.
  Differences between the code and the relay model belong in the textbook and the peer brief, not on a slide.
- The assignment of every glossary entry is in `docs/plan/glossary-segments.md`.

## The narrative

The story lines of all segments, in order. This is the talk in one page; everything else is slide and ad-lib.

0. Every lever and lamp on a CTC machine is the answer to a wreck, a lawsuit or a payroll. Tonight we walk a hundred years forward, one problem at a time, and pick up the words the way the railroads did, watching the East and the West, with different traffic and different money, solve the same problems differently.

1. Let's start at the beginning, before electricity and radios, when two trains tried to use the same track. The only protection was paper and people: a timetable, a train order by telegraph, a brakeman at a switch stand, and a derail for when all of that failed.

2. Paper can't be read at fifty miles an hour, so the railroads put up fixed signals: balls on masts, the train order board at the depot, then semaphores worked by wire. Everything still runs on muscle, and one idea arrives that lasts all night: what a signal looks like is not the same as what it means.

3. Put several switches and signals in one place, and sooner or later someone in a hurry lines them wrong. The tower's lever frame made the wrong move impossible: a mechanical locking bed that refuses in the operator's hand, the first interlocking, with no electricity at all.

4. The locking bed couldn't see the train. Robinson's track circuit could, with a battery, a relay and gravity, and it fails safe: a broken rail, a dead battery or a loose wire all read as a train.

5. Pipes froze and reached only so far, so power moved the switches and relays took over the locking. Relays can see the train, so the interlocking now proves the points, holds the route under the train and times the release, and that is what a FieldUnit is, in software.

6. Between the stations, trains still ran into each other on paper. Automatic block signals protected them without giving authority, APB added absolute head blocks for single track, and the signal itself went electric, each railroad picking its own look.

7. Signals only work if the engineer obeys them, so the ICC began putting the signal in the cab. The dense East paid for it; much of the West stayed under 80 mph instead, which is where 79 comes from, and all of it is outside what FieldUnit does.

8. On single track, every meet meant stopping so the brakeman could throw the switch by hand, and towers stood only at the bottlenecks, at three shifts a day each. In 1927 the railroads cut the lever frame in two: powered switches and signals at the sidings, the levers with a distant dispatcher, the locking in the field, and a relay code line of long and short pulses between them. The West built CTC to add capacity to single track; the East built it later to tear track out.

9. At the dispatcher's desk the lever no longer refuses: it is a wish, sent with the CODE button, and the answer is the lamp that lights, or doesn't. The desk keeps nobody safe; it states intent and shows the truth for a whole subdivision.

10. Out at the siding, an empty bungalow has the last word: it checks the locking and acts, or silently does nothing. Electric locks and dual-control switches let crews work by hand when the dispatcher allows it, or when the code line fails.

11. Relay code lines were slow and pole lines expensive, so the code line became data and the bungalow's relays became software that does what the relays would have done. That is what FieldUnit was built to provide: a real digital code line between the dispatcher and the field units under your layout.

12. PTC finished what train control started, enforcing authority and speed in the cab on every main line after 2008. It stands on everything we've seen tonight, and FieldUnit deliberately stops at that foundation.

13. Here is the whole hundred years on one drawing: your railroad, in the symbols you've seen all night. Pick your era and your railroad and FieldUnit fits, and here is what comes next: C/MRI hosting, MQTT appliances, CTC machines on a screen.

## Running order and timing

| # | Segment | Min | Clock |
|---|---|---|---|
| 0 | Cold open | 2 | 0:00 |
| 1 | The switch tender | 4 | 0:02 |
| 2 | Fixed signals, the strongarm era | 4 | 0:06 |
| 3 | The tower and the lever frame | 5 | 0:10 |
| 4 | Track circuits: electricity arrives | 4 | 0:15 |
| 5 | Power interlocking | 4 | 0:19 |
| 6 | Automatic block: ABS, then APB | 5 | 0:23 |
| 7 | Train control (context only) | 2 | 0:28 |
| 8 | CTC: meets without stopping | 6 | 0:30 |
| 9 | CTC, the office: the dispatcher's desk | 4 | 0:36 |
| 10 | CTC, the field: the control point | 4 | 0:40 |
| 11 | Digital code lines | 4 | 0:44 |
| 12 | PTC (context only) | 2 | 0:48 |
| 13 | Your layout, and what comes next | 6 | 0:50 |
| | Questions | 4 | 0:56 |

Segments 7 and 12 exist for continuity. They are history the project does not implement (plan principle 7).

---

## 0. Cold open (2 min)

**Story line.** Every lever and lamp on a CTC machine is the answer to a wreck, a lawsuit or a payroll. Tonight we walk a hundred years forward, one problem at a time, and pick up the words the way the railroads did, watching the East and the West, with different traffic and different money, solve the same problems differently.

- Open on one picture: a CTC machine (`docs/images/SPCoast-cTc-panel.png`) next to a rack of glass-cased relays
  (photo needed).
- The hook: "Every lever and every lamp on this desk is the answer to a wreck, a lawsuit, or a payroll.
  In the next hour we walk a hundred years of railroad history, and we pick up the vocabulary one era at a time.
  By the end, the desk will read like a sentence."
- State the two axes once: **era** explains why a word exists; **region** (East and West, for now) explains why
  railroads did it differently. Traffic density and money drove most of the differences.
- State the promise: no electronics, no code. One drawing language, used from the first slide to the last.

---

## 1. The switch tender (4 min)

**Story line.** Let's start at the beginning, before electricity and radios, when two trains tried to use the same track. The only protection was paper and people: a timetable, a train order by telegraph, a brakeman at a switch stand, and a derail for when all of that failed.

**Era.** From the first railroads in the 1830s; on light lines, well into the 20th century.
There is no general rulebook yet; each road writes its own.

**The problem.** One track, trains in both directions, and switches that a person must line by hand.
A switch left in the wrong position puts a train into a siding at speed. Two trains on one track meet head on.

**The invention.** Three things, none of them signals:

- The switch stand with a target, thrown by hand. At an ordinary siding the crew does it: stop, the brakeman throws the
  switch, waits for the other train, throws it back and climbs back on the caboose.
- The timetable and the train order: authority to occupy the track comes from paper, sent by telegraph.
  The first telegraphic train order is usually credited to Charles Minot on the Erie, 1851 [lead: KING1921].
- The derail, which sacrifices a car to save the main line.

**Terms.** switch, manual switch, derail, fouling point; timetable, train order, points, switch stand.

**Figure.** The end of a passing siding at a station, all hand-operated: the switch (Style `manual`) with a switch
stand and target (symbol needed), a derail on the siding, and the fouling point marked.

**The rulebook.** The protection is a person and the road's own rules. These practices would later be written into
the Standard Code: keeping right on double track as Rule D-151, and entering a siding only when the switch and derail
are known to be lined as Rule 105 in SP's numbering [PRR56; SP text: owner's transcription, GLOSSARY section 9].
Say this aloud: everything after this segment replaces a person's attention with a mechanism, then with a circuit.

**East/West.** East: double track came early on dense lines. West: long single track with sidings, where timetable
and train order lasted for decades.
FieldUnit setting: the switch Style, `manual` or `manual_sensed` [GLOSSARY section 7, manual switch].

**What FieldUnit does.** A hand-operated switch can be part of the model, with or without point detection.
A derail rests in its derailing position [GLOSSARY section 7, derail].

**Closing line.** "On a model railroad, this is a ground throw, a car card and a crew that reads the timetable.
Your car cards and switch lists are real railroad practice."

**Sources.** [PRR56] for Rule D-151; [CFR218] and [GCOR6] for the derail position; KING1921 for train orders.

---

## 2. Fixed signals, the strongarm era (4 min)

**Story line.** Paper can't be read at fifty miles an hour, so the railroads put up fixed signals: balls on masts, the train order board at the depot, then semaphores worked by wire. Everything still runs on muscle, and one idea arrives that lasts all night: what a signal looks like is not the same as what it means.

**Era.** 1830s to about 1900. Everything is mechanical: a person, a lever, a wire, an oil lamp.
No battery, no track circuit, no electric light.

**The problem.** A train at speed cannot stop to ask. The engineer needs an instruction at a fixed place, seen from
far away, day and night. And the station operator needs a way to stop a train to hand up orders.

**The invention.** The fixed signal, and the first two kinds of it:

- The ball signal: a ball hoisted on a mast. The New Castle and Frenchtown, 1832, is usually given as the first
  fixed signal in the US (flags first, then balls) [lead: WIKI-SIG, IRSE-US].
- The train order signal: a semaphore of its own style at the station, worked by the operator. It says one thing:
  there are orders for you, or there are not. It is not a block signal. It says nothing about the track ahead.
  It would later be governed by its own rule in the Standard Code (Rule 221 in most books; check [PRR56]).

The semaphore arm itself appears here: an arm worked by a wire from a lever, lit at night by an oil lamp behind a
colored roundel. Its use to protect a junction is segment 3; its use as a block signal is segment 6.

A fixed signal has two parts that the rest of the talk keeps apart: its appearance (the aspect) and its meaning in
the rulebook (the signal indication).

**Terms.** fixed signal, signal, mast, head, aspect, signal indication, semaphore signal; ball signal, train order
signal.

**Figure.** A station on single track with a train order signal at the depot, worked by the operator by a lever and a
wire (symbols needed: train order signal, lever, wire run). A ball signal inset. One signal labeled with its aspect on
one side and its signal indication on the other.

**The rulebook.** aspect = rulebook(signal indication, signal). The indication is the instruction; the aspect is how
one railroad, in one era, shows it [GLOSSARY section 8, aspect and signal indication].

**East/West.** Not much to say yet. Train orders, and so train order signals, ran everywhere; they lasted longest on
western single track.

**What FieldUnit does.** Nothing yet. A train order signal is planned: a KiCad symbol and a model object of its own,
not a block-signal mast (owner, 2026-11-03). The aspect and indication machinery FieldUnit does have
(`src/SignalAspectPolicy.h`) is for interlocking and block signals, and is shown in segments 3 and 6.

**Closing line.** "On a model railroad, this is the order board at the depot: the operator's way to say 'stop,
there's paper for you'."

**Sources.** [AAR56] for aspects and indications. WIKI-SIG and IRSE-US for 1832. KING1921 for train order signals.

---

## 3. The tower and the lever frame (5 min)

**Story line.** Put several switches and signals in one place, and sooner or later someone in a hurry lines them wrong. The tower's lever frame made the wrong move impossible: a mechanical locking bed that refuses in the operator's hand, the first interlocking, with no electricity at all.

**Era.** Mechanical interlocking: Saxby in England from the 1850s (Saxby and Farmer from 1863); the first North
American interlocking at Trenton, New Jersey, 1870, a Saxby and Farmer frame [lead: WIKI-INT, IRSE-US, TRAINS].
Still the strongarm era: the "armstrong" lever moves the switch through pipe and the signal through wire.

**The problem.** Where several switches and signals must agree, a person in a hurry can clear a signal over a switch
that is wrong.

**The invention.** The interlocking machine: a frame of levers, connected by pipe and wire to the switches and
signals, with a mechanical locking bed under the levers. A lever that must not move cannot move. The tower operator
feels the refusal in the hand [GLOSSARY section 2.3]. No electricity is involved.

**Terms.** interlocking, interlocking machine, tower, tower operator, lever, interlocking plant, route.

**Figure.** A busy station whose siding end is a bottleneck worth a tower (an ordinary siding was crew-worked), one end
interlocked from the tower: switch `1`, home semaphores `2` and `4`
facing each other, the interlocking limits shaded between them, and the pipe run and wire runs from the tower
(symbols needed: pipe run, bell crank, lever frame). Under it, the lever frame with numbered levers, and one line of
the locking sheet: "lever 2 reversed locks lever 1 normal". Lever positions follow AAR56: N and R for a switch;
L, N and R for a signal [GLOSSARY section 5, lever]. The odd-switch, even-signal numbering is a habit, not a rule
[GLOSSARY section 10.2, unverified].

**The locking sheet.** A truth table, before there were relays. "2R locks 1N" reads: signal 2 may clear only if switch
1 is normal, and while signal 2 is clear, switch 1 cannot move. This is a route, written as locking.

**East/West.** The cost of a tower was its operators, three shifts a day. Railroads put a tower only where traffic
density, operational complexity or a bottleneck in the track made split-second coordination vital: junctions, crossings
at grade, busy stations (owner, 2026-11-03). An ordinary passing siding was crew-worked (segment 1). Dense eastern main
lines had towers every few miles; in the West they stood mostly at junctions and crossings at grade.
FieldUnit setting: none directly. The tower operator role is not yet in the model [GLOSSARY section 4, tower operator].

**What FieldUnit does.** Each route in an interlocking model lists the switch positions it needs and the signal that
governs it: the locking sheet as data (`examples/CP_Corporal/CP_Corporal.json`, keys `aligns` and `governedBy`).
A tower lever frame is in the project's scope but not built yet.

**Closing line.** "On a model railroad, this would be the control panel at a yard throat, or CATS in JMRI."

**Sources.** [AAR56] for lever positions. WIKI-INT, IRSE-US and TRAINS for Saxby and Trenton.

---

## 4. Track circuits: electricity arrives (4 min)

**Story line.** The locking bed couldn't see the train. Robinson's track circuit could, with a battery, a relay and gravity, and it fails safe: a broken rail, a dead battery or a loose wire all read as a train.

**Era.** William Robinson's closed track circuit, 1872, first installed at Kinzua, Pennsylvania, on the
Philadelphia and Erie [lead: ROBINSON, IRSE-US].

**The problem.** The tower operator cannot see under a train. A switch thrown under a moving car derails it.
Nothing knows that a block holds a train, or that a rail is broken. In the mechanical tower the best answer was a
detector bar beside the rail, which the wheels held down.

**The invention.** A battery at one end of a section of rail, a relay at the other, insulated joints at both ends.
With the track empty, the battery holds the relay energized and its armature up. A train's wheels and axles short the
rails, the relay loses its current, and gravity literally drops the armature. A broken rail or a dead battery drops it
too. Every failure reads as "occupied", which is the safe answer [GLOSSARY section 10.5, `TR`; primer section 2].
This is the first battery and the first relay of the talk; everything electric after it is built on it.

**Terms.** track circuit, `TR`, OS section, detector locking, approach track circuit; insulated joint.

**Figure.** One track circuit between two insulated joints (`IRJ-Signal` symbols at the ends), the battery and the
`TR` coil, drawn twice: vacant (relay up) and shorted by a wheelset (relay down). The ASCII sketch of GLOSSARY section
10.5 is the brief. A second panel: the OS section over switch 1 of segment 3, named `1T1`.

**The relay layer.** `TR` up means vacant. Detector locking, in this era's terms: switch 1 may move only while its OS
track circuit is vacant, `1TR` up. The same idea as the detector bar, with no moving parts at the rail.

**East/West.** Not a regional story. Use the slot for the modeler's sidebar instead: current sensing, optical sensors
and axle counters are all one `TR` in the model; sensors on one input are one track circuit
[GLOSSARY section 7, aggregation].

**What FieldUnit does.** Every detector you wire becomes a track circuit that starts "occupied" and reports
"occupied" if it stops hearing from the sensor. A switch bound to its OS track circuit will not move under a train
[GLOSSARY section 7, track circuit; section 8, detector locking].

**Closing line.** "On a model railroad, this is your block detector, and it must say 'occupied' when it fails."

**Sources.** [AAR56] p. 31 for track circuit naming. [BNSF] for "OS" as "on sheet". ROBINSON for 1872 and Kinzua.

---

## 5. Power interlocking (4 min)

**Story line.** Pipes froze and reached only so far, so power moved the switches and relays took over the locking. Relays can see the train, so the interlocking now proves the points, holds the route under the train and times the release, and that is what a FieldUnit is, in software.

**Era.** Hydro-pneumatic at Bound Brook, New Jersey, 1884; Westinghouse's Union Switch and Signal builds
electro-pneumatic plants from 1891, about 54 in service by 1900; the first all-relay interlocking, GRS, 1929
[lead: WIKI-INT, CHI-L, NOCK1956].

**The problem.** Pipe and wire limit how far a lever can reach and how many levers one person can pull, and in the
winter the pipes and wires froze. Big terminals need more. And now that the track circuit exists, the locking should
use it.

**The invention.** Power moves the points: compressed air, then electric *switch machines*, and the tower's levers
shrink to miniature levers. Then relays take over the locking itself, and gain rules a mechanical bed could not hold,
because they can see the train:

- Switch correspondence: the signal clears only when the points are proved where the lever says.
- Route locking: the switches stay locked while the train is on the route, and release behind it.
- Approach locking and time locking: if the signal is taken away in front of an approaching train, the switches stay
  locked for a time. Some towers did this with a row of mechanical timers on the wall, started by hand like a kitchen
  timer; relay plants used a time element relay.

Relays are vital: the armature drops by gravity, so a failed relay leaves the signal at Stop [primer section 2].

**Terms.** switch correspondence, route locking, approach locking, time locking, vital; switch machine, relay.

**Figure.** One switch machine with its points proved, lighting the switch lamp only when they are home; the signal it
protects showing Stop while the points are moving. Relay symbols kept to the coil and one contact (symbol needed,
unless the Railroad library has them). The full relay drawing waits for segment 8.

**The relay layer.** One sentence per relay, no circuit yet: the switch is proved (`KR`); the signal may clear when its
lever is reversed, its switches are proved and its track is clear (`HR`); the switches may move when no train is on
the route and no time is running (`WLR`) [GLOSSARY sections 10.5 and 10.6].

**East/West.** One sentence: plant size, more than region, decided power interlocking. Big terminals and junctions
came first, wherever traffic was densest.

**What FieldUnit does.** This is the segment that answers "what does FieldUnit do?" Segment 3 gave the routes and
segment 4 the detectors. A field unit is a relay interlocking of this kind, in software:

- It will not show a proceed signal until the switches of the route are proved in position and its track is clear.
- It holds the switches of a route under a train and releases them behind it, section by section.
- If you take a signal away in front of an approaching train, it holds the switches for a set time (30 s by default)
  [GLOSSARY section 8, approach locking].
- When anything fails, the signal goes to Stop.

**Closing line.** "On a model railroad, this is the logic behind your panel: rules that, whatever breaks, leave every
signal at Stop."

**Sources.** [AAR56] p. 37 for `HSR`. NOCK1956, WIKI-INT and CHI-L for the dates. The wall timers: owner's knowledge.

---

## 6. Automatic block: ABS, then APB (5 min)

**Story line.** Between the stations, trains still ran into each other on paper. Automatic block signals protected them without giving authority, APB added absolute head blocks for single track, and the signal itself went electric, each railroad picking its own look.

**Era.** Hall's enclosed disc ("banjo") signals from the 1870s, the first automatic block signals [lead: WIKI-HALL];
the three-position upper-quadrant semaphore from about 1903 [GLOSSARY section 7; WIKI-SEM]; absolute permissive block,
conceived by GRS around 1910 and first installed in quantity on the Toronto, Hamilton and Buffalo in 1911
[owner, 2026-11-03; lead: HOWE1911]; color light signals through the 1910s and 1920s.

**The problem.** Between stations, trains run on timetable and train order. A following train runs into the one
ahead; on single track, two trains meet head on.

**The invention, in two steps.**

1. ABS, automatic block signals: track circuits drive signals with no person behind them. ABS is **protection, not
   authority**. The timetable and the train order still give a train the right to the track; the signals only tell it
   whether the block ahead is clear. The upper-quadrant arm adds a third position, Approach, so a signal can warn of
   the next one.
2. APB, absolute permissive block: on single track, ABS cannot stop two trains entering from opposite ends. APB puts
   absolute signals at the ends of the single track (the head blocks). Once a train enters, the opposing head block
   and every opposing intermediate signal are held at Stop to the next siding, while following trains may still close
   up behind it. APB quickly became the standard single-track ABS.

With batteries and relays at every signal, the lamp goes electric too: the SP's three-lamp color lights and
searchlights, the PRR's position lights, the B&O's color position lights, each a road's own answer.

**Terms.** block, automatic signal, signal location, cut section, traffic, direction of traffic, current of traffic;
ABS, APB, head block. Named but not stressed: color light signal, position light signal.

**Figure.** Single track between two sidings. Head-block masts with `Kind` `ABSOLUTE` at each end, back-to-back
intermediate masts with `Kind` `PERMISSIVE` and number plates (`IRJ-Signals`), and the `HD` line drawn as arrows
running from each signal location back to the one before it. A `RULE_251` sign on a double-track inset (ABS);
the single track is APB. Inset: one mast showing Approach four ways: an upper-quadrant semaphore, an SP three-lamp color light head, a PRR
position light and a B&O color position light.

**The relay layer.** The `HD` line: `H` reaches back to the signal behind when the track ahead is clear; `D` when the
next signal shows Approach or better [GLOSSARY section 7, HD line]. The directional stick relay `SR` lets a following
train close up behind one going away [GLOSSARY section 10.5, `SR`].

**East/West.** The richest sidebar of the talk.

- Track: double track with a current of traffic and ABS for following moves (later Rule 251 [PRR56]); or single
  track with APB.
- Head block: most restrictive indication Stop (Rule 292); intermediate signal with a number plate: Stop and Proceed
  (Rule 291) [AAR56 pp. 51, 53–54].
- Route signaling and speed signaling: western roads such as the SP showed which route was lined; eastern roads such
  as the NYC and PRR showed what speed was allowed, at the cost of more heads.
- Hardware: color light (three-lamp heads and searchlights) on many western roads, PRR position light, B&O color
  position light.
- Laugh line for the inset: GRS introduced the Type D three-lamp color light in the 1920s with a small hood over each
  lens. Snow piled up in front of the lenses, so the SP, running through the mountains, changed it to one big hood.
  After Star Wars came out in 1977, that hood got them called "Darth Vaders" (owner, 2026-11-03). A neat East/West
  point too: the West's terrain changed the hardware.

FieldUnit settings: mast `Kind`, `Plate-G` and `Plate-R` markers, the `RULE_251` sign [GLOSSARY sections 7 and 9];
the aspect policy of each mast: `sp1969`, `sp1985`, `gcorSpeed`, `nycSpeed`, `prrPositionLight`, `boCpl`,
`upperQuadrantSemaphore` (`src/SignalAspectPolicy.h`).

**What FieldUnit does.** Your masts can show any of those rulebooks, and you pick one per mast. Automatic signals
between control points are designed (FieldUnit ADR 0007; FieldUnit-Subdivision ADR 0013) but not built yet.

**Closing line.** "On a model railroad, these are the signals between your sidings, the ones that run themselves.
They protect the train; the dispatcher's orders still move it."

**Sources.** [AAR56], [PRR56], [CBQ1946], [RS1947] p. 92 for line circuits, [CFR236] §236.769 for traffic;
HOWE1911 for APB; WIKI-HALL and WIKI-SEM for Hall and 1903. Color light dates by type: railroadsignals.us styles
pages are the lead.

**[demo, optional]** One mast switching aspect policy live, SP 1969 to NYC speed, same indication.

---

## 7. Train control (context only, 2 min)

**Story line.** Signals only work if the engineer obeys them, so the ICC began putting the signal in the cab. The dense East paid for it; much of the West stayed under 80 mph instead, which is where 79 comes from, and all of it is outside what FieldUnit does.

**Era.** The ICC train control orders of 1922 [lead: ICC1922]; the ICC signaling order of 1947, Docket 29543
[lead: ICC1947]. Today: 80 mph or more needs cab signals, automatic train stop or automatic train control
([CFR236] §236.0, to be opened and quoted).

**The story.** Signals work only if the engineer sees and obeys them. The answer was to put the signal in the cab and
let it act on the brakes: automatic train stop (ATS) at the signal, continuous cab signals carried by coded track
circuits, and automatic train control (ATC), which enforces speed. This was mostly the dense eastern railroads.
Many western roads kept their trains under 80 mph instead: the 79 mph limit.

**Terms.** train control, ATS, cab signal, coded track circuit, ATC.

**Figure.** None. There is nothing on the track plan to see. One photo of a cab signal display, if wanted.

**What FieldUnit does.** Nothing. What happens in the cab is out of scope. This segment exists for continuity: it
explains 79 mph, and it sets up PTC in segment 12.

**Closing line.** "On a model railroad, this is the engineer. That is you."

**Sources.** [RS1944]; ICC1922, ICC1947, [CFR236] §236.0.

---

## 8. CTC: meets without stopping (6 min)

**Story line.** On single track, every meet meant stopping so the brakeman could throw the switch by hand, and towers stood only at the bottlenecks, at three shifts a day each. In 1927 the railroads cut the lever frame in two: powered switches and signals at the sidings, the levers with a distant dispatcher, the locking in the field, and a relay code line of long and short pulses between them. The West built CTC to add capacity to single track; the East built it later to tear track out.

**Era.** 1927: the first CTC on the New York Central, Stanley to Berwick, Ohio, built by GRS [EKEVING, one secondary
source]. Two-wire coded lines by the 1930s [RS1937].

**The problem.** On single track, every meet at an ordinary passing siding is crew work: stop, the brakeman throws the
switch by hand, waits for the other train, throws it back and climbs back on the caboose; and every meet is set up by
train orders that are slow to issue and to change. Towers stand only at the bottlenecks (junctions, crossings at grade,
busy stations), and each one costs three shifts of operators, every day. Single track is full, and double track costs
more than any of it.

**The invention.** Cut the interlocking machine in two. The levers go to one dispatcher's office; the locking stays
in the field. Ordinary sidings get powered switches and signals under the dispatcher, so trains meet without stopping;
the towers at the bottlenecks close, their levers moved to the dispatcher's desk. A code line carries the dispatcher's intent out (controls) and the truth back (office indications).
Segments 9 and 10 take the two halves one at a time; this segment is the idea and the wire between them.

Where the dispatcher's authority ends: at the CTC limits (the End CTC sign), and at yard limits, where movements may
be made on the main track without train orders under Rule 93 [PRR56]. Inside an interlocking that a tower still
works, the tower operator holds it [GLOSSARY sections 2.3 and 4].

**Terms.** CTC, code line, control, office indication, field station, CTC limits, yard limits; front contact,
back contact, stick circuit.

**Figure.** The two-panel drawing of the README, redrawn in KiCad symbols: a tower (levers, locking bed, track) beside
a CTC machine and a control point joined by two wires, with an End CTC sign (symbol needed) and a `Rule93-Yard Limits`
sign at the ends of the territory. Then, full width, the inside of one end of the code line: a
stepping relay, a slow-release relay and a line relay (symbols needed), with one code drawn as a pulse train of long
and short pulses.

**The relay layer: how relays think, and how a code line counts.** The deeper relay dive of the talk.

- A relay is a switch moved by a magnet. A front contact closes when it is up; a back contact closes when it is
  down. Contacts in series are AND; contacts in parallel are OR [GLOSSARY section 10.4].
- A stick circuit is a relay that holds itself up through its own front contact: one bit of memory. The signal's
  home stick relay remembers "the dispatcher asked for this signal" until a train knocks it down [GLOSSARY section
  10.5, `HSR`].
- Slow-release and slow-pickup relays add time: a relay that takes a moment to drop can tell a long pulse from a
  short one.
- The code line: both ends step through the same sequence of steps together, one stepping relay position per
  pulse. The station selection steps pick the field station; on each function step, a long or short pulse is one
  control or one indication [GLOSSARY section 6; lead: RRS506]. A US&S 506 code has 16 steps and serves up to 35
  field stations [RRS506].

**East/West.** The starkest difference in the talk, and money and track drove it [lead: UTAHRAILS, TRAINS-CTC].

- West: long single-track main lines. The Santa Fe, SP, UP and CB&Q installed the most CTC mileage
  [lead: RS1960-CON, TH-CTC]. CTC let a single track carry far more traffic (up to 80% more, by one account
  [lead: TRAINS-CTC]) because the dispatcher ran meets at sidings without anyone throwing a switch by hand. It cost much
  less than a second main track across thousands of miles. The SP's own installations are on record [RS1942, RS1945,
  RS1947].
- East: the PRR and NYC had laid double, triple and quadruple track decades before. ABS with directional running
  (Rule 251) and staffed towers carried the traffic; they did not need CTC for capacity.
- When eastern roads did install CTC in quantity, in the 1950s and after, it was to remove track: four main tracks
  became two, signaled in both directions under CTC, with the same capacity and much lower taxes and maintenance.
  The NYC under Alfred Perlman is the usual example [lead: TRAINS-CTC, RRNET-4TRK].

FieldUnit setting: the code line type, US&S 506 style or AAR tokens over MQTT
[GLOSSARY section 3, code line type].

**What FieldUnit does.** The SPCoast CTC machine can show the code going out, pulse by pulse, at roughly prototype
speed (`CODELINE_VISUAL_STEPPING`; long 350 ms, short 110 ms; a model design, primer section 9.5.1).

**Closing line.** "On a model railroad, this is two wires, or one network, from the dispatcher's desk to every
siding."

**Sources.** [EKEVING], [RS1937], [RS1942], [RS1945], [RS1947], [RRS506], [AARNOM46] for written circuits, [PRR56]
for Rule 93; RS1960-CON, TH-CTC, UTAHRAILS, TRAINS-CTC and RRNET-4TRK for the East/West story (owner-supplied leads).

**[demo, optional]** The SPCoast CTC machine with visual stepping on, sending one switch control.

---

## 9. CTC, the office: the dispatcher's desk (4 min)

**Story line.** At the dispatcher's desk the lever no longer refuses: it is a wish, sent with the CODE button, and the answer is the lamp that lights, or doesn't. The desk keeps nobody safe; it states intent and shows the truth for a whole subdivision.

**Era.** 1930s to 1980s: the US&S and GRS CTC machines; a pushbutton console by 1959 [RS1959].

**The problem.** One person, many control points, a whole subdivision. The dispatcher must see where every train is,
keep traffic moving, and handle exceptions: a maintainer on the line, a crew at a hand-operated switch.

**The invention.** The CTC machine: one panel column per control point, with a switch lever, a signal lever, a code
button, and track indication lamps on the model board above. The lever is free. Unlike a tower lever, it can be moved
to an unsafe position; the field refuses, and the dispatcher learns of the refusal by absence: the lamp that agrees
with the lever never lights (out of office correspondence) [GLOSSARY section 2.3]. Plus the extras that make it a
working tool: fleeting, maintainer call. The dispatcher is many tower operators at once.

**Terms.** CTC machine, dispatcher, panel column, code button, office correspondence, subdivision.

**Figure.** One panel column face in the KiCad `PanelColumn` symbol: switch lever N/R with its lamps, signal lever
L/N/R with its lamps, the CODE button, and track lamps above. Beside it, the subdivision: seven interlockings on one
line, Luchessa in the middle.

**The relay layer.** The office is not vital. Nothing at the desk enforces safety; it only states intent and shows
the truth [GLOSSARY section 5, vital].

**East/West.** West: long territories under one dispatcher, on one machine [RS1942, RS1945]. East: towers kept at busy
junctions, their operators working under the dispatcher (`docs/reference/README.md`; a lead only).

**What FieldUnit does.** The CTC machine sends a column's controls when you release CODE, never at power-up, and lets
you move any lever: the field decides [GLOSSARY sections 2.3 and 5].

**Closing line.** "On a model railroad, this is the dispatcher's job in your op session: one person, one panel,
the whole main line."

**Sources.** [AAR56] p. 34 for `TK`; [RS1959]; [USS1949].

**[demo, optional]** `tools/test_ctc_desk.py --walk` lights every station's lamps on the real desk.

---

## 10. CTC, the field: the control point (4 min)

**Story line.** Out at the siding, an empty bungalow has the last word: it checks the locking and acts, or silently does nothing. Electric locks and dual-control switches let crews work by hand when the dispatcher allows it, or when the code line fails.

**Era.** The same as segment 9. At the end of each siding, where a brakeman once threw the switch by hand, a bungalow
now stands; at a junction, it replaces the tower.

**The problem.** The locking that the tower held must now work with nobody there, on intent that arrives late, by
wire, and may be wrong.

**The invention.** The control point: a bungalow of relays beside the track, at a field station address on the code
line. It accepts a control only if the locking allows it, and otherwise does nothing; it sends no refusal. It moves the
powered switches the dispatcher controls. For switches the dispatcher does not power, it gives the crew a way in: an
electric lock the dispatcher releases, or a dual-control switch the crew may work by hand.

**Terms.** control point, bungalow, powered switch, vital control, electric lock, dual-control switch.

**Figure.** One control point at the end of a siding: switch `783` (Style `powered`), signal `784` with its masts,
OS circuit `783T1`, and the `MAIN HOUSE` / `House` symbol. A hand-operated spur nearby with an electric lock.

**The relay layer.** The same `HR` and `WLR` of segment 5, now taking their lever input from a received control.
The one sentence: the code line, the field station and the field unit together replace the locking bed; they do at a
distance, and after the fact, what the locking bed did in the operator's hand [GLOSSARY section 2.3].
The electric lock: the lock relay is energized to unlock and drops to locked when power fails [GLOSSARY section 3].

**Resilience, instead of East/West.** A dual-control switch is a powered switch that a crew can also work by hand,
once the dispatcher gives them authority. In normal service the dispatcher throws it. When the switch machine fails,
or the code line or the dispatcher's connection is lost, the crew moves the selector from motor to hand and the
railroad keeps running locally (FieldUnit ADR 0006, proposed; owner, 2026-11-03). Three kinds of switch from the
dispatcher's seat: powered, electrically locked (a manual switch with a remote release), and dual control.
FieldUnit setting: the switch Style, `powered`, `dual`, or `manual` with an electric lock [GLOSSARY section 7].

**What FieldUnit does.** This is the field unit. It applies a set of controls whole or not at all; an unsafe set
changes nothing, and the next indications tell the dispatcher so (FieldUnit ADR 0002)
[GLOSSARY section 5, control transaction].

**Closing line.** "On a model railroad, this is a box under the benchwork at each siding, and it has the last word."

**Sources.** `docs/GLOSSARY.md` sections 2.3, 3 and 7; FieldUnit ADR 0002 and ADR 0006 (dual control).

**[demo, optional]** `tools/test_ctc_desk.py --loopback` as a ghost plant: throw a switch, press CODE, watch the lamp go
dark and come back.

---

## 11. Digital code lines (4 min)

**Story line.** Relay code lines were slow and pole lines expensive, so the code line became data and the bungalow's relays became software that does what the relays would have done. That is what FieldUnit was built to provide: a real digital code line between the dispatcher and the field units under your layout.

**Era.** From the 1970s and 1980s: solid-state and then microprocessor code systems, data radio, and processor-based
interlockings configured per site (the industry's "application" [ARTC23]) [cite? for dates].

**The problem.** Relay code lines are slow and carry a few functions per station; pole lines are costly to keep.

**The invention.** The code line becomes data. A message carries an address and a whole set of functions; it no
longer needs a step per function, and it does not care whether it travels on a pole line, a radio or a network.
The interlocking logic becomes software, and the relay circuits become its specification: the software is correct when
it behaves as the circuit does [GLOSSARY section 10.6].

**Terms.** code line type, encoding, transport, token, control transaction, indication vector.

**Figure.** One field station's controls drawn three ways: as US&S 506 steps (16 slots), as AAR tokens in one MQTT
message (`783NWS, (783RWS), 784NGS`), and as bits on a CMRInet line. A transport under each (symbol needed).

**The relay layer.** None new. The relays are now the specification, not the hardware.

**East/West.** None. The digital code line was everybody's.

**What FieldUnit does.** This is what FieldUnit was built for. The office sends one message per field station: every
control at once, as AAR-named tokens. The field answers with every indication at once. A token is not a step; a code
line type is one encoding on one transport [GLOSSARY sections 3 and 6].

**Closing line.** "On a model railroad, this is what FieldUnit was designed to provide: a real digital code line
between a dispatcher's office and Arduino field units under your layout."

**Sources.** [ARTC23]; [RRS506] for the 16 steps; FieldUnit ADR 0001 (MQTT code line), ADR 0002.

**[demo, optional]** `tools/test_ctc_desk.py --monitor`: the tokens on the wire, as text.

---

## 12. PTC (context only, 2 min)

**Story line.** PTC finished what train control started, enforcing authority and speed in the cab on every main line after 2008. It stands on everything we've seen tonight, and FieldUnit deliberately stops at that foundation.

**Era.** Mandated by the Rail Safety Improvement Act of 2008.

**The story.** Segment 7 put the signal in the cab on some lines. PTC puts the train's authority and speed limits in
the cab on all of them, and stops the train if the crew does not. It is built on the same switches, signals, track
circuits and code lines as everything before it.

**Terms.** PTC (the entry says "history only").

**Figure.** None.

**What FieldUnit does.** Nothing. PTC needs train speed control (DCC, car and engine detection), which this project
does not do. Correct switches, signals and CTC are the foundation PTC stands on, and that is what FieldUnit provides.

**Closing line.** "On a model railroad, this is still you, with your hand on the throttle."

**Sources.** `docs/plan/user-docs-plan.md`, spine segment 12.

---

## 13. Your layout, and what comes next (6 min)

**Story line.** Here is the whole hundred years on one drawing: your railroad, in the symbols you've seen all night. Pick your era and your railroad and FieldUnit fits, and here is what comes next: C/MRI hosting, MQTT appliances, CTC machines on a screen.

**The problem.** The modeler wants all of this, for one era and one region, without becoming a signal engineer.

**The answer today.** Draw the railroad in the symbols the audience has now seen a dozen times. The drawing is the
interlocking model. The logic is the same for every railroad: the interlocking logic. Put them together on a small
computer under the benchwork and you have a field unit; put a CTC machine at the end of the aisle and you have a
subdivision.

**Terms.** interlocking model, interlocking logic, interlocking application, field processor, field unit.

**Figure.** The full Luchessa drawing from `Railroad/SPCoast/`, the one the reader will make (plan principle 4), with
call-outs back to each segment: the hand switch (1), masts (2, 6), interlocking limits (3), `783T1` (4), the route
locking (5), the `HD` line toward the next control point (6), the `PanelColumn` (9), the bungalow (10), the code line
(8, 11).

**The relay layer.** One equation [GLOSSARY section 2.4]:
field unit = (interlocking logic + interlocking model) on a field processor.

**East/West.** The take-home table of every choice the talk named: switch Style, mast aspect policy, semaphore
quadrant, mast `Kind` and plates, `RULE_251`, code line type. "Your givens and druthers fit."

**What comes next.** Say for each item whether it exists on the day of the talk.

- Today: field units on Arduino-class processors; the SPCoast CTC machine on an MQTT code line; a virtual SPCoast
  subdivision (FieldUnit-Subdivision).
- The I/O pieces exist and the integrations are next: a field unit as the host of a C/MRI system (`CmriIOBus`,
  `src/drivers/CmriIOBus.h`); appliances reached as MQTT IoT devices, JMRI's MQTT topics included (`MqttApplianceBus`,
  `src/drivers/MqttApplianceBus.h`).
- Planned: virtual CTC machines on a screen, driving MQTT appliances; a tower lever frame; automatic signals between
  control points; an existing JMRI layout brought in (plan, hands-on path 3).

**Closing line.** "On a model railroad, this is your railroad."

**Sources.** `docs/GLOSSARY.md` sections 2.4 and 2.5; `docs/plan/user-docs-plan.md`, hands-on paths.

**[demo, optional]** The virtual SPCoast subdivision (FieldUnit-Subdivision `spcoast_virtual_plant`) behind the desk:
line a route through Luchessa and watch the signal clear.

---

## Source leads

Found by web search on 2026-11-03 and not yet opened. railroadsignals.us and seinwezen.irse.nl could not be fetched
from the cloud session (network policy), so their timelines were seen only through search results.
Keys are provisional; a lead that is opened and kept moves into `docs/GLOSSARY.md` section 12 under the same key.

| Key | Source | Used for |
|---|---|---|
| KING1921 | King, E. E., *Railway Signaling*, 1921, archive.org/details/cu31924003632712 | 1, 2: train orders, train order signals |
| IRSE-US | IRSE Netherlands, US signaling history timeline, seinwezen.irse.nl/tijdlijn/geschiedenis_usa | 2, 3, 4: 1832, Trenton 1870, Robinson 1872 |
| WIKI-SIG | Wikipedia, "Railway signal" | 2: New Castle and Frenchtown, 1832 |
| WIKI-INT | Wikipedia, "Interlocking" | 3, 5: Saxby, Bricklayers Arms 1843, electro-pneumatic 1891 |
| TRAINS | Trains magazine, "Train signals", trains.com/?p=39372 | 3: Trenton 1870 |
| ROBINSON | Robinson, W., *The Invention of the Track Circuit*, American Railway Association (reprint) | 4: 1872, Kinzua |
| CHI-L | chicago-l.org, "Interlocking examples" | 5: US&S electro-pneumatic from 1891, 54 plants by 1900 |
| NOCK1956 | Nock, O. S., "Power Interlockings: Past History and Future Prospects", IRSE Proceedings 1956, jonroma.net | 5: power interlocking history |
| WIKI-HALL | Wikipedia, "Hall Signal Company" | 6: Hall, 1871, disc signals |
| WIKI-SEM | Wikipedia, "Railway semaphore signal" | 6: upper quadrant about 1903 |
| HOWE1911 | Howe, W. K., "Absolute-Permissive Block System", GRS, 1911 (jonroma.net 1911 index; irse.nl GRS bulletin 120) | 6: APB |
| ICC1922 | "Final Train Control Order Issued by ICC", *Railway Signaling* 1922, jonroma.net | 7 |
| RS1960-CON | "Signal construction steady in 1959", *Railway Signaling and Communications*, 1960, jonroma.net (owner-supplied) | 8: CTC mileage by road |
| TH-CTC | "Railroad history: centralized traffic control", transporthistory.wordpress.com, 2015 (owner-supplied) | 8: western CTC mileage, capacity |
| UTAHRAILS | "Train control", utahrails.net/train-control.html (owner-supplied) | 8: East/West adoption |
| TRAINS-CTC | "CTC: remotely directing the movement of trains", Trains, ABCs of railroading (owner-supplied) | 8: capacity, eastern track removal |
| RRNET-4TRK | "Four-track removal in NY state", railroad.net forum (owner-supplied; a forum, so color only) | 8: NYC four to two tracks |
| ICC1947 | "Signaling Order Issued by ICC" and "Territories Involved in the ICC Signaling Order", *Railway Signaling* 1947, jonroma.net; ICC hearings 1946–1947 on Docket 29543 | 7: the 80 mph rule and which roads it covered |

Other archives to mine for the textbook: jonroma.net *Railway Signaling* (1908 on); the O Scale Resource railroad
signal document archive (US&S, GRS, Hall catalogs); railroadsignals.us timeline and styles pages; Nock,
*Fifty Years of Railway Signalling* (archive.org); The Signal Box and the Signalling Record Society for British
background.

## Open questions for John

Settled in this round (owner, 2026-11-03): the train order signal gets a glossary entry (done), a KiCad symbol and a
model object (to do); the italic terms got [Prototype] and [Both] glossary entries (done); yard limits and CTC limits
moved to segment 8 as the places where the dispatcher's authority ends; segment 10's sidebar is dual control as
resilience.

1. **New glossary entries need review.** 22 entries were added to `docs/GLOSSARY.md`. Most carry "(unverified)":
   their facts come from the leads below, not opened sources. Rule 221 for the train order signal is still a guess.
2. **Signal type names.** Fixed in the glossary: color light is the generic name for the multi-lamp head (SP three-lamp
   heads; GRS Type D is one product) and the searchlight; color-position light (CPL) is the B&O system; position light
   (PL) is the PRR's. Still open: the KiCad Style word `CPL` names the three-lamp head today; rename it (for example
   `CL` for the three-lamp head, `SL` for the searchlight) in the symbol library and FieldUnit-Subdivision? And the
   position-color light (PCL) entry records your PRR-area description as (unverified): is there a railroad and era to
   cite?
3. **End CTC sign.** Segment 8's figure wants an End CTC sign. Is there a KiCad symbol, as there is for
   `Rule93-Yard Limits`?
4. **Claims with no lead:** digital code line dates (11), color light dates by type (6).
5. **Figures that need new symbols:** switch stand target, train order signal, lever and wire run, pipe run and bell
   crank, lever frame, relay coil and contacts, stepping and slow-release relays, transport, End CTC.
6. **Demo readiness.** Which demo hooks will be ready by early 2027: the ghost plant (`--loopback`) works today against
   the desk; the virtual SPCoast plant lives in FieldUnit-Subdivision.
