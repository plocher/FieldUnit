#ifndef FIELDUNIT_CONTROL_POINT_H
#define FIELDUNIT_CONTROL_POINT_H

#include "types.h"
#include "TrackCircuit.h"
#include "Turnout.h"
#include "SignalMast.h"
#include "SignalAuthority.h"
#include "ControlTable.h"

namespace FieldUnit {

static constexpr uint8_t MAX_APPLIANCES = 16;

// Ingress: Control Snapshot from dispatcher / CodeLine
struct TurnoutCommand {
    uint8_t turnoutId;
    TurnoutPosition position;
};

struct SignalCommand {
    uint8_t signalId;
    DirectionAuthority direction;
    bool fleet;
    bool callOn;
};

struct ControlSnapshot {
    uint8_t turnoutCommandCount;
    TurnoutCommand turnoutCommands[MAX_APPLIANCES];

    uint8_t signalCommandCount;
    SignalCommand signalCommands[MAX_APPLIANCES];

    bool maintainerCall;
};

// Egress: Indication Snapshot to dispatcher / CodeLine
struct TurnoutReport {
    uint8_t turnoutId;
    TurnoutPosition position;
    TurnoutLock locks;
};

struct TrackCircuitReport {
    uint8_t circuitId;
    Occupancy occupancy;
    Quality quality;
};

struct SignalMastReport {
    uint8_t mastId;
    Indication indication;
    Aspect aspect;
};

struct IndicationSnapshot {
    uint8_t turnoutCount;
    TurnoutReport turnouts[MAX_APPLIANCES];

    uint8_t trackCircuitCount;
    TrackCircuitReport trackCircuits[MAX_APPLIANCES];

    uint8_t mastCount;
    SignalMastReport masts[MAX_APPLIANCES];

    bool maintainerCall;
    bool timeLockActive;
};

class ControlPoint {
public:
    ControlPoint(const char* name)
        : name_(name),
          trackCircuitCount_(0),
          turnoutCount_(0),
          authorityCount_(0),
          mastCount_(0),
          maintainerCallActive_(false),
          detectorLockCouplingCount_(0) {}

    const char* name() const { return name_; }

    TrackCircuit* addTrackCircuit(const char* name) {
        if (trackCircuitCount_ >= MAX_APPLIANCES) return nullptr;
        trackCircuits_[trackCircuitCount_] = TrackCircuit(name);
        return &trackCircuits_[trackCircuitCount_++];
    }

    Turnout* addTurnout(const char* name) {
        if (turnoutCount_ >= MAX_APPLIANCES) return nullptr;
        turnouts_[turnoutCount_] = Turnout(name);
        return &turnouts_[turnoutCount_++];
    }

    SignalAuthority* addAuthority(const char* name) {
        if (authorityCount_ >= MAX_APPLIANCES) return nullptr;
        authorities_[authorityCount_] = SignalAuthority(name);
        return &authorities_[authorityCount_++];
    }

    SignalMast* addSignalMast(const char* name, MastType type) {
        if (mastCount_ >= MAX_APPLIANCES) return nullptr;
        masts_[mastCount_] = SignalMast(name, type);
        return &masts_[mastCount_++];
    }

    // Couple an OS track circuit to detector-lock a turnout
    void bindDetectorLock(Turnout* sw, TrackCircuit* tc) {
        if (detectorLockCouplingCount_ < MAX_APPLIANCES) {
            detectorLocks_[detectorLockCouplingCount_++] = {sw, tc};
        }
    }

    bool addRoute(const RouteDef& route) {
        return engine_.addRoute(route);
    }

