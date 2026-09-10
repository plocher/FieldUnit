#ifndef FIELDUNIT_CROSSOVER_H
#define FIELDUNIT_CROSSOVER_H

#include "Switch.h"

namespace FieldUnit {

/**
 * Crossover Appliance
 *
 * Couples two physical track switches (swA and swB) operated as a single logical crossover.
 * Inherits from Switch so it can be passed directly to Route::aligns() and wire codecs.
 *
 * In accordance with AAR prototype interlocking rules:
 * - Both switches are commanded in unison.
 * - Reports NORMAL only when BOTH switches report NORMAL.
 * - Reports REVERSE only when BOTH switches report REVERSE.
 * - If either switch is in transit, reports MOVING.
 * - If points are gapped or mismatched, reports OUT_OF_CORRESPONDENCE.
 */
class Crossover : public Switch {
public:
    Crossover()
        : Switch(""), swA_(nullptr), swB_(nullptr) {}

    Crossover(const char* name, Switch* swA, Switch* swB)
        : Switch(name), swA_(swA), swB_(swB) {
        if (swA_ && swB_) {
            swA_->pairCrossover(swB_);
        }
    }

    Switch* switchA() const { return swA_; }
    Switch* switchB() const { return swB_; }

    SwitchPosition commandedPosition() const override {
        return swA_ ? swA_->commandedPosition() : SwitchPosition::NORMAL;
    }

    SwitchPosition reportedPosition() const override {
        if (!swA_ || !swB_) return SwitchPosition::UNKNOWN;

        SwitchPosition posA = swA_->reportedPosition();
        SwitchPosition posB = swB_->reportedPosition();

        if (posA == SwitchPosition::NORMAL && posB == SwitchPosition::NORMAL) {
            return SwitchPosition::NORMAL;
        }
        if (posA == SwitchPosition::REVERSE && posB == SwitchPosition::REVERSE) {
            return SwitchPosition::REVERSE;
        }
        if (posA == SwitchPosition::MOVING || posB == SwitchPosition::MOVING) {
            return SwitchPosition::MOVING;
        }
        return SwitchPosition::OUT_OF_CORRESPONDENCE;
    }

    bool inCorrespondence() const override {
        if (!swA_ || !swB_) return false;
        return swA_->inCorrespondence() &&
               swB_->inCorrespondence() &&
               (swA_->reportedPosition() == swB_->reportedPosition());
    }

    bool isMovable() const override {
        if (!swA_ || !swB_) return false;
        return swA_->isMovable() && swB_->isMovable();
    }

    SwitchLock activeLocks() const override {
        SwitchLock locksA = swA_ ? swA_->activeLocks() : SwitchLock::UNLOCKED;
        SwitchLock locksB = swB_ ? swB_->activeLocks() : SwitchLock::UNLOCKED;
        return locksA | locksB;
    }

    void addLock(SwitchLock lock) override {
        if (swA_) swA_->addLock(lock);
        if (swB_) swB_->addLock(lock);
    }

    void removeLock(SwitchLock lock) override {
        if (swA_) swA_->removeLock(lock);
        if (swB_) swB_->removeLock(lock);
    }

    bool throwSwitch(SwitchPosition target, uint32_t nowMs = 0) override {
        if (!isMovable()) return false;
        bool okA = swA_ ? swA_->throwSwitch(target, nowMs) : false;
        bool okB = swB_ ? swB_->throwSwitch(target, nowMs) : false;
        return okA && okB;
    }

    void tick(uint32_t nowMs) override {
        if (swA_) swA_->tick(nowMs);
        if (swB_) swB_->tick(nowMs);
    }

private:
    Switch* swA_;
    Switch* swB_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_CROSSOVER_H
