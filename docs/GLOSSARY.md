# Glossary

This file is the source of truth for terms in FieldUnit and FieldUnit-Subdivision.
It follows the decisions in `FieldUnit-Subdivision/docs/review/vocabulary-review.md` (rev 7, 2026-10-01).

## 1. How to read this glossary

- Each entry has one scope tag:
  - [Prototype]: true of the real railroad. It does not describe FieldUnit.
  - [FieldUnit]: a term of this project, in both repositories. It does not describe the prototype.
  - [Both]: the prototype and the project use the term with the same meaning.
  - [US&S 506 code line]: true of the US&S 506 code line type only.
- Text in `code font` is a name in code, a schema or a KiCad library. A code name keeps its spelling when the prose term differs.
- "(unverified)" marks a statement that no opened source supports.
- Terms are alphabetical inside each section. Each term is defined in one place.
- Retired terms and synonyms appear only in section 11.
- Source keys in brackets, for example [AAR56], are listed in section 12.

## 2. Model structure

### 2.1 Three roles and one seam [FieldUnit]

| Layer | Terms | Same for every code line type |
|---|---|---|
| Roles | CTC machine, code line, field unit | yes |
| Interface on the seam | control, office indication, as named functions | yes |
| Code line type | step, code cycle, address, field station, capacity, encoding, timing | no; each type defines its own |

- A layout picks one implementation for each role.
- A 506-era machine, a rebuilt machine with an ESP32, a screen display and a simulator are all CTC machines.
- The tooling accepts the code line type that the layout picks. It does not assume a 506 code line.

### 2.2 Facets of one place [Both]

These terms answer different questions about the same place.

| Facet | Question | Term |
|---|---|---|
| Function | What is done here? | control point |
| Mechanism | How is safety enforced? | interlocking |
| Extent | Where does it start and stop? | interlocking limits, control point limits, CTC limits |
| Addressing | How does the office reach it? | field station (per code line type) |
| Logic | What solves it? | field unit |
| Enclosure | Where is the equipment? | bungalow |

### 2.3 Tower interlocking and CTC control point [Both]

The rows marked ► carry the difference in behavior.

| Facet | Tower interlocking | CTC control point |
|---|---|---|
| Function | control of train movement by signals | the same |
| Who states the intent | tower operator, at the site | dispatcher, at a distance |
| Where the intent is stated | the levers of the interlocking machine | the levers of the CTC machine |
| ► What an unsafe lever does | It does not move. The locking bed holds it. | It moves freely. Nothing at the office holds it. |
| ► When the intent takes effect | when the lever moves | when the dispatcher presses CODE and the field unit accepts the control |
| ► How a refusal is known | at once, in the hand: the lever is locked | later, by absence: no office indication arrives that agrees with the lever |
| ► What a lever position means | the state of the interlocking plant | the intent of the dispatcher only |
| ► What tells the truth | the lever, then lamps and the window | the office indication only |
| What enforces safety | the locking of the machine (mechanical, later relays) | the field unit (relay or software logic) |
| Where safety is enforced | in the tower | in the field |
| Link to the appliances | pipe and wire, or direct wires | code line, then local wires |
| Addressing | none: one machine, one interlocking plant | field station (per code line type) |
| Extent | interlocking limits | control point limits inside CTC limits |
| Enclosure | tower | bungalow |

One-sentence form: the code line, the field station and the field unit together replace the locking bed; they do at a distance, and after the fact, what the locking bed does in the operator's hand.

Consequences:

- Control and office indication are two terms. A CTC lever shows intent only, so the office needs a second term for the state of the interlocking plant.
- The field unit refuses a control by not acting on it. It sends no refusal message.
- The CODE button has no tower equivalent. It marks the moment when the intent is sent.
- Requirement: an implementation of the CTC machine role must let the operator state an unsafe intent. The field unit refuses it.
- A CTC machine that blocks a lever from its own copy of the state of the interlocking plant moves the locking to the office. The office is not vital.
- A CTC machine does not send controls at power-up. A lever moved while the machine was off holds intent that nobody confirmed.
- Analogy, stated once: the CTC machine is to the field unit as the tower operator is to the interlocking machine.

### 2.4 Six terms from interlocking plant to field unit [FieldUnit]

| # | Thing | Term | Luchessa example | Exists today |
|---|---|---|---|---|
| 0 | The real track, switches and signals | interlocking plant | Luchessa: SP Coast Line, about MP 78.3 to 79.9 (switches 783, 795, 799; signal 784) | on the layout |
| 1 | The generic relay, AAR and AREMA logic | interlocking logic | C++ class `InterlockingPlant`; a rename to `Interlocking` is planned | yes, as `InterlockingPlant` |
| 2 | The data that describes one interlocking plant | interlocking model | `Luchessa.kicad_sch` compiled to `generated/Luchessa.json`, id `spcoast.Luchessa` | yes |
| 3 | The logic combined with one model | interlocking application | `Luchessa.ino` (generated sketch) or a native build | not as a file; `spcoast_virtual_plant` builds it at start |
| 4 | A computer that can run (3) | field processor | a cpNode or ESP32 in the Luchessa bungalow; the Mac that runs the simulator | simulator only |
| 5 | (3) running on (4) | field unit | the unit that answers at `ctc/SPCoast/codeline/Luchessa/...` | virtual only |
| 6 | An address at which (5) answers on a code line | field station | US&S 506 encoding: `Luchessa`, `Gilroy`, `Carnadero` (the `MAIN HOUSE` Values; drawn today as `CP Luchessa` and so on, to be fixed). AAR token encoding: `Luchessa`. | AAR tokens over MQTT only |

field unit = (interlocking logic + interlocking model) on a field processor = interlocking application on a field processor

- One name runs from the interlocking plant to the field unit. Only the term in front of it changes.
- Tooling goal: schematics give an interlocking model; the model gives an interlocking application; the application on a field processor gives a field unit.
- The office side has the same form. `cTcMachine` with the model of one CTC machine is a CTC machine application. That application on a processor is a CTC machine.
- The schema name `InterlockingPlantModel` stays. It is a model of an interlocking plant.

