# How-To: Derails and OS Binding (FieldUnit + Studio Migration)

This note is the before/after contract for **FieldUnit**, **FieldUnit Studio**, and **FieldUnit-Subdivision** after derail support landed.

---

## 1. Position vocabulary (locked)

| Appliance | NORMAL | REVERSE |
|---|---|---|
| **Switch** | Main (or designated normal) route | Diverging route |
| **Derail** | Off-rail / clear — train may pass | On-rail / active — open cars are dumped |

Fail-safe rest for every derail is **REVERSE** (on-rail).

---

## 2. Naming rules

| Name form | Meaning |
|---|---|
| `"1"`, `"3"`, `"777"` | Switch (or independent derail if created via `addDerail`) |
| `"1A"`, `"1B"`, `"1C"` | Crossover / multi-machine ends ganged with `"1"` (never use `D` here) |
| `"1D"`, `"3D"`, `"777D"` | **Dependent derail** on base switch `"1"` / `"3"` / `"777"` |

`D` is reserved for derails. Double-crossover fourth ends use `C`, not `D`.

---

## 3. C++ plant API — before / after

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
// ... plus manual ControlTransaction rewrites ...
```

**After**

```cpp
// Dispatcher-controlled derail (own lever, own NWS/RWS, own NWK/RWK)
cp.addDerail("5", "5T1");

// Dependent derail: inverse-slaved to switch "1"; no separate CodeLine step
cp.addSwitch("1", "1T1");
cp.addDerail("1D", "1DT1");   // requires base "1" already declared
cp.addDerail("1D");           // dependent, no separate derail island
```

**Missing base is a hard error:** `addDerail("1D")` returns `nullptr` if `"1"` does not exist. A `*D` name never becomes an independent CodeLine derail.

**Do not add** `addSwitchWithDerail(...)`. Keep `addSwitch` and `addDerail` orthogonal.

### Dependent motion (field unit, automatic)

| Master command | Main points | Dependent derail |
|---|---|---|
| NORMAL | NORMAL | REVERSE (on-rail) |
| REVERSE | REVERSE | NORMAL (clear) |

Master `KR` / `NWK` / `RWK` light only when **both** ends prove that inverse pair. Routes and codecs name only the master (`"1"`), never `"1D"`.

---

## 4. Plant JSON — before / after

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

1. Apply **`switches` before `derails`** so dependent bases exist.
2. Optional `"os"` creates/finds the track circuit and binds detector lock.
3. Legacy `detectorLocks` arrays still load; bindings are idempotent.
4. Dependence is implied by the `*D` name — no separate `dependentOf` field required.

---

## 5. CodeLine / cTc desk

| Appliance | Controls | Indications | Desk lever |
|---|---|---|---|
| Switch `"1"` with dependent `"1D"` | `1NWS` / `1RWS` only | `1NWK` / `1RWK` (combined KR) | One switch lever |
| Independent derail `"5"` | `5NWS` / `5RWS` | `5NWK` / `5RWK` | Own switch-shaped lever |
| Dependent `"1D"` | none | none | none |

Office `withTrackLamps({ "1T1", "1DT1", ... })` only paints model-board occupancy. It does **not** replace field detector locks.

Route `.clears({ "1T1", "1DT1" })` is HR path occupancy, also not a substitute for OS detector lock.

---

## 6. Studio / Subdivision pivot checklist

1. **Schematic components**: add Derail appliance; property `independent` vs name-driven dependence (`baseId + "D"`).
2. **Name validator**: reject crossover ends named `*D`; suggest `A`/`B`/`C`.
3. **Code generator**: emit `addSwitch(id, os?)` and `addDerail(id, os?)`; never emit sketch-level demand rewrites for dependence.
4. **JSON exporter**: `switches[]` + `derails[]` with optional `os`; order switches then derails.
5. **Route synthesizer**: when path needs industry access through a dependent derail, align master only; KR proves derail clear.
6. **Virtual cTc**: one lever per CodeLine-visible switch/derail; no lever for `*D` dependents.
7. **Hardware profile**: still map drivers to physical `"1"` and `"1D"` machines even when `"1D"` is hidden on the CodeLine.
8. **Docs/UI copy**: derail NORMAL = clear; REVERSE = on-rail.

---

## 7. What not to copy from historical CP Corporal

The legacy Corporal sketch treated derail NORMAL as “derailing” and rewrote control demands by hard-coded index. That polarity and the sketch interlock are **not** requirements. Prefer `addDerail("1D", ...)` or a true independent `addDerail("5", ...)` with the vocabulary above.
