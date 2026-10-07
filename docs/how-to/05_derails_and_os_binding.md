# How-To: Derails and OS Binding (FieldUnit + Studio Migration)

This note is the before/after contract for **FieldUnit**, **FieldUnit Studio**, and **FieldUnit-Subdivision** after derail support landed.

---

## 1. Position vocabulary

| Appliance | NORMAL | REVERSE |
|---|---|---|
| **Switch** | Main (or designated normal) route | Diverging route |
| **Derail** | Derailing position (on the rail) | Clear — train may pass |

A derail rests in **NORMAL** (derailing). This is a FieldUnit rule.
It follows a prototype convention. No source was found for power-operated derails.
For fixed derails, 49 CFR 218.109 makes the derailing position the normal position, with exceptions.

---

## 2. Naming rules

| Name form | Meaning |
|---|---|
| `"1"`, `"3"`, `"777"` | Switch (or independent derail if created via `addDerail`) |
| `"1A"`, `"1B"`, `"1C"` | Ends of a crossover or of ganged machines, joined with `addCrossover` (never use `D` here) |
| `"1D"`, `"3D"`, `"777D"` | **Dependent derail** on base switch `"1"` / `"3"` / `"777"` |

`D` is reserved for derails. The suffix `D` must not name a crossover end.

---

## 3. C++ API — before / after

### Switches and OS (detector lock)

**Before**

```cpp
cp.addTrackCircuit("3T1");
cp.addSwitch("3");
cp.bindDetectorLock("3", "3T1");
```

**After**

```cpp
cp.addSwitch("3", "3T1");   // creates/finds OS TC + detector lock
cp.addSwitch("7");          // no OS — allowed and common
```

`bindDetectorLock` remains the low-level escape hatch.

### Derails

**Before** (sketch hacks; e.g. old CP Corporal)

```cpp
cp.addSwitch("5");  // pretended to be a derail
// ... plus manual rewrites of the controls in the ControlTransaction ...
```

**After**

```cpp
// Independent derail (own lever, own NWS/RWS controls, own NWK/RWK office indications)
cp.addDerail("5", "5T1");

// Dependent derail: moves with switch "1" (same position); no separate code line function
cp.addSwitch("1", "1T1");
cp.addDerail("1D", "1DT1");   // requires switch "1" already declared
cp.addDerail("1D");           // dependent, no OS track circuit of its own
```

**A missing switch is a hard error:** `addDerail("1D")` returns `nullptr` if `"1"` does not exist. A `*D` name never becomes an independent derail.

**Do not add** `addSwitchWithDerail(...)`. Keep `addSwitch` and `addDerail` orthogonal.

### Dependent motion (field unit, automatic)

| Control for the switch | Switch points | Dependent derail |
|---|---|---|
| NORMAL | NORMAL | NORMAL (derailing) |
| REVERSE | REVERSE | REVERSE (clear) |

The switch's `KR()`, `NWK` and `RWK` are true only when **both** machines prove the position (switch correspondence). Routes and codecs name only the switch (`"1"`), never `"1D"`.

---

## 4. FieldUnit JSON (interlocking model) — before / after

**Before**

```json
"switches": [
  { "name": "1" },
  { "name": "5" }
],
"detectorLocks": [
  { "switch": "1", "trackCircuit": "1T1" },
  { "switch": "5", "trackCircuit": "5T1" }
]
```

**After**

```json
"switches": [
  { "name": "1", "os": "1T1" },
  { "name": "3" }
],
"derails": [
  { "name": "1D", "os": "1DT1" },
  { "name": "5", "os": "5T1" }
]
```

Notes for loaders:

1. Apply **`switches` before `derails`** so the switch of a dependent derail exists.
2. Optional `"os"` creates or finds the OS track circuit and binds the detector lock.
3. Legacy `detectorLocks` arrays still load; bindings are idempotent.
4. Dependence is implied by the `*D` name — no separate `dependentOf` field required.

---

## 5. Code line and CTC machine

| Appliance | Controls | Office indications | Desk lever |
|---|---|---|---|
| Switch `"1"` with dependent `"1D"` | `1NWS` / `1RWS` only | `1NWK` / `1RWK` (combined KR) | One switch lever |
| Independent derail `"5"` | `5NWS` / `5RWS` | `5NWK` / `5RWK` | Own switch-shaped lever |
| Dependent `"1D"` | none | none | none |

The CTC machine's `withTrackLamps({ "1T1", "1DT1", ... })` only drives track indication lamps. It does **not** replace field detector locks.

Route `.clears({ "1T1", "1DT1" })` lists the track circuits that must be clear. It is also not a substitute for the detector lock on the OS track circuit.

---

## 6. Studio / Subdivision pivot checklist

1. **Schematic components**: add Derail appliance; property `independent` vs name-driven dependence (`baseId + "D"`).
2. **Name validator**: reject crossover ends named `*D`; suggest `A`/`B`/`C`.
3. **Code generator**: emit `addSwitch(id, os?)` and `addDerail(id, os?)`; never emit sketch-level rewrites of controls for dependence.
4. **JSON exporter**: `switches[]` + `derails[]` with optional `os`; order switches then derails.
5. **Route synthesizer**: when a path needs industry access through a dependent derail, align the switch only; `KR()` proves the derail is clear.
6. **Virtual CTC machine**: one lever per switch or derail that appears on the code line; no lever for `*D` dependents.
7. **Hardware profile**: still map drivers to physical `"1"` and `"1D"` machines even when `"1D"` is hidden on the code line.
8. **Docs/UI copy**: derail NORMAL = derailing (on the rail); REVERSE = clear.

---

## 7. What not to copy from historical CP Corporal

The legacy Corporal sketch rewrites controls by hard-coded index. That sketch interlock is **not** a requirement. Its polarity (derail NORMAL = derailing) is correct. Prefer `addDerail("1D", ...)` or a true independent `addDerail("5", ...)`.
