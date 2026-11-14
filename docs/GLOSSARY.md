# Glossary

This file is the source of truth for terms in FieldUnit and FieldUnit-Subdivision.
It follows the decisions in `FieldUnit-Subdivision/docs/archive/vocabulary-review.md` (rev 7, 2026-10-01).

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
| Extent | Where does it start and stop? | interlocking limits, CTC limits |
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
| Extent | interlocking limits | interlocking limits, inside CTC limits |
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

### 2.6 The Layout Data Model File and its writers [FieldUnit]

After FieldUnit-Subdivision ADR 0008 and ADR 0011. The file, its policies and its derivation rules last; a tool that
writes the file is one writer among others.

- **conformance suite** [FieldUnit]. The derivation rules' test cases, as data. Each case gives `given` and
  `relations` and expects a `derived` and a list of diagnostics, by code and symbol. A derivation conforms when it
  passes every case of the rules version it stamps.
- **derivation rules** [FieldUnit]. The normative statement of what `derived` holds, computed from `given` and
  `relations`: the relation table of ADR 0009 §3.12, circuits and their names, control point propagation, ports and
  links, a mast's heads. Versioned by date, with the conformance suite and the reference derivation, as one version.
- **derived** [FieldUnit]. The map of what the derivation rules compute, on every entity and row: every resolved
  relation, whether it came from a wire, a field or a chain, and every fact the drawing does not state. Complete for
  what the rules can reach, never a copy of a stated value, never written by hand, stamped with `provenance.rules`.
  Effectively a cache: a reader that does not hold its rules version rebuilds it.
- **given** [FieldUnit]. The map of the fields a person stated on a symbol, keyed as drawn, every value a string.
  KiCad's housekeeping fields and the key field are left out; nothing else is.
- **incomplete, in error** [FieldUnit]. Incomplete: a fact is not stated, and what depends on it is absent from
  `derived`. In error: stated facts contradict each other, and the facts in conflict derive nothing. Whether a file in
  error is saved is the writer's policy.
- **KiCad importer** [FieldUnit]. The writer that reads the KiCad project (netlist, sheets, symbol libraries) and
  writes the Layout Data Model File: the key field, `given`, each wire as a named relation, macros expanded; it calls
  the reference derivation for `derived`. Transient: it retires when Studio exists, or stays as an import path. It
  writes no file from a drawing in error. Code names today: `tools/layout_compiler`, `compile_layout.py`.
- **Layout Data Model File** [FieldUnit]. The public interface between the drawing and every consumer: one JSON file
  for a layout, under the schema `schemas/layout-model/`. Every entity and row holds the maps `given`, `relations`
  and `derived`.
- **proof-of-concept compilers** [FieldUnit]. The plant compiler (`tools/plant_graph`) and the controller compiler
  (`tools/controller_graph`). They read the archived per-project drawings directly and are kept to check the unified
  project against the archive, not to compile it. "Plant compiler" and "controller compiler" name these tools only.
- **reader policy** [FieldUnit]. For the meaning of a name, a reader looks in `derived`, then `relations`, then
  `given`; the first hit wins. A key the reader does not know is skipped and reported.
- **reference derivation** [FieldUnit]. The implementation of the derivation rules that every writer may call:
  `tools/layout_model/derive.py` (planned). It is one conforming derivation; the rules and the suite are the
  definition.
- **relations** [FieldUnit]. The map of the connections a person stated, each written as the name it reaches: a wire
  to a processor, a transport, a bus, a column or a house, and the trackplan's wires. A relation implied only by sheet
  hierarchy is not stated; it is resolved into `derived`.
- **Studio** [FieldUnit]. The planned layout editor whose native format is the Layout Data Model File. A writer; it
  may save work in progress, in error included.
- **writer policy** [FieldUnit]. `given` and `relations` hold exactly what was stated; `derived` holds everything the
  rules derive from them, by a conforming derivation, stamped; any change to `given` or `relations` re-derives it
  before the file is saved.

## 3. Roles, organization and equipment

- **appliance** [Both]. A device in the field that the field unit operates or reads. Examples: switch, derail, signal, track circuit, electric lock.
- **bungalow** [Both]. The prototype's word for a walk-in house (see house). It holds the relays or the field processor, batteries and wiring. Each control point has one house; on a KiCad schematic the `House` symbol declares it.
- **bus, IOBus** [FieldUnit]. A processor's device interface, or a CMRInet Node's: what drivers attach to with their address. Code names: `IOBus`, `CmriIOBus`, `MqttApplianceBus`.
- **CMRInet Node** [Both]. A driver on a CMRInet line, addressed by its UA, declaring its input and output byte arrays and providing a bus for its bits. Not a processor. The general sense of node (a computer) is retired; the processor that anchors the line is its CMRInet Host.
- **code line** [Both]. The communication path between the CTC machine and the field units. It carries controls to the field and office indications to the office. As an entity of the layout model: a code line encoding on a transport, the thing FieldUnits and the OfficeUnit attach to; one per FieldUnit (FieldUnit-Subdivision ADR 0009).
  - FieldUnit: the class `CodeLine` is the transport (`MqttCodeLine`, `StreamCodeLine`, `MockCodeLine`). `AarTextCodec` writes functions as tokens.
  - History: one account of the first CTC installation (NYC, 1927, GRS) describes one wire to each switch plus a common return [EKEVING]. This is one secondary source.
  - History: a 1937 article describes a "Union time-code C.T.C. system" on a "single series circuit of two wires" [RS1937].
- **code line type** [FieldUnit]. The definition of how one kind of code line carries functions: addressing, capacity, encoding and timing. A code line type is one encoding on one transport (`FieldUnit-Subdivision/docs/adr/0001-code-line-type-contract.md`). Examples: AAR tokens over MQTT; US&S 506 over a relay line. The AAR token encoding has no capacity limit.
- **CTC machine** [Both]. The equipment at the office from which the dispatcher sends controls and reads office indications. It has levers, code buttons and lamps.
  - US&S wrote "C.T.C. control machine" in 1949 [USS1949].
  - FieldUnit: `cTcMachine` is a code name. The aliases `CtcMachine` and `OfficeUnit` exist. Prose says CTC machine for the equipment; the office logic entity that runs it, drawn and keyed in the layout model, is the OfficeUnit (the owner, 2026-10-23: OfficeUnit, not machine).
  - "Desk" is fine in general prose (the owner, 2026-10-26). Where precision is needed, the office side is the OfficeUnit, the peer of the FieldUnit.
  - The SPCoast machine is "the SPCoast CTC machine (US&S style)". It has no model number.
- **device interface** [FieldUnit]. The boundary between the field unit and the hardware of its appliances. Code names: `IOBus`, `CmriIOBus`, `MqttApplianceBus`.
- **dispatcher** [Both]. The operator role that controls trains at a distance. The dispatcher uses a CTC machine, a code line and the distributed handshake of control and office indication. This is the only operator role that the model covers today. Compare tower operator and maintainer.
- **driver** [FieldUnit]. A chip or object with an address on a bus, declaring device pins: an I2C expander, a GPIO pin or pixel chain, a JMRI object (keyed by its object class and the device it serves), a CMRInet Node or one of its bits. Prose says driver, not chip.
- **encoding** [FieldUnit]. What rides a transport. A code line encoding (AAR tokens, US&S 506, bit packing) provides a code line; an I/O encoding (JMRI over MQTT, CMRInet polling) provides remote I/O for the processor that anchors its transport instance.
- **field** [Both]. All places and equipment outside the office.
- **field processor** [FieldUnit]. The microcontroller, computer or process that runs one or more field units. It exists when it is installed or started.
- **field station** [FieldUnit]. One address on a code line, and the set of controls and office indications carried under that address. Derived, never written: one per interlocking on an AAR line, one per house on a US&S 506 line (FieldUnit-Subdivision ADR 0009 §3.8).
  - A field station has no behavior. It is how one code line type lets the office reach a field unit.
  - A field station is the abstraction that the interlocking presents to the dispatcher. It carries the functions of the levers and lamps in its panel column. Routes, masts and aspects are not code line functions; the interlocking handles them in the field.
  - The code line type assigns field stations. On a 506 line a field unit can answer at several. On an MQTT line it answers at one.
  - On a 506 line each control point is one field station. Its name is the control point name: the Value of the KiCad `MAIN HOUSE` symbol. A name carries no `CP` prefix; a model board can add `CP ` for display. The tooling rejects a duplicate. It does not invent a name. A name used in an MQTT topic or a key has each space replaced by `-`. No code adds or strips a `CP` prefix.
  - An interlocking model with no `MAIN HOUSE` symbol is an error. The `MAIN HOUSE` symbol is the explicit source of the name. The tooling does not take it from the title block or the file name. The symbol can display a title block variable.
  - The address of a field station on a 506 line is authored. The tooling does not allocate it.
  - Prototype note: [RRS506] uses "field station" and "field location" for the places on a 506 line.
  - FieldUnit: the class `CtcStation` is the office-side record of one field station.
