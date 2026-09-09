#ifndef FIELDUNIT_SIGNAL_AUTHORITY_H
#define FIELDUNIT_SIGNAL_AUTHORITY_H

#include "types.h"

namespace FieldUnit {

class SignalAuthority {
public:
    SignalAuthority() : SignalAuthority("") {}

    SignalAuthority(const char* name)
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

    // AAR Relay Aliases:
    // HSR: Home Signal Stick Relay (picked up = authority active)
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

    // Called when a Control Message arrives from dispatcher
    void updateCommand(DirectionAuthority req, bool fleet, uint32_t nowMs) {
        fleetMode_ = fleet;

        // If dispatcher commands STOP while signal was actively cleared
        if (req == DirectionAuthority::STOP && active_ != DirectionAuthority::STOP) {
            commanded_ = DirectionAuthority::STOP;
            active_ = DirectionAuthority::STOP;
            stickDropped_ = false;
            // Initiate time locking to protect approaching trains
            timeLockRunning_ = true;
            timeLockExpiryMs_ = nowMs + timeLockDurationMs_;
            return;
        }

        // Fresh code transmission from dispatcher
        commanded_ = req;
        if (req != DirectionAuthority::STOP) {
            stickDropped_ = false; // Fresh code cycle re-picks up the stick
            active_ = req;
        }
    }

    // Called when train accepts the signal and shunts the entrance/OS circuit
    void knockdown() {
        if (active_ != DirectionAuthority::STOP) {
            active_ = DirectionAuthority::STOP;
            if (!fleetMode_) {
                // Standard stick behavior: stick drops; will NOT re-clear
                // until dispatcher sends a new code command
                stickDropped_ = true;
            }
        }
    }

    // Called every cycle by the interlocking engine
    void evaluate(bool plantClear, uint32_t nowMs) {
        // Advance approach time lock timer
        if (timeLockRunning_) {
            if (nowMs >= timeLockExpiryMs_) {
                timeLockRunning_ = false; // Time expired, plant is freed
            }
        }

        // If in fleeting mode and plant has cleared, automatically restore authority
        if (fleetMode_ && commanded_ != DirectionAuthority::STOP && active_ == DirectionAuthority::STOP) {
            if (plantClear && !timeLockRunning_) {
                active_ = commanded_; // Re-clear for following train
            }
        }

        // If stick dropped and not fleeting, keep active at STOP
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

#endif // FIELDUNIT_SIGNAL_AUTHORITY_H
