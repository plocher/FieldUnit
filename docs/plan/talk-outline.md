# Talk Outline: From the Switch Tender to Your Layout

Draft 5, 2026-11-03. Deliverable 1 of `docs/plan/user-docs-plan.md`.
Audience: NMRA Layout Design and Operations SIGs, in person, early 2027. One hour.
Tone: approachable, interesting, fun.

## How this outline works

The talk tells a story; the slides hold the details. Each segment has four parts:

- **The story**: what the speaker says, in the speaker's voice. It does not read the slide. It is a draft to be said
  aloud and cut, not a script to be read.
- **On the slide**: the one figure, in the KiCad symbol language of the Railroad library, and the few labels on it.
  The figure carries the details the story only touches. **(symbol needed)** marks a symbol not known to exist.
  The figures are drawn in deliverable 3.
- **In the moment**: details to reach for when a question comes, or when there is time: the East/West difference,
  the relay underneath, what FieldUnit does, dates. None of it has to be said.
- **Sources**: keys from `docs/GLOSSARY.md` section 12, or **[lead: KEY]** for a source found by web search and not
  yet opened (listed under "Source leads" at the end). **[cite?]** marks a claim with no lead.

Optional demos are marked **[demo, optional]**. Glossary terms are not listed here: which terms each segment
introduces is in `docs/plan/glossary-segments.md`, and that list is for the textbook, not the talk.

Draft history: draft 3 split CTC into three segments and PTC from digital code lines; draft 4 added 22 glossary
entries; draft 5 (this one) rewrites every segment around its story, and fixes the signal type names
(color light, B&O color position light, PRR position light).

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
| 8 | CTC: when towers cost too much | 6 | 0:30 |
| 9 | CTC, the office: the dispatcher's desk | 4 | 0:36 |
| 10 | CTC, the field: the control point | 4 | 0:40 |
| 11 | Digital code lines | 4 | 0:44 |
| 12 | PTC (context only) | 2 | 0:48 |
| 13 | Your layout, and what comes next | 6 | 0:50 |
| | Questions | 4 | 0:56 |

The thread through all of it: **each era replaced a person's attention with something that cannot forget**, first a
rule, then a mechanism, then a circuit, then software. And each time, money and traffic decided who bothered.

---

## 0. Cold open (2 min)

**The story.**
"This is a CTC machine. [beat] Every lever on it, every lamp, every button, is the answer to a wreck, a lawsuit, or a
payroll. Tonight we're going to earn every one of them. We'll start with a man standing next to a switch in 1850 and
walk forward a hundred and some years, and we'll pick up the words one era at a time, the way the railroads did.
By the end, this desk will read like a sentence.

Two things to watch for. The first is *when*: every word on that desk was invented to fix a particular problem at a
particular time. The second is *where*: the East and the West solved the same problems differently, and it almost
always came down to two things, how much traffic and how much money. If you model a particular road in a particular
year, that's where your choices come from.

No electronics tonight, and no code. Just one drawing language, and you'll be fluent in it by the end."

**On the slide.** The SPCoast CTC machine (`docs/images/SPCoast-cTc-panel.png`) beside a rack of glass-cased relays
(photo needed). No labels.

**In the moment.** Nothing. Keep it short.

---

## 1. The switch tender (4 min)

**The story.**
"It's 1850. You're running a railroad with one track and trains going both ways. How do two trains pass? One of them
goes into a siding. Somebody has to throw that switch, by hand, with a switch stand, and somebody has to remember to
throw it back. If they forget, the next train goes into the siding at track speed.

How does anyone know which train takes the siding? Paper. The timetable says who has the right to the track, and when
things go wrong, the dispatcher sends a train order by telegraph, an operator writes it out, and the crew carries it.
The first telegraphed train order is usually credited to Charles Minot on the Erie in 1851.

And when the paper fails, there's one more trick: the derail. It sits on the siding and puts a runaway car on the
ground before it can reach the main line. It's the railroad saying, 'I'd rather lose a car than a train.'