    // Ingress: Process incoming control snapshot from dispatcher
    // Binary rule: execute valid moves immediately, reject invalid moves immediately
    bool processControlSnapshot(const ControlSnapshot& ctl, uint32_t nowMs) {
        bool allAccepted = true;

        // 1. Process Turnout commands
        for (uint8_t i = 0; i < ctl.turnoutCommandCount; ++i) {
            const TurnoutCommand& cmd = ctl.turnoutCommands[i];
            if (cmd.turnoutId < turnoutCount_) {
                bool ok = turnouts_[cmd.turnoutId].throwSwitch(cmd.position);
                if (!ok) {
                    allAccepted = false; // Rejected: switch is locked or in use
                }
            }
        }

        // 2. Process Signal authority commands
        for (uint8_t i = 0; i < ctl.signalCommandCount; ++i) {
            const SignalCommand& cmd = ctl.signalCommands[i];
            if (cmd.signalId < authorityCount_) {
                authorities_[cmd.signalId].updateCommand(cmd.direction, cmd.fleet, nowMs);
            }
        }

        maintainerCallActive_ = ctl.maintainerCall;
        return allAccepted;
    }

    // Vital Cycle: Evaluate plant safety and route logic
    void tick(uint32_t nowMs) {
        // A. Clear and re-evaluate Detector Locks based on current track occupancy
        for (uint8_t i = 0; i < turnoutCount_; ++i) {
            turnouts_[i].removeLock(TurnoutLock::DETECTOR_LOCKED);
            turnouts_[i].removeLock(TurnoutLock::ROUTE_LOCKED);
        }

        for (uint8_t i = 0; i < detectorLockCouplingCount_; ++i) {
            Turnout* sw = detectorLocks_[i].sw;
            TrackCircuit* tc = detectorLocks_[i].tc;
            if (!tc->isClear()) {
                sw->addLock(TurnoutLock::DETECTOR_LOCKED);
            }
        }

        // B. Advance remote circuit staleness timers
        for (uint8_t i = 0; i < trackCircuitCount_; ++i) {
            trackCircuits_[i].tick(nowMs);
        }

        // C. Evaluate signal authority stick / fleet logic
        for (uint8_t i = 0; i < authorityCount_; ++i) {
            // Siding plant is clear if all local circuits are clear
            bool allClear = true;
            for (uint8_t j = 0; j < trackCircuitCount_; ++j) {
                if (!trackCircuits_[j].isClear()) {
                    allClear = false;
                    break;
                }
            }
            authorities_[i].evaluate(allClear, nowMs);
        }

        // D. Interlocking Control Table vital route evaluation
        engine_.evaluate();
    }

    // Egress: Generate self-consistent indication snapshot
    void exportIndicationSnapshot(IndicationSnapshot& ind) const {
        ind.turnoutCount = turnoutCount_;
        for (uint8_t i = 0; i < turnoutCount_; ++i) {
            ind.turnouts[i] = {
                i,
                turnouts_[i].reportedPosition(),
                turnouts_[i].activeLocks()
            };
        }

        ind.trackCircuitCount = trackCircuitCount_;
        for (uint8_t i = 0; i < trackCircuitCount_; ++i) {
            ind.trackCircuits[i] = {
                i,
                trackCircuits_[i].state().value,
                trackCircuits_[i].state().quality
            };
        }

        ind.mastCount = mastCount_;
        for (uint8_t i = 0; i < mastCount_; ++i) {
            ind.masts[i] = {
                i,
                masts_[i].currentIndication(),
                masts_[i].head1()
            };
        }

        ind.maintainerCall = maintainerCallActive_;
        ind.timeLockActive = false;
        for (uint8_t i = 0; i < authorityCount_; ++i) {
            if (authorities_[i].isTimeLocked()) {
                ind.timeLockActive = true;
                break;
            }
        }
    }

private:
    struct DetectorBinding {
        Turnout* sw;
        TrackCircuit* tc;
    };

    const char* name_;
    TrackCircuit trackCircuits_[MAX_APPLIANCES];
    uint8_t trackCircuitCount_;

    Turnout turnouts_[MAX_APPLIANCES];
    uint8_t turnoutCount_;

    SignalAuthority authorities_[MAX_APPLIANCES];
    uint8_t authorityCount_;

    SignalMast masts_[MAX_APPLIANCES];
    uint8_t mastCount_;

    bool maintainerCallActive_;

    DetectorBinding detectorLocks_[MAX_APPLIANCES];
    uint8_t detectorLockCouplingCount_;

    InterlockingEngine engine_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_CONTROL_POINT_H
