# CTC, Subdivision, and Plant Design

This document is the next layer outside
[Inside the Bungalow: An Introduction to AAR Signaling](AAR_SIGNALING_PRIMER.md).

| Layer | Document | Scope |
| --- | --- | --- |
| Bungalow / plant vital logic | `AAR_SIGNALING_PRIMER.md` | Relays, locking regimes, HR/DR, CodeLine, one interlocking plant |
| CTC territory and plant design | **this document** | Authority rules, route equations, control-point limits, multi-plant corridors, drawing rules |
| Runtime FieldUnit | sketches / API | Evaluates plant state; does not author the plant drawing |

The primer answers: *what must be true inside the plant before a signal may leave Stop?*
This document answers: *how do you structure routes, limits, and names so that plant can be designed, drawn, and later solved—alone and as part of a subdivision?*

This is working design understanding, not the final book chapter. Keep it accurate and short enough to revise.

---

## 1. Operating frame (GCOR CTC)

North American CTC practice is framed by the **General Code of Operating Rules (GCOR)**, especially:

- **Chapter 10** — Rules applicable only in Centralized Traffic Control
- **Chapter 9** — Block system rules (where ABS / intermediate signals apply between control points)
- **Chapter 6** — Movement of trains and engines (including reverse and backup moves)
- Current-of-traffic / double-track rules such as **D-151**, **D-152**, and main-track rules **251**, **261**, **262** (direction of traffic and related authority)

Exact GCOR wording and railroad special instructions govern operations. FieldUnit models the **vital and supervisory machinery** those rules assume. It does not replace the rulebook.

### 1.1 Concepts that bind design to the rulebook

**Authority to enter CTC (GCOR 10.1 family)**
A train must not enter or occupy CTC limits unless a **controlled signal** displays a proceed indication, or the dispatcher / control operator gives explicit authority (for example Track and Time).

**Signal indication as movement authority**
In CTC, dispatcher-controlled signal indications are absolute authority for movement in those limits. Timetable superiority does not clear a train past a Stop signal.

**Dispatcher control of switches and absolute signals**
At control points, the dispatcher (or control operator) controls power-operated and dual-control switches and absolute signals. The office sends **intent**. The field reports **verified truth** (see primer CodeLine chapter).

**Reverse and backup movements (GCOR 6.4 family)**
Reverse or backup moves outside the authority already granted by signal, or special moves inside interlocking limits, need explicit dispatcher permission. Design must not treat “any reverse geometry” as automatically authorized.

**Track and Time / worker protection (GCOR 10.3 family)**
Dispatchers grant exclusive Track and Time between specified limits. Signals must not clear into those limits while the authority is active. That is territory/dispatch procedure layered on plant vital logic.

**Direction of traffic (251 / 261 / 262 and D-rules)**
Double-track and single-track rules define the normal current of traffic and when movement against that current needs special authority. On a plant drawing, **Direction of Traffic (DoT) markers** and track designations (`MT`, `MT1`, `MT2`) record operating policy at limits. They are not a substitute for interlocking route equations inside the plant.

---

## 2. Two clocks: write the equations, then solve them

Treat plant signaling like a prepared system of equations.

### 2.1 Static enumeration (design time)

**Inputs:** track geometry, appliances, signal faces, switch C/N/R connectivity, labeled nets.
**Output:** the full set of **structural routes** (lined switch combinations and clear-track lists) owned by each signal face.

A structural route does **not** know whether the track is occupied right now. It only states:

> If these switches are lined and locked this way, and these track circuits are clear, and opposing requirements hold, this face may display up to this indication ceiling—subject to downstream signal state.

### 2.2 Dynamic evaluation (run time)

**Inputs:** current switch correspondence, track-circuit occupancy, opposing locks, downstream signal state, dispatcher stick/authority, timers.
**Output:** the indication (and aspect) each mast may show **now**.

FieldUnit implements evaluation (HR, DR, ASR, knockdown, and related chains in the primer).
Plant design and Subdivision tooling **author** the static route set and the bindings those chains consume.

Do not mix the two clocks:

- Empty geometry under one full switch plant is **not** automatically “this mast is always Stop.”
- Occupied track or a switch lined against a route **is** a run-time Stop (or non-match) for that equation.
- A route that ends inside the plant (industry stub, hold short of the next signal) can still have a non-Stop ceiling when the equation is satisfied.

---

## 3. Signal faces and structural routes

### 3.1 Signal face

A **signal face** is one directional protecting unit: typically one mast (or head set) attached at a **Signal IRJ**, facing one direction of travel.

- Faces are **direction-sensitive**.
  Example: a dwarf that only protects moves **out of** an industry does not end routes that are **entering** that industry.