Notice what's protecting everybody here. Not a machine. A person, paying attention, following rules. Every railroad
writes its own; the Standard Code that would later gather them up hasn't happened yet. Everything else tonight is
about replacing that person's attention with something that cannot forget.

On your layout, this is a ground throw, a car card, and a crew that reads the timetable. Those car cards and switch
lists you use in op sessions? That's real railroad practice."

**On the slide.** The end of a passing siding at a station, all hand-operated: the switch (Style `manual`) with its
switch stand and target (symbol needed), a derail on the siding, the fouling point marked.

**In the moment.**
- Rules that came later, in the future tense: keeping right on double track would become Rule D-151; entering a siding
  only when switch and derail are known to be lined would become Rule 105 in SP's numbering [PRR56; GLOSSARY section 9].
- East: double track came early on dense lines. West: long single track, where timetable and train order lasted for
  decades.
- FieldUnit: a hand-operated switch can be part of the model, with or without point detection (Style `manual` or
  `manual_sensed`); a derail rests in its derailing position [GLOSSARY section 7].

**Sources.** [PRR56]; [CFR218] and [GCOR6] for the derail; KING1921 for train orders.

---

## 2. Fixed signals, the strongarm era (4 min)

**The story.**
"Paper has a problem: a train at fifty miles an hour can't stop to read it. So the railroads put up fixed signals,
things at a known place that an engineer can see from a long way off.

The first ones were almost comically simple. On the New Castle and Frenchtown, in 1832, they hoisted a ball up a mast.
Ball up, ball down. That's where 'highball' comes from, or at least that's the story everybody tells.

Then came the train order signal. It stands at the station, the operator works it, and it says exactly one thing:
'I have orders for you' or 'I don't.' That's it. It is not a block signal. It knows nothing about the track ahead.
It's a mailbox flag.

And then the semaphore: an arm on a mast, worked by a wire from a lever, with an oil lamp behind a colored glass at
night. In this era the arm has two positions, stop and go. There's no 'approach' yet, no warning that the next signal
is red.

Everything in this era runs on muscle. A person, a lever, a wire, an oil lamp. No batteries, no electricity. That's why
I call it the strongarm era.

One idea from here matters all night: what a signal looks like and what it means are two different things. The look is
the aspect. The meaning, written in the rulebook, is the indication. Same meaning, different look, on different
railroads. Hang on to that.

On your layout, this is the order board at the depot: the operator's way of saying 'stop, there's paper for you.'"

**On the slide.** A station on single track: a train order signal at the depot, worked by a lever and a wire run
(symbols needed: train order signal, lever, wire run). A ball signal inset. One signal labeled "aspect" on one side and
"indication" on the other.

**In the moment.**
- The train order signal would later have its own rule in the Standard Code (Rule 221 in most books; check [PRR56]).
- Lower-quadrant semaphores: two positions, Stop and Clear [GLOSSARY section 7, semaphore signal].
- FieldUnit: no train order signal yet. A symbol and a model object of its own are planned; it is not a block-signal
  mast (owner, 2026-11-03). The aspect and indication machinery (`src/SignalAspectPolicy.h`) is for interlocking and
  block signals.
- "Highball" from the ball signal is folk etymology as far as these notes know [cite?]; say "the story everybody
  tells", or cut it.

**Sources.** [AAR56] for aspect and indication; WIKI-SIG and IRSE-US for 1832; KING1921 for train order signals.

---

## 3. The tower and the lever frame (5 min)

**The story.**
"Now put two or three switches and a couple of signals in one place, a junction, or a crossing with another railroad.
One person works them all from a tower, with a row of big levers. Pull a lever, and a pipe running out under the
ground throws a switch two hundred feet away. Pull another, and a wire drops a semaphore arm. They called them
armstrong levers, for good reason.

Here's the danger. The operator is in a hurry. He clears the signal for the main line, and the switch is still lined
for the siding.

The answer, from England in the 1850s, arriving at Trenton, New Jersey, in 1870, is the most important idea in this
talk: interlocking. Under the levers is a locking bed, a grid of steel bars and dogs. When the switch is in the wrong
position, the signal lever cannot move. Not 'shouldn't.' Can't. The operator feels the refusal in his hand.

