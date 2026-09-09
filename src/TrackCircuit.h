#ifndef FIELDUNIT_TRACK_CIRCUIT_H
#define FIELDUNIT_TRACK_CIRCUIT_H

#include "types.h"

namespace FieldUnit {

class TrackCircuit {
public:
    TrackCircuit() : TrackCircuit("") {}

    TrackCircuit(const char* name)
        : name_(name),
          state_{Occupancy::OCCUPIED, Quality::GOOD, 0},
          stalenessTimeoutMs_(5000),
          lastUpdateMs_(0) {}

    const char* name() const { return name_; }

    // Vital query: is this block safe for train movement?
    // Fail-safe rule: Must be VACANT AND GOOD quality.
    bool isClear() const {
        return (state_.value == Occupancy::VACANT) && (state_.quality == Quality::GOOD);
    }

    /**
     * AAR Relay Equivalent: TR (Track Relay)
     *
     * In prototype relay signaling:
     * - TR is energized (picked up) when the rails are un-shunted (VACANT).
     * - TR drops out by gravity when train wheels/axles shunt the rails (OCCUPIED).
     * - Broken rails, power losses, or failed sensors drop TR fail-safe.
     *
     * @return true if the block is VACANT and communication quality is GOOD.
     */
    bool TR() const {
        return isClear();
    }

    // Direct access to qualified state
    const Qualified<Occupancy>& state() const { return state_; }

    // Update from local sensor driver or network packet
    void update(Occupancy occ, Quality quality = Quality::GOOD, uint32_t nowMs = 0) {
        state_.value = occ;
        state_.quality = quality;
        state_.ageMs = 0;
        lastUpdateMs_ = nowMs;
    }

    // Advance clock to detect lost communication on remote approach circuits
    void tick(uint32_t nowMs) {
        if (lastUpdateMs_ > 0 && (nowMs - lastUpdateMs_ > stalenessTimeoutMs_)) {
            // Remote CP stopped broadcasting heartbeat / indications
            state_.quality = Quality::LOST_COMMS;
        }
        if (lastUpdateMs_ > 0) {
            state_.ageMs = nowMs - lastUpdateMs_;
        }
    }

    void setStalenessTimeout(uint32_t timeoutMs) {
        stalenessTimeoutMs_ = timeoutMs;
    }

private:
    const char* name_;
    Qualified<Occupancy> state_;
    uint32_t stalenessTimeoutMs_;
    uint32_t lastUpdateMs_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_TRACK_CIRCUIT_H
