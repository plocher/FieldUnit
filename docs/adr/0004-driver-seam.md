# 0004. The driver seam: what a device driver may see, and the two contexts

- Status: proposed (shaped with the owner, 2026-10-07 and 08)
- Date: 2026-10-08
- Builds on: ADR 0003 (field I/O symbols); FieldUnit-Subdivision ADR 0004 (heads as appliances, D6; field units, D13).
- Order of authority: the relay model and AAR practice (`docs/GLOSSARY.md` §10.6) govern; drivers sit below the vital logic.

Terms follow `docs/GLOSSARY.md`.

## Decisions requested

- R1. A driver renders the vital result for its own appliance, or reports a raw observation, and nothing else. It never reads interlocking state (routes, locks, other appliances). Default: yes.
- R2. A driver has three inputs: params (a transition of its own appliance's vital value), the appliance context (this symbol instance), the layout context (settings shared by the layout). Default: yes.
- R3. Params are always a transition, `{previous, current, sinceMs}`, of one value type per appliance family. Default: yes.
- R4. The Kind's driver owns the meaning of every appliance setting, polarity and direction included. In `begin()` it configures its channels through the IOBus; afterwards it works in asserted / not-asserted terms. Default: yes.
- R5. The IOBus offers capabilities and configuration: it applies a request in the chip where the chip can, in software where the operation is generic, and refuses what it cannot do. Default: yes.
- R6. A pin's direction comes from the drawing: the device or panel pin's KiCad electrical type says what that symbol does to its wire, and a programmable expander pin is configured as its complement. Default: yes.
- R7. The appliance identity (name and place, e.g. the middle head of a three-head mast) is opaque to drivers and only passed through to library calls. Default: yes.
- R8. `getHeadAppearance` is a pure library function, one overload per head technology, returning that technology's own head appearance type (a head shows a head appearance; the aspect is all of a signal's heads together, glossary §8). Default: yes.
- R9. The layout context is one namespaced dictionary of typed keys, resolved plant → layout → era profile → library default, plus runtime keys; its vocabulary is fixed now and the dictionary grows. Its registry lives in FieldUnit-Subdivision (`schemas/context/keys.toml`) until the data-model schema moves into FieldUnit. Default: yes.
- R10. Lifecycle: `begin` (configure), `drive` / `sample` (execute), `end` (release). Before `end`, the field unit renders the appliance's restrictive state; the driver only renders it. Default: yes.

## Context

ADR 0003 gave every field device symbol a `(Role, Kind)` and a pin set; the generator was to emit one constructor call per device. A constructor per row makes every row its own API: tests and generators special-case each one. A first sketch of a richer head driver let it read interlocking state, which would let a driver second-guess the vital logic. And the field library typed its pins from the expander's side, the opposite of the desk library and of KiCad's own meaning. This ADR fixes one surface for every driver, with nothing vital on it, and says where each fact about a channel lives.

## Three seams, three deployments

