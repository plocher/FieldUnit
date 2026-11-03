# Glossary Terms by Spine Segment

Draft 5, 2026-11-03 (13-segment spine; 22 new glossary entries assigned; yard limits moved to segment 8; signal type names corrected).
This list is for the textbook. The talk does not recite it. Deliverable 2 of
`docs/plan/user-docs-plan.md`; companion to `docs/plan/talk-outline.md`.

Each entry of `docs/GLOSSARY.md` is assigned to exactly one segment of the spine: the segment where the reader first
needs it. A term is not assigned to a segment whose era it does not belong to (for example, powered switch is a CTC
term, not a power-interlocking term). Terms are spelled as the glossary spells them.

- **Talk** = yes: the term is said aloud in that segment of the talk. The other terms of the segment belong to the
  textbook chapter (deliverable 4).
- **§** is the glossary section that defines the term.
- Section 10.6 restates `HR`, `DR` and `ASR` as design principles. They are the same terms as in section 10.5 and are
  assigned once. `ERS` is defined only in section 10.6.
- Section 11 (retired and synonym terms) is not assigned: a retired term is never introduced.
- The tables of sections 2.1 to 2.5, 6 (the 506 facts and step table), 9 (rule rows) and 10.1 to 10.3 (name structure,
  letters, token parts) are not entries. Where they carry teaching content, the segment that uses them is noted under
  "Glossary sections that are not entries".

Count: 163 entries in the glossary (160 distinct terms, since 10.6 restates three), after the 22 entries added on
2026-11-03 (marked "new" below). 145 terms are assigned to segments 1 to 13; 15 are reference-only.

## 1. The switch tender

| Term | § | Talk | Note |
|---|---|---|---|
| switch | 7 | yes | |
| manual switch | 7 | yes | Styles `manual`, `manual_sensed` only; `powered` and `dual` are CTC (segment 10) |
| derail | 7 | yes | |
| fouling point | 7 | yes | |
| timetable | 9 | yes | new |
| train order | 9 | yes | new |
| Form 19 | 9 | yes | new; orders on the fly, the hoop |
| Form 31 | 9 | yes | new; stop and sign |
| points | 7 | yes | new |
| switch stand | 7 | yes | new |

## 2. Fixed signals, the strongarm era

| Term | § | Talk | Note |
|---|---|---|---|
| signal | 7 | yes | |
| mast | 7 | yes | |
| head | 7 | yes | |
| aspect | 8 | yes | |
| signal indication | 8 | yes | |
| semaphore signal | 7 | yes | the arm, worked by wire; lower quadrant; upper quadrant returns in segment 6 |
| fixed signal | 7 | yes | new |
| ball signal | 7 | yes | new |
| train order signal | 7 | yes | new; not a block signal; KiCad symbol and layout model Appliance `train_order` exist (2026-11-03) |

## 3. The tower and the lever frame

| Term | § | Talk | Note |
|---|---|---|---|
| interlocking | 4 | yes | |
| interlocking machine | 4 | yes | |
| tower | 4 | yes | |
| tower operator | 4 | yes | |
| lever | 5 | yes | |
| interlocking plant | 3 | yes | |
| interlocking limits | 4 | | |
| crossover | 7 | | |
| dependent derail | 7 | | |
| independent derail | 7 | | |
| LEFT, RIGHT | 7 | | direction of a signal lever and a route |
| appliance | 3 | | the things the levers work |
| route | 8 | yes | new |

## 4. Track circuits: electricity arrives

| Term | § | Talk | Note |
|---|---|---|---|
| track circuit | 7 | yes | |
| `TR` | 10.5 | yes | |
| OS section | 7 | yes | "on sheet" |
| detector locking | 8 | yes | here only "switch may move while its OS `TR` is up" |
| approach track circuit | 7 | yes | |
| aggregation | 7 | | not a term of its own; the modeler's sidebar |
| insulated joint | 7 | yes | new |

## 5. Power interlocking

