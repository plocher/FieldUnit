#ifndef FIELDUNIT_SWITCH_H
#define FIELDUNIT_SWITCH_H

#include "types.h"

namespace FieldUnit {

/** How a switch is paired with another machine. */
enum class SwitchPairMode : uint8_t {
    NONE = 0,
    CROSSOVER = 1,         // Same-polarity pair (both ends of a crossover)
    DEPENDENT_DERAIL = 2   // Inverse-polarity pair (main switch + protecting derail)
};

/**
 * TrackSwitch / SwitchMachine appliance (AAR standard: Switch, not Turnout).
 *
 * Derails are switch-shaped appliances (often points + machine, no frog).
 * NORMAL on a derail = off-rail / clear (train may pass).
 * REVERSE on a derail = on-rail / active (cars are dumped).
 */
class Switch {
public:
    Switch() : Switch("") {}
    virtual ~Switch() = default;

    Switch(const char* name)
        : commanded_(SwitchPosition::NORMAL),
          reported_(SwitchPosition::NORMAL),
          locks_(SwitchLock::UNLOCKED),
          pairedSwitch_(nullptr),
          pairMode_(SwitchPairMode::NONE),
          isDerail_(false),
          isDependentSlave_(false),
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

    bool isDerail() const { return isDerail_; }
    bool isDependentDerail() const { return isDependentSlave_; }
    bool appearsOnCodeLine() const { return !isDependentSlave_; }
    SwitchPairMode pairMode() const { return pairMode_; }

    /** Master-side accessor: dependent derail appliance, or nullptr. */
    Switch* dependentDerail() const {
        if (pairMode_ == SwitchPairMode::DEPENDENT_DERAIL && !isDependentSlave_) {
            return pairedSwitch_;
        }
        return nullptr;
    }

    /**
     * Mark this appliance as a derail and set fail-safe on-rail rest position.
     * Called by ControlPoint::addDerail before optional dependence pairing.
     */
    void configureAsDerail() {
        isDerail_ = true;
        forceSettledPosition(SwitchPosition::REVERSE);
    }

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
        if (!selfInCorrespondence()) {
            return false;
        }
        if (pairMode_ == SwitchPairMode::DEPENDENT_DERAIL && !isDependentSlave_ && pairedSwitch_) {
            const SwitchPosition expect = inversePosition(commanded_);
            return pairedSwitch_->commandedPosition() == expect &&
                   pairedSwitch_->reportedPosition() == expect &&
                   pairedSwitch_->selfInCorrespondence();
        }
        if (pairMode_ == SwitchPairMode::CROSSOVER && pairedSwitch_) {
            return pairedSwitch_->selfInCorrespondence() &&
                   pairedSwitch_->reportedPosition() == reported_;
        }
        return true;
    }

    virtual bool NWCR() const {
        return reported_ == SwitchPosition::NORMAL && inCorrespondence();
    }

    virtual bool RWCR() const {
        return reported_ == SwitchPosition::REVERSE && inCorrespondence();
    }

    virtual bool KR() const {
        return inCorrespondence();
    }

    virtual bool WLR() const {
        return isMovable();
    }

    // Pair a crossover switch (bidirectional: both move and lock together, same polarity)
    void pairCrossover(Switch* other) {
        pairedSwitch_ = other;
        pairMode_ = SwitchPairMode::CROSSOVER;
        isDependentSlave_ = false;
        if (other) {
            other->pairedSwitch_ = this;
            other->pairMode_ = SwitchPairMode::CROSSOVER;
            other->isDependentSlave_ = false;
        }
    }

    /**
     * Pair this mainline switch with a dependent derail (inverse polarity).
     * Master NORMAL => derail REVERSE (on-rail).
     * Master REVERSE => derail NORMAL (clear).
     */
    void pairDependentDerail(Switch* derail) {
        if (!derail) return;
        pairedSwitch_ = derail;
        pairMode_ = SwitchPairMode::DEPENDENT_DERAIL;
        isDependentSlave_ = false;
        derail->pairedSwitch_ = this;
        derail->pairMode_ = SwitchPairMode::DEPENDENT_DERAIL;
        derail->isDependentSlave_ = true;
        derail->isDerail_ = true;
        // Rest: main stays as-is (typically NORMAL); derail on-rail
        derail->forceSettledPosition(inversePosition(commanded_));
    }