### 2.5 Cardinality [FieldUnit]

| Relation | Cardinality | Depends on |
|---|---|---|
| interlocking plant to control point | 1 : N | the drawing (one `MAIN HOUSE` symbol for each control point) |
| control point to field station | 1 : 1 on a 506-style code line; N : 1 on an AAR token code line | the code line type |
| interlocking plant to field unit | 1 : 1 in one deployment | nothing |
| field unit to field station | 1 : N | the encoding of the code line type (US&S 506: N can exceed 1; AAR tokens: N = 1) |
| field processor to field unit | 1 : N | the deployment only |

- One interlocking plant can have several field units at different times or places. Example: a unit in the bungalow and a virtual unit in a simulator.
- Luchessa answers at three field stations on a 506 line. It answers at one field station on an MQTT line.
- An interlocking that answers on two code lines is two interlocking applications, so two field units. A field processor runs one or more interlocking applications.
- A microcontroller in a bungalow is a field processor with one field unit. The simulator `spcoast_virtual_plant` is a field processor with seven.

## 3. Roles, organization and equipment

- **appliance** [Both]. A device in the field that the field unit operates or reads. Examples: switch, derail, signal, track circuit, electric lock.
- **bungalow** [Both]. A trackside enclosure for field equipment, of any size. It holds the relays or the field processor, batteries and wiring. Each control point has one. On a KiCad schematic the symbol `MAIN HOUSE` places it and declares its control point.
- **code line** [Both]. The communication path between the CTC machine and the field units. It carries controls to the field and office indications to the office.
  - FieldUnit: the class `CodeLine` is the transport (`MqttCodeLine`, `StreamCodeLine`, `MockCodeLine`). `AarTextCodec` writes functions as tokens.
  - History: one account of the first CTC installation (NYC, 1927, GRS) describes one wire to each switch plus a common return [EKEVING]. This is one secondary source.
  - History: a 1937 article describes a "Union time-code C.T.C. system" on a "single series circuit of two wires" [RS1937].
- **code line type** [FieldUnit]. The definition of how one kind of code line carries functions: addressing, capacity, encoding and timing. A code line type is one encoding on one transport (`FieldUnit-Subdivision/docs/adr/0001-code-line-type-contract.md`). Examples: AAR tokens over MQTT; US&S 506 over a relay line. The AAR token encoding has no capacity limit.
- **CTC machine** [Both]. The equipment at the office from which the dispatcher sends controls and reads office indications. It has levers, code buttons and lamps.
  - US&S wrote "C.T.C. control machine" in 1949 [USS1949].
  - FieldUnit: `cTcMachine` is a code name. The aliases `CtcMachine` and `OfficeUnit` exist. Prose uses "CTC machine".
  - The SPCoast machine is "the SPCoast CTC machine (US&S style)". It has no model number.
- **device interface** [FieldUnit]. The boundary between the field unit and the hardware of its appliances. Code names: `IOBus`, `CmriIOBus`, `MqttApplianceBus`.
- **dispatcher** [Both]. The operator role that controls trains at a distance. The dispatcher uses a CTC machine, a code line and the distributed handshake of control and office indication. This is the only operator role that the model covers today. Compare tower operator and maintainer.
- **field** [Both]. All places and equipment outside the office.
- **field processor** [FieldUnit]. The microcontroller, computer or process that runs one or more field units. It exists when it is installed or started.
- **field station** [FieldUnit]. One address on a code line, and the set of controls and office indications carried under that address.
  - A field station has no behavior. It is how one code line type lets the office reach a field unit.
  - A field station is the abstraction that the interlocking presents to the dispatcher. It carries the functions of the levers and lamps in its panel column. Routes, masts and aspects are not code line functions; the interlocking handles them in the field.
  - The code line type assigns field stations. On a 506 line a field unit can answer at several. On an MQTT line it answers at one.
  - On a 506 line each control point is one field station. Its name is the control point name: the Value of the KiCad `MAIN HOUSE` symbol. A name carries no `CP` prefix; a model board can add `CP ` for display. The tooling rejects a duplicate. It does not invent a name. A name used in an MQTT topic or a key has each space replaced by `-`. No code adds or strips a `CP` prefix.
  - An interlocking model with no `MAIN HOUSE` symbol is an error. The `MAIN HOUSE` symbol is the explicit source of the name. The tooling does not take it from the title block or the file name. The symbol can display a title block variable.
  - The address of a field station on a 506 line is authored. The tooling does not allocate it.
  - Prototype note: [RRS506] uses "field station" and "field location" for the places on a 506 line.
  - FieldUnit: the class `CtcStation` is the office-side record of one field station.
- **field unit** [FieldUnit]. An interlocking application running on a field processor. It reads the appliances, enforces the locking, drives the switches and signals, and answers the code line. `FieldUnit` is also the name of the C++ library.
- **interlocking application** [FieldUnit]. The interlocking logic combined with one interlocking model. Today it is not a file. `spcoast_virtual_plant` builds it at start.
  - The signalling industry uses "application" for logic configured with the data of one site. Example: the US&S "Microlok II System Application Logic Programming Guide" [ARTC23].
- **interlocking logic** [FieldUnit]. The generic logic of interlocking, in software. It is the same for every interlocking plant. Code name today: `InterlockingPlant`. A rename to `Interlocking` is planned.
- **interlocking model** [FieldUnit]. The data that describes one interlocking plant. Examples: the KiCad schematic, the FieldUnit JSON (`generated/<Interlocking>.json`) and the portable model (`InterlockingPlantModel`).
- **interlocking plant** [Both]. The real track, switches, derails, signals and track circuits that one interlocking controls. It can contain more than one control point. One field unit solves one interlocking plant.
- **maintainer** [Both]. The operator role that works at a location in maintenance or debug mode, without the interlocking. Compare dispatcher and tower operator. Not yet covered by the model. The maintainer call (section 5) calls the signal maintainer to a location.
- **office** [Both]. The place where the dispatcher and the CTC machine are.
- **panel column** [FieldUnit]. One vertical part of the CTC machine face. It holds levers, lamps and a code button. Code names: KiCad `PanelColumn`, C++ `PanelColumn`. A `CtcStation` can span up to four panel columns (`MAX_COLUMNS_PER_STATION`).
- **subdivision** [FieldUnit]. The set of control points, code lines and CTC machines that the subdivision linker joins into one model. Example: SPCoast South.