| Term | § | Talk | Note |
|---|---|---|---|
| switch correspondence | 7 | yes | |
| route locking | 8 | yes | |
| approach locking | 8 | yes | |
| time locking | 8 | yes | wall timers in towers, time element relays in relay plants |
| vital | 5 | yes | |
| `KR` | 10.5 | yes | one sentence, no circuit |
| `HR`, `DR` | 10.5 | yes | `HR` said; `DR` textbook; also 10.6 |
| `WLR` | 10.5 | yes | the full switch lock, with route and time locking |
| `HSR` | 10.5 | | |
| `ASR` | 10.5 | | also 10.6 |
| `NWCR`, `RWCR` | 10.5 | | |
| `RGPR` | 10.5 | | low confidence |
| indication ceiling | 8 | | FieldUnit term |
| switch machine | 7 | yes | new |
| relay | 10.4 | yes | new |

## 6. Automatic block: ABS, then APB

| Term | § | Talk | Note |
|---|---|---|---|
| block | 7 | yes | |
| automatic signal | 7 | yes | |
| signal location | 7 | yes | |
| cut section | 7 | yes | |
| traffic, direction of traffic | 8 | yes | APB |
| current of traffic | 9 | yes | ABS on double track, later Rule 251 |
| color light signal | 7 | yes | named, not stressed |
| position light signal | 7 | yes | named, not stressed |
| color-position light signal | 7 | | the B&O system (corrected 2026-11-03) |
| position-color light signal | 7 | | owner's description, unverified |
| station-to-station block | 7 | | |
| HD line | 7 | | |
| boundary track circuit | 7 | | FieldUnit term |
| `SR`, `ESR`, `WSR`, `NSR`, `SSR` | 10.5 | | |
| `TP`, `TPR` | 10.5 | | |
| `BPR` | 10.5 | | |
| `FR`, `FSR` (traffic) | 10.5 | | |
| ABS | 4 | yes | new |
| APB | 4 | yes | new |
| head block | 7 | yes | new |

## 7. Train control (context only)

| Term | § | Talk | Note |
|---|---|---|---|
| train control | 4 | yes | new |
| ATS | 4 | yes | new |
| ATC | 4 | yes | new |
| cab signal | 4 | yes | new |
| coded track circuit | 7 | yes | new |

## 8. CTC: meets without stopping

| Term | § | Talk | Note |
|---|---|---|---|
| CTC | 4 | yes | |
| code line | 3 | yes | |
| control | 5 | yes | |
| office indication | 5 | yes | |
| field station | 3 | yes | the address on the code line |
| front contact | 10.4 | yes | the relay dive |
| back contact | 10.4 | yes | |
| stick circuit | 10.4 | yes | |
| series contacts | 10.4 | | |
| parallel contacts | 10.4 | | |
| office | 3 | | |
| field | 3 | | |
| CTC limits | 4 | yes | where the dispatcher's authority ends; (unverified) in the glossary |
| yard limits | 4 | yes | where the dispatcher's authority ends; Rule 93 |
| slow-release relay, slow-pickup relay | 10.4 | yes | new |
| stepping relay | 10.4 | yes | new |
| step | 6 | | |
| US&S 506 time code | 6 | | |
| US&S 500-series time code systems | 6 | | |
| code cycle | 6 | | |
| code chart | 6 | | |
| conditioning step | 6 | | |
| station selection step | 6 | | |
| function step | 6 | | |
| delivery step | 6 | | |

## 9. CTC, the office: the dispatcher's desk

| Term | § | Talk | Note |
|---|---|---|---|
| CTC machine | 3 | yes | |
| dispatcher | 3 | yes | |
| panel column | 3 | yes | |
| code button | 5 | yes | |
| office correspondence | 5 | yes | |
| subdivision | 3 | yes | |
| track indication lamp | 5 | | |
| fleeting | 8 | | |
| `FSR` (fleet) | 10.5 | | |
| maintainer call | 5 | | |
| maintainer | 3 | | |
| power-off indication | 5 | | |

## 10. CTC, the field: the control point

| Term | § | Talk | Note |
|---|---|---|---|
| control point | 4 | yes | |
| bungalow | 3 | yes | |
| powered switch | 7 | yes | remotely controlled by the dispatcher |
| vital control | 5 | yes | |
| electric lock | 7 | yes | |
| dual-control switch | 7 | yes | |
| house | 3 | | |
| non-vital control | 5 | | |

## 11. Digital code lines

