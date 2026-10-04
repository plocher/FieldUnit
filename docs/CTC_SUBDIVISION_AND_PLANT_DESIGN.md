# CTC, Subdivision and Interlocking Plant Design

This document is the layer outside
[Inside the Bungalow: An Introduction to AAR Signaling](AAR_SIGNALING_PRIMER.md).
It uses the terms of [GLOSSARY.md](GLOSSARY.md).

| Layer | Document | Scope |
| --- | --- | --- |
| Interlocking logic | `AAR_SIGNALING_PRIMER.md` | Relays, locking, code line, one interlocking plant |
| Interlocking model and subdivision | this document | Operating rules that the design assumes, structural routes, control point limits, drawing rules |
| Field unit | FieldUnit library and sketches | Solves the interlocking plant at run time. It does not make the interlocking model. |

The primer answers this question: what must be true in the interlocking plant before a signal can leave Stop?
This document answers this question: how do you draw and name an interlocking plant so that the tooling can find its routes, alone and in a subdivision?

"Implemented" means that code does it today. "Planned" means that no code does it today.
This document defines three terms of its own: structural route (§2.1), signal face (§3.1) and route end (§3.3).

---

## 1. Operating frame

### 1.1 Rulebooks and era

- SPCoast models the years 1942 to 1985. For SPCoast the reference rulebook is the Standard Code.
- A rule number belongs to one rulebook and one era. This document gives the book with each number.
- The rule texts in glossary section 9 come from one road: the PRR book of 1956, with revisions to 1964 [PRR56]. The SP rule texts were not checked.
- CTC rule numbers differ by code and year. A 1960 western consolidated code used 265 to 273. This document gives no CTC rule number for SPCoast.
- The GCOR is the modern equivalent. The 2010 edition uses chapter numbers, and its CTC rules are in chapter 10. It does not use the numbers 251 or 261.
- The rulebook and the special instructions govern operation. FieldUnit models the equipment that the rules assume. It does not replace the rules.

### 1.2 Concepts that the design needs

| Concept | Source | Use in the design |
| --- | --- | --- |
| Signal indication is the authority for movement | Standard Code 251 and 261 [PRR56]: block signal indications "supersede the superiority of trains". 251 applies to trains in the same direction. 261 applies to opposing and following movements. | A field unit shows a signal indication above Stop only when it proves a route. |
| Current of traffic | Standard Code D-151 [PRR56]: on two main tracks, trains keep to the right unless the timetable states otherwise. | A direction of traffic marker records the rule of each track at the edge of a drawing (§4.1). |
| Direction of traffic | Standard Code 262 [PRR56]: a train for which the direction of traffic is established must not move in the opposite direction without a proper signal indication or a train order. The rule does not say who establishes it. Design assumption: in CTC, the signal controls of the dispatcher establish it. | Planned (§5). |
| Exclusive occupancy | No rule text was checked. The dispatcher gives exclusive use of the track between two points. Signals must not clear into those limits. | Planned. |
| Reverse movements | No rule text was checked. | A structural route has one direction. A movement in the opposite direction is a different route. |

The dispatcher sends controls from the CTC machine. A control states intent only. The field unit decides whether to act on it. The office indication reports the state of the interlocking plant.

---

## 2. Design time and run time

### 2.1 Design time

Inputs: the track, appliances and markers on the KiCad schematic, and its labeled track nets.

Output: all structural routes of each signal face.

A structural route is one movement from one signal face to one route end. It has a switch alignment (the position of each switch and derail on the route), a clear list (the track circuits that must be clear), a route end and an indication ceiling.

A structural route does not contain the occupancy of the track. It states one condition: when the switches and derails are in position and in switch correspondence, the clear list is clear and the signal control has the direction of the route, the signal face can show up to the indication ceiling.

### 2.2 Run time

Inputs: switch position and switch correspondence, track circuit occupancy, the direction of each signal control, locks and timers.

Output: the signal indication of each mast now.

FieldUnit does this work (implemented). It uses the structural routes that the design tooling makes.

- A signal face with no structural route for one alignment shows Stop for that alignment only.
- An occupied track circuit, or a switch out of position, makes a route give Stop at run time. It does not remove the route.
- A route that ends inside the interlocking plant can have an indication ceiling above Stop. Example: the Luchessa routes into the industry (§8.1).

---

## 3. Signal faces and structural routes

### 3.1 Signal face

A signal face is one direction of one mast at one Signal IRJ. The compiler code name is `signal_faces`.

