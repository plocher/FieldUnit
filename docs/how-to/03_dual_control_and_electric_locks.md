# How-To: Hand-Throw Switches and Electric Switch Locks (WL)

This guide explains how to model hand-operated track switches equipped with Electric Switch Locks (`WL` / `ESL`) in CTC territory.

---

## 1. The Prototype Problem

In CTC territory, main track switches are normally power-operated by the dispatcher.
However, industrial spurs, house tracks, and back sidings often have **hand-throw switch stands** operated manually by the train crew.

To prevent a train crew from throwing a switch in front of a high-speed train, the railroad installs an **Electric Switch Lock (`ESL`)** on the switch stand.
The crew cannot physically lift the throw lever until an electrical solenoid inside the lock housing releases the latch.

---

## 2. The Two-Phase Release Sequence

Prototype operating rules require a strict two-phase cooperative handshake:

```
[ Train Crew at Switch ]                    [ Dispatcher at CTC Office ]
           |                                             |
 1. Opens padlock on lock box                            |
    (Actuates door contact)                              |
           |                                             |
           |---- Door Open Indication (WAK) ------------>|
           |                                             |  (Dispatcher verifies
           |                                             |   no trains approaching)
           |                                             |
           |                                         2. Turns WL Lever to UNLOCK
           |                                         3. Presses Code Button
           |                                             |
           |<--- Unlock Command Sent (WLS) --------------|
           |
 4. Electric Lock solenoid clicks
    (Indicator lamp shows UNLOCKED)
           |
 5. Crew throws hand lever to REVERSE
    (Main track signal drops to STOP)
```

---

## 3. Modeling Electric Switch Locks in FieldUnit

In FieldUnit, a hand-throw switch with an electric lock is represented by a `Switch` coupled to an electric lock state:

### In Your Sketch Setup

```cpp
// Declare the hand-throw switch
auto sw7 = cp.addSwitch("SW7_INDUSTRY");

// Declare the local door/padlock detector contact
auto tcDoor = cp.addTrackCircuit("SW7_DOOR");

// Initially, the switch is locked by the electric lock
sw7->addLock(SwitchLock::HAND_UNLOCKED); // Locked until released
```

### In Your Control Table

Mainline signals passing over `SW7` require `SW7` to be locked in Normal:

```cpp
cp.route("MAIN_CLEAR")
  .governedBy(sig2, DirectionAuthority::RIGHT)
  .displays(mast2S, Indication::CLEAR)
  .aligns({ {sw7, SwitchPosition::NORMAL} }) // Must be locked Normal
  .clears({ tc1T1, tc1NA });
```

### When the Dispatcher Grants the WL Release

When the dispatcher sends the `WLS = UNLOCK` bit in a `ControlTransaction`:
1. The Control Point verifies that no approaching train has cleared a signal over the switch.
2. If the plant is clear, the solenoid energizes (`WLR` relay picks up).
3. `sw7->removeLock(SwitchLock::HAND_UNLOCKED)`.
4. If a signal was showing Clear, it immediately knocks down to Stop.
5. The crew can now manually throw the switch.

When the crew finishes and re-locks the switch stand:
1. `sw7->addLock(SwitchLock::HAND_UNLOCKED)`.
2. The CP verifies Normal correspondence (`sw7->NWCR()`).
3. Mainline signals can clear once again.