The railroad wrote down which levers lock which in a locking sheet. 'Two reversed locks one normal' means: you can't
clear signal 2 unless switch 1 is normal, and once signal 2 is clear, switch 1 is frozen. That line is a route.
It is a truth table, eighty years before anyone wired one out of relays.

There's no electricity in any of this. It's steel and leverage and somebody's very good idea.

On your layout, this would be the control panel at a yard throat, or CATS in JMRI: the panel that won't let you line
a conflicting route."

**On the slide.** A passing siding at a station, one end interlocked from a tower: switch `1`, home semaphores `2` and
`4`, the interlocking limits shaded, pipe and wire runs back to the tower (symbols needed: pipe run, bell crank, lever
frame). Under it the lever frame and one line of the locking sheet: "2 R locks 1 N".

**In the moment.**
- Lever positions: N and R for a switch; L, N and R for a signal [AAR56; GLOSSARY section 5, lever].
- Odd numbers for switches, even for signals: a habit, not a rule [GLOSSARY section 10.2, unverified].
- East/West: a tower costs three shifts of operators, every day. Dense eastern main lines paid for towers every few
  miles; in the West towers stood mostly at junctions and crossings at grade.
- FieldUnit: each route in an interlocking model lists the switch positions it needs and the signal that governs it,
  the locking sheet as data (`examples/CP_Corporal/CP_Corporal.json`, `aligns`, `governedBy`). A tower lever frame is
  in the project's scope but not built yet.

**Sources.** [AAR56]; WIKI-INT, IRSE-US and TRAINS for Saxby and Trenton.

---

## 4. Track circuits: electricity arrives (4 min)

**The story.**
"The locking bed has a blind spot: it can't see the train. The operator can throw a switch right under a moving car.
The best the mechanical tower could do was a detector bar, a long steel bar beside the rail that the wheels hold down.

In 1872 William Robinson, at Kinzua, Pennsylvania, did something cleverer. Put a battery at one end of a section of
rail and a relay at the other, with insulated joints at both ends. When the track is empty, the current flows through
the rails, the relay is energized, and its armature is held up. When a train comes in, its wheels and axles short the
rails together. The relay loses its current, and gravity literally pulls the armature down.

Now the beautiful part. What if a rail breaks? The relay drops. The battery dies? The relay drops. A wire comes loose?
The relay drops. Every way it can fail, it fails to 'there's a train here.' That's the safe answer. Remember gravity;
it's going to keep doing this job for the rest of the talk.

This is the first battery and the first relay of the evening, and everything electric from here on is built on it.

On your layout, this is your block detector, current sensing, optical, whatever, and it had better say 'occupied'
when it fails."

**On the slide.** One track circuit between two insulated joints (`IRJ-Signal`), the battery and the track relay,
drawn twice: vacant, relay up; and with a wheelset, relay down. The ASCII sketch in GLOSSARY section 10.5 is the
brief. A small second panel: the OS section over switch 1 from segment 3, named `1T1`.

**In the moment.**
- Detector locking, in this era's terms: switch 1 may move only while its OS track relay is up. The detector bar's job,
  with no moving parts at the rail.
- The modeler's version: current sensing, optical sensors and axle counters are all one track relay in the model;
  sensors on one input are one track circuit [GLOSSARY section 7, aggregation].
- FieldUnit: every detector starts "occupied" and reports "occupied" if it stops hearing from its sensor. A switch
  bound to its OS track circuit will not move under a train [GLOSSARY sections 7 and 8].
- "OS" is "on sheet", the dispatcher's record of a train passing [BNSF].

**Sources.** ROBINSON for 1872 and Kinzua; [AAR56] p. 31; [BNSF].

---

## 5. Power interlocking (4 min)

**The story.**
"Back to the tower. Pipes have limits. A man can only pull a switch so far away, a tower can only hold so many
levers, and in the winter the pipes and wires froze. Big terminals needed more.

So power came in: compressed air first, at Bound Brook in 1884, then Westinghouse's electro-pneumatic plants from 1891,
then electric switch machines. The levers shrank to little handles. And once there was electricity in the tower,
relays took over the locking itself.