- The compiler makes one signal face for each mast on the SIGNAL pin of a Signal IRJ (implemented).
- The letter in the mast Value gives the direction: N and W give LEFT, S and E give RIGHT.
- The side of the Signal IRJ toward a switch is the interlocking side. The side toward a terminal is the approach side.
- A signal face in the opposite direction does not end a route. Example: the Luchessa dwarf `784EC` governs movements out of the industry. Routes into the industry pass it.
- The compiler gives one approach side to each Signal IRJ, so all masts on one Signal IRJ face the same way. Two signal faces in opposite directions at one Signal IRJ are planned.

### 3.2 Where a structural route starts

A structural route starts on the interlocking side of its Signal IRJ. The track circuit on the approach side is the entrance track circuit. FieldUnit uses it as the approach track circuit (`Route::approaching()`).

### 3.3 Where a structural route ends

A route end is the place where the compiler stops one route walk.

| Route end | Code name | Where it is |
| --- | --- | --- |
| Next signal face | `next_face` | The approach side of another signal face in the same direction. A new set of routes starts there. |
| Dead end | `dead_end` | A `Bumper` symbol. |
| Dark-track exit | `dark_exit` | The IRJ before a track net that a `Rule6.28-OtherThanMain` marker marks as dark track. |
| Edge of the drawing | `cp_limit` (code name; the glossary has no control point limits) | A `NextCP` symbol, or a `Direction_L`, `Direction_R` or `Direction_BOTH` terminal. |

- The code name `cp_limit` is older than the glossary. The terminal is not the interlocking limits. The interlocking limits are at the opposing controlled signal (§5.1). A route usually crosses that signal before it reaches the terminal.
- A route does not continue past the next signal face (§8.2).

### 3.4 Route table

`parse_kicad_plant.py` (text format) prints one line for each structural route. Two Luchessa lines, with some columns left out:

| Route | Mast | Alignment | Signal control | End | Entrance | Home-clear | Downstream | Indication ceiling |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| MT-Branch | `784EAB` | `783 795 (799)` | `784(RIGHT)` | `cp_limit` | `1SA` | `2NAA 783T1 795T1 799T1` | `3NA` | Approach |
| Branch-Industry | `784WD` | `(795)(799)` | `784(LEFT)` | `dark_exit` | `3NA` | `2NAA 2SA 795T1 799T1` | | Diverging Restricting |

- The route name is the entry terminal and the exit terminal.
- Alignment: a bare number is Normal. A number in parentheses is Reverse. The column shows switches only. The interlocking model (`plant-model-json`) also lists each derail with the position REVERSE. Example: Branch-Industry requires `795D` REVERSE.
- §4.3 explains the track circuit columns. §6.1 explains the indication ceiling.
- `build_route_proof` walks every alignment to check that the harvest found every route. It is a test aid, not a design table.

---

## 4. Drawing rules inside the interlocking plant

These rules let a KiCad schematic be the source of structural routes. They do not change the interlocking logic.

### 4.1 Appliances and pins

- Switch: track ports C, N and R. Do not connect C, N and R to one net. The compiler makes the C–N edge and the C–R edge from the switch position.
- Derail: track ports C and R. R is the through path. REVERSE is the clear position. NORMAL is the derailing position and has no track pin.
  - A route through a derail requires the derail in REVERSE (implemented).
  - A dependent derail `<switch>D` takes the position of its switch. A route with that switch in Reverse crosses the derail and requires it in REVERSE.
- IRJ: track ports A and B. A train crosses the joint. A track circuit does not.
- Signal IRJ: an IRJ with a third pin, SIGNAL, for masts. Heads attach only to the head pins of a mast. The head Value is one letter, A to E.
- Direction of traffic marker (`Rule251-DoT-Left`, `Rule251-DoT-Right`, `Rule261-DoT-BiDirectional`): one pin on a track net. The Value is the track designation, for example `MT1`. The fields `Rulebook` and `Direction` are required. The compiler copies the designation and the `Rulebook` value to the `NextCP` or `Bumper` terminal on the same net. The marker is not a route end.
- `NextCP`: a terminal at the edge of the drawing toward the next control point.
- `Direction_L`, `Direction_R`, `Direction_BOTH`: a terminal with exactly one live track pin. The other pin needs a no-connect flag. The Value and `Rulebook` are required. No SPCoast plant netlist uses these symbols today.
- `Rule6.28-OtherThanMain`: marks a track net as dark track.
- `Bumper`: a dead end.
- The symbol names and `Rulebook` values are code names. They mix Standard Code numbers (251, 261) with GCOR-style numbers (6.28; 6.13 in `Rule6.13-Yard Limits`).

