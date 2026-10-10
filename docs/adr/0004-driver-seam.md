# 0004. The driver seam: what a device driver may see, and the two contexts

- Status: proposed (shaped with the owner, 2026-10-07 and 08)
- Date: 2026-10-08
- Builds on: ADR 0003 (field I/O symbols); FieldUnit-Subdivision ADR 0004 (heads as appliances, D6; field units, D13).
- Order of authority: the relay model and AAR practice (`docs/GLOSSARY.md` §10.6) govern; drivers sit below the vital logic.

Terms follow `docs/GLOSSARY.md`.

## Decisions requested
- R0. The design pattern for Appliance Drivers needs to be architected and designed.  The following "R*" requests begin to give shape to this subsystem, but are presented from a bottom-up and somewhat disjointed perspective.  What follows here is high level guidance and commentary for what needs to be incorporated into an acceptable design:
  - Drivers are, at their core, transformers.  They transform what I'll call **AAR Appliance** Vocabulary, Attributes and Actions into **some other domain**'s Vocabulary, Attributes and Actions.
  - All interactions with the modeled appliance are handled by an instance of a driver.  We might talk about a `color-position-light signal head driver for local digital I/O` or a `color-position-light signal head driver for local 12-bit PWM I/O `  as well as instances of a driver for `Signal 2 Mast 2SAB Head A` and `Signal 2 Mast 2SAB Head B`. 
  - While both of these example drivers present the same surface shape to the Field- or Office-Unit ecosystem, their internal details differ because they are translating for different output domains.
  - In the same way, we might postulate others: `lamp driver for local digital I/O` and `lamp driver for local 12-bit PWM I/O`.  As with the signal heads above, they would expose a common lamp-style surface to the ecosystem.  They would ALSO **use** target shapes that were similar to those used by their domain-peer signal heads - the digital and PWM I/O surfaces would be common.
  - Drivers follow certain shape-patterns related to their role and kind and to the domain they are translating to/from.  Lamps have a common shape, signal heads another.  These shapes can be codified and reused/applied across design families.
  - These codified shapes can be described by enumerating the
    - internal state that the driver instance needs to maintain
    - commands that this shape needs to support
    - notifications that this shape needs to expose
  - Examples:
    - Lamp Appliance for a digital I/O domain
      - Commands
        - on
        - off
        - flicker(level)
        - flash(rate)
      - Attributes
        - boolean: isOn
        - level: Flicker
        - rate: FlashRate
      - Notifications
        - Fault
  
    - A Lamp Appliance for a PWM domain
      - All the behavior of the above digital I/O Lamp, plus
        - fade_on(duration)
        - fade_off(duration)
        - brightness(level)
        - level: currentBrightness
  - Drivers need an environment in which to operate.  This environment supplies contextual data, utility functions and lifecycle support.

  - R0.questions:
    1. How does the existing Role/Kind taxonomy fit into this model?
    2. How does the existing symbol library and symbol naming fit?
    3. What are the appliance shapes? ... names?  The target domain shapes? ... names?
    4. do we need to evolve our existing naming now that we've poked at it here?

- R1. A driver converts the demand vocabulary of the Office/Field Unit to that of its connected electro-mechanical device's requirements and vice-versa.
    NOTE: This is "Translator" above.
  
- R2. A driver runs in an environment that includes rich context:
  - appliance identity,
  - activity record (previous state...)
  - desired action (desired state), 
  - attributes from this appliance/symbol instance
  - layout configuration settings.
  
  NOTE: This is the ecosystem above

- R3. merged into R2
  
- R4. The driver is responsible for all behaviors impacted by the appliance context, such as polarity and direction, initialization, etc. In it's `begin()` it configures its channels through the IOBus; afterwards it works in asserted / not-asserted terms. 
   
   NOTE: This last makes no sense in the context of an IOBus with non-binary behavior.
   NO DECISION

- R5. The IOBus offers capabilities and configuration: it applies a request in the chip where the chip can, in software where the operation is generic, and refuses what it cannot do.

  NOTE: Same as above - this statement doesn't have sufficient context or scope to have meaning...
  NO DECISION

- R6. A pin's direction comes from the drawing: the device or panel pin's KiCad electrical type says what that symbol does to its wire, and a programmable expander pin is configured as its complement. 

- R7. The appliance's identity parameter is an opaque "handle" that can be passed to library calls that need to know implementation details of the device.  An example might be a `getHeadAppearance(appliance, indication) => head display details` library function that needs to know which head on which mast (top, middle, ...) as well as what type of device it is (color position, semaphore...)
  
- R9. The layout context contains layout configuration choices, such as grographic location, era, railroad details, etc. 
  


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
