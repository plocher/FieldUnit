# 0002. How a field unit processes a control transaction

- Status: accepted (owner, 2026-10-02)
- Date: 2026-10-02

Terms follow `docs/GLOSSARY.md`.

## Decided

1. The two rules below are the principle for the interlocking logic (owner, 2026-10-02).
2. The two classes are named **vital** and **non-vital**. The class describes how the field unit processes a control. The code line that carries the control is not vital.

## Context

A control transaction is the complete set of controls that a CTC machine sends to one field unit
when the dispatcher presses CODE. The owner stated the processing rule on 2026-10-02. The comparison
is a network packet:

- A UDP packet with a bad checksum or a wrong length is malformed. Its content is not reliable. The
  receiver ignores it and updates its error counters.
- A UDP packet that passes those checks is reliable. The consumer then decides what to do with the content.

The code line control is a transactional demand. A lever frame refuses every unsafe movement because
the levers are interlocked. This rule gives the same protection: no part of an unsafe control escapes.

## Decision

### Rule 1. A malformed transaction is ignored as a whole

- A transaction is malformed when it is incomplete, or it contains unknown or corrupted data, or it
  fails any structural check of its code line.
- The field unit ignores all of its content. This includes the controls of both classes.
- The field unit updates its error counters.
- The field unit sends office indications of its current, unchanged state.

### Rule 2. A valid transaction that is unsafe: the vital controls are ignored together

- A transaction that passes Rule 1 is reliable. The field unit interprets it.
- If acting on the transaction would violate a safety protection, the field unit ignores **every**
  vital control in that transaction. It does not act on the safe ones and skip the
  unsafe one.
- The field unit acts on every non-vital control. By definition these cannot affect
  safety. The maintainer call is one of them.
- The field unit sends no refusal. The office learns the result from the office indications.

### The two classes

| Class | Meaning | Examples |
|---|---|---|
| vital | A control that can affect a safety protection. The interlocking logic checks it. | switch, signal, electric lock |
| non-vital | A control that cannot affect a safety protection. | maintainer call |

The class is a property of how the field unit processes a control. It is not a property of the code
line. The code line that carries the controls is not vital.

Names: the owner and the code say "vital" and "non-vital" (`vitalValid`, the `Vital` field
on panel symbols). The glossary rewrite of 2026-10-01 introduced "interlocked function" and "auxiliary
function" so that nobody reads the code line as vital. Decided: vital and non-vital, with the
sentence above about the code line. It is the owner's word, and it needs no rename in the code or in
the symbol library.

## Consequences

### Design

A dispatcher who codes a transaction in which one switch is locked sees no vital control
act, including the free switches. This is intended. If the dispatcher needs several routes active at
once, the interlocking must be designed with those operations explicit and protected.

### Code (defects against this ADR; `InterlockingPlant::applyControlTransaction`)

The existing code predates this work. It is illustrative and not authoritative.

1. Rule 1: when the transaction is marked invalid (`vitalValid` false), the code skips the vital
   controls but still applies maintainer calls. A malformed transaction therefore changes state.
2. Rule 2: the code skips only the control that is unsafe ("that specific movement cannot be
   executed") and acts on the others. A partial control escapes.
3. The code knows one non-vital control, the maintainer call. The class must be a property of
   each function in the interlocking model, so that a new function states its class.
4. No error counter is kept for malformed transactions (to verify).

### Documents

- Glossary: the entries "control transaction", "interlocked function", "auxiliary function" and
  "vital" follow the chosen names and state the two rules.
- Primer section 10: the tables and "The Auxiliary Function Rule" follow the chosen names. The text
  that says the maintainer call is applied even when the transaction is invalid is wrong under Rule 1.
- FieldUnit-Subdivision `docs/adr/0002-symbol-contract.md`: the class field on the symbols takes the chosen name.