- One Signal IRJ may later carry **two** faces (back-to-back / bi-directional main, crossover hold). Luchessa-style plants often use one face per Signal IRJ; the contract must allow two.

### 3.2 Where a structural route starts

Start at a face that protects movement from a defined approach onto plant geometry—commonly the face that protects the **C** side of the first switch in the move, or the absolute signal at the control-point limit for that approach.

### 3.3 Where a structural route ends

End at the **next protection in the direction of travel**, or at a **dead end**:

| End kind | Meaning |
| --- | --- |
| **Next same-direction signal face** | Opens a new route set ahead (in-plant absolute, or intermediate). This face’s structural routes cover track only up to that next face. |
| **Dead end** | Bumper, stub, or industry end with no further protecting face in that direction. |
| **Control-point limit placeholder** | On a single-plant drawing: a DoT / named exit net standing in for the **next CP entry signal** (and the ABS chain toward it). See §5. |

For a chain of signals **2 → 4 → 6** in one direction of travel:

- Structural routes for **2** cover steel **only to 4**.
- Structural routes for **4** cover steel **only to 6**.
- The **indication** on 2 still accounts for the indication on 4 (and 4 for 6) through distant / advance logic (primer `DR` chain). That cascade is **evaluation**, not one giant static route from 2 through 6.

### 3.4 Route identity (what to print on a design table)

A readable structural route line looks like:

```text
route-name   mast   switch-alignment   signal(lever)   clear-TCs   [indication ceiling]
```

Example shape (names from a schematic-first Luchessa-style plant):

```text
MT-Hollister Branch   784SAB   (783)795(799)   784S(RIGHT)   783T1 795T1 799T1 1SA 2NAA 3NA   —
```

Conventions that scan well:

- Switch alignment: bare number = Normal; `(number)` = Reverse.
- Lever side: map mast geographic face to office Left/Right for that desk (plant-specific; document it).
- Clear list: derived switch OS circuits (`<switch>T1`) plus labeled path nets on the route.
- Indication ceiling: design-time maximum when the equation is fully satisfied and downstream permits; run time may be more restrictive.

Combinatoric checks over all switch plants are **internal proofs** that the enumerator found every valid structural route. Lists of impossible entry/exit pairs are not product tables.

---

## 4. Inside the plant: drawing rules (schematic-first)

These rules support a KiCad (or similar) plant schematic as the source of structural routes. They complement the bungalow primer; they do not replace vital relay behavior.

### 4.1 Appliances and pins

- **Switch:** distinct C, N, R track ports. Never electrically short C/N/R. Compiler builds conditional edges C–N and C–R from position.
- **IRJ:** A/B track ports; optional **SIGNAL** port on a Signal IRJ for mast attachment. A/B do not short. Trains cross the joint; track circuits do not.
- **Signal mast / heads:** mast SIGNAL attaches to Signal IRJ SIGNAL; heads attach only to mast head pins.
- **DoT marker:** on CP-local drawings, treat as a **limit terminal** (typically one live track pin, other intentionally not connected), carrying operating **designation** and **Rulebook** (251 / 261 / …). Not an interior series edge for ordinary CP design.
- **Bumper:** dead-end terminal.

### 4.2 Names

Two naming patterns often appear on one drawing:

| Pattern | Typical use | Examples |
| --- | --- | --- |
| **Milepost / system** | Mainline switches, signals, corridor identity | Switch `783`, signal `784`, block `780SA` |
| **Plant-local** | Internal nets and circuits | `1SA`, `2NA`, `2NAA` |

- Prefer MP-style names for **key mainline appliances** when the desk and corridor care.
- Plant-local names for internal circuits keep the sheet readable.
- A **legend** may map local names to MP names.
- Tools must keep **graph identity** as drawn (string IDs) and allow an **alias / display** layer later. Do not require every net to be MP-numbered for the parser to work.

Odd switch / even signal numbering (primer) remains the office and AAR habit; MP pairs preserve the same parity idea at corridor scale.

### 4.3 Track circuits on a route

A structural route’s clear list includes:

1. **Switch OS circuits** derived as `<SwitchName>T1` spanning that switch’s C/N/R path nets (not one KiCad net).
2. **Labeled path nets** actually traversed (approach, interior, exit labels as drawn).

Occupied clear-list circuits force Stop (or non-clear) in evaluation—primer `TR` front contacts on the HR path.

---

## 5. Between control points: ABS, tumble-down, and DoT placeholders

### 5.1 Single track between CPs

On many CTC territories (including SP Coast-style single track between control points):

- **Absolute signals** stand at CP limits.
- **Automatic Block Signals (ABS)** act as intermediates on the open track.
- When a dispatcher lines a route that claims the single track toward the next CP, **tumble-down** logic (APB-like) drops **opposing** intermediate signals to Stop between the CPs, while **following** moves still obey occupancy.

