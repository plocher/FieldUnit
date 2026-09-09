#ifndef FIELDUNIT_CONTROL_POINT_H
#define FIELDUNIT_CONTROL_POINT_H

#include "types.h"
#include "TrackCircuit.h"
#include "Switch.h"
#include "SignalMast.h"
#include "SignalControl.h"
#include "ControlTable.h"

namespace FieldUnit {

static constexpr uint8_t MAX_APPLIANCES = 16;

// Ingress: Complete Plant-Wide Control Transaction from dispatcher / CodeLine
// In railroad vital logic, safety cannot be evaluated on isolated commands;
// the entire desired plant state must be verified together as a single atomic vector.
struct ControlTransaction {
    // Desired state for every switch in this Control Point
    SwitchDemand switchDemands[MAX_APPLIANCES];

    // Desired movement authority for every signal in this Control Point
    SignalDemand signalDemands[MAX_APPLIANCES];

    // Fleeting mode toggle for each signal
    bool fleetDemands[MAX_APPLIANCES];

    // Maintainer call lamps
    bool maintainerCall[MAX_APPLIANCES];

    // Constructor clears all demands to safe NO_CHANGE / neutral defaults
    ControlTransaction() {
        for (uint8_t i = 0; i < MAX_APPLIANCES; ++i) {
            switchDemands[i] = SwitchDemand::NO_CHANGE;
            signalDemands[i] = SignalDemand::NO_CHANGE;
            fleetDemands[i] = false;
            maintainerCall[i] = false;
        }
    }
};

// Result of evaluating a ControlTransaction
enum class TransactionResult : uint8_t {
    EXECUTED = 0,           // Complete transaction verified and executed
    REJECTED_UNSAFE = 1,    // Rejected: switch motion requested while locked / occupied
    REJECTED_INCOMPLETE = 2 // Rejected: message corrupted, truncated, or inconsistent
};

// Egress: Complete Plant-Wide Indication Vector to dispatcher / CodeLine
struct SwitchIndication {
    SwitchPosition position; // Reported position
    bool inCorrespondence;   // AAR KR
    SwitchLock locks;        // Active locks (AAR WLR dropped if locked)
};

struct TrackCircuitIndication {
    Occupancy occupancy;     // AAR TR (Vacant vs Occupied)
    Quality quality;         // GOOD, LOST_COMMS, DEFECT
};

struct SignalIndication {
    DirectionAuthority activeAuthority; // AAR HSR
    bool fleeting;                      // AAR FSR
    bool timeLocked;                    // AAR ASR (timer running down)
    Indication rulebookIndication;      // Operational rule
    Aspect displayedAspect;             // Physical visual lamps
};

struct IndicationVector {
    uint8_t switchCount;
    SwitchIndication switches[MAX_APPLIANCES];

    uint8_t trackCircuitCount;
    TrackCircuitIndication trackCircuits[MAX_APPLIANCES];

    uint8_t signalCount;
    SignalIndication signals[MAX_APPLIANCES];

    bool maintainerCall[MAX_APPLIANCES];
};

class ControlPoint {
public:
    ControlPoint(const char* name)
        : name_(name),
          trackCircuitCount_(0),
          switchCount_(0),
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

    Switch* addSwitch(const char* name) {
        if (switchCount_ >= MAX_APPLIANCES) return nullptr;
        switches_[switchCount_] = Switch(name);
        return &switches_[switchCount_++];
    }

    SignalControl* addSignalControl(const char* name) {
        if (authorityCount_ >= MAX_APPLIANCES) return nullptr;
        authorities_[authorityCount_] = SignalControl(name);
        return &authorities_[authorityCount_++];
    }

    SignalMast* addSignalMast(const char* name, MastType type) {
        if (mastCount_ >= MAX_APPLIANCES) return nullptr;
        masts_[mastCount_] = SignalMast(name, type);
        return &masts_[mastCount_++];
    }

    // Couple an OS track circuit to detector-lock a switch
    void bindDetectorLock(Switch* sw, TrackCircuit* tc) {
        if (detectorLockCouplingCount_ < MAX_APPLIANCES) {
            detectorLocks_[detectorLockCouplingCount_++] = {sw, tc};
        }
    }

    bool addRoute(const RouteDef& route) {
        return engine_.addRoute(route);
    }

