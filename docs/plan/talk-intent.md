# Talk Intent: Why This Talk, for This Audience

Draft 1, 2026-11-04. Intent refinement for deliverable 1 of `docs/plan/user-docs-plan.md`, before more work on
`docs/plan/talk-outline.md`. Everything here is a proposal for the owner to accept, change or reject.

## 1. What prompted this

An informal discussion with experienced modelers (2026-11-04, the owner's summary) asked two questions:

- What is the reason for this presentation?
- What will the audience get out of it, and why should they find it worth their time?

The venue is the Layout **Design** and Layout **Operations** SIGs. So the questions become three:

1. What does this talk add to the **play value of operations**?
2. What does it add to the **layout design** conversation?
3. What do those two mean for a layout's **design and construction**: physical room for masts, track spacing,
   aisle widths, wiring complexity and cost?

The discussion also exposed a gap in the documentation as a whole. We wrote the railroad's rules and the design
principles of the logic. We did not write about the **objects on the layout** that the logic works: switch machines
(servos, Tortoises), point detection, signal masts with their bases and wires, occupancy detectors, DCC power and its
gaps, fascia panels, and the wiring between them.

## 2. The diagnosis

The outline today is a good history of signaling and a good explanation of FieldUnit. Both answer "what is it?".
Neither answers this audience's question: **"what would it do for my layout, and what would it cost me in room,
wiring and money?"**

The history stays. It is what makes the vocabulary stick and what earns the peers' trust (plan principles 1 to 3).
But in this room it has to serve the layout: every era should end on what it adds to an operating session and what it
asks of the benchwork.

## 3. Proposed purpose and takeaways

**Purpose (draft).** Show Design and Ops SIG members what prototype signaling adds to an operating session, and what
it asks of the layout, era by era, so they can decide how much of it to build, and where.

**The audience leaves with (draft):**

1. **Jobs and play.** Each era of signaling creates operating jobs and interactions: the train order operator, the
   tower operator, the dispatcher at a CTC machine, crews who read signals, a crew asking for an electric lock release.
   They can choose the era by the session they want to run.
2. **A planning checklist.** What signaling asks of a layout at each level (TT&TO, ABS, interlocked junctions, CTC):
   where signals and detection go, what room they need, what crosses the fascia, how the wiring is laid out.
3. **A sense of scale.** For one control point at the end of a siding: how many devices, how many I/O points, how
   many wires, roughly what it costs. Measured on a real build (plan, hands-on path 2), not estimated.
4. **The why**, unchanged: the history that explains the vocabulary, and the East/West choices that fit their
   railroad.
5. **Where FieldUnit fits**: the logic and the code line, so they buy commodity hardware and do not write the
   interlocking themselves.

## 4. What the talk adds to operations (play value)

Proposals for each era. "Job" is a person at the session; "interaction" is something two people must do together.

| Era (segment) | Jobs it creates | Interactions and play |
|---|---|---|
| Switch tender, TT&TO (1, 2) | train order operator; crews who read the timetable | meets planned on paper; copying orders; the train order signal stops a train for orders |
| Tower and interlocking (3, 5) | tower operator | the operator lines routes for trains on demand; the locking refuses a wrong move; time locking punishes a hasty change of mind |
| Track circuits (4) | none new | occupancy shown on a panel; a dropped detector reads as a train, a fault to find |
| ABS and APB (6) | none new; crews now read signals | crews obey aspects; the timetable and orders still give authority, so ABS adds rules without adding a person |
| CTC (8 to 10) | the dispatcher at a CTC machine; crews on the radio | meets without stopping; the dispatcher reads lamps, not reports; an out-of-correspondence lamp is a puzzle; electric lock releases and dual-control switches make crews and dispatcher work together |
| Digital code line (11) | the same as CTC | the dispatcher can sit anywhere: another room, another city, a screen |

Points to make in the talk:

- Signaling **changes who is busy**. TT&TO keeps operators and crews busy with paper; CTC moves the work to one
  dispatcher and gives crews more running time. Pick the era for the session you want.
- Prototype pace is part of the play: a switch takes seconds to move and a code takes seconds to send
  (`docs/AAR_SIGNALING_PRIMER.md` section 9.8 proposes a fast-clock cadence).
- Failure is play, if it is prototypical: a refusal, a lamp that does not light, a time-locking wait.

## 5. What the talk adds to layout design, and what it asks of construction

These are the questions the experts asked. The answers below are the questions to answer, with what is known today.
Physical dimensions are not stated here: each one must be measured, or taken from a source, before it goes on a slide.

| Topic | The design question | Known today | To find out |
|---|---|---|---|
| Signal placement | where must a mast stand: at the insulated joint, clear of the fouling point, on the engineer's side | `docs/tutorials/01_drawing_your_signaling_track_plan.md` Rules 2 and 3 | prototype placement rules [cite?] |
| Room for masts | how far from the track a mast stands; whether it fits between two tracks at the layout's track centers; when a dwarf, a bridge or a cantilever is needed | nothing in our docs | mast base footprints in the common scales; prototype practice for masts between tracks [cite?] |
| Track spacing | whether signals between tracks force wider track centers, or move to bridges and dwarfs | nothing | as above |
| Visibility | whether a crew in the aisle can read the aspect: mast height, facing, distance, lighting; aisle width | nothing | owner and SIG experience |
| Detection | where the track circuits are cut; how they line up with signals and fouling points; non-shunting cars | tutorial 1; glossary (track circuit, boundary track circuit) | resistor wheelsets, optical sensors, and their cost per block |
| DCC power | how detection sections relate to power districts and gaps; current detection in series with the track feed | nothing | wiring patterns for common detectors |
| Switch machines | servo, stall motor or solenoid; point detection; room under the benchwork; manual override at the fascia | nothing beyond Style words | the common machines and their I/O |
| Fascia | local panels for electric locks, dual control, tower levers; room on the fascia and in the aisle | primer Act VI (fascia lamps for locks, maintainer call) | panel sizes; where crews stand |
| Wiring | one field unit per control point under the benchwork, or one central; wire runs; buses (I2C, CMRInet, WiFi) | `src/drivers/`; how-to 2 (C/MRI) | a wire list for one control point |
| Cost | devices and I/O for one control point | the plan's proof point: 1 switch, 1 signal with 3 masts, OS detector on a 16-I/O field processor | a bill of materials from that build |
| The dispatcher's desk | where it goes: on the layout, in another room, a screen | the SPCoast CTC machine | room and network needs |

The scale of one control point, to be confirmed by building it: the proof point of the plan has one switch (an
output for the machine, inputs for point detection), one signal with three masts (an output for each lamp), and one
OS detector (one input), on 16 I/O. That count is the plan's; whether it closes is what the build will show.

## 6. The documentation gap

What we have (concepts and logic):

- The history and vocabulary: glossary, primer, this talk.
- Placement in the logic: tutorial 1 (insulated joints, fouling sensors, signal placement and naming).
- The software: tutorials 2 and 3, how-tos 1 to 5.

What we do not have (the physical layer):

- The **appliances as objects on the layout**: switch machines and point detection, signal masts and heads (LED types,
  common anode or cathode, bases, wiring), semaphore servos (how-to 4 has the software side only), occupancy detectors
  of each kind, insulated gaps and DCC power districts, fascia controls.
- **Planning**: room, track centers, sight lines, aisle widths, where the field units go.
- **Wiring and cost**: an I/O count, a wire list and a bill of materials for one control point, then for a siding,
  then for a subdivision.

Plan principle 5 says "No hardware or software manual in the learning track ... The documents teach concepts and
decisions." This gap does not contradict it. Where a mast stands, whether it fits between tracks, how many wires a
control point needs and what it costs are **decisions**, and the Design SIG makes exactly those decisions. A
step-by-step build of a particular board is still a manual and still belongs elsewhere.

Proposal:

- Amend principle 5: the learning track teaches the **physical decisions** (room, placement, sight lines, I/O count,
  wiring layout, cost) as well as the logical ones; build instructions for particular hardware stay out of it.
- Add a deliverable, **"Planning a signaled layout"**: the physical layer as decisions, one short section per
  appliance and one per planning topic, with numbers measured on the hands-on build. It comes before the textbook
  (deliverable 4), because the talk needs it.
- Make hands-on path 2 (one end of a siding as a control point) the measurement: photos, dimensions, an I/O count, a
  wire list and a bill of materials. Its results feed the talk's numbers.

## 7. What changes in the talk outline (proposed, not yet applied)

Options for the hour:

- **A. Weave.** Keep the 13 segments; give each one an "On your layout" block of two parts, ops (jobs and
  interactions) and design (room, detection, wiring), replacing the single closing line. Cut the history in each
  segment to fit.
- **B. Two halves.** About 30 minutes of history, compressed, then about 25 minutes of "planning a signaled layout":
  jobs by era, then what it asks of the benchwork, ending on one control point built and measured.
- **C. Lead with the layout.** Open on an operating session and a benchwork photo; tell the history as the answer to
  "why is that signal there?"

Recommendation: **A, with segment 13 rebuilt as the planning checklist.** It keeps the history doing its job
(each era earns its vocabulary) while every segment answers the room's question. The cost is history detail: segments
5, 7 and 12 shrink to a minute or two, and segment 13 grows to about 8 minutes: jobs by era, the physical checklist, and
the measured control point.

Also proposed:

- A new title that names the payoff, for example "Signals on Your Layout: What a Hundred Years of Railroading Adds to
  Your Op Sessions, and What It Asks of Your Benchwork" (placeholder).
- The cold open asks the room's question first: "What would signals add to your sessions, and what would they cost
  you?" The history then answers it.
- Each story line gains a last clause for the layout, for example segment 6: "... and on your layout, every mast is a
  hole in the scenery, a few wires, and one more thing a crew must read from the aisle."

## 8. Questions for the owner

1. **Purpose and takeaways** (section 3): accept, edit, or rewrite?
2. **Shape of the hour** (section 7): A (recommended), B or C?
3. **Principle 5 amendment and the new deliverable** (section 6): accept? And does "Planning a signaled layout" come
   before the textbook, as proposed?
4. **Numbers**: is hands-on path 2 ready to be built and measured before the talk (early 2027)? If not, which numbers
   can come from the SIG members' experience, credited to them?
5. **Who else to ask**: which Design and Ops SIG members would review the physical checklist (section 5)?