The office unit (the CTC machine's logic) and the field unit can each run in three places:

| | Deployment | SEAM-1: hardware I/O | SEAM-2: remote I/O link |
|---|---|---|---|
| A | virtual | `MockIOBus` (simulated devices) | none |
| B | firmware on a processor near the devices | I²C expanders, `digitalWrite()` | none |
| C | a host application | on the remote I/O processor | CMRInet (IB / OB), MQTT, … |

SEAM-0, the code line, connects office and field units whatever their deployment. A driver talks only to an `IOBus`; which deployment it runs in is the IOBus's business (`MockIOBus`, `I2CexpanderIOBus`, `CmriIOBus`, `MqttApplianceBus`). In C, input ports feed IB and OB feeds output ports: the same directions, carried over SEAM-2.

## The seam

```cpp
template <class T>
struct Transition {          // R3
    T previous;
    T current;
    uint32_t sinceMs;        // when current took effect
};

template <class T>
class ApplianceDriver {      // R10
public:
    virtual ~ApplianceDriver() = default;
    virtual void   begin(const ApplianceContext& a, const LayoutContext& l, IOBus& io) {}       // configure
    virtual void   drive(const Transition<T>& p, const ApplianceContext& a, const LayoutContext& l, IOBus& io) {}
    virtual Sample sample(const ApplianceContext& a, const LayoutContext& l, IOBus& io) { return {}; }
    virtual void   end(const ApplianceContext& a, IOBus& io) {}                                  // release
};
```

| Family (first part of the Kind, ADR 0003 D2) | `T` | Direction |
|---|---|---|
| `HEAD_*` | `Indication` of the head's signal | drive |
| `SWITCH_*` (device) | the demand the `Switch` has approved (`N`, `R`) | drive; sensed Kinds also sample point position |
| `LAMP_*` (device), `PANEL` lamps | the state of the token shown (off, on, flashing; a request lamp: off, waiting, steady) | drive |
| `DETECTOR_*`, `INPUT_BIT` | none | sample |

The field unit owns all decisions: it computes each appliance's vital value, hands the transition to that appliance's driver, and takes `Sample`s in as observations it then judges. A driver never sees another appliance.

### The appliance context (per symbol instance; from the drawing)

- channels: for each pin, the driver chip, the bit or channel, and the pin's direction (R6).
- appliance settings, defined by the Kind's `(Role, Kind)` row: polarity per BIT pin (`~{X}` or the instance override, ADR 0003 D6), `NormalIs`, `Hold_ms`, angles, a lamp's `Color`, … A row may name a layout key as a setting's default (`Hold_ms` defaults to `timing.hold_ms`) or the table its value is looked up in (`Color` in `color.palette`).
- `identity`: opaque (R7).

### The layout context (R9)

| Namespace | Holds | Set at |
|---|---|---|
| `era.*`, `rulebook.*` | the era and rulebook represented; which aspect each signal indication takes (Approach yellow or lunar…) | layout, plant |
| `timing.*` | flash period; layout defaults for hold, travel, fade | profile, layout |
| `motion.*` | layout defaults for servo motion | profile, layout |
| `color.*` | named colours to values, era-appropriate | profile, layout |
| `light.*` | gamma, brightness, night dimming | layout |
| `clock.*` | time of day, daylight | runtime |
| `mode.*` | lamp test, maintainer test | runtime |

### Where polarity and direction are decided (R4, R5, R6)

| Stage | Knows | Does |
|---|---|---|
| Drawing | facts: `~{X}`, the pin's electrical type, `NormalIs`, `Freq_hz`, … | nothing |
| Compiler | the `(Role, Kind)` row | checks spelling, type and direction; carries values unchanged |
| Driver `begin()` | what the facts mean for this Kind | configures channels: direction (the complement of its pin's), pull-up, inversion, frequency, drive mode |
| Driver `drive()` / `sample()` | the request and its configured channels | asserted / not-asserted operations |
| Driver `end()` | the restrictive state the field unit just rendered | releases the hardware |
| IOBus | chip capabilities | applies configuration in hardware where possible (MCP23017 GPPU and IPOL; PCA9685 prescaler, INVRT, OUTDRV), in software where generic (bit inversion); refuses the impossible (a pull-up the chip lacks); arbitrates per-chip settings |

What "inverted" means differs by Kind: a lamp on a bit sinks instead of sourcing; a stall motor swaps its normal and reverse level; a servo swaps the ends of its travel; a lamp on PWM inverts its duty; a motor on PWM may need a different frequency or drive mode altogether (15 Hz instead of 1245 Hz). Only the Kind's driver knows which, so only the driver decides; a wrongly drawn fact is fixed in the drawing.

KiCad's pin types also give a check for free: a programmable expander pin is `bidirectional`, an output-only chip pin (PCA9685) `output`, an input-only one `input`; ERC then flags a lever wired to an output-only pin.

### Head appearances (R8)

```cpp
SemaphoreAppearance   s    = getHeadAppearance(p.current,  Semaphore::UpperQuadrant, a.identity, l);
CplAppearance         c    = getHeadAppearance(p.current,  Cpl{},                    a.identity, l);
SearchlightAppearance from = getHeadAppearance(p.previous, Searchlight{},            a.identity, l);
SearchlightAppearance to   = getHeadAppearance(p.current,  Searchlight{},            a.identity, l);
```

The rulebook and era come from the layout context; the head's place from the identity. Each head technology has its own head-appearance vocabulary (a searchlight's wheel colour; a semaphore's quadrant and angle, with a lamp lit by daylight). A driver keeps its own mechanism state (where the searchlight wheel is), never decision state. The same function lets the compiler check statically that every indication a signal's routes can produce maps to head appearances that each of its heads, and their devices, can show.

## Open

- O1. Per-chip settings shared by several devices (one PCA9685 prescaler for 16 channels) can conflict. Today a refused configuration at `begin()`; statically, each row would declare its channel needs as data and the compiler would group devices by chip.
- O2. `open_collector` for grounding contacts and open-collector detectors would draw "needs a pull-up" as an electrical fact, separate from `~{X}`.
- O3. Whether lamp-test and maintainer modes need the interlocking's consent (only with signals at STOP); if so they arrive as params, not layout context.

## Consequences

- Every driver class is tested the same way: a `Transition<T>`, a fake appliance and layout context, a `MockIOBus`; no interlocking is needed and none is reachable.
- The generator emits data, not code: one record per symbol instance (`(Role, Kind)`, identity, channels with direction, appliance settings). The field unit instantiates the class registered for that `(Role, Kind)`.
- Existing drivers (`SignalMastDriver`, `SemaphoreDriver`, `SwitchDriver`, `TrackCircuitDriver`, `MockSwitchDriver`) move to the seam: per-head drivers instead of per-mast `addHead`; the `Aspect` enum's per-head and whole-mast values separate; the inline flash clock becomes `timing.flash_ms` and a shared phase. `IOBus` gains configuration (direction, pull-up, inversion, frequency, drive mode) and duty and angle writes.
- `AspectResolver` policies (`sp1969`, `sp1985`, …) become the rulebook data behind `getHeadAppearance`, keyed by head technology. The owner's working name for the call was `getAspectForIndication`; the glossary's terms rename it.
- FieldUnit-Subdivision: the `(Role, Kind)` table names each row's delivery, pin directions and appliance settings; its lint checks the libraries against them.