- **field unit** [FieldUnit]. An interlocking application running on a field processor. It reads the appliances, enforces the locking, drives the switches and signals, and answers the code line. `FieldUnit` is also the name of the C++ library, and the spelling of the entity in the layout model and its seam vocabulary (the owner, 2026-10-23: FieldUnit and OfficeUnit, never "unit" alone; a unit is a KiCad symbol unit).
- **head device** [FieldUnit]. The hardware that renders a head's aspects: aspect to I/O, with the style fixed per symbol. The trackplan head owns indication to aspect. Prose says head device, not cast.
- **house** [Both]. Instrument house: the structure at a control point that holds its field equipment; the KiCad `House` symbol declares the control point. Bungalow is the prototype's word for a walk-in house; case for a cabinet.
- **interlocking application** [FieldUnit]. The interlocking logic combined with one interlocking model. Today it is not a file. `spcoast_virtual_plant` builds it at start.
  - The signalling industry uses "application" for logic configured with the data of one site. Example: the US&S "Microlok II System Application Logic Programming Guide" [ARTC23].
- **interlocking logic** [FieldUnit]. The generic logic of interlocking, in software. It is the same for every interlocking plant. Code name today: `InterlockingPlant`. A rename to `Interlocking` is planned.
- **interlocking model** [FieldUnit]. The data that describes one interlocking plant. Examples: the KiCad schematic, the FieldUnit JSON (`generated/<Interlocking>.json`) and the portable model (`InterlockingPlantModel`).
- **interlocking plant** [Both]. The real track, switches, derails, signals and track circuits that one interlocking controls. It can contain more than one control point. One field unit solves one interlocking plant.
- **local I/O, remote I/O** [FieldUnit]. Drivers on a processor's own bus; drivers reached through an encoding on a transport (JMRI) or through a CMRInet Node on a line.
- **maintainer** [Both]. The operator role that works at a location in maintenance or debug mode, without the interlocking. Compare dispatcher and tower operator. Not yet covered by the model. The maintainer call (section 5) calls the signal maintainer to a location.
- **office** [Both]. The place where the dispatcher and the CTC machine are.
- **panel column** [FieldUnit]. One vertical part of the CTC machine face. It holds levers, lamps and a code button. Code names: KiCad `PanelColumn`, C++ `PanelColumn`. A `CtcStation` can span up to four panel columns (`MAX_COLUMNS_PER_STATION`).
- **port** [FieldUnit]. A FieldUnit or OfficeUnit sheet's connection to the layout: Codeline, Processor, IOBus, PanelIOBus.
- **processor** [FieldUnit]. The thing logic runs on and connections are anchored on: a field processor, the OfficeUnit's processor, or a virtual host application. A FieldUnit or OfficeUnit runs on exactly one.
- **seam** [FieldUnit]. After FieldUnit ADR 0005: Seam 1, the code line between office and field logic; Seam 2, hardware I/O, local or remote, on the panel side and the field side.
- **subdivision** [FieldUnit]. The set of control points, code lines and CTC machines that the subdivision linker joins into one model. Example: SPCoast South.
- **transport** [FieldUnit]. A mechanism that carries encodings: an MQTT broker, a CMRInet serial line, a US&S time-code line, LocoNet. Declared once; placed once per processor that runs an encoding on it. A FieldUnit reaches its code line's transport through the code line, so no instance is drawn per field processor (the owner, 2026-10-24).

## 4. Control point, interlocking, CTC and limits

- **ABS** (automatic block signal system) [Prototype]. A block signal system in which track circuits control the signals, with no operator. ABS protects trains. It does not give them authority: the timetable and train orders, or another system, give authority. Compare APB and CTC. (unverified: no rule text was opened.)
- **APB** (absolute permissive block) [Prototype]. ABS for single track. An absolute signal at each end of a single-track section (a head block) holds opposing movements at Stop; the intermediate signals are permissive and let following movements close up. Conceived by GRS about 1910; first major installation on the Toronto, Hamilton and Buffalo in 1911 (owner, 2026-11-03; W. K. Howe, GRS, 1911, not opened). FieldUnit: a mast's `Kind`, `ABSOLUTE` or `PERMISSIVE` (see automatic signal).
- **ATC** (automatic train control) [Prototype]. Train control that enforces a speed limit and stops the train when the engineer does not obey a signal. (unverified) Out of scope for FieldUnit: the project does not control train speed.
- **ATS** (automatic train stop) [Prototype]. Train control that applies the brakes when a train passes a restrictive signal and the engineer does not acknowledge it. Usually worked by an inductor on the track at the signal. (unverified) Out of scope for FieldUnit: the project does not control train speed.
- **cab signal** [Prototype]. A signal in the cab of the locomotive that repeats the wayside signal indication, or replaces it. A continuous cab signal takes its information from a coded track circuit [RS1944]. Out of scope for FieldUnit: the project does not control train speed.
- **control point** [Both]. A named place where a dispatcher or operator controls train movements by controlled absolute signals.
  - The term names a function. Controlled signals make a place a control point. A switch does not. A control point can have no switch.
  - An interlocking contains one or more control points.
  - FieldUnit: the designer declares each control point. One KiCad `MAIN HOUSE` symbol declares one control point, and its Value is the name (`CP Luchessa`). The `CP` field of an appliance assigns it to a control point. The tooling does not derive the number of control points.
  - Example: Luchessa is one interlocking with three control points: `CP Luchessa`, `CP Gilroy`, `CP Carnadero`.
  - GCOR: "Control Point: The location of absolute signals controlled by a control operator." [GCOR6]
  - NORAC uses "Controlled Point (CP)": "A station designated in the Timetable where signals are remotely controlled from the control station." [NORAC9]
  - 49 CFR 236.782 defines "controlled point" as a location where signals or other functions of a traffic control system are controlled from the control machine. This is a paraphrase [CFR236].
  - Example name: `Luchessa` (drawn today as `CP Luchessa`; the prefix goes, ADR 0001 D12).
  - A control point has one bungalow. On a 506-style code line it has one field station, with the same name.
- **control point limits**. Not a term (owner, 2026-10-03). A control point has no limits of its own. The interlocking limits bound every control point of the interlocking. The `CP` field assigns an appliance to a control point; it draws membership, not a boundary. An appliance inside the interlocking limits that is not on the code line (a local hand-throw switch) can belong to no control point.
- **CTC** (centralized traffic control) [Both]. A traffic control system in which a dispatcher controls the signals and switches of control points from a distance.
  - CTC adds a code line, field stations, controls, office indications and a remote dispatcher to the interlocking.
  - 49 CFR 236 uses the term "traffic control system" [CFR236].
  - Some sources write "cTc" (Wikipedia for the 1927 GRS machine; rrsignal for US&S machines). A trademark record was not found (unverified). US&S wrote "C.T.C." [RS1937, USS1949].
- **CTC limits** [Both]. The part of the railroad where the CTC rules apply. (unverified: the rule text was not opened.)
- **interlocking** [Both]. An arrangement that forces switch and signal movements to follow each other in a safe sequence.
  - History: first a mechanical machine (levers, locking bed, pipe and wire); later relays.
  - Interlocking is the mechanism facet. A CTC control point uses the same kind of field logic.
- **interlocking limits** [Both]. The track between the opposing home signals of an interlocking. The tooling can derive them: cut the track graph at every controlled signal; each piece that contains a powered switch or derail is one interlocking (`FieldUnit-Subdivision/docs/archive/spike-cp-membership.md`; `FieldUnit-Subdivision/docs/adr/0013-between-control-points.md` §4).
- **interlocking machine** [Prototype]. The lever frame and its locking that a tower operator works. The locking bed is the part that holds a lever that must not move.
- **PTC** (positive train control) [Prototype]. Train control that enforces the authority and the speed limits of a train in the cab, and stops the train when the crew does not. Required on certain lines by the Rail Safety Improvement Act of 2008 (unverified: the act was not opened). History only. PTC needs train speed control (DCC, car and engine detection). Out of scope for FieldUnit: the project does not control train speed. Correct switches, signals and CTC are the foundation that PTC is built on.
- **tower** [Prototype]. The building at an interlocking from which the tower operator works the interlocking machine.
- **tower operator** [Both]. The operator role that works directly on a locking bed, real or virtual. On the prototype this is the person who works an interlocking machine in a tower. The tower operator does not use a code line. Compare dispatcher and maintainer. Not yet covered by the model.
- **train control** [Prototype]. Equipment on the train and the track that acts on the brakes when the engineer does not obey a signal or a speed limit: ATS, ATC and cab signals, later PTC. The ICC ordered train control on many lines from 1922 and set speed rules in 1947; today 49 CFR 236 requires cab signals, ATS or ATC for trains at 80 mph or more (unverified: the orders and §236.0 were not opened). Many western roads kept passenger trains below 80 mph instead, the 79 mph limit. Out of scope for FieldUnit: the project does not control train speed.
- **yard limits** [Prototype]. A part of main track, designated by the railroad, inside which the yard limit rule applies. Standard Code Rule 93 [PRR56]: "Within yard limits, movements not authorized by timetable schedule or train order may be made on the main track by proper signal indication or permission of the operator without protecting against extra trains or engines." GCOR 6.13 is the modern equivalent. The KiCad symbol is `Rule93-Yard Limits` (`RULE_93`; was `Rule6.13-Yard Limits`).

