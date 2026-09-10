#ifndef FIELDUNIT_SIGNAL_CONTROL_H
#define FIELDUNIT_SIGNAL_CONTROL_H

#include "types.h"

namespace FieldUnit {

// SignalControl appliance (AAR standard: HSR / Home Signal Stick and Authority)
class SignalControl {
public:
    SignalControl() : SignalControl("") {}

    SignalControl(const char* name)
        : commanded_(DirectionAuthority::STOP),
          active_(DirectionAuthority::STOP),
          fleetMode_(false),
          index_(0),
          stickDropped_(false),
          timeLockRunning_(false),
          timeLockDirection_(DirectionAuthority::STOP),
          timeLockExpiryMs_(0),
          timeLockDurationMs_(30000) {
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

    DirectionAuthority commandedDirection() const { return commanded_; }
    DirectionAuthority activeDirection() const { return active_; }
    bool isFleet() const { return fleetMode_; }
    bool isTimeLocked() const { return timeLockRunning_; }
    DirectionAuthority timeLockDirection() const { return timeLockDirection_; }

    uint32_t approachTimeRemainingMs(uint32_t nowMs) const {
        if (!timeLockRunning_ || nowMs >= timeLockExpiryMs_) return 0;
        return timeLockExpiryMs_ - nowMs;
    }

    // -------------------------------------------------------------
    // AAR Standard Relay Contact Logic
    // -------------------------------------------------------------

    /**
     * AAR Relay: HSR (Home Signal Stick Relay)
     *
     * In prototype relay signaling:
     * - Picks up when dispatcher transmits directional code authority.
     * - Stays energized through its own front contact (stick path).
     * - Drops immediately when a train enters the entrance track circuit (1TR).
     * - Remains dropped until the dispatcher transmits a brand new code.
     *
     * @return true if movement authority is actively latched.
     */
    bool HSR() const {
        return active_ != DirectionAuthority::STOP;
    }

    /**
     * AAR Relay: FSR (Fleet Stick Relay)
     *
     * In prototype CTC:
     * - Energized when dispatcher toggles Fleeting ON.
     * - Bypasses the HSR stick-down requirement.
     * - When a train departs and clears the route, the signal re-clears
     *   automatically for following trains without dispatcher intervention.
     *
     * @return true if fleeting mode is active.
     */
    bool FSR() const {
        return fleetMode_;
    }

    /**
     * AAR Relay: ASR (Approach Stick Relay)
     *
     * Enforces Approach and Time Locking:
     * - Picks up (energizes) when the plant is at rest (no permissive signals).
     * - Drops when a signal clears, locking all route switches.
     * - If the dispatcher cancels a clear signal before train arrival,
     *   ASR stays dropped while a safety timer runs down (30-60s model, 3-5m proto).
     * - While ASR is dropped, no switch can move and no opposing signal can clear.
     *
     * @return true if plant is free (time lock is NOT running).
     */
    bool ASR() const {
        return !timeLockRunning_;
    }

    // Called when a Control Message arrives from dispatcher or local tower lever
    // Implements prototype Approach Locking (ASR):
    // - If cancelled while approach track is VACANT: plant releases immediately with zero delay.
    // - If cancelled while approach track is OCCUPIED: ASR drops and engages countdown timer (TER).
    void updateCommand(DirectionAuthority req, bool fleet, uint32_t nowMs, bool approachOccupied = true) {
        fleetMode_ = fleet;

        // If direction changes while signal is actively cleared, or if commanded to STOP:
        if (active_ != DirectionAuthority::STOP && req != active_) {
            DirectionAuthority prevActive = active_;
            commanded_ = req;
            active_ = DirectionAuthority::STOP; // Immediately cancel permissive aspect
            stickDropped_ = false;

            if (approachOccupied) {
                // Hazardous cancellation: train is approaching, engage ASR time lock countdown
                timeLockRunning_ = true;
                timeLockDirection_ = prevActive;
                timeLockExpiryMs_ = nowMs + timeLockDurationMs_;
            } else {
                // Safe cancellation: approach track is vacant, release plant immediately!
                timeLockRunning_ = false;
                timeLockDirection_ = DirectionAuthority::STOP;
                timeLockExpiryMs_ = 0;
            }
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
                timeLockDirection_ = DirectionAuthority::STOP;
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
    char name_[32];
    DirectionAuthority commanded_;
    DirectionAuthority active_;
    bool fleetMode_;
    uint8_t index_;
    bool stickDropped_;
    bool timeLockRunning_;
    DirectionAuthority timeLockDirection_;
    uint32_t timeLockExpiryMs_;
    uint32_t timeLockDurationMs_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_SIGNAL_CONTROL_H
