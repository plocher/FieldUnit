#ifndef FIELDUNIT_CONTROL_POINT_H
#define FIELDUNIT_CONTROL_POINT_H

#include <string.h>
#include "types.h"
#include "TrackCircuit.h"
#include "Switch.h"
#include "Crossover.h"
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

    // Electric switch lock demands (WLS)
    ElectricLockDemand lockDemands[MAX_APPLIANCES];

    // Desired movement authority for every signal in this Control Point
    SignalDemand signalDemands[MAX_APPLIANCES];

    // Fleeting mode toggle for each signal
    bool fleetDemands[MAX_APPLIANCES];

    // Maintainer call lamps
    bool maintainerCall[MAX_APPLIANCES];

    // Safety gate: certified true only if transaction has zero vital conflicts
    bool vitalValid;

    // Constructor clears all demands to safe NO_CHANGE / neutral defaults
    ControlTransaction() : vitalValid(true) {
        for (uint8_t i = 0; i < MAX_APPLIANCES; ++i) {
            switchDemands[i] = SwitchDemand::NO_CHANGE;
            lockDemands[i] = ElectricLockDemand::NO_CHANGE;
            signalDemands[i] = SignalDemand::NO_CHANGE;
            fleetDemands[i] = false;
            maintainerCall[i] = false;
        }
    }
};

// Egress: Complete Plant-Wide Indication Vector to dispatcher / CodeLine
struct SwitchIndication {
    SwitchPosition position; // Reported position
    bool inCorrespondence;   // AAR KR
    SwitchLock locks;        // Active locks (AAR WLR dropped if locked)
    bool electricLockUnlocked; // AAR WLK (true if electric lock released)
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

struct MastIndication {
    Indication rulebookIndication;      // Operational rule
    Aspect displayedAspect;             // Composite visual appearance
    Aspect head1;
    Aspect head2;
    Aspect head3;
    uint8_t markers;                    // B&O CPL / Position light orbital markers
};

struct IndicationVector {
    uint8_t switchCount;
    SwitchIndication switches[MAX_APPLIANCES];

    uint8_t trackCircuitCount;
    TrackCircuitIndication trackCircuits[MAX_APPLIANCES];

    uint8_t signalCount;
    SignalIndication signals[MAX_APPLIANCES];

    uint8_t mastCount;
    MastIndication masts[MAX_APPLIANCES];

    bool maintainerCall[MAX_APPLIANCES];
};

class ControlPoint {
public:
    ControlPoint(const char* name, AspectResolver defaultPolicy = AspectPolicies::defaultRoute)
        : name_(name),
          defaultAspectPolicy_(defaultPolicy ? defaultPolicy : AspectPolicies::defaultRoute),
          trackCircuitCount_(0),
          switchCount_(0),
          authorityCount_(0),
          mastCount_(0),
          crossoverCount_(0),
          maintainerCallActive_(false),
          detectorLockCouplingCount_(0) {}

    const char* name() const { return name_; }

    uint8_t trackCircuitCount() const { return trackCircuitCount_; }
    TrackCircuit* trackCircuit(uint8_t idx) { return (idx < trackCircuitCount_) ? &trackCircuits_[idx] : nullptr; }

    uint8_t switchCount() const { return switchCount_; }
    Switch* getSwitch(uint8_t idx) { return (idx < switchCount_) ? &switches_[idx] : nullptr; }

    uint8_t mastCount() const { return mastCount_; }
    SignalMast* mast(uint8_t idx) { return (idx < mastCount_) ? &masts_[idx] : nullptr; }

    void setDefaultAspectPolicy(AspectResolver policy) {
        defaultAspectPolicy_ = policy ? policy : AspectPolicies::defaultRoute;
    }

    AspectResolver defaultAspectPolicy() const { return defaultAspectPolicy_; }

    TrackCircuit* addTrackCircuit(const char* name, uint32_t dropoutDelayMs = 0) {
        if (trackCircuitCount_ >= MAX_APPLIANCES) return nullptr;
        trackCircuits_[trackCircuitCount_] = TrackCircuit(name, dropoutDelayMs);
        trackCircuits_[trackCircuitCount_].setIndex(trackCircuitCount_);
        return &trackCircuits_[trackCircuitCount_++];
    }

    Switch* addSwitch(const char* name) {
        if (switchCount_ >= MAX_APPLIANCES) return nullptr;
        switches_[switchCount_] = Switch(name);
        switches_[switchCount_].setIndex(switchCount_);
        return &switches_[switchCount_++];
    }

