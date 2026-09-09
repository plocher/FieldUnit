#ifndef FIELDUNIT_SWITCH_H
#define FIELDUNIT_SWITCH_H

#include "types.h"

namespace FieldUnit {

// TrackSwitch / SwitchMachine appliance (AAR standard: Switch, not Turnout)
class Switch {
public:
    Switch() : Switch("") {}

    Switch(const char* name)
        : name_(name),
          commanded_(SwitchPosition::NORMAL),
          reported_(SwitchPosition::NORMAL),
          locks_(SwitchLock::UNLOCKED),
          pairedSwitch_(nullptr),
          motionStartMs_(0),
          travelTimeoutMs_(5000) {}

    const char* name() const { return name_; }

    SwitchPosition commandedPosition() const { return commanded_; }
    SwitchPosition reportedPosition() const { return reported_; }
    SwitchLock activeLocks() const { return locks_; }

    bool isMovable() const {
        return locks_ == SwitchLock::UNLOCKED;
    }

    bool inCorrespondence() const {
        return (reported_ == commanded_) && 
               (reported_ == SwitchPosition::NORMAL || reported_ == SwitchPosition::REVERSE);
    }

    // -------------------------------------------------------------
    // AAR Standard Relay Contact Logic
    // -------------------------------------------------------------
    // NWCR: Normal Switch Correspondence Relay (picked up = locked Normal)
    bool NWCR() const {
        return reported_ == SwitchPosition::NORMAL && inCorrespondence();
    }

    // RWCR: Reverse Switch Correspondence Relay (picked up = locked Reverse)
    bool RWCR() const {
        return reported_ == SwitchPosition::REVERSE && inCorrespondence();
    }

    // KR: Switch Indication Relay (points locked in either Normal or Reverse)
    bool KR() const {
        return inCorrespondence();
    }

    // WLR: Switch Lock Relay (picked up = points free to throw)
    bool WLR() const {
        return isMovable();
    }

    // Pair a crossover switch (moves in unison, both must correspond)
    void pairCrossover(Switch* slave) {
        pairedSwitch_ = slave;
    }

    Switch* pairedSwitch() const { return pairedSwitch_; }

    // Lock arbitration (managed by Interlocking Control Table and OS detector track)
    void addLock(SwitchLock lock) {
        locks_ = locks_ | lock;
        if (pairedSwitch_) {
            pairedSwitch_->addLock(lock);
        }
    }

    void removeLock(SwitchLock lock) {
        locks_ = static_cast<SwitchLock>(static_cast<uint8_t>(locks_) & ~static_cast<uint8_t>(lock));
        if (pairedSwitch_) {
            pairedSwitch_->removeLock(lock);
        }
    }

    // AAR WR (Switch Control Relay):
    // Binary rule: execute if unlocked (WLR picked up), reject immediately if locked
    bool throwSwitch(SwitchPosition target, uint32_t nowMs = 0) {
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

        if (pairedSwitch_) {
            pairedSwitch_->throwSwitch(target, nowMs);
        }

        return true;
    }

    // Advance non-blocking travel timer
    void tick(uint32_t nowMs) {
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
    void updateFeedback(SwitchPosition physicalPoints) {
        reported_ = physicalPoints;
    }

private:
    const char* name_;
    SwitchPosition commanded_;
    SwitchPosition reported_;
    SwitchLock locks_;
    Switch* pairedSwitch_;
    uint32_t motionStartMs_;
    uint32_t travelTimeoutMs_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_SWITCH_H