### 4.2 Names

- The KiCad Value is the railroad name. A KiCad reference (`SW1`, `S7`) is not a name.
- SPCoast switch and signal numbers are the milepost in tenths: switches `783`, `795`, `799` and signal `784` at Luchessa. The SP Coast Division timetable of 1952 shows automatic signals numbered this way. That reading rests on three numbers.
- Odd switch numbers and even signal numbers are a common convention, not an AAR rule. AAR56 Fig. 6 numbers switch-type appliances with even numbers.
- AAR56 (p. 31) names a track circuit with a number and T. Inside interlocking limits the number is that of a frog, switch or derail in it. The project default for an OS track circuit is `<switch>T1`.
- Other track circuit names (`1SA`, `2NA`, `2NAA`) follow no project rule today. A grammar for all names is proposed in `FieldUnit-Subdivision/docs/adr/0003-name-grammar.md`. It is not adopted.
- The tooling keeps each name as drawn. `parse_kicad_plant.py --aliases` maps drawn names to display names (implemented).

### 4.3 Track circuits on a route

The compiler gives each track circuit on a route one role:

| Role | Meaning | On the clear list |
| --- | --- | --- |
| entrance | The track circuit on the approach side of the signal face. | no |
| home-clear | A track circuit on the route after the signal face. On a `cp_limit` route, only those before the first opposing signal that the route crosses. | yes |
| downstream | On a `cp_limit` route, a track circuit after that opposing signal. | no |
| unresolved | The exit track circuit of a `cp_limit` route that crosses no opposing signal. | no |

The clear list is:

1. the OS track circuit of each switch on the route: `<switch>T1`, or the name in the `TC` field of the switch;
2. the track circuit in the `TC` field of each derail on the route, when the field has a value;
3. the home-clear track circuits.

Known gap: the route harvest (`tools/plant_graph/routes.py`) always writes `<switch>T1` and ignores the `TC` field of the switch. The OS track circuit list of the model uses the `TC` field.

At run time the field unit shows Stop on a route when a track circuit on its clear list is occupied.

---

## 5. Between control points

### 5.1 Interlocking limits and control points

- Controlled signals make a place a control point. A switch does not.
- The designer declares each control point with a `MAIN HOUSE` symbol. The `CP` field of an appliance assigns it to a control point. An interlocking can contain several control points. Example: Luchessa has three.
- A spike (`FieldUnit-Subdivision/docs/archive/spike-cp-membership.md`) found the interlocking limits from the signals alone: cut the track graph at every Signal IRJ that carries a signal face; each piece that contains a switch or a derail is one interlocking.
- The compiler does not use this rule. It uses the opposing signal only to give the roles home-clear and downstream (§4.3).
- The Value of a `MAIN HOUSE` symbol is the control point name, such as `CP Luchessa`. On a 506-style code line it is also the field station name.

### 5.2 Track between control points (planned)

- The design assumes controlled signals at the interlocking limits and automatic block signals between interlockings.
- On single track, when the dispatcher clears a signal toward the next control point, the opposing automatic block signals between the two control points go to Stop. This document calls this tumble-down. Automatic permissive block (APB) signals behave the same way.
- Following movements still need clear track circuits.
- This is subdivision behavior. It adds no routes to one control point.

### 5.3 Route ends at the edge of a drawing

- A `cp_limit` route end stands for the entering signal of the next control point and the automatic block signals toward it.
- Today the route ends at the terminal. The terminal carries the designation and `Rulebook` value of the direction of traffic marker on its net.
- Planned: subdivision binding joins the terminal to the signal face of the next control point and to the blocks between. The signal indication at the exit can then depend on the next signal.
- Keep the terminal on the drawing. The local route end and the subdivision binding are both necessary.

---

## 6. Signal indication

### 6.1 Today (implemented)

FieldUnit (`InterlockingEngine::evaluateIndication` in `ControlTable.h`) gives each route of a mast the least favorable of:

1. the indication ceiling of the route;
2. Stop, unless the signal control has the direction of the route;
3. Stop, unless every switch of the route is in switch correspondence and in position;
4. Stop, unless every track circuit on the clear list is clear;
5. the ceiling reduced from Clear to Approach, when the approach track circuit is occupied.

Each mast shows the most favorable result of its routes. A mast with no result above Stop shows Stop.

