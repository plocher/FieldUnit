# 0004. The driver seam: what a device driver may see, and the context vocabulary

- Status: proposed (shaped with the owner, 2026-10-07)
- Date: 2026-10-07
- Builds on: ADR 0003 (field I/O symbols); FieldUnit-Subdivision ADR 0004 (heads as appliances, D6; field units, D13).
- Order of authority: the relay model and AAR practice (`docs/GLOSSARY.md` §10.6) govern; drivers sit below the vital logic.

Terms follow `docs/GLOSSARY.md`.

## Decisions requested

- R1. A driver renders the vital result for its own appliance, or reports a raw observation, and nothing else. It never reads interlocking state (routes, locks, other appliances). Default: yes.
- R2. A driver has three inputs: params (a transition of its own appliance's vital value), binding (this symbol instance), context (a read-only dictionary of environment settings). Default: yes.
- R3. Params are always a transition, `{previous, current, sinceMs}`, of one value type per appliance family. Default: yes.
- R4. The binding's channels carry their polarity; drivers write and read *asserted*, never a raw level. Default: yes.
- R5. The appliance identity (name and place, e.g. the middle head of a three-head mast) is opaque to drivers and only passed through to library calls. Default: yes.
- R6. `getHeadAppearance` is a pure library function, one overload per head technology, returning that technology's own head appearance type (a head shows a head appearance; the aspect is all of a signal's heads together, glossary §8). Default: yes.
- R7. The context is one namespaced dictionary of typed keys, resolved through a cascade (drawing, plant, layout, era profile, library default) plus runtime keys; the vocabulary is fixed now, the dictionary grows. Default: yes.
- R8. The key registry lives in FieldUnit-Subdivision (`schemas/context/keys.toml`) until the data-model schema moves into FieldUnit, then moves with it. Default: yes.

## Context

ADR 0003 gave every field device symbol a `(Role, Kind)` and a pin set; the generator was to emit one constructor call per device. A constructor per row makes every row its own API: tests and generators special-case each one. And the first sketch of a richer head driver let it read interlocking state, which would let a driver second-guess the vital logic. This ADR fixes one surface for every driver, with nothing vital on it.

## The seam

```cpp
template <class T>
struct Transition {          // R3
    T previous;
    T current;
    uint32_t sinceMs;        // when current took effect
};

template <class T>
class ApplianceDriver {
public:
    virtual ~ApplianceDriver() = default;
    virtual void   begin(const Binding& b, const Context& ctx) {}
    virtual void   drive(const Transition<T>& p, const Binding& b, const Context& ctx) {}   // render only
    virtual Sample sample(const Binding& b, const Context& ctx) { return {}; }             // raw observation only
};
```

| Family (first part of the Kind, ADR 0003 D2) | `T` | Direction |
|---|---|---|
| `HEAD_*` | `Indication` of the head's signal | drive |
| `SWITCH_*` (device) | the demand the `Switch` has approved (`N`, `R`) | drive; sensed Kinds also sample point position |
| `LAMP_*` (device), `PANEL` lamps | the state of the token shown (off, on, flashing; a request lamp: off, waiting, steady) | drive |
| `DETECTOR_*`, `INPUT_BIT` | none | sample |

The field unit owns all decisions: it computes each appliance's vital value, hands the transition to that appliance's driver, and takes `Sample`s in as observations it then judges. A driver never sees another appliance.

### Binding (per symbol instance; static; from the drawing)

- `channel(name)`: the pin's driver, bit or channel, and polarity (from `~{X}` or the instance override, ADR 0003 D6). R4: `io.write(channel, asserted)` applies the inversion; a wrongly inverted lamp is a drawing fix, never a code fix.
- the instance's resolved context keys (its attributes, R7).
- `identity`: opaque (R5). Drivers pass it to library calls; they cannot inspect it, so no driver can say "I am the second head, therefore…".

### Head appearances (R6)

```cpp
SemaphoreAppearance   a    = getHeadAppearance(p.current,  Semaphore::UpperQuadrant, b.identity, ctx);
CplAppearance         a    = getHeadAppearance(p.current,  Cpl{},                    b.identity, ctx);
SearchlightAppearance from = getHeadAppearance(p.previous, Searchlight{},            b.identity, ctx);
SearchlightAppearance to   = getHeadAppearance(p.current,  Searchlight{},            b.identity, ctx);
```

The rulebook and era come from the context; the head's place from the identity. Each head technology has its own head-appearance vocabulary (a searchlight's wheel colour, a semaphore's quadrant and angle with a lamp lit by daylight). A driver keeps its own mechanism state (where the searchlight wheel is), never decision state. The same function lets the compiler check statically that every indication a signal's routes can produce maps to head appearances that each of its heads, and their devices, can show.

### Context (R7)

A read-only dictionary of typed keys in namespaces. Static keys resolve, most specific first: drawing (a symbol attribute) → plant (the `INTERLOCKING`) → layout (the root sheet) → era profile → library default; the compiler resolves them per instance. Runtime keys come from the field unit.

| Namespace | Holds | Set at |
|---|---|---|
| `era.*`, `rulebook.*` | the era and rulebook represented; which aspect each signal indication takes (Approach yellow or lunar…) | layout, plant |
| `timing.*` | flash period, searchlight step, servo speed, fade, detector hold | profile, layout, drawing |
| `color.*` | named colours to RGB (`amber`, `lunar`…), era-appropriate | profile, layout |
| `light.*` | gamma, brightness, night dimming | layout |
| `clock.*` | time of day, daylight | runtime |
| `mode.*` | lamp test, maintainer test | runtime |

Today's symbol attributes are drawing-level values of keys (`Hold_ms` sets `timing.hold_ms` for one detector; a lamp's `Color` names a `color.*` entry). A driver reads only the keys it declares; a contract test checks the declaration against the registry (R8). Open: whether lamp-test and maintainer modes need the interlocking's consent (only with signals at STOP); if so they arrive as params, not context.

## Consequences

- Every driver class is tested the same way: a `Transition<T>`, a fake `Binding` and `Context`, a `MockIOBus`; no interlocking is needed and none is reachable.
- The generator emits data, not code: one binding record per symbol instance (`(Role, Kind)`, identity, channels, resolved keys). The field unit instantiates the class registered for that `(Role, Kind)`.
- Existing drivers (`SignalMastDriver`, `SemaphoreDriver`, `SwitchDriver`, `TrackCircuitDriver`, `MockSwitchDriver`) move to the seam: per-head drivers instead of per-mast `addHead`; the `Aspect` enum's per-head and whole-mast values separate; the inline flash clock becomes `timing.flash_ms` and a shared phase.
- `AspectResolver` policies (`sp1969`, `sp1985`, …) become the rulebook data behind `getHeadAppearance`, keyed by head technology. The owner's working name for the call was `getAspectForIndication`; the glossary's terms rename it.
- FieldUnit-Subdivision: the `(Role, Kind)` table's `delivery` names the class, the family and `ApplianceDriver`; its `attributes` reference registered keys.