## 4. Control point, interlocking, CTC and limits

- **control point** [Both]. A named place where a dispatcher or operator controls train movements by controlled absolute signals.
  - The term names a function. Controlled signals make a place a control point. A switch does not. A control point can have no switch.
  - An interlocking contains one or more control points.
  - FieldUnit: the designer declares each control point. One KiCad `MAIN HOUSE` symbol declares one control point, and its Value is the name (`CP Luchessa`). The `CP` field of an appliance assigns it to a control point. The tooling does not derive the number of control points.
  - Example: Luchessa is one interlocking with three control points: `CP Luchessa`, `CP Gilroy`, `CP Carnadero`.
  - GCOR: "Control Point: The location of absolute signals controlled by a control operator." [GCOR6]
  - NORAC uses "Controlled Point (CP)": "A station designated in the Timetable where signals are remotely controlled from the control station." [NORAC9]
  - 49 CFR 236.782 defines "controlled point" as a location where signals or other functions of a traffic control system are controlled from the control machine. This is a paraphrase [CFR236].
  - Example name: `CP Luchessa`.
  - A control point has one bungalow. On a 506-style code line it has one field station, with the same name.
- **control point limits** [Both]. The track and appliances that belong to one control point. In an interlocking with one control point they are the interlocking limits. In an interlocking with several control points the `CP` field draws them. (unverified: no rulebook definition was opened.)
- **CTC** (centralized traffic control) [Both]. A traffic control system in which a dispatcher controls the signals and switches of control points from a distance.
  - CTC adds a code line, field stations, controls, office indications and a remote dispatcher to the interlocking.
  - 49 CFR 236 uses the term "traffic control system" [CFR236].
  - Some sources write "cTc" (Wikipedia for the 1927 GRS machine; rrsignal for US&S machines). A trademark record was not found (unverified). US&S wrote "C.T.C." [RS1937, USS1949].
- **CTC limits** [Both]. The part of the railroad where the CTC rules apply. (unverified: the rule text was not opened.)
- **interlocking** [Both]. An arrangement that forces switch and signal movements to follow each other in a safe sequence.
  - History: first a mechanical machine (levers, locking bed, pipe and wire); later relays.
  - Interlocking is the mechanism facet. A CTC control point uses the same kind of field logic.
- **interlocking limits** [Both]. The track between the opposing home signals of an interlocking. The tooling can derive them: cut the track graph at every controlled signal; each piece that contains a switch or derail is one interlocking (`FieldUnit-Subdivision/docs/review/spike-cp-membership.md`).
- **interlocking machine** [Prototype]. The lever frame and its locking that a tower operator works. The locking bed is the part that holds a lever that must not move.
- **tower** [Prototype]. The building at an interlocking from which the tower operator works the interlocking machine.
- **tower operator** [Both]. The operator role that works directly on a locking bed, real or virtual. On the prototype this is the person who works an interlocking machine in a tower. The tower operator does not use a code line. Compare dispatcher and maintainer. Not yet covered by the model.
- **yard limits** [Prototype]. A part of main track, designated by the railroad, inside which the yard limit rule applies. The KiCad symbol is `Rule6.13-Yard Limits`. (unverified: the rule text was not opened.)

## 5. Controls, indications and functions

- **code button** [Both]. The push button that sends the controls of one field station. The label on the machine is CODE.
  - FieldUnit: `PanelInput::CODE_BUTTON`. `cTcMachine` sends when the button is released (`OneShot`).
- **control** [Both]. A function sent from the office to the field. It states the intent of the dispatcher. The field unit decides whether to act on it. Examples: `783NWS`, `784NGS`.
- **control transaction** [FieldUnit]. The complete set of controls for one field unit, applied as one unit. Code name: `ControlTransaction`. Two rules govern it (FieldUnit `docs/adr/0002-control-transaction-classes.md`):
  - A malformed transaction is ignored as a whole. This includes the controls of both classes. The field unit updates its error counters and sends office indications of its current state.
  - A valid transaction that would violate a safety protection has every vital control ignored together. The field unit does not act on the safe ones and skip the unsafe one. It processes every non-vital control.
- **function** [FieldUnit]. One named control or office indication. Its written form is a token. Example: `783NWS`.
- **indication vector** [FieldUnit]. The complete set of office indications from one field unit, sent as one unit. Code name: `IndicationVector`.
- **lever** [Both]. A handle on a CTC machine or an interlocking machine. It states one intent for one switch or signal.
  - AAR56 names the positions of a three-position lever L and R, as in `10L` and `10R` [AAR56 p. 34]. The middle position is N, the normal position [AAR56 Figs. 18, 22].
  - A switch lever has the positions N and R. A signal lever has the positions L, N and R.
  - FieldUnit: `PanelInput::SW_NORMAL`, `SW_REVERSE`, `SIG_LEFT`, `SIG_STOP`, `SIG_RIGHT`. A switch lever with neither contact closed sends no switch control.
- **maintainer call** [Both]. A control and lamp that call the signal maintainer to a location. A 1959 machine had a "maintainer's call" control and a "maintainers' call lamp" [RS1959]. No source says that train crews used it.
  - FieldUnit: tokens `MC<n>S` and `MC<n>K`. It is a non-vital control. The prefix `MC` is a FieldUnit name.
- **non-vital control** [FieldUnit]. A control that cannot affect a safety protection. The interlocking logic does not check it against the locking.
  - Processing: when a transaction is malformed, the field unit ignores it with the rest of the transaction. When a valid transaction is unsafe, the field unit still processes every non-vital control.
  - Examples: maintainer call `MC<n>S`.
  - The code line that carries a non-vital control is not vital. The class describes how the field unit processes the control.