    Crossover* addCrossover(const char* name, Switch* swA, Switch* swB) {
        if (crossoverCount_ >= MAX_APPLIANCES) return nullptr;
        crossovers_[crossoverCount_] = Crossover(name, swA, swB);
        return &crossovers_[crossoverCount_++];
    }

    SignalControl* addSignalControl(const char* name) {
        if (authorityCount_ >= MAX_APPLIANCES) return nullptr;
        authorities_[authorityCount_] = SignalControl(name);
        authorities_[authorityCount_].setIndex(authorityCount_);
        return &authorities_[authorityCount_++];
    }

    SignalMast* addSignalMast(const char* name, MastType type, AspectResolver policy = nullptr) {
        if (mastCount_ >= MAX_APPLIANCES) return nullptr;
        AspectResolver effectivePolicy = policy ? policy : defaultAspectPolicy_;
        masts_[mastCount_] = SignalMast(name, type, effectivePolicy);
        masts_[mastCount_].setIndex(mastCount_);
        return &masts_[mastCount_++];
    }

    // Couple an OS track circuit to detector-lock a switch
    void bindDetectorLock(Switch* sw, TrackCircuit* tc) {
        if (detectorLockCouplingCount_ < MAX_APPLIANCES) {
            detectorLocks_[detectorLockCouplingCount_++] = {sw, tc};
        }
    }

    void bindDetectorLock(const char* switchName, const char* trackCircuitName) {
        Switch* sw = findSwitch(switchName);
        TrackCircuit* tc = findTrackCircuit(trackCircuitName);
        if (sw && tc) {
            bindDetectorLock(sw, tc);
        }
    }

    Crossover* addCrossover(const char* name, const char* swAName, const char* swBName) {
        Switch* swA = findSwitch(swAName);
        Switch* swB = findSwitch(swBName);
        if (swA && swB) {
            return addCrossover(name, swA, swB);
        }
        return nullptr;
    }

    // Configuration-time name lookups
    TrackCircuit* findTrackCircuit(const char* name) {
        if (!name) return nullptr;
        for (uint8_t i = 0; i < trackCircuitCount_; ++i) {
            if (strcmp(trackCircuits_[i].name(), name) == 0) return &trackCircuits_[i];
        }
        return nullptr;
    }

    Switch* findSwitch(const char* name) {
        if (!name) return nullptr;
        for (uint8_t i = 0; i < switchCount_; ++i) {
            if (strcmp(switches_[i].name(), name) == 0) return &switches_[i];
        }
        for (uint8_t i = 0; i < crossoverCount_; ++i) {
            if (strcmp(crossovers_[i].name(), name) == 0) return &crossovers_[i];
        }
        return nullptr;
    }

    SignalControl* findSignalControl(const char* name) {
        if (!name) return nullptr;
        for (uint8_t i = 0; i < authorityCount_; ++i) {
            if (strcmp(authorities_[i].name(), name) == 0) return &authorities_[i];
        }
        return nullptr;
    }

    SignalMast* findSignalMast(const char* name) {
        if (!name) return nullptr;
        for (uint8_t i = 0; i < mastCount_; ++i) {
            if (strcmp(masts_[i].name(), name) == 0) return &masts_[i];
        }
        return nullptr;
    }

    TrackCircuit* findDetectorCircuitForSwitch(const Switch* sw) const {
        if (!sw) return nullptr;
        for (uint8_t i = 0; i < detectorLockCouplingCount_; ++i) {
            if (detectorLocks_[i].sw == sw) {
                return detectorLocks_[i].tc;
            }
            if (sw->pairedSwitch() != nullptr && detectorLocks_[i].sw == sw->pairedSwitch()) {
                return detectorLocks_[i].tc;
            }
        }
        for (uint8_t i = 0; i < crossoverCount_; ++i) {
            if (static_cast<const Switch*>(&crossovers_[i]) == sw) {
                if (crossovers_[i].switchA()) {
                    TrackCircuit* tcA = findDetectorCircuitForSwitch(crossovers_[i].switchA());
                    if (tcA) return tcA;
                }
            }
        }
        return nullptr;
    }