    Switch* pairedSwitch() const { return pairedSwitch_; }

    virtual void addLock(SwitchLock lock) {
        if ((locks_ & lock) == lock) {
            return;
        }
        locks_ = locks_ | lock;
        if (pairedSwitch_) {
            pairedSwitch_->addLock(lock);
        }
    }

    virtual void removeLock(SwitchLock lock) {
        if ((locks_ & lock) == SwitchLock::UNLOCKED) {
            return;
        }
        locks_ = static_cast<SwitchLock>(static_cast<uint8_t>(locks_) & ~static_cast<uint8_t>(lock));
        if (pairedSwitch_) {
            pairedSwitch_->removeLock(lock);
        }
    }

    virtual bool throwSwitch(SwitchPosition target, uint32_t nowMs = 0) {
        if (target != SwitchPosition::NORMAL && target != SwitchPosition::REVERSE) {
            return false;
        }

        // Dependent derails are not commanded directly from the CodeLine
        if (isDependentSlave_) {
            return false;
        }

        if (!WLR()) {
            return false;
        }

        if (pairMode_ == SwitchPairMode::DEPENDENT_DERAIL && pairedSwitch_) {
            if (!pairedSwitch_->WLR()) {
                return false;
            }
            const SwitchPosition derailTarget = inversePosition(target);
            const bool okSelf = throwSelf(target, nowMs);
            pairedSwitch_->throwSelf(derailTarget, nowMs);
            return okSelf;
        }

        if (commanded_ == target && inCorrespondence()) {
            return true;
        }

        const bool ok = throwSelf(target, nowMs);
        if (pairMode_ == SwitchPairMode::CROSSOVER && pairedSwitch_ &&
            pairedSwitch_->commandedPosition() != target) {
            pairedSwitch_->throwSelf(target, nowMs);
        }
        return ok;
    }

    virtual void tick(uint32_t nowMs) {
        if (reported_ == SwitchPosition::MOVING) {
            if (travelTimeoutMs_ > 0 && (nowMs - motionStartMs_ > travelTimeoutMs_)) {
                reported_ = SwitchPosition::OUT_OF_CORRESPONDENCE;
            }
        }
    }

    void setTravelTimeout(uint32_t timeoutMs) {
        travelTimeoutMs_ = timeoutMs;
    }

    virtual void updateFeedback(SwitchPosition physicalPoints) {
        reported_ = physicalPoints;
    }

    /** Configuration helper: set commanded and reported without motion. */
    void forceSettledPosition(SwitchPosition pos) {
        commanded_ = pos;
        reported_ = pos;
    }

    static SwitchPosition inversePosition(SwitchPosition pos) {
        if (pos == SwitchPosition::NORMAL) return SwitchPosition::REVERSE;
        if (pos == SwitchPosition::REVERSE) return SwitchPosition::NORMAL;
        return pos;
    }

private:
    bool selfInCorrespondence() const {
        return (reported_ == commanded_) &&
               (reported_ == SwitchPosition::NORMAL || reported_ == SwitchPosition::REVERSE);
    }

    bool throwSelf(SwitchPosition target, uint32_t nowMs) {
        if (commanded_ == target && selfInCorrespondence()) {
            return true;
        }
        commanded_ = target;
        reported_ = SwitchPosition::MOVING;
        motionStartMs_ = nowMs;
        return true;
    }

    char name_[MAX_APPLIANCE_NAME_LEN];
    SwitchPosition commanded_;
    SwitchPosition reported_;
    SwitchLock locks_;
    Switch* pairedSwitch_;
    SwitchPairMode pairMode_;
    bool isDerail_;
    bool isDependentSlave_;
    uint8_t index_;
    uint32_t motionStartMs_;
    uint32_t travelTimeoutMs_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_SWITCH_H