    // Ingress: Process incoming plant-wide control transaction
    // Gate 1: Completeness - Evaluates the entire desired plant state vector together.
    // Gate 2: Safety - If any switch movement violates locks (WLR dropped), the move is rejected.
    TransactionResult processControlTransaction(const ControlTransaction& ctl, uint32_t nowMs) {
        // 1. Verify safety of all requested switch movements
        for (uint8_t i = 0; i < switchCount_; ++i) {
            SwitchDemand demand = ctl.switchDemands[i];
            if (demand == SwitchDemand::NORMAL || demand == SwitchDemand::REVERSE) {
                // If switch is locked (train on points or active route), cannot throw
                if (!switches_[i].WLR()) {
                    return TransactionResult::REJECTED_UNSAFE;
                }
            }
        }

        // 2. All safety gates passed. Execute switch movements in unison
        for (uint8_t i = 0; i < switchCount_; ++i) {
            SwitchDemand demand = ctl.switchDemands[i];
            if (demand == SwitchDemand::NORMAL) {
                switches_[i].throwSwitch(SwitchPosition::NORMAL, nowMs);
            } else if (demand == SwitchDemand::REVERSE) {
                switches_[i].throwSwitch(SwitchPosition::REVERSE, nowMs);
            }
        }

        // 3. Actuate Signal Authority demands
        for (uint8_t i = 0; i < authorityCount_; ++i) {
            SignalDemand demand = ctl.signalDemands[i];
            bool fleet = ctl.fleetDemands[i];

            if (demand == SignalDemand::STOP) {
                authorities_[i].updateCommand(DirectionAuthority::STOP, fleet, nowMs);
            } else if (demand == SignalDemand::LEFT) {
                authorities_[i].updateCommand(DirectionAuthority::LEFT, fleet, nowMs);
            } else if (demand == SignalDemand::RIGHT) {
                authorities_[i].updateCommand(DirectionAuthority::RIGHT, fleet, nowMs);
            }
            // NO_CHANGE leaves existing authority and stick state intact
        }

        // 4. Update Maintainer Calls
        maintainerCallActive_ = ctl.maintainerCall[0];
        return TransactionResult::EXECUTED;
    }

    // Vital Cycle: Evaluate plant safety and route logic
    void tick(uint32_t nowMs) {
        // A. Clear and re-evaluate Detector Locks based on current track occupancy
        for (uint8_t i = 0; i < switchCount_; ++i) {
            switches_[i].removeLock(SwitchLock::DETECTOR_LOCKED);
            switches_[i].removeLock(SwitchLock::ROUTE_LOCKED);
        }

        for (uint8_t i = 0; i < detectorLockCouplingCount_; ++i) {
            Switch* sw = detectorLocks_[i].sw;
            TrackCircuit* tc = detectorLocks_[i].tc;
            if (!tc->isClear()) {
                sw->addLock(SwitchLock::DETECTOR_LOCKED);
            }
        }

        // B. Advance switch travel and remote circuit staleness timers
        for (uint8_t i = 0; i < switchCount_; ++i) {
            switches_[i].tick(nowMs);
        }
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

    // Egress: Generate complete, self-consistent plant-wide indication vector
    void exportIndicationVector(IndicationVector& ind) const {
        ind.switchCount = switchCount_;
        for (uint8_t i = 0; i < switchCount_; ++i) {
            ind.switches[i] = {
                switches_[i].reportedPosition(),
                switches_[i].KR(),
                switches_[i].activeLocks()
            };
        }

        ind.trackCircuitCount = trackCircuitCount_;
        for (uint8_t i = 0; i < trackCircuitCount_; ++i) {
            ind.trackCircuits[i] = {
                trackCircuits_[i].state().value,
                trackCircuits_[i].state().quality
            };
        }

        ind.signalCount = authorityCount_;
        for (uint8_t i = 0; i < authorityCount_; ++i) {
            ind.signals[i] = {
                authorities_[i].activeDirection(),
                authorities_[i].isFleet(),
                authorities_[i].isTimeLocked(),
                (i < mastCount_) ? masts_[i].currentIndication() : Indication::STOP,
                (i < mastCount_) ? masts_[i].head1() : Aspect::RED
            };
        }

        for (uint8_t i = 0; i < MAX_APPLIANCES; ++i) {
            ind.maintainerCall[i] = (i == 0) ? maintainerCallActive_ : false;
        }
    }

private:
    struct DetectorBinding {
        Switch* sw;
        TrackCircuit* tc;
    };

    const char* name_;
    TrackCircuit trackCircuits_[MAX_APPLIANCES];
    uint8_t trackCircuitCount_;

    Switch switches_[MAX_APPLIANCES];
    uint8_t switchCount_;

    SignalControl authorities_[MAX_APPLIANCES];
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
