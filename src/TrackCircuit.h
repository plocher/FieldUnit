#ifndef FIELDUNIT_TRACK_CIRCUIT_H
#define FIELDUNIT_TRACK_CIRCUIT_H

#include "types.h"

namespace FieldUnit {

class TrackCircuit {
public:
    TrackCircuit() : TrackCircuit("", 0) {}

    TrackCircuit(const char* name, uint32_t dropoutDelayMs = 0)
        : name_(name),
          state_{Occupancy::OCCUPIED, Quality::GOOD, 0},
          rawOccupancy_(Occupancy::OCCUPIED),
          index_(0),
          dropoutDelayMs_(dropoutDelayMs),
          clearanceStartMs_(0),
          clearingActive_(false),
          stalenessTimeoutMs_(5000),
          lastUpdateMs_(0) {}

    const char* name() const { return name_; }
    uint8_t index() const { return index_; }
    void setIndex(uint8_t idx) { index_ = idx; }

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
    // Shunting to OCCUPIED happens immediately.
    // Clearing to VACANT applies dropoutDelayMs to bridge inter-car optical sensor gaps.
    void update(Occupancy occ, Quality quality = Quality::GOOD, uint32_t nowMs = 0) {
        rawOccupancy_ = occ;
        state_.quality = quality;
        lastUpdateMs_ = nowMs;

        if (occ == Occupancy::OCCUPIED) {
            // Immediate vital occupancy: cancel any clearance countdown
            state_.value = Occupancy::OCCUPIED;
            clearingActive_ = false;
            state_.ageMs = 0;
        } else {
            // Sensor reports vacant
            if (dropoutDelayMs_ == 0) {
                state_.value = Occupancy::VACANT;
                clearingActive_ = false;
            } else if (state_.value == Occupancy::OCCUPIED && !clearingActive_) {
                // Start dropout delay countdown
                clearingActive_ = true;
                clearanceStartMs_ = nowMs;
            }
        }
    }

    // Advance clock to evaluate dropout delay and remote staleness
    void tick(uint32_t nowMs) {
        // Evaluate dropout delay (hysteresis) for optical sensors
        if (clearingActive_) {
            if (nowMs - clearanceStartMs_ >= dropoutDelayMs_) {
                state_.value = Occupancy::VACANT;
                clearingActive_ = false;
                state_.ageMs = 0;
            }
        }

        // Remote approach circuit staleness detection
        if (lastUpdateMs_ > 0 && (nowMs - lastUpdateMs_ > stalenessTimeoutMs_)) {
            state_.quality = Quality::LOST_COMMS;
        }
        if (lastUpdateMs_ > 0) {
            state_.ageMs = nowMs - lastUpdateMs_;
        }
    }

    void setDropoutDelay(uint32_t delayMs) {
        dropoutDelayMs_ = delayMs;
    }

    void setStalenessTimeout(uint32_t timeoutMs) {
        stalenessTimeoutMs_ = timeoutMs;
    }

private:
    const char* name_;
    Qualified<Occupancy> state_;
    Occupancy rawOccupancy_;
    uint8_t index_;
    uint32_t dropoutDelayMs_;
    uint32_t clearanceStartMs_;
    bool clearingActive_;
    uint32_t stalenessTimeoutMs_;
    uint32_t lastUpdateMs_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_TRACK_CIRCUIT_H