Here's why that matters. Relays can see the train. A steel locking bed only knows where the levers are. A relay
interlocking knows where the train is, because the track circuits tell it. So it can do things the locking bed never
could. It can prove the switch points actually went over before it clears the signal. It can hold a route locked while
a train is on it, and release it behind the train, one piece at a time. And if the operator takes a signal away in
front of an approaching train, it can hold the switches for a few minutes, in case the engineer can't stop. Some
towers did that last part with a row of mechanical timers on the wall, started by hand like a kitchen timer.

And because every relay drops by gravity when anything fails, every failure ends with the signal at stop.

This is the segment where I can answer the question 'what does FieldUnit actually do?' Segment 3 gave us routes.
Segment 4 gave us detectors. This is what you get when you put them together: a relay interlocking, and that's what
a FieldUnit is, in software.

On your layout, this is the logic behind your panel: rules that, whatever breaks, leave every signal at stop."

**On the slide.** One switch machine with its points proved, lighting its lamp only when the points are home; the
signal it protects at Stop while the points move. Relay symbols kept to a coil and one contact (symbol needed, unless
the Railroad library has them).

**In the moment.**
- The relays, one sentence each: `KR`, the switch is proved; `HR`, the signal may clear when its lever is reversed,
  its switches are proved and its track is clear; `WLR`, the switches may move when no train is on the route and no
  time is running [GLOSSARY sections 10.5 and 10.6].
- FieldUnit holds the switches for 30 s by default after a signal is taken away in front of a train [GLOSSARY
  section 8, approach locking].
- The first all-relay interlocking: GRS, 1929.
- East/West: plant size, more than region, decided power interlocking. Big terminals and junctions came first.

**Sources.** WIKI-INT, CHI-L and NOCK1956 for the dates; [AAR56]; the wall timers are the owner's knowledge.

---

## 6. Automatic block: ABS, then APB (5 min)

**The story.**
"So far we've protected junctions. What about the miles in between, where trains follow each other on paper? A late
train, a dispatcher's mistake, and the following train runs into the one ahead.

Robinson's track circuit makes the answer possible: signals that run themselves. Chop the line into blocks, each with
its own track circuit, and let each block drive the signal behind it. Hall was putting up automatic signals in the
1870s. That's automatic block signals: ABS.

Now the most important sentence in this segment: ABS is protection, not authority. The signals tell you whether the
block ahead is clear. They don't give you the right to be there. That still comes from the timetable and the train
order. The signal can say 'clear' and you still have to wait for your meet.

Around 1903 the semaphore arm learns to go up instead of down, and gets a third position: approach. Now a signal can
warn you that the next one is red.

Single track has a nastier problem. ABS stops you running into the train ahead; it does nothing about the one coming
the other way. Around 1910 General Railway Signal came up with the fix, first installed in quantity on the Toronto,
Hamilton and Buffalo in 1911: absolute permissive block. Put an absolute signal at each end of the single track, the
head block. Once a train enters, every signal against it, all the way to the next siding, goes to stop. Trains going
the same way can still follow each other in. It became the standard way to signal single track.

And with batteries and relays at every signal anyway, the oil lamp goes electric. Every railroad picks its own answer:
the Southern Pacific's three-lamp color lights and searchlights, the Pennsylvania's rows of amber position lights, the
B&O's color position lights. Same indications, different aspects. Remember that from segment 2?

On your layout, these are the signals between your sidings, the ones that run themselves. They protect the train; the
dispatcher's orders still move it."

**On the slide.** Single track between two sidings: head-block masts (`Kind` `ABSOLUTE`) at each end, back-to-back
intermediate masts (`Kind` `PERMISSIVE`) with number plates (`IRJ-Signals`), and the `HD` line as arrows from each
signal location back to the one before. Inset: one indication, Approach, shown four ways: an upper-quadrant semaphore,
an SP three-lamp color light head, a PRR position light, a B&O color position light.

