# How-To: Hand-Throw Switches with Electric Locks

This guide explains how to model a hand-operated track switch that has an electric lock (`WL` in the AAR list of letters), and how the dispatcher releases that lock with a control.

An electric lock is not a dual-control switch.
A dual-control switch is a different appliance, and FieldUnit has no API for it.
This guide does not cover it.

---

## 1. The Appliance

A power-operated switch is thrown by the dispatcher's control.
A **hand-throw switch** has a stand that someone throws by hand, for example on an industrial spur or a siding.

An **electric lock** is a lock on a hand-operated switch.
The dispatcher releases it by a control.
Until it is released, nobody can throw the switch by hand.

---

## 2. The Release Sequence

```
[ Someone at the Switch ]                   [ Dispatcher at the CTC machine ]
           |                                             |
           |                                  (Dispatcher checks that no train
           |                                   is approaching the switch)
           |                                             |
           |                                  1. Sends control 7WLS (release)
           |                                     with the code button
           |<--- 7WLS --------------------------------- |
           |
 2. The field unit checks that every signal is at stop
    and that no time locking runs. If so, it releases the lock.
           |
           |---- office indication 7WLK (released) ---->|
           |
 3. The switch can now be thrown by hand
           |
           |---- office indications 7NWK / 7RWK ------->|
           |
           |                                  4. Sends control (7WLS) (lock)
           |<--- (7WLS) --------------------------------|
           |
 5. The field unit locks the switch
           |---- office indication (7WLK) ------------->|
```

How the person at the switch asks the dispatcher to release the lock is outside FieldUnit.
The library has no appliance and no function for a lock-box door contact or a request button.
If you must see such a contact at the office, you can read it with a track circuit as a stand-in (a `TrackCircuit` and a `TrackCircuitDriver`), and the office sees it as a track circuit office indication such as `7DOORK`.
This is a workaround. It is not a model of the door.
Do not name that track circuit in the `clears(...)` of any route.

---

## 3. Modeling Electric Locks in FieldUnit

In FieldUnit, a hand-throw switch with an electric lock is a `Switch` that carries the lock `SwitchLock::HAND_LOCKED`.
The field unit adds and removes that lock when it accepts a `7WLS` control.

### In Your Sketch Setup

```cpp
// Declare the hand-throw switch (no OS track circuit here; "7" has none)
auto sw7 = cp.addSwitch("7");

// The switch starts with the electric lock engaged
sw7->addLock(SwitchLock::HAND_LOCKED);

// The switch has no motor. Read the position contacts only; leave the motor output empty.
SwitchDriver sw7_drv(sw7, OutputBit(), InputBit(0, 1, 4, Polarity::INVERTED), InputBit(0, 1, 5, Polarity::INVERTED));
```

The lock itself (the solenoid) has no driver class in FieldUnit.
After `cp.exportIndicationVector(ind)`, the field `ind.switches[sw7->index()].electricLockUnlocked` is true while the lock is released.
Write your own output from it.

### In Your Control Table

A route over `7` asks for the switch in Normal, in switch correspondence:

```cpp
cp.route("MAIN_CLEAR")
  .governedBy(sig2, DirectionAuthority::RIGHT)
  .displays(mast2S, Indication::CLEAR)
  .aligns({ {sw7, SwitchPosition::NORMAL} })
  .clears({ tc1T1, tc1NA });
```

The route logic does not look at `HAND_LOCKED`.
The field unit checks the signals only at the moment it releases the lock.
Nothing in the library stops the dispatcher from clearing a signal after the lock is released.

### When the Dispatcher Sends the Release

When the dispatcher sends `7WLS` in a control transaction:
1. The field unit checks that every signal control is at `STOP` and that no time locking runs.
2. If both are true, it calls `sw7->removeLock(SwitchLock::HAND_LOCKED)`.
3. If either is false, it does nothing and sends no refusal. The office sees that `7WLK` does not arrive.
4. With the lock removed and no other lock active, `sw7->WLR()` is true.
5. The switch can now be thrown by hand. The position contacts report the new position through `7NWK` or `7RWK`.

When the dispatcher sends `(7WLS)`:
1. The field unit calls `sw7->addLock(SwitchLock::HAND_LOCKED)` at once.
2. It does not check the position of the switch. The dispatcher reads `7NWK` and `7RWK`.

---

## 4. Code Line Wire Mapping (`WLS` and `WLK`)

An `AarTextCodec` carries the electric lock directly:

### In Your Codec Configuration

```cpp
codec.decodeControls({
    decodeSwitch(sw1),
    decodeElectricLock(sw7)  // 7WLS: bare = release, parenthesized = lock
});

codec.encodeIndications({
    encodeSwitch(sw1),
    encodeSwitch(sw7),       // 7NWK, 7RWK
    encodeElectricLock(sw7)  // 7WLK: bare = released, parenthesized = locked
});
```

`sw7` has no `decodeSwitch(...)`, because it has no motor and takes no `NWS` or `RWS` control.

For a `CodeLineCodec` (bit-packed bytes), use `mapElectricLockControl(switchIndex, byte, bit)` and `mapElectricLockIndication(switchIndex, byte, bit)`.
A control bit of 1 means release. A control bit of 0 means lock, in every packet.

### Operational Execution

1. **Dispatcher releases lock**: Sends `7WLS` across the code line.
   - `InterlockingPlant::applyControlTransaction` checks that all signal controls are at `STOP` and no time locking runs.
   - If safe, `SwitchLock::HAND_LOCKED` is removed.
   - The field unit sends `7WLK` so the released lamp lights on the CTC machine.
2. **Dispatcher locks the switch**: Sends `(7WLS)` across the code line.
   - `SwitchLock::HAND_LOCKED` engages immediately.
   - The field unit sends `(7WLK)` (locked, lamp dark).