- **office correspondence** [Both]. Agreement between a lever and the office indication of its appliance.
  - Out of office correspondence: the lever and the office indication disagree. This is how the office knows that the field unit did not act.
  - A tower lever has no such state.
- **office indication** [Both]. A function sent from the field to the office. It reports the state of the interlocking plant. It is the only source of truth at the office. Examples: `783NWK`, `784NGK`.
- **power-off indication** [Prototype]. A lamp that shows loss of power at a location [RS1959].
- **token** [FieldUnit]. The written name of one function on an AAR text code line. Example: `783NWS`. A token is not a step (see section 6).
- **track indication lamp** [Both]. A lamp on the CTC machine that shows the condition of a track circuit.
  - AAR56: `TK`, "indicator, indicating condition of a track circuit" [AAR56 p. 34].
  - FieldUnit: `PanelColumn::withTrackLamps()`, `PanelOutput::TRACK_LAMP_1` to `TRACK_LAMP_6`.
- **vital** [Both]. Two senses; the context tells which.
  - (a) A property of a circuit or logic function whose failure must leave the interlocking plant in a safe, restrictive state. Vital logic is in the field. The code line and the CTC machine are not vital.
  - (b) The class of a control that can affect a safety protection (see vital control). The class describes how the field unit processes the control. It does not make the code line vital. Code: `vitalValid`; the `Vital` field on panel symbols.
- **vital control** [FieldUnit]. A control that can affect a safety protection. The interlocking logic checks it before the field unit acts.
  - Processing: a malformed transaction is ignored as a whole. When a valid transaction would violate a safety protection, the field unit ignores every vital control in it together, including the safe ones, and sends no refusal. The office learns the result from the office indications.
  - Examples: `NWS`, `RWS`, `NGS`, `SGS`, `HS`, `WLS`.
  - The code line that carries a vital control is not vital. Vital logic is in the field.

## 6. US&S 506 code line

Facts [US&S 506 code line], from [RRS506]:

- One code has 16 steps: 1 conditioning step, 7 station selection steps, 7 function steps, 1 delivery step.
- A field station on 506 or 506A carries 7 controls and 7 office indications.
- One 506 code line serves up to 35 field stations.
- 506C carries 7 controls and 35 office indications for each field station.
- rrsignal also describes a 514, similar to the 506, with 35 field locations [RRS514, paraphrase].

Entries:

- **code chart** [US&S 506 code line]. For one field station: its name, its address, and the ordered list of steps with the encoding on each step. It is data for each field station. It is not a constant for each appliance type.
- **code cycle** [US&S 506 code line]. One transmission of the 16 steps to or from one field station.
- **conditioning step** [US&S 506 code line]. The first step of a code.
- **delivery step** [US&S 506 code line]. The last step of a code.
- **function step** [US&S 506 code line]. One of the 7 steps that carry controls or office indications.
- **station selection step** [US&S 506 code line]. One of the 7 steps that select the field station.
- **step** [US&S 506 code line]. One time slot of a code.
- **US&S 500-series time code systems** [Prototype]. The family of US&S time code systems. Sources confirm 506 and 514 only.
- **US&S 506 time code** [US&S 506 code line]. The code line type of the SPCoast CTC machine.

A token is not a step. FieldUnit sends `NWS` and `RWS` for one switch. A 506 line can carry the same intent in other ways. The code chart maps tokens to steps. The capacity check counts steps, not tokens or symbol pins.

Illustrative step assignment for office indications. Source: JMRI developers list, supplied by the owner (hearsay). It is not checked against a primary source.

| Step | Function |
|---|---|
| 1 | line check and lockout |
| 2 to 8 | station selection |
| 9, 11 | switch Normal, Reverse; both short = not in switch correspondence |
| 10 | OS track |
| 12 | approach track |
| 13, 15 | signal Left, Right; both short = Stop; both long = time release running |
| 14 | commercial power |
| 16 | delivery |

- This list has two signal steps. FieldUnit has three signal tokens (`NGK`, `SGK`, `TEK`). `TEK` is the "both long" value.
- This list has room for one OS track and one approach track.
- Illustrative control assignment, from the owner: two steps for a switch (Normal, Reverse); neither asserted means "do not move". A second summary gives one step (long = Reverse, short = Normal). Neither is confirmed.

## 7. Track and appliances

- **approach track circuit** [Both]. A track circuit in approach of a signal: a train passes it before it reaches the signal. Code name: `Route::approaching()`.
- **block** [Both]. A length of track between consecutive signals that govern movement into it. A block can contain several track circuits. (unverified: no rulebook definition was opened.)
- **crossover** [Both]. Two switches that connect two tracks and work together from one lever.
  - AAR56 names functions of one lever with A, B, C after the lever number, as in `10A`, `10B` [AAR56 p. 34].
  - FieldUnit: `addCrossover(name, swA, swB)`. Both ends move and lock together in the same position. The suffix `D` must not name a crossover end.
- **derail** [Both]. A switch-shaped appliance that derails a car before it fouls another track.
  - FieldUnit rule: NORMAL is the derailing position. REVERSE is the clear position. A derail rests in NORMAL.
  - Prototype convention (recall). No source was found for power-operated derails.
  - 49 CFR 218.109 makes the derailing position the normal position of fixed derails, with exceptions. This is a paraphrase [CFR218].
  - GCOR 8.20: "Sidings having hand-thrown derails will have derail locked in non-derailing position, except when engines or cars are left unattended on siding." [GCOR6]
  - A route through a derail requires REVERSE.
- **dependent derail** [FieldUnit]. A derail that moves with its switch, in the same position. Switch Normal gives derail Normal. Switch Reverse gives derail Reverse.
  - Name: `<switch>D`, for example `795D`. `addDerail("795D")` requires switch `795` to exist.
  - It has no lever and no tokens of its own. The switch's `NWK` and `RWK` require both machines in switch correspondence.