The design tooling (`tools/plant_graph/indications.py`) compiles the indication ceiling as the least favorable of these values:

- each switch: Normal gives Clear and Reverse gives Approach, unless the `Indications` field of the switch (`NORMAL/REVERSE`) gives other values; a field that cannot be read gives Stop;
- a switch in Reverse that the route passes trailing (R to C): Approach;
- a `dark_exit` route: Restricting, or Diverging Restricting when the route passes a switch in Reverse facing.

### 6.2 Planned

The state of the next signal (§5.3), exclusive occupancy, call-on and permission for reverse movements. Fleeting (`SignalControl::FSR()`) and engine return (`Route::engineReturn`) are implemented in FieldUnit.

---

## 7. Design checklist

Use this list when you draw or review an interlocking plant.

1. Identify the interlocking plant. Do not derive it from the number of field stations or panel columns.
2. Place the switches (C, N, R) and derails (C, R). Give each a Value. Do not use a list position as a name.
3. Place the IRJs and Signal IRJs. Attach masts only to the SIGNAL pin.
4. End every track edge with a terminal: `NextCP`, `Bumper` or `Direction_*`. Put a direction of traffic marker on each `NextCP` net. Without it, the terminal has no `Rulebook` value. Mark dark track with `Rule6.28-OtherThanMain`.
5. Label the track nets. OS legs, dark track and derail nets need no label.
6. Compile the drawing. Check the routes, route ends, clear lists and indication ceilings of each signal face.
7. Do not put live occupancy in the drawing.
8. For each `cp_limit` route end, record what the subdivision must bind there (planned).

---

## 8. Examples

### 8.1 One interlocking plant: Luchessa

- The compiler finds five signal faces of signal 784 and ten structural routes.
- `MT` and `Branch` (Rulebook 261) and `MT1` and `MT2` (Rulebook 251) are `NextCP` terminals. `Industry` is a `Bumper` on dark track (Rule 6.28).
- The dwarf `784EC` governs movements out of the industry only. Routes into the industry end at the dark-track exit, with the ceiling Diverging Restricting.
- Every route with switch 795 in Reverse requires the dependent derail `795D` in REVERSE.
- The legacy profile `CP_Luchessa.json` uses switch numbers 1, 3, 5 and signal number 2. It is migration evidence. The generated interlocking model uses the schematic Values.

### 8.2 Signals in a row

- Signals 2, 4 and 6 govern one direction, in that order.
- The routes of 2 end at 4. The routes of 4 end at 6. No structural route goes from 2 to 6.
- Today the signal indication of 2 uses only its own route conditions (§6.1). A dependency on the state of 4 is planned. It is run-time evaluation, not a longer structural route.

### 8.3 Two control points on single track (planned)

The route from the first control point ends at its `cp_limit` terminal. The subdivision binds that terminal to the entering signal face of the second control point and the automatic block signals between (§5.2, §5.3).

---

## 9. Ownership

| Thing | Term | Owner | Status |
| --- | --- | --- | --- |
| Track, switches, derails, signals, track circuits | interlocking plant | the layout | |
| Locking, signal indication evaluation, knockdown | interlocking logic | FieldUnit, class `InterlockingPlant` | implemented |
| Drawing, route harvest, terminals, indication ceilings | interlocking model | design tooling (`FieldUnit-Subdivision/tools/plant_graph`) | implemented |
| Interlocking logic with one interlocking model | interlocking application | `spcoast_virtual_plant` builds it at start | generated sketch planned |
| A computer that runs interlocking applications | field processor | the deployment | simulator only |
| An interlocking application on a field processor | field unit | the deployment | virtual only |
| Levers, code buttons, lamps, panel columns | CTC machine | FieldUnit `cTcMachine`, desk schematic | implemented |
| Transport of controls and office indications | code line | FieldUnit `CodeLine` | implemented |
| Steps, code cycle, addresses, field stations, capacity, encoding | code line type | each code line type (US&S 506, MQTT) | ADR proposed (`FieldUnit-Subdivision/docs/adr/0001-code-line-type-contract.md`) |
| Binding of `cp_limit` route ends, direction of traffic, automatic block signals, tumble-down | subdivision | subdivision tooling | planned |
| Exclusive occupancy, call-on, reverse movements | dispatcher procedures | not assigned | planned |

- FieldUnit is the authority for what a field unit does at run time.
- The interlocking model is the authority for which structural routes exist.
- A change to the interlocking model reaches a field unit only through a new interlocking application.