**In the moment.**
- The richest East/West sidebar of the night:
  - Track: double track, a current of traffic and ABS for following moves (later Rule 251) in the East; single track
    with APB in the West.
  - Head block: most restrictive indication Stop (Rule 292). Intermediate signal with a number plate: Stop and Proceed
    (Rule 291) [AAR56 pp. 51, 53–54].
  - Route signaling (the SP shows which way the switches are lined) against speed signaling (the NYC and PRR show how
    fast you may go, at the cost of more heads).
- The relay underneath: the `HD` line reaches back to the signal behind, `H` when the track ahead is clear, `D` when
  the next signal shows Approach or better; the directional stick relay lets a following train close up
  [GLOSSARY sections 7 and 10.5].
- FieldUnit: every mast picks its rulebook (`sp1969`, `sp1985`, `gcorSpeed`, `nycSpeed`, `prrPositionLight`, `boCpl`,
  `upperQuadrantSemaphore`, in `src/SignalAspectPolicy.h`). Automatic signals between control points are designed
  (FieldUnit ADR 0007; FieldUnit-Subdivision ADR 0013) but not built yet.

**Sources.** WIKI-HALL; WIKI-SEM; HOWE1911 and the owner for APB and the TH&B; [AAR56]; [PRR56]; [CBQ1946].

**[demo, optional]** One mast switching rulebook live, SP 1969 to NYC speed, same indication, different aspect.

---

## 7. Train control (context only, 2 min)

**The story.**
"Signals only work if the engineer sees them and obeys them. Fog, a long night, one missed signal, and you still get a
wreck. So in 1922 the ICC started ordering railroads to put the signal in the cab and let it touch the brakes:
automatic train stop at the signal, cab signals fed through the rails by coded track circuits, and automatic train
control that enforces speed.

That cost a lot of money, and it was mostly the dense eastern roads that paid it. After the 1947 order, a lot of
western railroads made a different choice: keep the trains under 80 miles an hour, and you don't need it. That's where
the famous 79 comes from.

FieldUnit doesn't go into the cab. That's out of scope. On your layout, this is the engineer, and that's you."

**On the slide.** No track plan; there is nothing on it to see. One photo of a cab signal display, if wanted.

**In the moment.**
- Today's rule: 80 mph or more needs cab signals, automatic train stop or automatic train control
  ([CFR236] §236.0, to be opened and quoted).
- The segment exists for continuity: it explains 79 mph and sets up PTC (segment 12).

**Sources.** ICC1922, ICC1947, [RS1944], [CFR236].

---

## 8. CTC: when towers cost too much (6 min)

**The story.**
"Let's do the arithmetic the railroads did. A tower needs an operator around the clock: three shifts, every day of the
year, at every passing siding. Train orders are slow. Double track costs a fortune. Something has to give.

In 1927, on the New York Central between Stanley and Berwick, Ohio, General Railway Signal tried something radical.
Take the interlocking machine, cut it in two. The levers go to one dispatcher in an office, maybe a hundred miles away.
The locking stays out in the field, next to the track, where the train is. And in between, a wire. That's centralized
traffic control, CTC.

Now: a hundred miles of wire, and you want to control fifty switches and a hundred signals with it. You can't string a
wire per lever. So the code line is a trick, and it's a relay trick. Both ends have a stepping relay that clicks along
one position per pulse, in step with each other. The first few pulses pick which station you're talking to. Each pulse
after that carries one bit, long or short. A slow-release relay is the one that tells long from short: it just takes
a moment to let go. Out goes 'switch 7, reverse.' Back comes 'switch 7 is reverse.'