- **electric lock** [Both]. A lock on a hand-operated switch. The dispatcher releases it by a control.
  - AAR56: `WL`, switch lock [AAR56 p. 35].
  - FieldUnit: tokens `WLS` (release) and `WLK` (released). `SwitchLock::HAND_LOCKED`. The field unit releases the lock only when every signal is at stop and no time locking runs.
  - An electric lock is not a dual-control switch.
- **fouling point** [Prototype]. The point beyond which a car on one track can be struck by a movement on another track.
- **head** [Both]. One unit of lamps on a mast. It shows one part of an aspect. KiCad head Value: one letter, A to E.
- **independent derail** [FieldUnit]. A derail with its own lever and its own tokens. Example: Corporal derail `5` in the legacy profile.
- **LEFT, RIGHT** [FieldUnit]. The two directions of a signal lever and of a route. Code names: `DirectionAuthority::LEFT`, `DirectionAuthority::RIGHT`. Today the KiCad compiler (`tools/plant_graph`) maps mast letters N and W to LEFT, and S and E to RIGHT.
- **mast** [Both]. The structure that carries one or more heads. KiCad mast Value today: `^\d+[NSEW][A-E]+$`, for example `784EAB`.
- **OS section** [Both]. The track between opposing signals in a control point. It is usually covered by the track circuit over the switches.
  - "OS" means "on sheet": the dispatcher's record of a train that passes a location. The BNSF source gives both meanings (paraphrase) [BNSF].
  - FieldUnit: the OS track circuit detector-locks its switch. Declare it with `addSwitch(id, osName)` or `addDerail(id, osName)`.
- **signal** [Both]. An appliance that shows an aspect to govern a train movement. One signal can use heads on one or more masts.
  - AAR56 names functions of one lever position with A, B, C. Fig. 26 uses them for separate signals and for two arms on one mast [AAR56 pp. 33–34].
  - FieldUnit separates the control of a signal (`SignalControl`, one for each signal lever) from its display (`SignalMast`).
- **switch** [Both]. An appliance with movable points that routes a train from one track to another. AAR56 uses the letter W for switch [AAR56 p. 32]. Code name: `Switch`.
- **switch correspondence** [Both]. Agreement between the position of a switch and its control.
  - Out of switch correspondence: the switch is moving, or it did not reach the position of its control.
  - FieldUnit: `Switch::inCorrespondence()` is true when the reported position equals the position of its control (`commandedPosition()`) and is NORMAL or REVERSE. After the travel timeout (default 5000 ms) a moving switch reports `SwitchPosition::OUT_OF_CORRESPONDENCE`.
- **track circuit** [Both]. An electrical circuit in the rails that detects a train. The wheels and axles shunt the rails.
  - AAR56: "A track circuit is designated by the letter T preceded by a number" [AAR56 p. 31].
  - FieldUnit: `TrackCircuit`. Occupancy can come from any detector. A dropout delay (`dropoutDelayMs`) bridges gaps between cars for optical sensors. This is a model workaround.
  - FieldUnit: a track circuit starts OCCUPIED. A remote circuit with no update for 5000 ms gets quality `LOST_COMMS` and is not clear.

## 8. Signal indications and locking

- **approach locking** [Both]. Locking that holds the route switches after a signal clears, while a train can be approaching. The passage of the train or a timer releases it.
  - FieldUnit: when a cleared signal is cancelled and its approach track circuit is occupied, a timer runs. The route switches get `SwitchLock::TIME_LOCKED`. `ASR()` is false and `TEK` is asserted.
  - FieldUnit: if the approach track circuit is vacant, the field unit releases the switches at once.
  - FieldUnit: if the route declares no approach track circuit (`Route::approaching()`), the field unit also releases the switches at once.
  - FieldUnit: the default time is 30 s (`SignalControl::setTimeLockDuration`).
- **aspect** [Both]. The appearance of a signal: the colors, positions and flashing of all its heads together. A head alone does not show an aspect.
  - FieldUnit: the enum `Aspect` holds one-head values (`RED`) and two-head values (`RED_OVER_YELLOW`). `SignalMast::compositeAspect()` is the aspect. `head1()` to `head3()` are head appearances.
- **detector locking** [Both]. Locking that holds a switch while its OS track circuit is occupied. FieldUnit: `bindDetectorLock()`, `SwitchLock::DETECTOR_LOCKED`.
- **engine return** [FieldUnit]. A route mode that shows RESTRICTING for an engine that returns to cars it left standing. The design principle is the `ERS` circuit (section 10.6).
  - The mast shows RESTRICTING when the route switches are in position, the OS track circuit is clear, and the standing-cars track circuit is occupied.
  - Code: `Route::engineReturn(standingCars, os)`. FieldUnit has no relay object for it.
  - Prototype: the AAR list has `TSR`, track stick relay [AARLIST]. Railway Signaling names directional stick relays [RS1927, RS1944]. "Engine return stick" is a search lead only (unverified).
- **fleeting** [Both]. A mode in which a signal clears again for a following train without a new control. A 1959 machine had a fleeting control [RS1959]. FieldUnit: `ControlTransaction::fleetDemands`, `SignalControl::FSR()`.
- **indication ceiling** [FieldUnit]. The most favorable signal indication that a route permits. Code name: the second argument of `Route::displays()`, read by `Route::aspectCeiling()`.
- **route locking** [Both]. Locking that holds the switches of a route while the signal is clear or a train is on the route. Each switch is released when its releasing track circuit clears (sectional release). FieldUnit: `SwitchLock::ROUTE_LOCKED`, `SectionState`.
- **signal indication** [Both]. The meaning of an aspect, as the rulebook states it. Examples: Stop, Restricting, Clear. Code name: enum `Indication`.
- **time locking** [Both]. Locking that holds the route switches for a set time after a signal is restored to stop. FieldUnit uses one timer for approach locking and time locking (see approach locking).

## 9. Operating rules [Prototype]

- A rule number belongs to one rulebook and one era. Give the book with the number.
- SPCoast models 1942 to 1985 (title block). Use Standard Code numbers for SPCoast. Name GCOR only as the modern equivalent.
- The texts below are from one road: the PRR book effective October 28, 1956, with 1964 revisions [PRR56]. Another road's wording can differ.

