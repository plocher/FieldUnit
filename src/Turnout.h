#ifndef FIELDUNIT_TURNOUT_H
#define FIELDUNIT_TURNOUT_H

#include "types.h"

namespace FieldUnit {

class Turnout {
public:
    Turnout() : Turnout("") {}

    Turnout(const char* name)
        : name_(name),
          commanded_(TurnoutPosition::NORMAL),
          reported_(TurnoutPosition::NORMAL),
          locks_(TurnoutLock::UNLOCKED),
          pairedSwitch_(nullptr),
          motionStartMs_(0),
          travelTimeoutMs_(5000) {}

    const char* name() const { return name_; }

    TurnoutPosition commandedPosition() const { return commanded_; }
    TurnoutPosition reportedPosition() const { return reported_; }
    TurnoutLock activeLocks() const { return locks_; }

    bool isMovable() const {
        return locks_ == TurnoutLock::UNLOCKED;
    }

    bool inCorrespondence() const {
        return (reported_ == commanded_) && 
               (reported_ == TurnoutPosition::NORMAL || reported_ == TurnoutPosition::REVERSE);
    }

    // AAR Relay Aliases:
    // NWCR: Normal Switch Correspondence Relay (picked up = points locked Normal)
    bool NWCR() const {
        return reported_ == TurnoutPosition::NORMAL && inCorrespondence();
    }

    // RWCR: Reverse Switch Correspondence Relay (picked up = points locked Reverse)
    bool RWCR() const {
        return reported_ == TurnoutPosition::REVERSE && inCorrespondence();
    }

    // KR: Switch Indication Relay (points are in correspondence in either position)
    bool KR() const {
        return inCorrespondence();
    }

    // WLR: Switch Lock Relay (picked up = switch is free to move)
    bool WLR() const {
        return isMovable();
    }

    // Pair a crossover slave switch (moves in unison, both must correspond)
    void pairCrossover(Turnout* slave) {
        pairedSwitch_ = slave;
    }

    Turnout* pairedSwitch() const { return pairedSwitch_; }

    // Lock arbitration (managed by Interlocking Control Table and OS track occupancy)
    void addLock(TurnoutLock lock) {
        locks_ = locks_ | lock;
        if (pairedSwitch_) {
            pairedSwitch_->addLock(lock);
        }
    }

    void removeLock(TurnoutLock lock) {
        locks_ = static_cast<TurnoutLock>(static_cast<uint8_t>(locks_) & ~static_cast<uint8_t>(lock));
        if (pairedSwitch_) {
            pairedSwitch_->removeLock(lock);
        }
    }

    // Command execution: Binary rule (execute if unlocked, reject immediately if locked)
    bool throwSwitch(TurnoutPosition target, uint32_t nowMs = 0) {
        if (target != TurnoutPosition::NORMAL && target != TurnoutPosition::REVERSE) {
            return false;
        }

        // Check if movement is allowed
        if (!isMovable()) {
            return false; // Rejected: switch is locked
        }

        if (commanded_ == target && inCorrespondence()) {
            return true; // No-op, already in desired state
        }

        commanded_ = target;
        reported_ = TurnoutPosition::MOVING;
        motionStartMs_ = nowMs;

        if (pairedSwitch_) {
            pairedSwitch_->throwSwitch(target, nowMs);
        }

        return true;
    }

    // Advance non-blocking travel timer
    void tick(uint32_t nowMs) {
        if (reported_ == TurnoutPosition::MOVING) {
            if (travelTimeoutMs_ > 0 && (nowMs - motionStartMs_ > travelTimeoutMs_)) {
                // Points failed to make contact within timeout
                reported_ = TurnoutPosition::OUT_OF_CORRESPONDENCE;
            }
        }
    }

    void setTravelTimeout(uint32_t timeoutMs) {
        travelTimeoutMs_ = timeoutMs;
    }

    // Called by hardware driver when point detector contacts settle
    void updateFeedback(TurnoutPosition physicalPoints) {
        reported_ = physicalPoints;
    }

private:
    const char* name_;
    TurnoutPosition commanded_;
    TurnoutPosition reported_;
    TurnoutLock locks_;
    Turnout* pairedSwitch_;
    uint32_t motionStartMs_;
    uint32_t travelTimeoutMs_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_TURNOUT_H