## 5. Controls, indications and functions

- **code button** [Both]. The push button that sends the controls of one field station. The label on the machine is CODE.
  - FieldUnit: `PanelInput::CODE_BUTTON`. `cTcMachine` sends when the button is released (`OneShot`).
- **control** [Both]. A demand sent from the office to the field over a code line, as a function. It states the intent of the dispatcher. The field unit decides whether to act on it. Examples: `783NWS`, `784NGS`.
- **control transaction** [FieldUnit]. The complete set of controls for one field unit, applied as one unit. Code name: `ControlTransaction`. Two rules govern it (FieldUnit `docs/adr/0002-control-transaction-classes.md`):
  - A malformed transaction is ignored as a whole. This includes the controls of both classes. The field unit updates its error counters and sends office indications of its current state.
  - A valid transaction that would violate a safety protection has every vital control ignored together. The field unit does not act on the safe ones and skip the unsafe one. It processes every non-vital control.
- **demand** [FieldUnit]. A request that an appliance take a state: a switch to go to normal or reverse, a signal to clear, a lock to release.
  - A demand that comes from the office over a code line is a control.
  - A demand that a field unit reads from its I/O is a local demand.
  - Every demand is an input to the interlocking logic of the interlocking that owns the appliance. Only that logic drives the appliance, and it decides whether to act on the demand.
  - Source: FieldUnit-Subdivision ADR 0017, decision 3 (owner's ruling, 2026-11-14).
  - FieldUnit: `SwitchDemand`, `ElectricLockDemand`, `SignalDemand` (`src/types.h`, lines 80 to 94); `ControlTransaction::switchDemands`, `lockDemands`, `signalDemands`, `fleetDemands` (`src/InterlockingPlant.h`, lines 26 to 35). Each is a demand of the dispatcher in a control transaction, so these names carry controls. An input for a local demand is the subject of FieldUnit issue 45.
- **function** [FieldUnit]. One named control or office indication. Its written form is a token. Example: `783NWS`.
- **indication vector** [FieldUnit]. The complete set of office indications from one field unit, sent as one unit. Code name: `IndicationVector`.
- **lever** [Both]. A handle on a CTC machine or an interlocking machine. It states one intent for one switch or signal.
  - AAR56 names the positions of a three-position lever L and R, as in `10L` and `10R` [AAR56 p. 34]. The middle position is N, the normal position [AAR56 Figs. 18, 22].
  - A switch lever has the positions N and R. A signal lever has the positions L, N and R.
  - FieldUnit: `PanelInput::SW_NORMAL`, `SW_REVERSE`, `SIG_LEFT`, `SIG_STOP`, `SIG_RIGHT`. A switch lever with neither contact closed sends no switch control.
- **local demand** [FieldUnit]. A demand that a field unit reads from I/O: a lever, a key or any sensor, over any transport. Where the sensor is wired is free.
  - It never crosses a code line as a control. The field unit can report it to the office as an office indication, as it reports a track circuit.
  - As every demand does, it goes through the interlocking logic of the interlocking that owns the appliance.
  - What a field unit learns of another interlocking is not a demand. The `H`, `D` and `F` of a block, which rest on track circuits the other interlocking owns, reach it as line inputs, and its own logic decides (the seam of FieldUnit-Subdivision ADR 0013; see line input in section 10.5).
- **maintainer call** [Both]. A control and lamp that call the signal maintainer to a location. A 1959 machine had a "maintainer's call" control and a "maintainers' call lamp" [RS1959]. No source says that train crews used it.
  - FieldUnit: tokens `<n>MCS` and `<n>MCK`, from the name `<n>MC` (`1MC`: `1MCS`, `1MCK`). It is a non-vital control. `MC` is the AAR abbreviation per the owner (AAR Signal Manual Parts 33 and 91, tokens `MCS`, `MCK`; unverified, ADR 0003 D12). The number prefix is FieldUnit (section 10.2).
- **non-vital control** [FieldUnit]. A control that cannot affect a safety protection. The interlocking logic does not check it against the locking.
  - Processing: when a transaction is malformed, the field unit ignores it with the rest of the transaction. When a valid transaction is unsafe, the field unit still processes every non-vital control.
  - Examples: maintainer call `<n>MCS` (`1MCS`).
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
- **aggregation** [Both]. There is no aggregation term of its own. The AAR relays cover it (owner, 2026-10-03).
  - Several sensors wired in parallel to one input are one track relay (`TR`). They are the wiring of one track circuit and are not in the model.
  - Several sensors on separate inputs are separate track relays, so separate track circuits, named as cut sections of one block (`01AT`, `01BT`). Their one occupancy is a track repeater (`TP`), picked up only when every one is clear.
  - A block of track circuits (`TC = 01T, 02T`) has the same form: the block's occupancy is a `TP` over the circuits' `TR` contacts.
  - "Detector", "sensor" and "input pin" are not model terms. They are wiring inside a track relay's circuit, drawn on the field I/O sheet. The code chart decides whether the office sees the circuits, the block, or both.
- **automatic signal** [Both]. A block signal controlled automatically by track circuits and `HD` lines, without a dispatcher or tower operator (`FieldUnit-Subdivision/docs/adr/0013-between-control-points.md` §5, §15).
  - In the layout model a mast states `Kind` `ABSOLUTE` (an APB head block, no number plate) or `PERMISSIVE` (a number plate; modified by a `Plate-G` or `Plate-R` grade or restricting marker) [AAR56 pp. 51, 53–54]. The most restrictive indication of both is Stop. The plate changes the aspect the crew reads at Stop, not the indication the mast presents: Stop and Hold (Rule 292) on an unplated mast, Stop and Proceed (Rule 291; Rule 290 with a marker) on a plated one (see number plate, section 8).
  - An automatic signal gives no movement authority.
  - FieldUnit: `AutomaticSignal` (FieldUnit ADR 0008), one per mast and direction: the block it governs into (`governs()`), the block behind it (`approach()`), the HD line it reads from ahead (`reads(h, d)`), its `SR`, and an optional `opposing()` cut at the same location. Its display is a route with no signal control. It produces the HD line to the rear as two facts named `<mast>H` and `<mast>D`.
- **ball signal** [Prototype]. An early fixed signal: a ball hoisted on a mast. The New Castle and Frenchtown Railroad, 1832, is usually given as the first fixed signal in the US (unverified).
- **block** [Both]. One or more track circuits treated as one occupancy for one purpose. Its occupancy is a track repeater (`TP`) fed through the track relays of its circuits; with one circuit it is that circuit's `TR`.
  - Signal block [Prototype]: the track between consecutive signals that govern movement into it. A long one is several cut sections.
  - Station-to-station block [Both]: the main-track block between two control points or APB head blocks (see station-to-station block).
  - OS section: the block over the switches of a control point (see OS section).
  - Logical detection block [FieldUnit]: several track circuits treated as one occupancy. Drawn today as the list in the `TC` field of a switch; a desk lamp's `IndicationToken` list is the same idea on the office side (`FieldUnit-Subdivision/docs/adr/0002-symbol-contract.md`, request 14).
  - FieldUnit: the model has no block or `TP` entity yet; `addSwitch(id, os)` finds an existing OS circuit by name, so several switches can share one circuit. The block entity is planned (FieldUnit-Subdivision issue #24).
  - A block is not a track circuit. A track circuit is one `TR`. A block is the unit that locking, a route or a lamp uses.
- **boundary track circuit** [FieldUnit]. A point detector at an insulated joint (`Appliance track_circuit`, `Style` `boundary`) whose occupancy counts toward the track circuits on both sides of the joint (`FieldUnit-Subdivision/docs/adr/0013-between-control-points.md` §10, §19.1). Because a consumer ORs its occupancy into both sides, a train passing the masts occupies the circuit ahead before it releases the circuit behind (make-before-break), keeping the rear block occupied for cars with non-shunting wheels. It never splits a circuit or ends a block.
- **coded track circuit** [Prototype]. A track circuit whose current is switched on and off at a code rate. The rate carries information, for example the aspect for a cab signal [RS1944]. Compare track circuit. FieldUnit has no coded track circuit.
- **crossover** [Both]. Two switches that connect two tracks and work together from one lever.
  - Name: one gang of switches that share a gang id (the Value minus its trailing letter): `815`, `815A`; a double crossover runs to `815C`. Any letter A to Z can name a member, including D (FieldUnit-Subdivision ADR 0003 D7, D8; proposed).
- **cut section** [Both]. A track circuit that is one part of a longer block. It has its own track relay. AAR56 names the sections of a signal-governed block with letters (`784LAT`, `784LBT`) [AAR56 p. 31]. Where the letter goes (`784LAT` or `A10T`) is open (ADR 0003 D6).
  - Prototype: a long block is cut into sections because ballast leakage limits the length of one circuit.
  - Model: a sensor with its own input is its own track relay, so its own track circuit; two such sensors on one section are two cut sections (`01AT`, `01BT`) of one block.
  - Examples by the owner's reading of AAR practice (ADR 0003 D15): successive circuits that govern signal 10 take progressive letters (`A10T`, `B10T`, `C10T` in the owner's statement; `10AT` in the suffix form).
- **derail** [Both]. A switch-shaped appliance that derails a car before it fouls another track.
  - FieldUnit rule: NORMAL is the derailing position. REVERSE is the clear position. A derail rests in NORMAL.
  - Prototype convention (recall). No source was found for power-operated derails.
  - 49 CFR 218.109 makes the derailing position the normal position of fixed derails, with exceptions. This is a paraphrase [CFR218].
  - GCOR 8.20: "Sidings having hand-thrown derails will have derail locked in non-derailing position, except when engines or cars are left unattended on siding." [GCOR6]
  - A route through a derail requires REVERSE.
- **dependent derail** [FieldUnit]. A derail that moves with its switch, in the same position. Switch Normal gives derail Normal. Switch Reverse gives derail Reverse.
  - Name: a derail symbol whose gang id (the Value minus its trailing letter) matches a switch or lock gang of the same interlocking. Convention: the letter D, for example `795D`. The symbol kind and the name relation decide dependence, not the letter D. Values are unique across switches, locks and derails (FieldUnit-Subdivision ADR 0003 D8, proposed).
  - Code today: `addDerail("795D")` parses the trailing D and requires switch `795` to exist; it is to take the base from the model (`controllingSwitch`).
  - It has no lever and no tokens of its own. The switch's `NWK` and `RWK` require both machines in switch correspondence.
- **color light signal** [Both]. A signal whose head shows its aspect by the color of its light, one color at a time. Two forms (owner, 2026-11-03):
  - Multi-lamp head: one lamp for each color, one lit at a time; the SP's green, yellow and red heads. GRS introduced the Type D vertical color light in the 1920s with a small hood over each of its three lenses. Snow piled up in front of the lenses, so the SP, which ran through the mountains, changed the design to one large hood. After the 1977 film Star Wars, that hood got the SP heads called "Darth Vaders" (owner, 2026-11-03). In the model: one pin for each lamp, or a select code. Style word today `CPL` (FieldUnit-Subdivision checkpoint E65). That word is a misnomer: CPL is the B&O color-position light system. A rename is open.
  - Searchlight: one lamp behind a moving colored roundel (the vane). In the model: one RGB LED with three legs (red is R, yellow is R plus G, green is G; brightness per leg on PWM) or one point on a pixel chain. Style word `CL` (E47, E65, E71).
  - Compare color-position light signal and position light signal.
- **color-position light signal** (CPL) [Both]. The B&O system: a round target with pairs of lamps, whose position and color together carry the aspect, and marker lamps around the target. FieldUnit: aspect policy `boCpl` (`src/SignalAspectPolicy.h:326`), driver `CplMastDriver` (`src/drivers/CplMastDriver.h`); see `docs/how-to/04_semaphores_and_eastern_signaling.md` section 4. Style word: none yet, because `CPL` names the multi-lamp color light head today (see color light signal). Before 2026-11-03 this glossary used this term for the multi-lamp head and called the B&O system position-color light; both were wrong.
- **position-color light signal** (PCL) [Both]. Per the owner (2026-11-03): an eastern, PRR-area system of amber position lights with added color elements (unverified: no source was opened). Style word `PCL`, reserved; no symbol yet. It is not the B&O system, which is the color-position light signal.
- **position light signal** [Both]. The PRR system: rows of amber lamps whose position alone carries the aspect; the word color does not appear. Style word `PL`, reserved; no symbol yet (owner, 2026-10-12).
- **electric lock** [Both]. A lock on a hand-operated switch. The dispatcher releases it by a control.
  - AAR56: `WL`, switch lock [AAR56 p. 35].
  - FieldUnit: tokens `WLS` (release, control) and `WLK` (released, indication: asserted while the lock relay is energized and the switch is released; deasserted when locked, the fail-safe state, since the lock bar and the lock relay both drop on loss of power). `WLQK` is the crew's unlock request, non-vital (FieldUnit-Subdivision ADR 0004 D1). `SwitchLock::HAND_LOCKED`. The field unit releases the lock only when every signal is at stop and no time locking runs. The full exchange is in `AAR_SIGNALING_PRIMER.md` section 18.
  - An electric lock is not a dual-control switch.
  - SPCoast: a lock carries the lock tokens, `WLS` (control) and `WLK`, `WLQK` (indications), as the desk's lock levers are drawn and as `WireCodec` packs `WLK` (the owner, 2026-10-27; this replaces the earlier simplification that locks carried the switch tokens). The crew, dispatcher and field coordination of a real electric lock is still deferred (FieldUnit-Subdivision ADR 0002 D14).
- **fixed signal** [Prototype]. A signal at a fixed location that governs train movements: a ball signal, a train order signal, a semaphore, a light signal. A hand signal or a flag is not a fixed signal. (unverified: no rule text was opened.)
- **fouling point** [Prototype]. The point beyond which a car on one track can be struck by a movement on another track.
- **head** [Both]. One unit of lamps on a mast. It shows one part of an aspect. A head has a letter. The proof-of-concept plant compiler accepts A to E today; ADR 0003 D9 allows any letter A to Z.
  - Head letters are unique for one signal number in one direction. The masts carry them in their names: `784WA` has head A, `784WBC` has heads B and C, and `784WD` has head D (the owner, 2026-11-12).
  - Layout model (FieldUnit-Subdivision rulings 2026-11-04, landed 2026-11-12; `FieldUnit-Subdivision/docs/review/signal-symbols-worklist.md`): a head device has no name. It is a position on the pins of its field mast device, and the pins are lettered A, B, C from the top down (`relations.position`). Thus position A of `784WBC` is head B. A head letter and a pin position are two different things that use one alphabet, and the model states only the position. This is an open item with FieldUnit-Subdivision (2026-11-12).
  - What a head shows comes from a table, not from a route. The route gives the mast one indication (see route). The part of each head is a lookup with four inputs: the mast's indication, the number of heads on the mast, the head's number, and the context. The context is the railroad's and the era's choices: route or speed signaling, fast and slow speed switches, the set of indication names in use (the owner, 2026-11-12). FieldUnit: an `AspectResolver`, which `SignalMast::setIndication()` calls with the indication, the head count and the dwarf flag (`src/SignalMast.h:102-109`). To assign a route to a head has a meaning only under route signaling.
- **head block** [Prototype]. The absolute signal at the entrance of an APB section. It holds an opposing movement at Stop while a train is in the section. Its most restrictive indication is Stop (Rule 292) [AAR56 pp. 51, 53–54]. FieldUnit: a mast with `Kind` `ABSOLUTE`.
- **HD line** [Both]. The directed two-value line circuit between neighbouring signal locations (`FieldUnit-Subdivision/docs/adr/0013-between-control-points.md` §15). Named for a mast and sent to the previous signal in that mast's direction of travel:
  - `H` feeds the receiving signal's home relay (`HR`) when the segment's track circuits and permission conditions are met.
  - `D` feeds the receiving signal's proceed-indication relay (`DR`) when the producer's mast indicates Approach or better (its `HR` is up: more favorable than Stop or Restricting; a number plate changes how the crew reads a Stop, not whether `HR` is up) [AAR56; AARTC71 Dwg 8000, 8066; CBQ1946 pp. 613–615]. `D` is read only while `H` is asserted.
  - On the prototype this is a chain of two-wire polarized line circuits re-fed at each signal location [CBQ1946; RS1947 p. 92].
- **independent derail** [FieldUnit]. A derail with its own lever and its own tokens. Example: Corporal derail `5` in the legacy profile.
  - Name: a derail symbol whose gang id matches no switch or lock gang. The letter is optional; convention: none or D (ADR 0003 D8, proposed).
- **insulated joint** [Both]. A rail joint that insulates one rail from the next. Insulated joints bound a track circuit. KiCad: `IRJ-Signal` and `IRJ-Signals` place a joint with its mast or masts (see signal location).
- **LEFT, RIGHT** [FieldUnit]. The two directions of a signal lever and of a route. Code names: `DirectionAuthority::LEFT`, `DirectionAuthority::RIGHT`. A signal lever has the positions LEFT, STOP and RIGHT. A mast name carries a geographic letter for its direction: W (west) or N (north) is LEFT, and E (east) or S (south) is RIGHT (the owner, 2026-11-12). The proof-of-concept plant compiler (`tools/plant_graph`) maps the letters this way. The layout model states the direction as `relations.facing` on the mast, so a consumer does not read the letter.
- **mast** [Both]. The structure that carries one or more heads. KiCad mast Value today: `^\d+[NSEW][A-E]+$`, for example `784EAB`; ADR 0003 D9 widens the head letters to A to Z.
  - The name has three parts (the owner, 2026-11-12): the number of the signal lever that controls the mast, the letter of its direction (see LEFT, RIGHT), and the letters of its heads (see head). The lever `834` controls the masts `834SAB` and `834NC`. This follows the AAR rule that a name begins with the number of its principal lever (section 10, [AAR56 p. 31]).
  - Layout model: the link from a signal lever to its masts is not a stated relation today. The desk lever serves `Luchessa:784`, no entity has that name, and no mast names its lever, so a consumer must cut the number from the mast name. A model must not require a consumer to parse a name (the owner, 2026-11-12). Reported to FieldUnit-Subdivision on 2026-11-12.
- **OS section** [Both]. The track between opposing signals in a control point, treated as one block. It is usually one track circuit over the switches. It can be several (see block, aggregation).
  - "OS" means "on sheet": the dispatcher's record of a train that passes a location. The BNSF source gives both meanings (paraphrase) [BNSF].
  - FieldUnit: the OS track circuit detector-locks its switch. Declare it with `addSwitch(id, osName)` or `addDerail(id, osName)`.
- **points** [Both]. The two movable rails of a switch that guide the wheels to one track or the other. A switch machine or a switch stand moves them. Point detection proves their position (see switch correspondence).
- **semaphore signal** [Both]. A signal whose head is a pivoted arm (the blade); the arm's position is the aspect by day, and a lamp behind colored roundels repeats it by night. Two Style words, by quadrant, because the quadrant fixes the aspect set (`LQ`, `UQ`; E65):
  - `LQ`, lower quadrant: two positions, horizontal STOP and inclined down CLEAR. The only form before 1903.
  - `UQ`, upper quadrant: three positions, horizontal STOP, inclined up APPROACH, vertical CLEAR. Introduced from 1903, universal on new work by the First World War.
  - The aspect set a route needs is an era fact in the era indication tables, so a two-position head on a route whose table asks for APPROACH is an era error, never a wiring or link error (E48). `AAR_SIGNALING_PRIMER.md` section 17.
  - The night lamp is a color point in the model. A pixel shows the color directly; a white lamp behind the arm's roundel is the same point reduced by a cast to lit or dark on its expander or PWM pin; an RGB LED is the arm plus three legs as in the color light head (E71).
- **signal** [Both]. An appliance that shows an aspect to govern a train movement. One signal can use heads on one or more masts.
  - AAR56 names functions of one lever position with A, B, C. Fig. 26 uses them for separate signals and for two arms on one mast [AAR56 pp. 33–34].
  - FieldUnit separates the control of a signal (`SignalControl`, one for each signal lever) from its display (`SignalMast`).
- **signal location** [Both]. A block boundary with its insulated joint, its signal mast or back-to-back pair of masts (`IRJ-Signal` or `IRJ-Signals`), and the track circuits on each side (`FieldUnit-Subdivision/docs/adr/0013-between-control-points.md` §6). A control point is a signal location with one side toward the station-to-station block.
- **station-to-station block** [Both]. The piece of main track between two control points (or APB head-block signals), cut at every `CONTROLLED` and `ABSOLUTE` signal joint, holding no powered switch or derail and lying under a block signal rule [RS1942] (`FieldUnit-Subdivision/docs/adr/0013-between-control-points.md` §4). It may contain intermediate automatic signals and non-powered switches (hand-throw, spring, or electrically locked). It belongs to the layout, not to an interlocking, and reaches the office as one virtual track circuit indication (`BPR`).
- **switch** [Both]. An appliance with movable points that routes a train from one track to another. AAR56 uses the letter W for switch [AAR56 p. 32]. Code name: `Switch`.
- **switch correspondence** [Both]. Agreement between the position of a switch and its control.
  - Out of switch correspondence: the switch is moving, or it did not reach the position of its control.
  - FieldUnit: `Switch::inCorrespondence()` is true when the reported position equals the position of its control (`commandedPosition()`) and is NORMAL or REVERSE. After the travel timeout (default 5000 ms) a moving switch reports `SwitchPosition::OUT_OF_CORRESPONDENCE`.
- **switch machine** [Both]. The mechanism that moves and locks the points of a powered switch, by electric motor or compressed air. Its contacts prove the position of the points; its lock contacts dominate point detection (`AAR_SIGNALING_PRIMER.md` section 9.7). It replaced pipe and lever at power interlockings. FieldUnit: `SwitchDriver`.
- **switch stand** [Prototype]. The hand-operated mechanism beside a manual switch: a lever, a rod to the points and a target that shows the position of the switch. (unverified)
- **powered switch** [Both]. A switch whose points are moved by a switch machine, electric or pneumatic, under the dispatcher's control. Style word `powered`. The lever and the office show it NORMAL or REVERSE (`NWS`/`RWS`, `NWK`/`RWK`).
- **manual switch** [Both]. A switch whose points are moved by hand, with a switch stand; the counterpart of a powered switch. Style words `manual` and, with point detection, `manual_sensed`. "Manual" names who moves the points, not who controls: an electrically locked switch is a manual switch with a remote release. "Hand-throw" is not the term: switches were thrown by hand, then hand-operated, then manually operated, as the stands became electro-mechanical, and the Style outlives the mechanism (owner, 2026-10-11).
- **dual-control switch** [Both]. A powered switch that the crew may also work by hand at the location, after the dispatcher releases it; a selector lever disconnects the machine from the control and a hand lever moves the points. Style word `dual`. While released, the dispatcher's control has no effect and the switch is out of correspondence until it is returned to power operation. The model has no crank: hand operation is a local lever driving the machine under a release, the electric lock flow applied to a powered switch (FieldUnit ADR 0006, proposed). Compare electric lock, which locks a manual switch.
- **track circuit** [Both]. The section of track that one track relay (`TR`) detects: an electrical circuit in the rails, between insulated joints, that the wheels and axles shunt. One track circuit has one `TR` and one name (`01T`). It is the unit of detection and the unit that the office can see. A block is the unit of use (see block, cut section).
  - Names (ADR 0003 D3, D5, D15; proposed): inside interlocking limits a circuit that encloses switch 11 is `11T`, and the OS circuit of switch 11 is `11T1`, so `833T` and `833T1` are different circuits. Outside, a circuit is numbered from the signal that governs over it (`10T`). A track with no interlocked switch that governs no signal takes an arbitrary number starting with the digit zero (`01T`).
  - Directional coded-track form: track number, direction, T (`1WT`, `1ET`: track 1, west or east of the insulated joint). It is the source of names such as `1SAT` (track 1, southbound approach).
  - AAR56: "A track circuit is designated by the letter T preceded by a number" [AAR56 p. 31].
  - FieldUnit: `TrackCircuit`. Occupancy can come from any detector. A dropout delay (`dropoutDelayMs`) bridges gaps between cars for optical sensors. This is a model workaround.
  - FieldUnit: a track circuit starts OCCUPIED. A remote circuit with no update for 5000 ms gets quality `LOST_COMMS` and is not clear.
- **train order signal** [Prototype]. A fixed signal at a train order office (a depot, a station or a tower), worked by the operator. It tells an approaching train whether the operator holds train orders for it. Also called an order board or a train order board. It is not a block signal: it says nothing about whether the track ahead is occupied. It says only whether the crew must take new orders before it goes on. Standard Code Rule 221 governs it: when orders are held for a train, the signal displays stop for that train's direction, and the operator handles the clearance (unverified). Compare Form 19, Form 31.
  - What it shows, in general practice (the owner, 2026-11-09; unverified: no source was opened). The details differ by road and by the style of the board.
    - Stop: the operator holds orders that the train must stop and sign for (Form 31).
    - Slow: the operator holds orders that the crew takes on the fly from the hoop (Form 19). The train slows enough to catch them. Many roads had this position; not all.
    - Clear: no orders. The train passes without slowing.
  - Form (same source): a semaphore whose blade moves between positions, colored lamps or lanterns (red, yellow, green or white, by era), or cross-arms or balls on the earliest boards. It stands directly beside the office. Its blades or its style differ from those of a block signal.
  - Era (same source): in use from the 1850s. It went out of use between the 1960s and the 1980s, as radio, track warrants and CTC replaced timetable and train order operation. It belongs to timetable and train order territory, never to CTC.
  - Layout model (FieldUnit-Subdivision, ruled 2026-11-05 and reworked 2026-11-08; `FieldUnit-Subdivision/docs/review/signal-symbols-worklist.md`). The model's word is train order board.
    - Trackplan: Appliance `train_order`, Style `single` or `double`, with a Name and a `Station` field that names its timetable station. The symbol has no pins. A board is never on the track, in a route or in a control point, and no mast rule reads it.
    - Field: a mast device, Appliance `train_order_mast`, with one pin for each face. The faces of a double board are named for the directions: `E` and `W`, or `N` and `S`. A head device wired to a face pin is the head of that face. A fascia lever (Appliance `lever`, Style `train_order`) wired to the same pin works that face.
    - The lever has two inputs, `STOP` and `SLOW`. Nothing asserted shows CLEAR. `SLOW` shows APPROACH. `STOP` shows STOP. The more restrictive input wins. These words are indications, never a color or a blade angle. A face that shows all three needs a three-position head.
    - The control is local and non-vital. No code line carries it. CALLON is not a train order control: it is an indication of the block signal, used with the board to bring a train that stopped at the block boundary up to the station (the owner, 2026-11-08).
  - FieldUnit: `TrainOrderFace` (`src/TrainOrderFace.h`, FieldUnit ADR 0010, accepted 2026-11-12). A face is a ranked list of lever inputs and a one-head mast, outside the interlocking. The words and the rank are given at setup. Nothing yet generates a face from the layout model.

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
  - Authority: a return movement to equipment left on the main track is a movement under authority. On controlled track the train dispatcher or control operator authorizes it and governs the signals for it, in places through an engine-return push button or a signal override at an interlocking (GCOR Rule 6.20, Equipment Left on Main Track [GCOR6]; the owner, 2026-11-02). Code today: `Route::engineReturn()` checks no signal control (section 10.6, `ERS`).
- **fleeting** [Both]. A mode in which a signal clears again for a following train without a new control. A 1959 machine had a fleeting control [RS1959]. FieldUnit: `ControlTransaction::fleetDemands`, `SignalControl::FSR()`.
- **indication ceiling** [FieldUnit]. The most favorable signal indication that a route permits. Code name: the second argument of `Route::displays()`, read by `Route::aspectCeiling()`.
- **number plate** [Both]. A plate on a mast that makes its Stop aspect a Stop and Proceed (Rule 291) instead of a Stop and Hold (Rule 292). It is a fixed attribute of the mast, not a state of the signal: a plated mast presents the same indications as an unplated one, and Stop is the most restrictive indication of both. The plate came from semaphore heads, which could add no blade position for a fourth indication, so the mast got a plate instead (the owner, 2026-11-02; unverified against a cited source).
  - What the plate changes is the aspect the crew reads and the rule they act on. What it does not change is anything a relay sees: the mast's `HR` is down at Stop with or without a plate, and the signal in rear reads only `HR` (section 10.6, `DR`).
  - Layout model: `Kind` `ABSOLUTE` or `PERMISSIVE` on the mast (see automatic signal, section 7). FieldUnit: `enum Indication` has no Stop and Proceed and gets none; the plate is a mast attribute for the drawing, the crew and the export. Code today: `SignalMast` has no such attribute.
- **route** [Both]. A path through an interlocking from one signal to the limit beyond it: the switch positions it needs, the track circuits it uses and the signal that governs it. A tower's locking sheet states routes as locking; route locking holds them. FieldUnit: `Route`. In the interlocking model a route has `governedBy`, `aligns`, `clears`, `entrance` and `approaching` (`src/PlantSerializer.h:788` to `:891`).
  - How the routes of a mast are found (the owner, 2026-11-12). A route starts at a controlled mast, which is an absolute signal. It follows one of the paths through each switch that it meets, until it comes to the next signal that governs the same direction. The routes of a mast are all the combinations of the paths through those switches.
  - Where the interlocking's part ends. A route leaves the interlocking at its limit. From the limit to the next governing signal the track is a block. The state of that track and of that signal is what the `H` and `D` line facts tell the interlocking (see HD line; FieldUnit ADR 0007 D2). SPCoast: for a train that leaves Luchessa the next governing signals are Christopher's `816SA` (main track 1, south), Christopher's `814SABC` (main track 2, south) and Gilroy's `778NAB` (north) (the owner, 2026-11-12).
  - What the mast shows (the owner, 2026-11-12). Each route is evaluated for its least permissive item. The mast shows the most permissive result of its routes. FieldUnit: a route starts from its indication ceiling and takes the least permissive of its terms (`src/ControlTable.h:578-588`); a route whose switches are not lined gives Stop (`:610-612`); the mast takes the most permissive of its routes (`:512`, `:540`). What each head then shows is a table lookup (see head).
  - Layout model: no route is stored. A consumer derives the routes from the track, the joints and the switch legs. The shared derivation is FieldUnit-Subdivision issue 62, open on 2026-11-12. A walk by hand of Luchessa in the compiled SPCoast file (schema `layout-model/v3.2.0`) gave ten routes from five masts with no fact missing.
- **route locking** [Both]. Locking that holds the switches of a route while the signal is clear or a train is on the route. Each switch is released when its releasing track circuit clears (sectional release). FieldUnit: `SwitchLock::ROUTE_LOCKED`, `SectionState`.
- **signal indication** [Both]. The meaning of an aspect, as the rulebook states it. Examples: Stop, Restricting, Clear. Code name: enum `Indication`.
  - A number plate does not change the indication a mast presents; it changes the aspect the crew reads at Stop (see number plate).
- **time locking** [Both]. Locking that holds the route switches for a set time after a signal is restored to stop. FieldUnit uses one timer for approach locking and time locking (see approach locking).
- **traffic, direction of traffic** [Both]. The established direction of movement on a single-track or either-direction block (`RULE_261` / CTC) [CFR236 §236.769; CBQ1946; RS1942]. Occupancy of the block or an opposing cleared signal prevents changing the direction of traffic (**traffic locking**). On SP CTC, the signal levers and the `HD` line circuits through the track relays establish and hold traffic in the field without a separate traffic lever (`FieldUnit-Subdivision/docs/adr/0013-between-control-points.md` §16). Compare current of traffic (section 9).

## 9. Operating rules [Prototype]

- A rule number belongs to one rulebook and one era. Give the book with the number.
- **current of traffic** [Both]. The movement of trains on a main track in one direction, specified in the timetable (`RULE_251`) [PRR56 Definitions; CFR236 §236.734]. In the layout model, a `RULE_251` sign's `LEFT` or `RIGHT` states the lever direction of the signals that govern with the current of traffic (`FieldUnit-Subdivision/docs/adr/0013-between-control-points.md` §8). Compare traffic, direction of traffic (section 8).
- **timetable** [Prototype]. The document that authorizes the regular trains of a division, gives their schedules and superiority, and holds special instructions. Under timetable and train order operation it is one of the two sources of authority. (unverified)
- **timetable station** [Both]. A place on the line that the timetable names. A train order signal stands at one. (unverified: the Standard Code definition of station was not opened.)
  - Layout model: an entry of `layout.stations`, keyed by its Name. It is drawn as a `STRUCTURE` symbol of Kind `STATION` with its `Milepost`. `relations.interlocking` is the interlocking whose sheet it is drawn under (`FieldUnit-Subdivision/schemas/layout-model/v3.1.0.json`; the owner, 2026-11-03). The `Station` field of a train order board names one.
  - It is not a field station. A field station is an address on a code line. A timetable station is a place.
  - SPCoast: `Gilroy`, milepost 77.9, drawn under Gilroy Interchange.
- **train order** [Prototype]. A written order from the dispatcher to a train crew. The dispatcher sends it by telegraph (later by telephone) to the operator at a train order office. The operator copies it and delivers it to the crew. It adds to or changes the authority of the timetable. The first telegraphic train order is usually credited to Charles Minot on the Erie, 1851 (unverified). Compare train order signal, Form 19, Form 31.
  - What a train order does (the owner, 2026-11-09; unverified: no source was opened):
    - It authorizes an extra train: one that the timetable does not list.
    - It sets a meeting or passing point: it directs one train to take a siding for another.
    - It changes a schedule: it adds a section to a regular train, or it annuls a scheduled train.
    - It warns of a track condition: a slow order, a washout, a maintenance crew on the line.
    - With the timetable, it keeps two trains from holding the same piece of single track.
  - Why it existed (same source): the timetable is fixed in advance. When a train ran late or failed, a single track line needed a lawful way to change which train had the right to the track. Before two-way radio, the telegraph to a trackside office was the only way to reach a crew on the road.
  - Era (same source): from the 1850s until the late 20th century, when radio, track warrants and CTC replaced it.
  - The model does not cover train orders. It covers the train order signal and the timetable station.
- **Form 19** [Prototype]. A train order that the crew can take without stopping: the operator hands it up on a hoop and the crew takes it on the fly. The conductor does not sign for it (unverified). Compare Form 31.
- **Form 31** [Prototype]. A train order that the train must stop for: the conductor (on some roads the engineman too) signs for it before the operator gives the clearance. It is used for the more restrictive orders (unverified). Compare Form 19.
- SPCoast models 1942 to 1985 (title block). Use Standard Code numbers for SPCoast. Name GCOR only as the modern equivalent.
- The texts below are from one road: the PRR book effective October 28, 1956, with 1964 revisions [PRR56]. Another road's wording can differ.

| Rule | Text [PRR56] |
|---|---|
| 251 | "On portions of the railroad and on designated tracks so specified on the time-table, trains will run with reference to other trains in the same direction by block signals whose indications will supersede the superiority of trains." |
| 261 | "On portions of the railroad and on designated tracks so specified on the time-table, trains will be governed by block signals whose indications will supersede the superiority of trains for both opposing and following movements on the same track." |
| 262 | "A train for which the direction of traffic has been established must not move in the opposite direction without proper interlocking or manual block signal indication or train order." |
| D-151 | "Where two main tracks are in service, trains must keep to the right unless otherwise provided on the time-table." |
| D-152 | "When a train or engine crosses over to or obstructs a track where block signal system rules are in effect, the movement must be protected by the operator as provided by Rules 327 or 504, except where 605 is in effect. (Rev. 10-18-64)" The heading in the source reads "152." |
| 93 | Yard limits: see the yard limits entry [PRR56]. |
| 105 | SP Coast Division Rules & Regulations, effective 1943-02-15 (owner's transcription): "Before entering a siding or other track, it must be known that switch, and derail if any, is properly lined... All movements on sidings must be made with caution." Later editions: at restricted speed. GCOR 6.28 is the modern equivalent. KiCad symbol `Rule105-RestrictedSpeed` (`RULE_105`). |

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
- A dependent derail is a derail whose gang id matches a switch or lock gang; by convention `<id>D` (ADR 0003 D8, proposed).
- The default OS track circuit name is `<switch>T1`. A `TC` field overrides it. AAR56 uses `<number>T`.
- A maintainer call is named with its number as a prefix, AAR style: `1MC`, with tokens `1MCS` and `1MCK`. The number comes first, as for any AAR name (section 10.1) and for a gang (`3`, `3A`). AAR's own tokens carry no number. The library symbols build the name with the macro `Name=${Value}MC` and the tokens with `Control.MCS` and `Indication.MCK`. A user may change the macro or those fields to name it another way.
- Names are produced with case preserved and compared without case.
- One grammar for all names is proposed in `FieldUnit-Subdivision/docs/adr/0003-name-grammar.md`. It is not adopted.

### 10.3 Letters

AAR letters [AAR56 p. 32]. A name uses one meaning of each letter.

| Letter | Meanings |
|---|---|
| A | approach |
| B | block [AARNOM46] |
| D | proceed indication, detector, decoding, dragging |
| E | east |
| F | traffic [AARNOM46] |
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
| `MC` | maintainer call | AAR Signal Manual Parts 33 and 91 per the owner (unverified; ADR 0003 D12). The instance number in front, `<n>MC`, is FieldUnit. |

FieldUnit tokens by appliance:

| Appliance | Controls | Office indications |
|---|---|---|
| switch | `NWS`, `RWS` | `NWK`, `RWK` (asserted only in switch correspondence) |
| signal | `NGS` (LEFT), `SGS` (RIGHT), `HS` (stop) | `NGK` (LEFT), `SGK` (RIGHT), `TEK` (time locking runs) |
| track circuit | none | `<name>K` (asserted = occupied) |
| electric lock | `WLS` (release) | `WLK` (asserted = released; deasserted = locked, fail-safe), `WLQK` (asserted = crew requests release; non-vital) |
| maintainer call | `<n>MCS` | `<n>MCK` |

On the AAR text code line, an asserted token is written `TOKEN`. A dropped token is written `(TOKEN)`.

Some references describe the lock indication as "locked when 0" and others as "unlocked when active"; both describe the same fail-safe relay, energized to unlock. This glossary says only asserted and deasserted, and names what the asserted state means, as in the table above.

### 10.4 Contacts and circuits [Both]

- **back contact**. Closed when the relay is de-energized (dropped). Software: `!x`. Sketch symbol: `[/]`.
- **front contact**. Closed when the relay is energized (picked up). Software: `x`. Sketch symbol: `[  ]`.
- **parallel contacts**. Paths that branch around each other. Software: `||`.
- **relay** [Both]. An electrically operated switch. Current in the coil picks up the armature and closes the front contacts. Without current, gravity drops the armature and closes the back contacts. A vital relay is built so that a failure drops it. FieldUnit: a relay is a boolean in the interlocking logic (section 10.6).
- **series contacts**. Contacts in one path. Software: `&&`.
- **slow-release relay, slow-pickup relay** [Prototype]. A relay built to drop, or to pick up, a short time after its coil current changes. On a code line it tells a long pulse from a short one. (unverified)
- **stepping relay** [Prototype]. A relay that advances one position for each pulse, so that both ends of a code line count the steps of a code together (see section 6). (unverified)
- **stick circuit**. A circuit in which a relay's own front contact holds its coil energized after the pickup path opens.

The sketches below are illustrative. They are not prototype circuit plans.

### 10.5 Relays

- **`ASR`** [Both]. Approach stick relay. It enforces approach locking.
  - FieldUnit: `SignalControl::ASR()` is true when no time locking runs. It is false only while the timer runs.
  - FieldUnit difference: `ASR()` does not drop when a signal clears. Route locking holds the switches while the signal is clear.

```cpp
bool plantFree = sig.ASR(); // false while the time-locking timer runs
```

- **`BPR`** [Prototype]. Block repeater relay, repeating the track circuits in a block [AARNOM46 p. 3]. In the layout model, a `virtual` track circuit whose `Members` are the station-to-station block's circuits plays this role (`FieldUnit-Subdivision/docs/adr/0013-between-control-points.md` §7.2).
- **`FR`, `FSR` (traffic)** [Prototype]. Traffic relay and traffic stick relay [AARNOM46 p. 4; AARTC71 Dwg 8066, 8082; CBQ1946]. Not a collision with FieldUnit's fleet stick relay `FSR` below: AAR abbreviations are not unique across domains (`FieldUnit-Subdivision/docs/adr/0013-between-control-points.md` §21).
- **`FSR` (fleet)** [FieldUnit]. Fleet stick relay. FieldUnit name. `SignalControl::FSR()` is true while fleeting is on. The signal then clears again when the route is clear, without a new control.
- **`HR`, `DR`** [Both]. Relay names with H (home, approach indication) and D (proceed indication). D does not mean "distant" on the AAR list. FieldUnit has no `HR` or `DR` object: both are evaluated, not stored. `InterlockingEngine::evaluateRouteIndication` and `evaluateMastIndication` are the equations as functions; a value above the Restricting tier is `HR` up, Clear is `DR` up. Section 10.6 gives the circuits that the logic follows.
- **fact** [FieldUnit]. A named vital boolean (`Fact`) that a route requires with `home()` or `proceed()`, or that a signal location reads. Sources: a line input; a track circuit (`TrackCircuit` is a `Fact`, asserted when clear: the `TR`); the `H` and `D` an automatic signal produces for the location in rear; the `H`, `D` and `F` a control point line produces at the seam (FieldUnit ADR 0008 D2, D3, D5). `InterlockingPlant::findFact()` resolves a name in that order. Names bind when the configuration call is made: add every source before a binding names it. A name with no source binds to an implicit line input and is reported (FieldUnit ADR 0009 D1, D3); see **implicit fact**.
- **implicit fact** [FieldUnit]. A line input the plant declares itself when a `home()`, `proceed()`, `governs()`, `approach()`, `block()` or `reads()` name has no source (FieldUnit ADR 0009 D1). Released until something drives it with `update()` and `touch()`, stale like a wire, so the term it stands in is Stop. Derived, not given: `LineInput::isImplicit()`, not written by the serializer, re-derived on read (D4). A *ghost* (the emulator, Studio, a test) drives it through the same two verbs a wire uses; the ghost's heuristics are its own, never the plant's (D5). No fact stands in for an appliance: a route or location that names a track circuit, switch, signal control or mast the plant lacks is incomplete and at Stop (D2). The plant reports every gap (`unresolved(i)`, `isComplete()`) and takes no gate; whether a deployment refuses to drive hardware while incomplete is the sketch's policy (D3, D6).
- **line input** [FieldUnit]. The consumer end of a line circuit: a named fact (`LineInput`, a `Fact`) that a route requires with `home()` (an `HR` term) or `proceed()` (a `DR` term). At a block end the facts are `segment_clear` (`H`), `traffic_free` (`F`) and `next_proceed` (`D`) of the interlocking–block seam (FieldUnit-Subdivision ADR 0013 §13). State and liveness are separate verbs: `update(value, nowMs)` reports a change, `touch(nowMs)` reports "still watching, no change" and is inert before the first update. Released until first set and released again when neither arrives within the staleness timeout; a timeout of 0 marks an in-memory fact (FieldUnit ADR 0007 D2). A line input that states no timeout takes the interlocking's default (`defaultStalenessTimeoutMs`; the library default `DEFAULT_STALENESS_TIMEOUT_MS` is 5000 ms, overridable at compile time with `FIELDUNIT_DEFAULT_STALENESS_TIMEOUT_MS`; FieldUnit ADR 0009). The right value is the carrier's: a timeout shorter than the code line's full cycle releases a healthy line, and CTC-era machines took 10 to 30 s for one station's control message while C/MRI hosts poll one to five times a second (the owner, 2026-11-02; ADR 0009 answer 3). The plant declares one itself for a name with no source: an **implicit fact**. `TrackCircuit` has the same two verbs.

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

- **`RGPR`** [Prototype]. Relay repeating a signal mechanism at stop (red repeater) [AARNOM46; RS1945].
- **`SR`, `ESR`, `WSR`, `NSR`, `SSR`** [Both]. Directional stick relay [AARNOM46; AAR56 p. 34; CBQ1946 p. 615; RS1927, RS1944]. At an intermediate signal location, a mast's `SR` picks up when a train passes the mast in its governing direction (the circuit beyond becomes occupied while the mast shows proceed, before slow-release `HR` drops) and sticks through its own front contact and `HR` back contact until `H` from ahead is asserted again, allowing following moves behind a receding train (`FieldUnit-Subdivision/docs/adr/0013-between-control-points.md` §15.3). FieldUnit has no object for it; FieldUnit ADR 0007 D3 puts it in the automatic signal location class.
- **`TP`, `TPR`** [Both]. Track repeater relay. It repeats one track relay to give more contacts, or it is fed through the front contacts of several track relays in series, so that it is picked up only when every circuit is clear. The second form is the occupancy of a block (see block, aggregation). FieldUnit has no object for it yet; the block entity will carry this role.
- **`TR`** [Both]. Track relay, one for each track circuit. In the model it is one input bit; sensors wired in parallel to that input are inside its circuit and are not model objects. It is energized (picked up) when the rails are not shunted. It drops when wheels and axles shunt the rails. A broken rail or a power loss also drops it.

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

This section stops at the interlocking–block seam (FieldUnit-Subdivision ADR 0013 §13). The seam ports map onto these relays as follows (FieldUnit ADR 0007): `requested` is the control as held; `cleared` is the leaving signal's `HR` front; `in_time` is its `ASR` back; `home_proceed` is the `HR` front of the home signal facing the block; `segment_clear` (`H`) feeds the leaving signal's `HR`; `next_proceed` (`D`) feeds its `DR`; `traffic_free` (`F`) is a further `HR` term that no directional stick bypasses. Between plants, ADR 0013 Part 2 governs. Code: `ControlPointLine` (FieldUnit ADR 0008 D5) produces `H`, `D` and `F` out from these ports, with the ADR 0013 §14.2 `blockRules` axes as parameters; a route's `home()` and `proceed()` consume `H`, `F` and `D` in.

- **`HR`** (home relay). It picks up when the signal control has the direction, every route switch is in switch correspondence, every route track circuit is clear, and the opposing signals are held at stop. With `HR` up the signal can show at least Approach.
- **`DR`**. It picks up when `HR` is up, the block beyond is clear and the next signal is not at stop. With `DR` up the signal shows Clear. (The AAR list reads D as "proceed indication", not "distant".)

```
  HR:  ──[ 2HSR front ]──[ route KR fronts ]──[ route TR fronts ]──[ opposing ASR fronts ]──( 2HR )

  DR:  ──[ 2HR front ]──[ advance TR front ]──[ next signal HR front ]──( 2DR )
```

  - Code today: `InterlockingEngine::evaluateIndication` has the `HR` conditions, including a route's `home()` facts, and the `DR` condition as the `proceed()` fact (FieldUnit ADR 0007 D2, ADR 0008). The next-signal front is the `D` the location ahead produces: a line input when that location is on another processor, an `AutomaticSignal` or `ControlPointLine` output when it is local. No mast is read; `D` is the location's `HR` as a function of its block and its own line from ahead. The "advance `TR` front" is the route's own track circuits for a local next signal and part of `H` for a remote one. Clear is also still lowered to Approach from the `approaching()` track circuit (issue #19).

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
| compiler (alone), Data Model compiler, layout compiler, milestone D compiler, seam reader | KiCad importer (what reads the drawing), reference derivation (what computes `derived`), or plant compiler and controller compiler (the proof-of-concept tools). The code names `tools/layout_compiler` and `compile_layout.py` keep their spelling. |
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
| CPL (for a multi-lamp head with one lamp per color) | color light signal; CPL is the B&O color-position light signal |
| CP Boundary, control point limits | interlocking limits (a control point has no limits of its own) |
| cTc, cTc machine (in prose) | CTC machine |
| detection block | track circuit |
| distant (as the meaning of D), Distant Relay | D: proceed indication, detector, decoding |
| Exit Block | track circuit |
| field controller | field unit |
| unit (alone, for a FieldUnit or an OfficeUnit) | FieldUnit, OfficeUnit; a unit is a KiCad symbol unit |
| Form 504, Form 506, Form 508, Form 510, Type L | US&S 506 time code; the others are not sourced |
| host, plant host (for what runs a FieldUnit or OfficeUnit) | processor; for the CMRInet master, the processor that anchors the line (CMRInet Host) |
| node (alone, for a computer) | processor; node (CMRInet) is CMRInet Node |
| chip (for a driver) | driver |
| cast (for a head device) | head device |
| relay house, shelter, instrument case | house; bungalow for a walk-in house, case for a cabinet |
| indication (alone) | signal indication or office indication |
| indication snapshot | indication vector |
| Indications-as-Truth | office indication |
| interlocked function | vital control |
| Interlocking Plant (meaning the logic) | interlocking logic |
| interlocking frame | interlocking machine |
| island, Island Block (for a switch section) | OS section |
| Kontrol (for K) | indicator |
| leverman, towerman, TowerMaster | tower operator |
| line signal | HD line |
| line station, Line Station | field station |
| mnemonic | token |
| Model 503, Model 506, Type 506 machine, US&S506 machine | the SPCoast CTC machine (US&S style) |
| neutral contact (for front contact) | front contact |
| Non-Vital Command, Non-Vital Circuit (for a code line function) | non-vital control |
| Occupied Section, On-Sheet Section | OS section |
| out of correspondence (alone) | out of switch correspondence or out of office correspondence |
| panel (for the whole machine) | CTC machine |
| passed memory | SR (directional stick relay) |
| position-color light (for the B&O system) | color-position light signal |
| plant (alone) | interlocking plant, interlocking model or interlocking logic |
| Send (for S) | control (FieldUnit final S) or stick (AAR S) |
| settings (for the fields stated on a symbol) | given |
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
| AARNOM46 | AAR Signal Section, *Circuit Nomenclature, Written Circuits and Graphical Symbols* (Manual Part 33, 1943; complete to Oct 1946). `docs/reference/AAR. Signal Section. Circuit nomenclature, written circuits, and graphical symbols. 1946-10.pdf`. |
| AARTC71 | AAR Communication & Signal Section, *Typical Circuits Representing Current Practice for Railway Signaling*, compiled 1971, jonroma.net. |
| ARTC23 | ARTC SCP 23, extranet.artc.com.au/docs/eng/signal/procedures/design/SCP23.pdf |
| BNSF | BNSF Railtalk, "ABCs", bnsf.com/news-media/railtalk/heritage/abcs.html (paraphrase) |
| CBQ1946 | Zane, W. F., "New Signal Control Line Circuits in Centralized Traffic Control Territory on the Burlington", *Railway Signaling*, Sept 1946, pp. 612–616, jonroma.net. |
| CFR218 | 49 CFR 218.109, law.cornell.edu (paraphrase) |
| CFR236 | 49 CFR Part 236 (including §§236.405, 236.734, 236.769, 236.782), ecfr.gov. |
| EKEVING | "1927: Dispatcher Control on the New York Central", ekeving.se/ctc/us/NYC_1927.html (paraphrase) |
| GCOR6 | General Code of Operating Rules, 6th edition, 2010, fobnr.org copy |
| NORAC9 | NORAC Operating Rules, 9th edition, 2008, rail.pgengler.net copy |
| PRR56 | PRR rulebook effective October 28, 1956, with revisions to 1964, redoveryellow.com copy |
| RRS506 | rrsignal.com/railroad/ctc/uss506.htm |
| RRS514 | rrsignal.com/railroad/ctc/uss514.htm (paraphrase) |
| RS1927 | Railway Signaling, July 1927, pp. 269–270, "Various Circuits for Directional Control of Single Track Signals", jonroma.net |
| RS1937 | Railway Signaling, October 1937, "Ends of Double Track Controlled by CTC", jonroma.net |
| RS1942 | Railway Signaling, October 1942, pp. 538–545, "C.T.C. Saves Train Time on a Section of the Southern Pacific", jonroma.net |
| RS1944 | Railway Signaling, 1944, "Coded Track Circuits for CTC and Cab Signaling", jonroma.net |
| RS1945 | Railway Signaling, February 1945, pp. 94–99, "Southern Pacific Installs CTC on 72 Miles of Single Track", jonroma.net |
| RS1947 | Railway Signaling, February 1947, p. 92, "Southern Pacific Installs C.T.C.", jonroma.net |
| RS1959 | Railway Signaling, July 1959, "Pushbutton console for CTC", jonroma.net |
| RS1960 | Railway Signaling and Communications, 1960, "Fourteen RRs adopt new operating code", jonroma.net |
| USS1949 | Farrington, Union Switch & Signal history, 1949, utahrails.net |

The full verdicts and quotes are in `FieldUnit-Subdivision/docs/archive/vocabulary-sources.md`.