| Rule | Text [PRR56] |
|---|---|
| 251 | "On portions of the railroad and on designated tracks so specified on the time-table, trains will run with reference to other trains in the same direction by block signals whose indications will supersede the superiority of trains." |
| 261 | "On portions of the railroad and on designated tracks so specified on the time-table, trains will be governed by block signals whose indications will supersede the superiority of trains for both opposing and following movements on the same track." |
| 262 | "A train for which the direction of traffic has been established must not move in the opposite direction without proper interlocking or manual block signal indication or train order." |
| D-151 | "Where two main tracks are in service, trains must keep to the right unless otherwise provided on the time-table." |
| D-152 | "When a train or engine crosses over to or obstructs a track where block signal system rules are in effect, the movement must be protected by the operator as provided by Rules 327 or 504, except where 605 is in effect. (Rev. 10-18-64)" The heading in the source reads "152." |

- SP numbering: an SP 1960 excerpt shows D-251 and D-254. The SP and SPCoast rule texts were not opened.
- CTC rule numbers vary by code and year. A 1960 consolidated western code used 265 to 273 [RS1960]. A 1962 Canadian code used 263 and 264.
- GCOR 6th edition (2010) uses chapter numbers such as 1.1 and 8.20. It does not use 251 or 261 [GCOR6]. The first edition (1985) was not opened (unverified).
- NORAC 9th edition (2008) still has Rule 251 [NORAC9].

## 10. Names and relay nomenclature

### 10.1 Structure of a name [Prototype]

- An AAR name has a number prefix and letters. The number is that of the principal lever, signal, track circuit or other device [AAR56 p. 31].
- The last letter gives the general kind of unit. The letters before it describe the unit [AAR56 p. 31]. Example: `10HR` is signal 10, home function, relay.
- One letter has several meanings. Its position decides the meaning [AAR56 p. 31].
- A track circuit is a number plus T. Inside interlocking limits it takes the number of a frog, switch or derail in it [AAR56 p. 31].
- AAR56 Fig. 6 numbers switch-type appliances with even numbers. AAR56 does not reserve odd numbers for switches [AAR56 p. 9].

### 10.2 Project names today [FieldUnit]