That is **territory evaluation**, not additional rows in one CP’s switch combinatorics.

### 5.2 DoT on a single-plant sheet

On a plant schematic, a DoT + named exit track at a CP limit is a **placeholder** for:

- operating designation and rulebook at that face, and
- the **next CP’s entry signal** (and the ABS chain toward it) in the subdivision model.

So:

- **CP-local static routes** may end at that DoT/net.
- **Subdivision binding** later resolves the placeholder to the neighbor entry face and corridor blocks.
- Indication at the exit face can then depend on corridor and neighbor state the same way an interior mast depends on the next mast (`DR` / advance logic).

Do not delete DoT ends from CP design because “the real end is Hollister.” Keep both scopes: local equation end, territory resolution.

---

## 6. Indication solve (evaluation sketch)

When several structural routes could apply to one face under the current plant:

1. Select routes whose **switch alignments** match verified correspondence.
2. For each such route, compute the indication allowed by clear list, opposing locks, policy ceiling, and **downstream face indication** (cascade).
3. Take the **most restrictive** result **on that route**.
4. Among surviving route results, the face shows the **least restrictive** indication (or Stop if none survive).

This matches bungalow HR/DR thinking and GCOR “signal indication is authority”: only a proceed indication that the plant actually supports may be displayed.

Fleeting, call-on, Track and Time, and reverse moves add explicit dispatcher products on top of this core.

---

## 7. Design checklist (creating a plant instance)

Use this when drawing or reviewing a control point for FieldUnit / Subdivision:

1. **Identify the interlocking plant** (junction of tracks and absolute signals), separate from CodeLine station count and panel columns (primer §9.5).
2. Place **switches** with C/N/R; derive OS names; never rely on list index as identity.
3. Place **IRJs** and **Signal IRJs**; attach masts only on SIGNAL ports; record face direction.
4. Enumerate **structural routes** per face: start face → next same-direction face or dead-end or CP-limit DoT.
5. Attach **clear TC lists** (OS + path labels).
6. Mark **DoT / designation / rulebook** at limits; treat them as neighbor placeholders for subdivision.
7. Keep **local vs MP names** honest; add legend if both appear.
8. Hand structural routes to FieldUnit-style evaluation (primer chains); do not bake live occupancy into the drawing.
9. For multi-CP territory, plan **ABS / tumble-down** on single track as subdivision behavior bound at DoT ends.

---

## 8. Worked scale examples

### 8.1 One plant (Luchessa-shaped)

- Main and branch limits as DoT terminals with designations and rulebook.
- Signal `784` faces (N/S masts and industry dwarf) own structural routes with MP switch names.
- Industry dwarf protects **exit from industry only**; inbound industry moves end at the stub, not at that dwarf as an “exit face.”
- Historical ordinal tables (`1/3/5`, signal `2`) are migration evidence; canonical design uses MP appliance names and schematic labels.

### 8.2 Chain of signals on one railroad

Structural: `2` ends at `4`; `4` ends at `6`.
Evaluation: `2`’s clear indication requires plant OK **and** acceptable state at `4` (and so on).
Same pattern as primer Home + Distant relays.

### 8.3 Two CPs and single track

Luchessa exit DoT toward Hollister ends the **Luchessa** structural route.
Subdivision binds that end to Hollister entry and intermediate ABS.
Lining the corridor claims tumble-down on opposing ABS between the CPs.

---

## 9. Relationship to FieldUnit runtime

| Artifact | Owner |
| --- | --- |
| Vital relay behavior, locking, knockdown, CodeLine tokens | FieldUnit (primer + code) |
| Plant drawing, structural route harvest, limit placeholders | Design tooling / Subdivision |
| Desk columns, 15-step cycles, shared CODE | cTc machine / panel model |
| Track and Time, fleets, call-on UX | Dispatcher products over the same plant |

FieldUnit remains the authority for **execution safety**.
Design documents and schematic harvest remain the authority for **what equations exist**.
Neither silently rewrites the other.

---

## 10. Summary

- **Primer** = inside the bungalow (microscopic plant vital logic).
- **This layer** = CTC authority frame (GCOR), structural route design, CP limits as placeholders, subdivision ABS/tumble-down, and naming discipline.
- **Write routes as static equations; solve indications at run time.**
- **End routes at the next same-direction protection or a dead end; cascade indications across faces and CPs.**
- **DoT markers on a plant sheet are operating limits and stubs for the next CP—not interior toys and not the whole territory graph.**

When the book form is written, this document is draft material for the “territory and design” chapters that follow the bungalow chapters.
