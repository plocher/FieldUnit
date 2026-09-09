#ifndef FIELDUNIT_SIGNAL_CONTROL_H
#define FIELDUNIT_SIGNAL_CONTROL_H

#include "types.h"

namespace FieldUnit {

// SignalControl appliance (AAR standard: HSR / Home Signal Stick and Authority)
class SignalControl {
public:
    SignalControl() : SignalControl("") {}

    SignalControl(const char* name)
        : name_(name),
          commanded_(DirectionAuthority::STOP),
          active_(DirectionAuthority::STOP),
          fleetMode_(false),
          stickDropped_(false),
          timeLockRunning_(false),
          timeLockExpiryMs_(0),
          timeLockDurationMs_(30000) {}

    const char* name() const { return name_; }

    DirectionAuthority commandedDirection() const { return commanded_; }
    DirectionAuthority activeDirection() const { return active_; }
    bool isFleet() const { return fleetMode_; }
    bool isTimeLocked() const { return timeLockRunning_; }

    // -------------------------------------------------------------
    // AAR Standard Relay Contact Logic
    // -------------------------------------------------------------
    // HSR: Home Signal Stick Relay (picked up = signal clearance authority active)
    bool HSR() const {
        return active_ != DirectionAuthority::STOP;
    }

    // FSR: Fleet Stick Relay (picked up = fleeting mode active)
    bool FSR() const {
        return fleetMode_;
    }

    // ASR: Approach Stick Relay (picked up = plant clear / time expired; dropped = time locked)
    bool ASR() const {
        return !timeLockRunning_;
    }

    // Called when a Control Message arrives from dispatcher or local tower lever
    void updateCommand(DirectionAuthority req, bool fleet, uint32_t nowMs) {
        fleetMode_ = fleet;

        // If direction changes while signal is actively cleared, or if commanded to STOP:
        if (active_ != DirectionAuthority::STOP && req != active_) {
            commanded_ = req;
            active_ = DirectionAuthority::STOP; // Immediately cancel permissive aspect
            stickDropped_ = false;
            // Initiate ASR approach time locking to protect approaching trains
            timeLockRunning_ = true;
            timeLockExpiryMs_ = nowMs + timeLockDurationMs_;
            return;
        }

        // If ASR time-lock is running, cannot clear in any direction until timer expires!
        if (timeLockRunning_) {
            commanded_ = req;
            return; // Held at STOP
        }

        // Fresh code transmission or lever change
        commanded_ = req;
        if (req != DirectionAuthority::STOP) {
            stickDropped_ = false; // Fresh code cycle re-picks up the HSR stick
            active_ = req;
        }
    }

    // Called when train accepts the signal and shunts the entrance/OS circuit
    void knockdown() {
        if (active_ != DirectionAuthority::STOP) {
            active_ = DirectionAuthority::STOP;
            if (!fleetMode_) {
                // Standard AAR HSR stick behavior: stick drops; will NOT re-clear
                // until operator sends a new code command
                stickDropped_ = true;
            }
        }
    }

    // Called every cycle by the interlocking engine
    void evaluate(bool plantClear, uint32_t nowMs) {
        // Advance ASR approach time lock timer
        if (timeLockRunning_) {
            if (nowMs >= timeLockExpiryMs_) {
                timeLockRunning_ = false; // Time expired, ASR picks back up
            }
        }

        // If in fleeting mode (FSR picked up) and plant has cleared, automatically restore authority
        if (fleetMode_ && commanded_ != DirectionAuthority::STOP && active_ == DirectionAuthority::STOP) {
            if (plantClear && !timeLockRunning_) {
                active_ = commanded_; // Re-clear for following train
            }
        }

        // If HSR stick dropped and not fleeting, keep active at STOP
        if (stickDropped_) {
            active_ = DirectionAuthority::STOP;
        }
    }

    void setTimeLockDuration(uint32_t durationMs) {
        timeLockDurationMs_ = durationMs;
    }

private:
    const char* name_;
    DirectionAuthority commanded_;
    DirectionAuthority active_;
    bool fleetMode_;
    bool stickDropped_;
    bool timeLockRunning_;
    uint32_t timeLockExpiryMs_;
    uint32_t timeLockDurationMs_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_SIGNAL_CONTROL_H
