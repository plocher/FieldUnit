# User Documentation Plan

Living document. First written 2026-11-02 from a brainstorming session.
It records who the user documentation is for, how it is shaped, and the order of work.
Implementation documentation (ADRs, reviews, handoffs, schema and code generation) is out of scope here.

## Problem

The existing references (Chubb's C/MRI handbooks, `Signal_Aspects.pdf`, our own primer and glossary) are good reference material.
They are hard to learn from, because each one tries to be a history lesson, a textbook, a build manual and a reference at once.
Agent-written documents add a second problem: they collect facts without a story line that leads the reader through them.

## Audiences

| Tag | Who | What they need |
| --- | --- | --- |
| **C/E: peers** | Builders in the same ecosystem: JMRI (Bob Jacobsen), C/MRI (Bruce Chubb), NMRA LD and Ops SIGs, others | Trust in the foundation: fidelity to historical fact, and room for their own givens and druthers. Correct, but not rivet counters. |
| **F: prototype-minded modelers** | People who model prototype operations, scenery, rolling stock and trackwork, and want prototypical signaling, but who are not software or electrical engineers | A way through the initial complexity. A story that makes the vocabulary make sense. |

A later internal-developer set (schema, code generation) is out of scope for this tranche.

## Principles

1. **History is the scaffold for the glossary.** Each era solved one new problem and added a few terms.
   Following the timeline is how the reader gets to the present without learning 100 concepts at once.
   (The same method works for private pilot training.)
2. **Two axes: era and region.** Era explains why the vocabulary exists.
   Region (East vs. West for now) explains why railroads differ: density of traffic and money drove most choices.
   Each "where railroads differed, and why" sidebar links to the FieldUnit setting that lets a modeler choose.
   This is how the peers see that their druthers fit.
3. **Legitimacy comes from the relay layer.** For each technical concept, show three layers:
   picture (symbol-language figure) → relay circuit or equation (AREMA/AAR form) → what FieldUnit does (one line, link to reference).
   Chubb's level of detail is sufficient. Cite the source of a claim; do not write a bibliography as long as the text.
   Primary sources we already hold are in `docs/reference/`.
4. **Figures share the KiCad symbol design language.** Each figure is a small KiCad sheet in a docs-figures project in
   `~/Dropbox/KiCad/Railroad/`, exported with `kicad-cli sch export svg`. The reader learns the notation while learning the history,
   and the last chapter looks like the drawing the reader will make.
5. **No hardware or software manual in the learning track.** Hardware comes from commodity sources; the software is this project.
   The documents teach concepts and decisions.
6. **Documents may lead the software.** Nothing written at this level becomes obsolete when the software catches up.
   A tutorial that cannot be followed yet is the use case that grounds the software work.
7. **Claims stop where project support stops.** History can go further (to PTC); claims about what FieldUnit does cannot.

## The spine (era × region)

Each segment: the problem, the invention, the 3–6 glossary terms it introduces, one figure, the relay layer where it applies,
an East/West sidebar where it applies, and a last line: "on a model railroad, this is …".

1. The switch tender: points, hand-thrown switches, derails, timetable and train orders
2. Fixed signals, the strongarm era: ball signals, train order signals, wire-worked semaphores, aspect vs. indication, mast and head
3. The tower and the lever frame: interlocking, mechanical locking, pipe and wire, route
4. Track circuits, electricity arrives: occupancy, detector locking, OS
5. Power interlocking: electro-pneumatic, switch machines, relays, route, approach and time locking
6. Automatic block, ABS then APB: protection without authority, then absolute head blocks and traffic direction; electric signals
7. Train control (ATS, cab signals, ATC), context only: the ICC orders, and the 79 mph limit as the West's answer
8. CTC, meets without stopping: crew-thrown sidings and costly towers replaced; the lever frame cut in two, the code line, how relays and a stepping code line work
9. CTC, the office: the dispatcher's desk, panel columns, office correspondence, subdivision
10. CTC, the field: the control point, bungalow, powered switches, electric locks and dual control
11. Digital code lines: the code line as data, logic in software, what FieldUnit was built to provide
12. PTC, context only: PTC needs train speed control (DCC, car and engine detection) that this project does not do.
    Correct switches, signals and CTC are the foundation for it; PTC itself is out of scope.
13. Your layout, and what comes next: the whole story mapped onto FieldUnit, then C/MRI hosting, MQTT appliances, virtual CTC machines

(Spine revised 2026-11-03 from the owner's review of the talk outline: segments 8 to 10 split CTC; PTC split from digital code lines.)

Every glossary term is assigned to exactly one segment (where it is introduced). Terms with no segment are either
reference-only or a sign of a gap.

## Hands-on paths (after the spine)

1. A fully virtual or emulated layout (SPCoast, or smaller), app or web based.
2. Proof point: one end of a siding as a control point: 1 switch, 1 signal with 3 masts, OS detector, on a 16-I/O cpNode-Xiao/IOX field unit.
   Bonus: double ended, 2 control points in one interlocking.
3. Proof of usability: a second KiCad layout project for an existing layout, probably one already running with JMRI
   (CMRInet or JMRI-MQTT). Includes a virtual CTC machine and existing hardware. Possible spike: import JMRI panels and tables instead of KiCad as the source.

## Course correction (owner, 2026-11-04)

An informal review by experienced Design and Ops SIG members, and the owner's conclusions from it.

**What they said.** This audience already knows prototype signaling as well as we present it. They do not need a
history of signaling or another introduction to the topic. One comment: "this is a 2 to 3 page handout and one slide
of context. You've done something interesting with the software: what did you learn, what have you done, what can I
do with it?" Tables of material they know or can learn elsewhere (jobs by era, for example) are filler.

**What we conclude.**

- The history and concepts outline is good, and necessary, but not sufficient. It has done its job of focusing our
  thinking. It rests now (`talk-outline.md`, `glossary-segments.md`); its likely future is the handout and one slide
  of context, and the textbook for the F audience.
- What is new, and what this audience came for, is our tooling, concepts and capabilities: what makes it easier,
  better, faster or cheaper to put all of this on a layout.
- The talk-intent brief (`talk-intent.md`) had the audience wrong: it still assumed the room needed the prototype
  explained. Its physical checklist (section 5) and gap list (section 6) are kept as seed for the next distillation;
  the rest is superseded.

**The headline (owner, 2026-11-04): composability.** The biggest feature of this work is composability at several
levels, and a cosmopolitan attitude toward alternative instantiations. FieldUnit does not insist on its own stack:
any role can be real, virtual or someone else's (JMRI or CATS as the office, an existing CMRInet layout as the I/O,
IoT devices as the appliances), and the vital logic stays the same. Levels at which a layout chooses, each with more
than one instantiation today or planned (`docs/plan/realization-inventory.md`):

- roles: office, code line, field unit, devices, each real, virtual, or another project's;
- deployments: virtual, firmware near the devices, host with remote I/O (FieldUnit ADR 0004);
- code line: encoding (AAR tokens, bit-packed C/MRI) × transport (MQTT, serial, ...);
- I/O: buses (local expanders, C/MRI buffers, MQTT topics, mock) and device drivers;
- the model: KiCad drawing, JSON, or the fluent API, describing the same plant;
- the railroad: aspect rulebook per mast, switch Style, mast kind, era.

This is what the SIG talk is about. The history is the context slide.

**What comes next: two more distillations, then the talk.** Each is first a gap analysis and a plan to find the
missing information, not agent-led authoring. Neither is an immediate task.

1. **Realization on a layout.** Slice by how the system is realized on a layout, not by prototype era and railroad:
   devices (switch machines and point detection, masts and heads, detectors, fascia controls), wiring (power, DCC
   districts and gaps, buses, where field units go, room and track spacing), concepts (what FieldUnit models and what
   it leaves to the layout), and interoperability (JMRI, CMRInet, LCC, DCC-EX, MQTT). For each slice: what we know,
   what we have built, what is missing, and who or what can supply it (a build, a measurement, a SIG member, a source).
   The frame is the seams, not a feature list: each role (office, code line, field unit, devices) can be real or
   virtual on its own, and each seam can be crossed several ways, so the combinations are the product (a virtual
   layout; a new office on an existing CMRInet layout; JMRI/CATS as the office with field units; field units driving
   IoT appliances over MQTT). Seed inventory: `docs/plan/realization-inventory.md`.
2. **Operations and play value, once it is in place.** Slice again: what running a session on a FieldUnit layout is
   like, and how it compares to existing norms (JMRI panels and CATS, plain DCC with verbal authority, other CTC
   approaches).
3. **Then the talk**, rebuilt around "what we did, what we learned, what you can do with it", with the history as one
   slide and a handout.

The plan's audiences still hold, but the SIGs are a C/E audience on signaling itself. The F audience is the one that
needs the history spine.

## Deliverables, in order

1. **Talk outline = spine.** A 1-hour, in-person presentation for the NMRA LD/Ops SIG, early 2027: approachable, interesting, fun.
   Pure story at minimum; a demo if the emulation is ready. 2 to 6 minutes per segment.
   **Resting (2026-11-04)**: see "Course correction" above. The outline is history and concepts; its likely future is
   a handout and the textbook. The SIG talk will be rebuilt after the two distillations.
2. Glossary-to-segment assignment. **Resting with the outline.**
3. Docs-figures KiCad project in `Railroad/`, and the first figures for the talk.
4. The textbook: the talk written down, one short chapter per segment. Mostly cut from the existing primer and glossary along the spine.
5. Peer brief: the architecture, seams and interoperation (JMRI, C/MRI, MQTT), with links into the reference.
6. Tutorials for the hands-on paths.
7. Reorganize this `docs/` tree to own the user documentation for FieldUnit and FieldUnit-Subdivision.

## Later

- Interactive documents that demonstrate the software (live lever and locking widgets).
- Per-railroad and per-era resources, as historical data sites are found.
- A finer region split (eastern roads such as CB&Q, B&O, NYC differ quickly).
