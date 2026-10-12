# 0006. The dual-control switch

- Status: proposed (shaped with the owner, 2026-10-11)
- Date: 2026-10-11
- Builds on: FieldUnit-Subdivision ADR 0004 (the `WLQK` sequence D1 to D5); the I/O design checkpoint 2026-10-10 (E63); `docs/GLOSSARY.md` entries powered switch, manual switch, dual-control switch, electric lock.
- Order of authority: the relay model of interlocking logic and AAR practice (`docs/GLOSSARY.md` section 10.6) govern; where the code differs it has a defect.

Terms follow `docs/GLOSSARY.md`.

## Decisions requested

- D1. A dual-control switch is a powered switch plus a release, drawn as the plant symbol `Switch_Dual` (Style `dual`), served by `Device-Switch` and `Device-Lock` with a `Local-Switch` plate. Default: yes.
- D2. Hand operation in the model is the local lever driving the switch machine while the release is granted. Default: yes; a `~{HAND}` selector contact is added to the device only when a layout has one.
- D3. The release follows the electric lock exchange of ADR 0004 D1 to D5: the crew requests, the unit grants when every signal governing the switch is at stop and no time locking runs, the dispatcher sees the request and the release. Default: reuse the `WLQK`, `WLS` and `WLK` tokens and the lock lever plate, since the desk already shows them; an alternative is a dual-specific token set, rejected as a second mechanism for one exchange.
- D4. While released: the dispatcher's switch control is ignored, `NWK` and `RWK` are both deasserted (out of correspondence), and every signal governing the switch is held at stop. When the crew returns the switch to power operation the release is withdrawn and correspondence returns from the point detection. Default: yes.
- D5. Era: dual control appears with power switch machines; the era indication tables carry it like every other era fact. Default: no era rule in the logic.

## Context

The owner's comparison (2026-10-11), paraphrased. Three kinds of switch from the dispatcher's seat. A power-operated switch is driven by a switch machine directly under the dispatcher's control and shown NORMAL or REVERSE. An electrically locked switch is a manual switch with an electric lock tied to the signal system: the dispatcher sends a release, the crew unlocks and throws it by hand, and the display shows locked or released, then NORMAL or REVERSE once thrown. A dual-control switch is a power switch with a clutch and a hand lever: the dispatcher works it remotely in normal service; when it fails or is out of correspondence, the dispatcher gives the crew authority, the crew moves the selector from MOTOR to HAND, which disconnects the dispatcher, and throws it by hand. SPCoast has powered and electrically locked switches today, and needs dual control.

## Consequences

- The Style word `dual` joins `powered`, `manual`, `manual_sensed`, `lock` and `derail`. The device signature does not change.
- The vital logic gains one state on a powered switch, released, with the rules of D4; the lock sequence is shared with the electric lock, not duplicated.
- Open: whether a real selector contact, when present, replaces the request (the crew's act of selecting HAND is the request) or confirms it.