- The KiCad Value is the railroad name. A KiCad reference (`SW1`, `S7`) is not a name.
- Odd switch levers and even signal levers are a common convention that goes back to lever-and-pipe interlocking plants (owner's knowledge; unverified). They are not an AAR rule.
- A dependent derail is `<switch>D`.
- The default OS track circuit name is `<switch>T1`. A `TC` field overrides it. AAR56 uses `<number>T`.
- Names are produced with case preserved and compared without case.
- One grammar for all names is proposed in `FieldUnit-Subdivision/docs/adr/0003-name-grammar.md`. It is not adopted.

### 10.3 Letters

AAR letters [AAR56 p. 32]. A name uses one meaning of each letter.

| Letter | Meanings |
|---|---|
| A | approach |
| D | proceed indication, detector, decoding, dragging |
| E | east |
| G | signal (operating mechanism) |
| H | home, approach indication |
| K | indicator |
| L | left, lever, lock |
| N | normal, north |
| P | repeater [AARLIST] |
| R | right, reverse, relay, stop indication |
| S | south, stick, storage |
| T | track, time |
| W | switch, west |
| Z | special |

FieldUnit token letters. The rows marked "FieldUnit" are FieldUnit dialect, not AAR usage.

| Token part | Meaning in FieldUnit | Origin |
|---|---|---|
| final `S` | control | FieldUnit. AAR56 reads S as south, stick or storage. |
| final `K` | office indication | AAR56: K is indicator. |
| `NW`, `RW` | switch normal, reverse | AAR56 (`NWK`, `RWK` are switch indicators, p. 35) |
| `N`, `S` in `NGS`, `SGS`, `NGK`, `SGK` | LEFT, RIGHT | FieldUnit. AAR56 reads N as normal or north, S as south or stick. |
| `HS` | control: put the signal at stop | FieldUnit. AAR56: HS is the positive control of the home stick relay (`HSR`) [AAR56 p. 37]. |
| `TE` in `TEK` | time locking runs | FieldUnit. TE is not on the AAR list. |
| `WL` in `WLS`, `WLK` | electric lock | AAR56: WL is switch lock (p. 35). |
| `MC` | maintainer call | FieldUnit. No source. |

FieldUnit tokens by appliance:

| Appliance | Controls | Office indications |
|---|---|---|
| switch | `NWS`, `RWS` | `NWK`, `RWK` (asserted only in switch correspondence) |
| signal | `NGS` (LEFT), `SGS` (RIGHT), `HS` (stop) | `NGK` (LEFT), `SGK` (RIGHT), `TEK` (time locking runs) |
| track circuit | none | `<name>K` (asserted = occupied) |
| electric lock | `WLS` (release) | `WLK` (released) |
| maintainer call | `MC<n>S` | `MC<n>K` |

On the AAR text code line, an asserted token is written `TOKEN`. A dropped token is written `(TOKEN)`.

### 10.4 Contacts and circuits [Both]

- **back contact**. Closed when the relay is de-energized (dropped). Software: `!x`. Sketch symbol: `[/]`.
- **front contact**. Closed when the relay is energized (picked up). Software: `x`. Sketch symbol: `[  ]`.
- **parallel contacts**. Paths that branch around each other. Software: `||`.
- **series contacts**. Contacts in one path. Software: `&&`.
- **stick circuit**. A circuit in which a relay's own front contact holds its coil energized after the pickup path opens.

The sketches below are illustrative. They are not prototype circuit plans.

### 10.5 Relays

- **`ASR`** [Both]. Approach stick relay. It enforces approach locking.
  - FieldUnit: `SignalControl::ASR()` is true when no time locking runs. It is false only while the timer runs.
  - FieldUnit difference: `ASR()` does not drop when a signal clears. Route locking holds the switches while the signal is clear.

```cpp
bool plantFree = sig.ASR(); // false while the time-locking timer runs
```

- **`ESR`, `WSR`** [Prototype]. East and west stick relays; likewise north and south [AAR56 p. 34]. They are directional stick relays [RS1927, RS1944]. FieldUnit has no object for them.
- **`FSR`** [FieldUnit]. Fleet stick relay. FieldUnit name. `SignalControl::FSR()` is true while fleeting is on. The signal then clears again when the route is clear, without a new control.
- **`HR`, `DR`** [Both]. Relay names with H (home, approach indication) and D (proceed indication). D does not mean "distant" on the AAR list. FieldUnit has no `HR` or `DR` object; section 10.6 gives the circuits that its logic follows.

```cpp
// InterlockingEngine::evaluateIndication (ControlTable.h), summarized.
// The signal indication is the least favorable of:
//   the indication ceiling of the route,
//   STOP unless the signal control has the direction of this route,
//   STOP unless every route switch is in switch correspondence and in position,
//   STOP unless every route track circuit is clear,
//   the ceiling reduced (CLEAR to APPROACH) when the approaching() circuit is occupied.
```

- **`HSR`** [Both]. Home stick relay [AAR56 p. 37].
  - FieldUnit: `SignalControl::HSR()` is true while a direction (LEFT or RIGHT) is active.
  - It picks up when the field unit accepts a LEFT or RIGHT control.
  - It drops when a train occupies the entrance track circuit of the route (`knockdown()`).
  - It stays dropped until a new control arrives, unless fleeting is on.

```
                     1TR front
  signal control ───[  ]───────┬───────────────( 1HSR )
                               │
         1HSR front            │
     ┌──────[  ]──────┐        │
     │                ├────────┘
     │   FSR front    │
     └──────[  ]──────┘
```

```cpp
bool directionActive = sig.HSR(); // true while LEFT or RIGHT is active
bool fleeting        = sig.FSR(); // true while fleeting is on
```

- **`KR`** [FieldUnit]. FieldUnit name. `Switch::KR()` is true when the switch is in switch correspondence, in either position.
- **`NWCR`, `RWCR`** [FieldUnit]. FieldUnit names. `NWCR()` is true when the switch reports NORMAL and is in switch correspondence. `RWCR()` is the same for REVERSE. The AAR56 names for the switch indicators are `NWK` and `RWK`.

```
  ──[ 1NW control ]──[ points detected Normal  ]──( 1NWCR )
  ──[ 1RW control ]──[ points detected Reverse ]──( 1RWCR )

  ──┬──[ 1NWCR front ]──┬──( 1KR )
    └──[ 1RWCR front ]──┘
```

```cpp
bool normalInCorrespondence  = sw.NWCR();
bool reverseInCorrespondence = sw.RWCR();
bool inCorrespondence        = sw.KR();
```

- **`TP`, `TPR`** [Prototype]. Track repeater relay. It repeats a track relay to give more contacts. FieldUnit has no object for it.
- **`TR`** [Both]. Track relay. It is energized (picked up) when the rails are not shunted. It drops when wheels and axles shunt the rails. A broken rail or a power loss also drops it.

```
          rail
   +-------------------------------+
   |                               |
[battery]                      [TR coil]   picked up: track vacant
   |                               |
   +-------------------------------+
          rail

          rail
   +─────────[wheels]──────────────+
   |             |                 |
[battery]     (shunt)          [TR coil]   dropped: track occupied
   |             |                 |
   +─────────────┴─────────────────+
          rail
```

```cpp
bool vacant = tc.TR(); // true when VACANT and quality GOOD
```

- **`WLR`** [FieldUnit]. FieldUnit name, from the AAR letters W (switch), L (lock), R (relay). `Switch::WLR()` is true when the switch has no active lock: detector, route, time or electric lock. A switch moves only when `WLR()` is true.

```
  ──[ 1TR front ]──[ no route lock ]──[ no time lock ]──( 1WLR )
```

```cpp
bool canThrow = sw.WLR(); // true when no lock is active
```

### 10.6 Relay model of the interlocking logic (design principle) [FieldUnit]

FieldUnit's interlocking logic is designed from relay circuits. The circuits below state the principle.
The code is correct when it behaves as the circuit does. A difference between the code and a circuit
is a defect in the code. It is not a reason to change the circuit. Known differences are listed with each circuit.

- **`HR`** (home relay). It picks up when the signal control has the direction, every route switch is in switch correspondence, every route track circuit is clear, and the opposing signals are held at stop. With `HR` up the signal can show at least Approach.
- **`DR`**. It picks up when `HR` is up, the block beyond is clear and the next signal is not at stop. With `DR` up the signal shows Clear. (The AAR list reads D as "proceed indication", not "distant".)

```
  HR:  ──[ 2HSR front ]──[ route KR fronts ]──[ route TR fronts ]──[ opposing ASR fronts ]──( 2HR )

  DR:  ──[ 2HR front ]──[ advance TR front ]──[ next signal HR front ]──( 2DR )
```

  - Code today: `InterlockingEngine::evaluateIndication` has the `HR` conditions. It has no `DR`: it does not read the next signal, and it lowers Clear to Approach from the `approaching()` track circuit.

- **`ASR`** (approach stick relay). It is up while the signal is at stop and no approach locking is in effect. It drops when the signal clears. It picks up again when the signal is at stop and either the approach track circuit is clear, or the train has accepted the signal, or the time-locking timer has run.

```
  ──[ signal at stop ]──┬──[ approach TR front ]────────┬──( 2ASR )
                        ├──[ train accepted the signal ]┤
                        └──[ time element run          ]┘
```

  - Code today: `SignalControl::ASR()` is false only while the timer runs. It does not drop when the signal clears. A route with no `approaching()` track circuit gets no time locking.

- **`ERS`** (engine return stick; project name). It picks up when a forward route is active and the engine occupies the OS track circuit and then the exit track. It holds through its own front contact while the cars stand on the exit track. With `ERS` up the return signal shows Restricting without the time-locking wait. It drops when the exit track clears.

```
  pickup: ──[ route active ]──[ OS TR back ]──[ exit TR back ]──┐
                                                                 ├──( 2ERS )
  hold:   ──[ 2ERS front ]───────────────────[ exit TR back ]───┘
```

  - Code today: `Route::engineReturn(standingCars, os)` has no stick. It shows Restricting when the route switches are in position, the OS track circuit is clear and the standing-cars track circuit is occupied. It does not check the signal control or time locking.

## 11. Retired and synonym terms

Use the term in the right column. Code names in `code font` elsewhere in this file are not retired.

| Term | Use instead |
|---|---|
| 15-step (US&S code line) | 16-step (US&S 506) |
| 20-step, 32-step | no replacement; not sourced |
| Approach Block | approach track circuit |
| auxiliary function | non-vital control |
| Aspect Ceiling | indication ceiling |
| bit (for a code line function) | function |
| central instrument location, CIL | no replacement; not sourced |
| Centralized Hosts | field processor |
| code starting push button | code button |
| code station | field station |
| CodeLine (in prose) | code line |
| command (for a control) | control |
| console | CTC machine |
| control machine | CTC machine |
| Control Point Engine | interlocking logic |
| control snapshot | control transaction |
| Controlled Point, controlled point | control point (a place) or field station (an address) |
| controller (as a role) | dispatcher, tower operator or maintainer. The names `controllers{}`, `tools/controller_graph` and `parse_kicad_controller.py` are code names and keep their spelling. |
| Controls-as-Demands | control |
| corridor | subdivision |
| correspondence (alone) | switch correspondence or office correspondence |
| CP (as a common noun) | control point |
| CP Boundary | control point limits |
| cTc, cTc machine (in prose) | CTC machine |
| demand (for a control) | control |
| desk | CTC machine |
| detection block | track circuit |
| distant (as the meaning of D), Distant Relay | D: proceed indication, detector, decoding |
| Exit Block | track circuit |
| field controller | field unit |
| Field Unit (capitalized, for the role) | field unit |
| Form 504, Form 506, Form 508, Form 510, Type L | US&S 506 time code; the others are not sourced |
| host, plant host, node (for a computer that runs field units) | field processor |
| house, relay house, shelter, instrument case | bungalow |
| indication (alone) | signal indication or office indication |
| indication snapshot | indication vector |
| Indications-as-Truth | office indication |
| interlocked function | vital control |
| Interlocking Plant (meaning the logic) | interlocking logic |
| interlocking frame | interlocking machine |
| island, Island Block (for a switch section) | OS section |
| Kontrol (for K) | indicator |
| leverman, towerman, TowerMaster | tower operator |
| line station, Line Station | field station |
| mnemonic | token |
| Model 503, Model 506, Type 506 machine, US&S506 machine | the SPCoast CTC machine (US&S style) |
| neutral contact (for front contact) | front contact |
| Non-Vital Command, Non-Vital Circuit (for a code line function) | non-vital control |
| Occupied Section, On-Sheet Section | OS section |
| out of correspondence (alone) | out of switch correspondence or out of office correspondence |
| panel (for the whole machine) | CTC machine |
| plant (alone) | interlocking plant, interlocking model or interlocking logic |
| Send (for S) | control (FieldUnit final S) or stick (AAR S) |
| station (for a code line address) | field station |
| TCS | CTC ("traffic control system" is the name in 49 CFR 236) |
| territory | subdivision |
| Territory / Subdivision | subdivision |
| TOL, track occupancy light | track indication lamp |
| US&S cTc | CTC machine |
| vital engine | interlocking logic |
| Vital Command, Vital Circuit (for a code line function) | vital control |

## 12. Sources

Quotes marked "paraphrase" came through a web fetch tool. Their wording is not verified.

| Key | Source |
|---|---|
| AAR56 | *American Railway Signaling Principles and Practices*, Chapter II, AAR Signal Section, revised June 1956. `docs/reference/Chapter-02-Symbols-Aspects-and-Indications-1956.pdf`. Printed page numbers. |
| AARLIST | General AAR Abbreviations, as reproduced at railroadsignals.us/basics/nomenclature.htm (paraphrase). Not AREMA text. |
| ARTC23 | ARTC SCP 23, extranet.artc.com.au/docs/eng/signal/procedures/design/SCP23.pdf |
| BNSF | BNSF Railtalk, "ABCs", bnsf.com/news-media/railtalk/heritage/abcs.html (paraphrase) |
| CFR218 | 49 CFR 218.109, law.cornell.edu (paraphrase) |
| CFR236 | 49 CFR 236.782, law.cornell.edu (paraphrase) |
| EKEVING | "1927: Dispatcher Control on the New York Central", ekeving.se/ctc/us/NYC_1927.html (paraphrase) |
| GCOR6 | General Code of Operating Rules, 6th edition, 2010, fobnr.org copy |
| NORAC9 | NORAC Operating Rules, 9th edition, 2008, rail.pgengler.net copy |
| PRR56 | PRR rulebook effective October 28, 1956, with revisions to 1964, redoveryellow.com copy |
| RRS506 | rrsignal.com/railroad/ctc/uss506.htm |
| RRS514 | rrsignal.com/railroad/ctc/uss514.htm (paraphrase) |
| RS1927 | Railway Signaling, 1927, "Various Circuits for Directional Control of Single Track Signals", jonroma.net |
| RS1937 | Railway Signaling, October 1937, "Ends of Double Track Controlled by CTC", jonroma.net |
| RS1944 | Railway Signaling, 1944, "Coded Track Circuits for CTC and Cab Signaling", jonroma.net |
| RS1959 | Railway Signaling, July 1959, "Pushbutton console for CTC", jonroma.net |
| RS1960 | Railway Signaling and Communications, 1960, "Fourteen RRs adopt new operating code", jonroma.net |
| USS1949 | Farrington, Union Switch & Signal history, 1949, utahrails.net |

The full verdicts and quotes are in `FieldUnit-Subdivision/docs/review/vocabulary-sources.md`.