    Route& route(const char* name) {
        return engine_.addRoute(name, this);
    }

// Ingress: Process incoming plant-wide control transaction
    // In railroad practice, the CP does not send error messages or NACKs.
    // It verifies completeness and vital safety:
    // - If valid and safe: moves points and latches authorities.
    // - If invalid or unsafe: does not actuate the plant.
    // The CP simply continues reporting its true current Indication vector.
    // The controlling entity (cTc machine) detects non-execution by observing
    // that the reported indication does not match its commanded intent.
    void applyControlTransaction(const ControlTransaction& ctl, uint32_t nowMs) {
        // Vital Safety Gate: If transaction is marked invalid or corrupted,
        // do NOT invoke vital appliances at all ("don't poke a sleeping bear").
        if (ctl.vitalValid) {
            // 1. Verify safety of all requested switch movements
            // If any switch movement in the transaction violates an active lock,
            // that specific movement cannot be executed.
            for (uint8_t i = 0; i < switchCount_; ++i) {
                SwitchDemand demand = ctl.switchDemands[i];
                if (demand == SwitchDemand::NORMAL || demand == SwitchDemand::REVERSE) {
                    // If switch is locked (train on points or active route), skip actuation
                    if (!switches_[i].WLR()) {
                        continue; // Leave switch in existing position
                    }
                    if (demand == SwitchDemand::NORMAL) {
                        switches_[i].throwSwitch(SwitchPosition::NORMAL, nowMs);
                    } else if (demand == SwitchDemand::REVERSE) {
                        switches_[i].throwSwitch(SwitchPosition::REVERSE, nowMs);
                    }
                }
            }

            // 2. Actuate Signal Authority demands
            for (uint8_t i = 0; i < authorityCount_; ++i) {
                SignalDemand demand = ctl.signalDemands[i];
                bool fleet = ctl.fleetDemands[i];

                if (demand == SignalDemand::STOP || demand == SignalDemand::LEFT || demand == SignalDemand::RIGHT) {
                    DirectionAuthority req = DirectionAuthority::STOP;
                    if (demand == SignalDemand::LEFT) req = DirectionAuthority::LEFT;
                    else if (demand == SignalDemand::RIGHT) req = DirectionAuthority::RIGHT;

                    bool approachOccupied = true;
                    if (authorities_[i].activeDirection() != DirectionAuthority::STOP && req != authorities_[i].activeDirection()) {
                        const Route* activeR = engine_.activeRouteForAuthority(&authorities_[i]);
                        if (activeR != nullptr && activeR->approachBlock() != nullptr) {
                            approachOccupied = !activeR->approachBlock()->isClear();
                        } else {
                            approachOccupied = false;
                        }
                    }
                    authorities_[i].updateCommand(req, fleet, nowMs, approachOccupied);
                }
                // NO_CHANGE leaves existing authority and stick state intact
            }
            // 3. Process Electric Switch Lock demands (WLS)
            for (uint8_t i = 0; i < switchCount_; ++i) {
                ElectricLockDemand demand = ctl.lockDemands[i];
                if (demand == ElectricLockDemand::UNLOCK) {
                    bool signalsSafe = true;
                    for (uint8_t a = 0; a < authorityCount_; ++a) {
                        if (authorities_[a].activeDirection() != DirectionAuthority::STOP || authorities_[a].isTimeLocked()) {
                            signalsSafe = false;
                            break;
                        }
                    }
                    if (signalsSafe) {
                        switches_[i].removeLock(SwitchLock::HAND_LOCKED);
                    }
                } else if (demand == ElectricLockDemand::LOCK) {
                    switches_[i].addLock(SwitchLock::HAND_LOCKED);
                }
            }
        }

        // 4. Update Maintainer Calls (non-vital, safe to process regardless of vital safety gate)
        maintainerCallActive_ = ctl.maintainerCall[0];
    }

    // Vital Cycle: Evaluate plant safety and route logic
    void tick(uint32_t nowMs) {
        // A. Clear and re-evaluate Detector Locks based on current track occupancy
        for (uint8_t i = 0; i < switchCount_; ++i) {
            switches_[i].removeLock(SwitchLock::DETECTOR_LOCKED);
            switches_[i].removeLock(SwitchLock::ROUTE_LOCKED);
            switches_[i].removeLock(SwitchLock::TIME_LOCKED);
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
            bool routeClear = true;
            DirectionAuthority cmdDir = authorities_[i].commandedDirection();
            if (cmdDir != DirectionAuthority::STOP) {
                for (uint8_t rIdx = 0; rIdx < engine_.routeCount(); ++rIdx) {
                    const Route& r = engine_.route(rIdx);
                    if (r.authority() == &authorities_[i] && r.direction() == cmdDir) {
                        if (!engine_.checkRouteBlocksClear(r)) {
                            routeClear = false;
                            break;
                        }
                    }
                }
            }
            authorities_[i].evaluate(routeClear, nowMs);
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
                switches_[i].activeLocks(),
                (switches_[i].activeLocks() & SwitchLock::HAND_LOCKED) == SwitchLock::UNLOCKED
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
                (i < mastCount_) ? masts_[i].compositeAspect() : Aspect::RED
            };
        }

