#ifndef FIELDUNIT_SWITCH_H
#define FIELDUNIT_SWITCH_H

#include "types.h"

namespace FieldUnit {

// TrackSwitch / SwitchMachine appliance (AAR standard: Switch, not Turnout)
class Switch {
public:
    Switch() : Switch("") {}
    virtual ~Switch() = default;

    Switch(const char* name)
        : commanded_(SwitchPosition::NORMAL),
          reported_(SwitchPosition::NORMAL),
          locks_(SwitchLock::UNLOCKED),
          pairedSwitch_(nullptr),
          index_(0),
          motionStartMs_(0),
          travelTimeoutMs_(5000) {
        setName(name);
    }

    void setName(const char* name) {
        if (!name) { name_[0] = '\0'; return; }
        strncpy(name_, name, sizeof(name_) - 1);
        name_[sizeof(name_) - 1] = '\0';
    }

    const char* name() const { return name_; }
    uint8_t index() const { return index_; }
    void setIndex(uint8_t idx) { index_ = idx; }

    virtual SwitchPosition commandedPosition() const { return commanded_; }
    virtual SwitchPosition reportedPosition() const { return reported_; }
    virtual SwitchLock activeLocks() const { return locks_; }

    virtual bool isMovable() const {
        return locks_ == SwitchLock::UNLOCKED;
    }

    virtual bool isDetectorLocked() const {
        return (locks_ & SwitchLock::DETECTOR_LOCKED) == SwitchLock::DETECTOR_LOCKED;
    }

    virtual bool isRouteLocked() const {
        return (locks_ & SwitchLock::ROUTE_LOCKED) == SwitchLock::ROUTE_LOCKED;
    }

    virtual bool isTimeLocked() const {
        return (locks_ & SwitchLock::TIME_LOCKED) == SwitchLock::TIME_LOCKED;
    }

    virtual bool inCorrespondence() const {
        return (reported_ == commanded_) && 
               (reported_ == SwitchPosition::NORMAL || reported_ == SwitchPosition::REVERSE);
    }

    // -------------------------------------------------------------
    // AAR Standard Relay Contact Logic
    // -------------------------------------------------------------

    /**
     * AAR Relay: NWCR (Normal Switch Correspondence Relay)
     *
     * In prototype interlocking plants:
     * - Physical switch point circuit controller contacts close only when
     *   points reach and mechanically lock in the Normal position.
     * - NWCR energizes only when commanded Normal AND feedback verifies Normal.
     *
     * @return true if points are mechanically locked in Normal position.
     */
    virtual bool NWCR() const {
        return reported_ == SwitchPosition::NORMAL && inCorrespondence();
    }

    /**
     * AAR Relay: RWCR (Reverse Switch Correspondence Relay)
     *
     * In prototype interlocking plants:
     * - Circuit controller contacts close only when points reach and lock Reverse.
     * - RWCR energizes only when commanded Reverse AND feedback verifies Reverse.
     *
     * @return true if points are mechanically locked in Reverse position.
     */
    virtual bool RWCR() const {
        return reported_ == SwitchPosition::REVERSE && inCorrespondence();
    }

    /**
     * AAR Relay: KR (Switch Indication Relay)
     *
     * Proves that the switch is locked in full correspondence (NWCR || RWCR).
     * If the points are in motion, gapped, or out of correspondence, KR drops.
     * Interlocking circuits require active KR before clearing any signal.
     *
     * @return true if points are locked in correspondence (either Normal or Reverse).
     */
    virtual bool KR() const {
        return inCorrespondence();
    }

    /**
     * AAR Relay: WLR / LR (Switch Lock Relay)
     *
     * In prototype relay signaling:
     * - De-energizes (drops) when the switch is locked by:
     *   1. Detector locking (train occupies island track circuit across points).
     *   2. Route locking (an active cleared route reserves this switch).
     *   3. Time locking (approach timer running down after signal cancellation).
     * - Power to the switch motor is routed through a front contact of WLR.
     * - If WLR drops, the motor cannot energize under any circumstances.
     *
     * @return true if switch is completely unlocked and free to throw.
     */
    virtual bool WLR() const {
        return isMovable();
    }

    // Pair a crossover switch (bidirectional: both move and lock together)
    void pairCrossover(Switch* other) {
        pairedSwitch_ = other;
        if (other && other->pairedSwitch_ != this) {
            other->pairedSwitch_ = this;
        }
    }

    Switch* pairedSwitch() const { return pairedSwitch_; }

    // Lock arbitration (managed by Interlocking Control Table and OS detector track)
    virtual void addLock(SwitchLock lock) {
        if ((locks_ & lock) == lock) {
            return; // Already has this lock, breaks recursion
        }
        locks_ = locks_ | lock;
        if (pairedSwitch_) {
            pairedSwitch_->addLock(lock);
        }
    }

    virtual void removeLock(SwitchLock lock) {
        if ((locks_ & lock) == SwitchLock::UNLOCKED) {
            return; // Already cleared, breaks recursion
        }
        locks_ = static_cast<SwitchLock>(static_cast<uint8_t>(locks_) & ~static_cast<uint8_t>(lock));
        if (pairedSwitch_) {
            pairedSwitch_->removeLock(lock);
        }
    }

    // AAR WR (Switch Control Relay):
    // Binary rule: execute if unlocked (WLR picked up), reject immediately if locked
    virtual bool throwSwitch(SwitchPosition target, uint32_t nowMs = 0) {
        if (target != SwitchPosition::NORMAL && target != SwitchPosition::REVERSE) {
            return false;
        }

        // Check WLR (Switch Lock Relay)
        if (!WLR()) {
            return false; // Rejected: switch is locked
        }

        if (commanded_ == target && inCorrespondence()) {
            return true; // No-op, already in desired state
        }

        commanded_ = target;
        reported_ = SwitchPosition::MOVING;
        motionStartMs_ = nowMs;

        if (pairedSwitch_ && pairedSwitch_->commandedPosition() != target) {
            pairedSwitch_->throwSwitch(target, nowMs);
        }

        return true;
    }

    // Advance non-blocking travel timer
    virtual void tick(uint32_t nowMs) {
        if (reported_ == SwitchPosition::MOVING) {
            if (travelTimeoutMs_ > 0 && (nowMs - motionStartMs_ > travelTimeoutMs_)) {
                // Points failed to make contact within timeout
                reported_ = SwitchPosition::OUT_OF_CORRESPONDENCE;
            }
        }
    }

    void setTravelTimeout(uint32_t timeoutMs) {
        travelTimeoutMs_ = timeoutMs;
    }

    // Called by hardware driver when point detector contacts settle
    virtual void updateFeedback(SwitchPosition physicalPoints) {
        reported_ = physicalPoints;
    }

private:
    char name_[32];
    SwitchPosition commanded_;
    SwitchPosition reported_;
    SwitchLock locks_;
    Switch* pairedSwitch_;
    uint8_t index_;
    uint32_t motionStartMs_;
    uint32_t travelTimeoutMs_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_SWITCH_H