| Term | § | Talk | Note |
|---|---|---|---|
| code line type | 3 | yes | |
| encoding | 3 | yes | |
| transport | 3 | yes | |
| token | 5 | yes | |
| control transaction | 5 | yes | |
| indication vector | 5 | yes | |
| function | 5 | | |

## 12. PTC (context only)

| Term | § | Talk | Note |
|---|---|---|---|
| PTC | 4 | yes | new; history only |

## 13. Your layout, and what comes next

| Term | § | Talk | Note |
|---|---|---|---|
| interlocking model | 3 | yes | |
| interlocking logic | 3 | yes | |
| interlocking application | 3 | yes | |
| field processor | 3 | yes | |
| field unit | 3 | yes | |
| processor | 3 | | |
| device interface | 3 | | |
| bus, IOBus | 3 | | C/MRI hosting, MQTT appliances |
| driver | 3 | | |
| CMRInet Node | 3 | | |
| local I/O, remote I/O | 3 | | |
| head device | 3 | | |
| seam | 3 | | also the peer brief (deliverable 5) |
| engine return | 8 | | FieldUnit route mode |
| `ERS` | 10.6 | | project name |

## Reference-only (no segment)

| Term | § | Why |
|---|---|---|
| conformance suite | 2.6 | Layout Data Model File internals; internal-developer set, out of scope for this tranche |
| derivation rules | 2.6 | same |
| derived | 2.6 | same |
| given | 2.6 | same |
| incomplete, in error | 2.6 | same |
| KiCad importer | 2.6 | same; a tutorial (hands-on path 2) may name it |
| Layout Data Model File | 2.6 | same; the peer brief may name it |
| proof-of-concept compilers | 2.6 | same |
| reader policy | 2.6 | same |
| reference derivation | 2.6 | same |
| relations | 2.6 | same |
| Studio | 2.6 | same; a planned tool, named in tutorials when it exists |
| writer policy | 2.6 | same |
| port | 3 | a KiCad sheet connection; tutorial and peer-brief material |
| control point limits | 4 | not a term, by the owner's decision; the entry exists to send readers to interlocking limits |

None of these is a gap: they belong to the reference, the peer brief or the tutorials, not to the history spine.

## Glossary sections that are not entries

| Section | Content | Used in segment |
|---|---|---|
| 2.1 | three roles and one seam | 8 (roles), 11 (code line type row) |
| 2.2 | facets of one place | 10 (control point as a function, interlocking as a mechanism) |
| 2.3 | tower interlocking and CTC control point | 9 (the free lever) and 10 (the field refuses) |
| 2.4 | six terms from interlocking plant to field unit | 13, the closing equation |
| 2.5 | cardinality | reference |
| 6 (facts, step table) | 506 steps and the hearsay step assignment | 8, the stepping code line; the step table is hearsay and stays out of the talk |
| 9 (rule rows) | 105, D-151 → segment 1 (future tense); 93 → segment 8; 251, 262, D-152 → segment 6; 261 → segment 8 | as listed |
| 10.1 to 10.3 | AAR name structure, letters, FieldUnit token parts | 5 (letters, with the relay names); 11 (token parts) |

## Gaps: terms the spine uses that the glossary still does not define

The 22 terms that talk outline draft 3 marked in italics now have entries (2026-11-03). These remain, used inside
other entries or in the talk without an entry of their own:

| Segment | Missing term | Where it is used today |
|---|---|---|
| 1 | frog, superiority of trains | plan, segment 1; rule texts in section 9 |
| 2 | distant signal, governing direction, govern | inside signal location, cut section, HD line |
| 2 | Stop and Proceed, Restricting, Approach, Clear (as indications) | inside automatic signal, signal indication |
| 3 | locking bed, mechanical locking, locking sheet, pipe and wire | inside interlocking machine and interlocking; talk segment 3 |
| 3 | home signal, absolute signal, controlled signal | inside interlocking limits, control point, automatic signal |
| 4 | detector bar | talk segment 4 |
| 5 | pick up, drop, electro-pneumatic | section 10 throughout; talk segment 5 |
| 6 | number plate, route signaling, speed signaling, searchlight | inside automatic signal and color light signal; talk segment 6 |
| 8 | line relay, pulse (long, short), End CTC sign | talk segment 8 |
| 9 | model board | `docs/AAR_SIGNALING_PRIMER.md` section 9.6; talk segment 9 |