        ind.mastCount = mastCount_;
        for (uint8_t i = 0; i < mastCount_; ++i) {
            ind.masts[i] = {
                masts_[i].currentIndication(),
                masts_[i].compositeAspect(),
                masts_[i].head1(),
                masts_[i].head2(),
                masts_[i].head3(),
                masts_[i].markers()
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
    AspectResolver defaultAspectPolicy_;
    TrackCircuit trackCircuits_[MAX_APPLIANCES];
    uint8_t trackCircuitCount_;

    Switch switches_[MAX_APPLIANCES];
    uint8_t switchCount_;

    Crossover crossovers_[MAX_APPLIANCES];
    uint8_t crossoverCount_;

    SignalControl authorities_[MAX_APPLIANCES];
    uint8_t authorityCount_;

    SignalMast masts_[MAX_APPLIANCES];
    uint8_t mastCount_;

    bool maintainerCallActive_;

    DetectorBinding detectorLocks_[MAX_APPLIANCES];
    uint8_t detectorLockCouplingCount_;

    InterlockingEngine engine_;
};

// -------------------------------------------------------------
// Route String-Based Configuration Implementations
// Resolves string names once during setup to retain O(1) runtime pointer execution.
// -------------------------------------------------------------

inline Route& Route::governedBy(const char* signalName, DirectionAuthority dir) {
    authority_ = cp_ ? cp_->findSignalControl(signalName) : nullptr;
    direction_ = dir;
    return *this;
}

inline Route& Route::displays(const char* mastName, uint8_t headIndex, Indication maxIndication) {
    mast_ = cp_ ? cp_->findSignalMast(mastName) : nullptr;
    targetHeadIndex_ = headIndex;
    aspectCeiling_ = maxIndication;
    return *this;
}

inline Route& Route::displays(const char* mastName, Indication maxIndication) {
    return displays(mastName, 0, maxIndication);
}

inline Route& Route::aligns(std::initializer_list<NamedSwitchRequirement> swList) {
    switchCount_ = 0;
    for (const auto& s : swList) {
        if (switchCount_ < MAX_ROUTE_SWITCHES) {
            Switch* sw = cp_ ? cp_->findSwitch(s.switchName) : nullptr;
            TrackCircuit* rel = (cp_ && s.releasingBlockName) ? cp_->findTrackCircuit(s.releasingBlockName) : nullptr;
            if (sw) {
                switches_[switchCount_++] = { sw, s.requiredPosition, rel };
            }
        }
    }
    return *this;
}

inline TrackCircuit* Route::releasingBlock(uint8_t idx) const {
    if (idx >= switchCount_) return nullptr;
    if (switches_[idx].releasingBlock != nullptr) {
        return switches_[idx].releasingBlock;
    }
    if (cp_ != nullptr && switches_[idx].switchRef != nullptr) {
        TrackCircuit* boundTc = cp_->findDetectorCircuitForSwitch(switches_[idx].switchRef);
        if (boundTc != nullptr) {
            return boundTc;
        }
    }
    return entranceBlock();
}

inline Route& Route::clears(std::initializer_list<const char*> tcNames) {
    blockCount_ = 0;
    for (const char* tcName : tcNames) {
        if (blockCount_ < MAX_ROUTE_BLOCKS) {
            TrackCircuit* tc = cp_ ? cp_->findTrackCircuit(tcName) : nullptr;
            if (tc) {
                blocks_[blockCount_++] = tc;
            }
        }
    }
    return *this;
}

inline Route& Route::entrance(const char* tcName) {
    entranceBlock_ = cp_ ? cp_->findTrackCircuit(tcName) : nullptr;
    return *this;
}

inline Route& Route::approaching(const char* tcName) {
    approachBlock_ = cp_ ? cp_->findTrackCircuit(tcName) : nullptr;
    return *this;
}

inline Route& Route::engineReturn(const char* standingCarsName, const char* islandName) {
    TrackCircuit* standing = cp_ ? cp_->findTrackCircuit(standingCarsName) : nullptr;
    TrackCircuit* island = cp_ ? cp_->findTrackCircuit(islandName) : nullptr;
    return engineReturn(standing, island);
}

} // namespace FieldUnit

#endif // FIELDUNIT_CONTROL_POINT_H