[If there's time, the relay primer:] A relay is a switch moved by a magnet. A front contact closes when it's up, a
back contact when it's down. Put contacts in a row, that's AND. Side by side, that's OR. And a relay that holds itself
up through its own contact is memory: one bit. That's all of computing, built out of relays, in the 1930s.

The dispatcher's authority has edges too: the CTC limits, the 'End CTC' sign, and yard limits, where trains move on
sight without orders.

And here's the East/West story at its starkest. The West had thousands of miles of single track. CTC let a single track
carry far more trains, for a fraction of the price of a second main. The Santa Fe, the SP, the UP and the Burlington
installed the most of it. The East had already built two, three, four main tracks, and ran them with ABS and towers.
When the East finally bought CTC, after the war, it was to tear track out: four tracks became two, signaled both ways,
and the tax bill went down.

On your layout, this is two wires, or one network, from the dispatcher's desk to every siding."

**On the slide.** Left: a tower (levers, locking bed, track). Right: a CTC machine and a control point joined by two
wires, an End CTC sign (symbol needed) and a `Rule93-Yard Limits` sign at the ends of the territory. Below, full
width: one end of the code line, a stepping relay, a slow-release relay and a line relay (symbols needed), and one
code drawn as a train of long and short pulses.

**In the moment.**
- A US&S 506 code has 16 steps and serves up to 35 field stations [RRS506; GLOSSARY section 6].
- Two-wire coded lines were in service by 1937 [RS1937]. The 1927 installation used a wire per function plus a common
  [EKEVING].
- East/West detail: up to 80% more capacity on single track, by one account [lead: TRAINS-CTC]; western CTC mileage
  [lead: RS1960-CON, TH-CTC]; the NYC under Alfred Perlman as the example of track removal [lead: TRAINS-CTC,
  RRNET-4TRK]; the SP's own installations [RS1942, RS1945, RS1947].
- Yard limits: Rule 93 [PRR56].
- FieldUnit: the SPCoast CTC machine can show the code going out pulse by pulse, at about prototype speed
  (`CODELINE_VISUAL_STEPPING`; a model design, primer section 9.5.1).

**Sources.** [EKEVING], [RS1937], [RS1942], [RS1945], [RS1947], [RRS506], [PRR56]; UTAHRAILS, TRAINS-CTC,
RS1960-CON, TH-CTC, RRNET-4TRK (owner-supplied leads).

**[demo, optional]** The SPCoast CTC machine with visual stepping on, sending one switch control.

---

## 9. CTC, the office: the dispatcher's desk (4 min)

**The story.**
"Sit down at the desk. Each column is one control point: a switch lever, a signal lever, a code button, and up top a
model board with lamps that light when a train is on a track circuit. One person, the whole subdivision.

Here is the strangest thing about this desk, compared to the tower. In the tower, if a move was unsafe, the lever
wouldn't move. Here, the lever moves. Always. You can put this lever anywhere you like. It's not a command, it's a
wish. You press CODE, the wish goes down the wire, and the field decides.

So how do you find out the answer? You wait for the lamp. If the lamp that agrees with your lever lights up, it
happened. If it never lights, it didn't, and nobody will tell you why. The railroads called that being out of
correspondence, and dispatchers learned to read silence.

The desk itself is not vital. Nothing here keeps anyone safe. The safety is all out in the field. The desk just says
what you want and shows you the truth.

On your layout, this is the dispatcher's job in an op session: one person, one panel, the whole main line."

**On the slide.** One panel column in the KiCad `PanelColumn` symbol: switch lever with its lamps, signal lever with
its lamps, the CODE button, track lamps above. Beside it, the subdivision: seven interlockings on one line, Luchessa in
the middle.

**In the moment.**
- Extras that make it a working tool: fleeting, maintainer call; a pushbutton console by 1959 [RS1959].
- East/West: in the West, long territories under one dispatcher [RS1942, RS1945]; in the East, towers kept at busy
  junctions with their operators working under the dispatcher.
- FieldUnit: the CTC machine sends a column's controls when you release CODE, never at power-up, and lets you move any
  lever; the field decides [GLOSSARY sections 2.3 and 5].

**Sources.** [AAR56] p. 34; [RS1959]; [USS1949].

**[demo, optional]** `tools/test_ctc_desk.py --walk` lights every station's lamps on the real desk.

---

## 10. CTC, the field: the control point (4 min)

**The story.**
"Now go out to the end of the siding. The tower is gone. In its place is a steel bungalow full of relays, and nobody
inside. It hears the code line. When a control comes in, it checks the locking, exactly like the tower's relays did,
and if the move is safe it throws the powered switch and clears the signal. If it isn't safe, it does nothing. It
doesn't argue. It doesn't call back. It has the last word.

Not every switch on the line is worth a switch machine. A spur to an industry gets an electric lock: the crew asks,
the dispatcher sends a release, and only then can they throw it by hand.

And then there's my favorite, the dual-control switch. Normally the dispatcher throws it. But it has a hand lever and
a selector. If the switch machine fails, or the code line goes down and the dispatcher can't reach it, the crew gets
authority, flips the selector from motor to hand, and the railroad keeps running. That's resilience, built into a
switch.

On your layout, this is a box under the benchwork at each siding, and it has the last word."

**On the slide.** One control point at the end of a siding: switch `783` (Style `powered`), signal `784` with its
masts, OS circuit `783T1`, and the `MAIN HOUSE` / `House` symbol. A spur nearby with an electric lock.

**In the moment.**
- The one-sentence version: the code line, the field station and the field unit together replace the locking bed;
  they do at a distance, and after the fact, what the locking bed did in the operator's hand [GLOSSARY section 2.3].
- The electric lock fails safe: the lock relay is energized to unlock, so a power failure leaves the switch locked
  [GLOSSARY section 3].
- FieldUnit: applies a set of controls whole or not at all; an unsafe set changes nothing, and the next indications
  tell the dispatcher so (FieldUnit ADR 0002). Switch Styles `powered`, `dual`, or `manual` with an electric lock;
  dual control is FieldUnit ADR 0006 (proposed).

**Sources.** `docs/GLOSSARY.md` sections 2.3, 3 and 7; FieldUnit ADR 0002 and ADR 0006.

**[demo, optional]** `tools/test_ctc_desk.py --loopback` as a ghost plant: throw a switch, press CODE, watch the lamp
go dark and come back.

---

## 11. Digital code lines (4 min)

**The story.**
"The relay code line had a good run. But it's slow: a few seconds per code, a handful of bits per station. And those
pole lines along the right of way are expensive to keep up.

So from the 1970s and 80s the code line turns into data. One message carries a station address and every control
for that station at once. It doesn't care whether it travels on a pole line, a microwave link or a radio. And the
relays in the bungalow get replaced by a computer running the same logic. Here's the important part: the railroads
didn't throw the relay circuits away. They became the specification. The software is right when it does what the
relays would have done.

That's exactly what FieldUnit is. The dispatcher's desk sends one message per station, every control at once, written
with the same AAR names the relay draftsmen used. The field unit answers with everything it knows. On MQTT, or C/MRI,
or whatever your layout already runs.

On your layout, this is what FieldUnit was designed to provide: a real digital code line between a dispatcher's
office and Arduino field units under your layout."

**On the slide.** One station's controls drawn three ways: as US&S 506 steps (16 slots), as one MQTT message of AAR
tokens (`783NWS, (783RWS), 784NGS`), and as bits on a CMRInet line, with the transport under each (symbol needed).

**In the moment.**
- A token is not a step; a code line type is one encoding on one transport [GLOSSARY sections 3 and 6].
- The industry calls per-site configured logic an "application" [ARTC23].
- Dates for digital code systems need a source [cite?].

**Sources.** [ARTC23]; [RRS506]; FieldUnit ADR 0001 and ADR 0002.

**[demo, optional]** `tools/test_ctc_desk.py --monitor`: the tokens on the wire, as text.

---

## 12. PTC (context only, 2 min)

**The story.**
"One more chapter, for completeness. Remember train control, putting the signal in the cab? After some terrible
wrecks, Congress made it universal: the Rail Safety Improvement Act of 2008. Positive train control knows the train's
authority and its speed limits, and stops the train if the crew doesn't. It's built on everything we've talked about
tonight: switches, signals, track circuits, code lines.

FieldUnit stops short of it, on purpose. PTC means controlling the train, and that's your throttle. What FieldUnit
gives you is the foundation PTC stands on.

On your layout, this is still you, with your hand on the throttle."

**On the slide.** None, or one photo.

**In the moment.** PTC needs train speed control (DCC, car and engine detection), which this project does not do
[GLOSSARY section 4, PTC].

**Sources.** `docs/GLOSSARY.md` section 4.

---

## 13. Your layout, and what comes next (6 min)

**The story.**
"So here's the whole hundred years on one sheet. [the Luchessa drawing] The hand switch from segment 1. The masts from
segment 2. The interlocking limits from the tower. The track circuit over the switch. The route locking. The line
toward the next signal. The dispatcher's column. The bungalow. The code line.

You draw your railroad in these symbols, the same ones you've been looking at all night. That drawing is the
interlocking model. The logic is the same for every railroad on earth. Put the two together on a little computer
under the benchwork, and you have a field unit. Put a CTC machine at the end of the aisle, and you have a subdivision.

And it's yours. Pick your era, pick your railroad. Semaphores or searchlights, route signaling or speed signaling, APB
or CTC, a 506 code line or MQTT. Your givens and druthers fit.

What's next: today we have field units on Arduino-class boards, the SPCoast CTC machine on MQTT, and a virtual SPCoast
subdivision. The pieces exist for a field unit to host a C/MRI system, and to reach signals and switches as MQTT
devices, JMRI's included; wiring those together is next. After that: CTC machines on a screen, a tower lever frame,
automatic signals between control points, and bringing an existing JMRI layout in.

On a model railroad, this is your railroad."

**On the slide.** The full Luchessa drawing from `Railroad/SPCoast/`, the one the audience will make (plan principle
4), with call-outs back to each segment. A take-home table of choices: switch Style, mast rulebook, semaphore
quadrant, mast `Kind` and plates, `RULE_251`, code line type.

**In the moment.**
- The equation: field unit = (interlocking logic + interlocking model) on a field processor [GLOSSARY section 2.4].
- Say for each "next" item whether it exists on the day of the talk. Today: `CmriIOBus` (`src/drivers/CmriIOBus.h`)
  and `MqttApplianceBus` (`src/drivers/MqttApplianceBus.h`) exist; the integrations are next.
- The hands-on paths of the plan: a virtual layout; one end of a siding as a control point on a 16-I/O field
  processor; an existing JMRI layout.

**Sources.** `docs/GLOSSARY.md` sections 2.4 and 2.5; `docs/plan/user-docs-plan.md`.

**[demo, optional]** The virtual SPCoast subdivision behind the desk: line a route through Luchessa and watch the
signal clear.

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

1. **Signal type names, fixed in the glossary in this draft.** Color light = the generic name, for both forms: one
   lamp per color (the SP's three-lamp heads; GRS Type D is a product of this form) and the searchlight. Color position
   light (CPL) = the B&O system. Position light (PL) = the PRR system. The code already agreed (`boCpl`,
   `CplMastDriver`); the glossary had the B&O system under "position-color light" and "CPL" on the three-lamp head.
   Still open:
   - The KiCad Style word `CPL` names the three-lamp color light head today (FieldUnit-Subdivision checkpoint E65).
     Rename it (for example `CL` for the three-lamp head and `SL` for the searchlight), and reserve `CPL` for the
     B&O system? That change lives in the Railroad symbol library and FieldUnit-Subdivision.
   - The position-color light (PCL) entry now records your description, a PRR-area system of amber position lights
     with added color elements, as (unverified). These notes found no source for it; is there a railroad and era to
     cite?
2. **New glossary entries need review.** The 22 entries of draft 4. Rule 221 for the train order signal is a guess.
3. **"Highball"** from the ball signal: keep it as "the story everybody tells", or cut it?
4. **End CTC sign.** Is there a KiCad symbol, as there is for `Rule93-Yard Limits`?
5. **Claims with no lead:** digital code line dates (11), color light dates by type (6).
6. **Figures that need new symbols:** switch stand target, train order signal, lever and wire run, pipe run and bell
   crank, lever frame, relay coil and contacts, stepping and slow-release relays, transport, End CTC.
7. **Demo readiness.** Which demo hooks will be ready by early 2027: the ghost plant (`--loopback`) works today against
   the desk; the virtual SPCoast plant lives in FieldUnit-Subdivision.
