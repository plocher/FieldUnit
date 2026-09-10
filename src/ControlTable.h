#ifndef FIELDUNIT_CONTROL_TABLE_H
#define FIELDUNIT_CONTROL_TABLE_H

#include <initializer_list>
#include "types.h"
#include "TrackCircuit.h"
#include "Switch.h"
#include "SignalMast.h"
#include "SignalControl.h"

namespace FieldUnit {

class ControlPoint;

static constexpr uint8_t MAX_ROUTE_SWITCHES = 8;
static constexpr uint8_t MAX_ROUTE_BLOCKS   = 8;
static constexpr uint8_t MAX_ROUTES         = 32;

enum class RouteState : uint8_t {
    IDLE = 0,
    CLEARED = 1,
    TRAVERSING = 2
};

enum class SectionState : uint8_t {
    LOCKED = 0,    // Initial state: locked ahead of approaching train
    OCCUPIED = 1,  // Train is currently over this switch's fouling block
    RELEASED = 2   // Train occupied and then vacated this block (sectional release)
};

struct SwitchRequirement {
    Switch* switchRef;
    SwitchPosition requiredPosition;
    TrackCircuit* releasingBlock;

    constexpr SwitchRequirement()
        : switchRef(nullptr), requiredPosition(SwitchPosition::NORMAL), releasingBlock(nullptr) {}

    constexpr SwitchRequirement(Switch* sw, SwitchPosition pos, TrackCircuit* rel = nullptr)
        : switchRef(sw), requiredPosition(pos), releasingBlock(rel) {}
};

struct NamedSwitchRequirement {
    const char* switchName;
    SwitchPosition requiredPosition;
    const char* releasingBlockName;

    constexpr NamedSwitchRequirement()
        : switchName(nullptr), requiredPosition(SwitchPosition::NORMAL), releasingBlockName(nullptr) {}

    constexpr NamedSwitchRequirement(const char* swName, SwitchPosition pos, const char* relName = nullptr)
        : switchName(swName), requiredPosition(pos), releasingBlockName(relName) {}
};

// Fluent Route Definition in the Interlocking Control Table
class Route {
public:
    Route()
        : authority_(nullptr),
          direction_(DirectionAuthority::STOP),
          mast_(nullptr),
          targetHeadIndex_(0),
          aspectCeiling_(Indication::STOP),
          switchCount_(0),
          blockCount_(0),
          entranceBlock_(nullptr),
          approachBlock_(nullptr),
          isEngineReturn_(false),
          originBlock_(nullptr),
          osBlock_(nullptr),
          cp_(nullptr),
          state_(RouteState::IDLE) {
        name_[0] = '\0';
        for (uint8_t i = 0; i < MAX_ROUTE_SWITCHES; ++i) {
            sectionStates_[i] = SectionState::LOCKED;
        }
    }

    void setParent(ControlPoint* cp) { cp_ = cp; }

    RouteState state() const { return state_; }
    bool isIdle() const { return state_ == RouteState::IDLE; }
    bool isCleared() const { return state_ == RouteState::CLEARED; }
    bool isTraversing() const { return state_ == RouteState::TRAVERSING; }

    SectionState sectionState(uint8_t idx) const {
        return (idx < switchCount_) ? sectionStates_[idx] : SectionState::LOCKED;
    }

    void setSectionState(uint8_t idx, SectionState s) {
        if (idx < MAX_ROUTE_SWITCHES) {
            sectionStates_[idx] = s;
        }
    }

    void resetTraversal() {
        state_ = RouteState::IDLE;
        for (uint8_t i = 0; i < MAX_ROUTE_SWITCHES; ++i) {
            sectionStates_[i] = SectionState::LOCKED;
        }
    }

    void setCleared() {
        state_ = RouteState::CLEARED;
        for (uint8_t i = 0; i < MAX_ROUTE_SWITCHES; ++i) {
            sectionStates_[i] = SectionState::LOCKED;
        }
    }

    void setTraversing() {
        state_ = RouteState::TRAVERSING;
    }

    TrackCircuit* releasingBlock(uint8_t idx) const;

    Route& name(const char* n) {
        if (!n) { name_[0] = '\0'; return *this; }
        strncpy(name_, n, sizeof(name_) - 1);
        name_[sizeof(name_) - 1] = '\0';
        return *this;
    }

    Route& governedBy(SignalControl* auth, DirectionAuthority dir) {
        authority_ = auth;
        direction_ = dir;
        return *this;
    }

    Route& governedBy(const char* signalName, DirectionAuthority dir);

    Route& displays(SignalMast* mast, uint8_t headIndex, Indication maxIndication) {
        mast_ = mast;
        targetHeadIndex_ = headIndex;
        aspectCeiling_ = maxIndication;
        return *this;
    }

    Route& displays(SignalMast* mast, Indication maxIndication) {
        return displays(mast, 0, maxIndication);
    }

    Route& displays(const char* mastName, uint8_t headIndex, Indication maxIndication);
    Route& displays(const char* mastName, Indication maxIndication);

    Route& aligns(std::initializer_list<SwitchRequirement> swList) {
        switchCount_ = 0;
        for (const auto& s : swList) {
            if (switchCount_ < MAX_ROUTE_SWITCHES) {
                switches_[switchCount_++] = s;
            }
        }
        return *this;
    }

    Route& aligns(std::initializer_list<NamedSwitchRequirement> swList);

    Route& align(Switch* sw, SwitchPosition pos, TrackCircuit* rel = nullptr) {
        if (switchCount_ < MAX_ROUTE_SWITCHES && sw) {
            switches_[switchCount_++] = { sw, pos, rel };
        }
        return *this;
    }

    Route& align(const char* swName, SwitchPosition pos, const char* relName = nullptr);

    Route& clears(std::initializer_list<TrackCircuit*> tcList) {
        blockCount_ = 0;
        for (auto tc : tcList) {
            if (blockCount_ < MAX_ROUTE_BLOCKS) {
                blocks_[blockCount_++] = tc;
            }
        }
        return *this;
    }

    Route& clears(std::initializer_list<const char*> tcNames);

    Route& clearBlock(TrackCircuit* tc) {
        if (blockCount_ < MAX_ROUTE_BLOCKS && tc) {
            blocks_[blockCount_++] = tc;
        }
        return *this;
    }

    Route& clearBlock(const char* tcName);

    Route& entrance(TrackCircuit* tc) {
        entranceBlock_ = tc;
        return *this;
    }

    Route& entrance(const char* tcName);

    Route& approaching(TrackCircuit* tc) {
        approachBlock_ = tc;
        return *this;
    }

    Route& approaching(const char* tcName);

    // Engine Return: Permits Restricting back onto cars standing on originBlock
    Route& engineReturn(TrackCircuit* standingCarsBlock, TrackCircuit* islandBlock) {
        isEngineReturn_ = true;
        originBlock_ = standingCarsBlock;
        osBlock_ = islandBlock;
        return *this;
    }

    Route& engineReturn(const char* standingCarsName, const char* islandName);

    // Accessors for vital evaluation
    const char* name() const { return name_; }
    SignalControl* authority() const { return authority_; }
    DirectionAuthority direction() const { return direction_; }
    SignalMast* mast() const { return mast_; }
    uint8_t targetHeadIndex() const { return targetHeadIndex_; }
    Indication aspectCeiling() const { return aspectCeiling_; }

    uint8_t switchCount() const { return switchCount_; }
    const SwitchRequirement& switchReq(uint8_t idx) const { return switches_[idx]; }

    uint8_t blockCount() const { return blockCount_; }
    TrackCircuit* block(uint8_t idx) const { return blocks_[idx]; }
    TrackCircuit* entranceBlock() const {
        if (entranceBlock_ != nullptr) return entranceBlock_;
        return (blockCount_ > 0) ? blocks_[0] : nullptr;
    }
    TrackCircuit* approachBlock() const { return approachBlock_; }

    bool isEngineReturn() const { return isEngineReturn_; }
    TrackCircuit* originBlock() const { return originBlock_; }
    TrackCircuit* osBlock() const { return osBlock_; }

private:
    char               name_[32];
    SignalControl*     authority_;
    DirectionAuthority direction_;
    SignalMast*        mast_;
    uint8_t            targetHeadIndex_;
    Indication         aspectCeiling_;

    uint8_t            switchCount_;
    SwitchRequirement  switches_[MAX_ROUTE_SWITCHES];

    uint8_t            blockCount_;
    TrackCircuit*      blocks_[MAX_ROUTE_BLOCKS];
    TrackCircuit*      entranceBlock_;

    TrackCircuit*      approachBlock_;

    bool               isEngineReturn_;
    TrackCircuit*      originBlock_;
    TrackCircuit*      osBlock_;
    ControlPoint*      cp_;
    RouteState         state_;
    SectionState       sectionStates_[MAX_ROUTE_SWITCHES];
};

class InterlockingEngine {
public:
    InterlockingEngine() : routeCount_(0) {}

    Route& addRoute(const char* name, ControlPoint* cp = nullptr) {
        if (routeCount_ >= MAX_ROUTES) {
            return dummyRoute_;
        }
        Route& r = routes_[routeCount_++];
        r = Route();
        r.name(name);
        r.setParent(cp);
        return r;
    }

    // Main vital evaluation cycle
    void evaluate() {
        // Reset all signal masts to STOP initially
        for (uint8_t i = 0; i < routeCount_; ++i) {
            if (routes_[i].mast() != nullptr) {
                routes_[i].mast()->forceStop();
            }
        }

        // Evaluate each route in priority order
        for (uint8_t i = 0; i < routeCount_; ++i) {
            Route& r = routes_[i];
            if (r.mast() == nullptr) continue;

            // 1. If route is currently traversing (train has entered plant):
            if (r.isTraversing()) {
                // Advance sectional release progression
                for (uint8_t s = 0; s < r.switchCount(); ++s) {
                    TrackCircuit* relBlock = r.releasingBlock(s);
                    if (r.sectionState(s) == SectionState::LOCKED) {
                        if (relBlock != nullptr && !relBlock->isClear()) {
                            r.setSectionState(s, SectionState::OCCUPIED);
                        }
                    } else if (r.sectionState(s) == SectionState::OCCUPIED) {
                        if (relBlock != nullptr && relBlock->isClear()) {
                            r.setSectionState(s, SectionState::RELEASED);
                        }
                    }
                }

                // Check if route traversal is complete:
                // Traversal completes when all route blocks are clear
                if (checkBlocksClear(r)) {
                    r.resetTraversal();
                    // Plant is now completely clear; fall through to evaluate if new demand is present
                } else {
                    // Route is still traversing; apply route locks to all unreleased switches
                    for (uint8_t s = 0; s < r.switchCount(); ++s) {
                        if (r.sectionState(s) != SectionState::RELEASED) {
                            r.switchReq(s).switchRef->addLock(SwitchLock::ROUTE_LOCKED);
                        }
                    }
                    continue;
                }
            }

            // A. Check if Engine Return applies
            if (r.isEngineReturn()) {
                if (evaluateEngineReturn(r)) {
                    r.mast()->setHeadIndication(r.targetHeadIndex(), Indication::RESTRICTING);
                    applyRouteLocks(r);
                    continue;
                }
            }

            // B. Standard Dispatcher-Governed Route
            if (r.authority() == nullptr) {
                r.resetTraversal();
                continue;
            }

            // Check if dispatcher granted authority in this direction
            if (r.authority()->activeDirection() != r.direction()) {
                r.resetTraversal();
                continue;
            }

            // Check switch alignment and correspondence
            if (!checkSwitchesAligned(r)) {
                r.resetTraversal();
                continue;
            }

            // Check route track circuits
            bool pathClear = checkBlocksClear(r);

            // Check if train has entered the plant (knockdown)
            TrackCircuit* ent = r.entranceBlock();
            if (ent != nullptr && !ent->isClear()) {
                r.authority()->knockdown();
                r.setTraversing();

                // Initialize sectional release state on entrance
                for (uint8_t s = 0; s < r.switchCount(); ++s) {
                    TrackCircuit* relBlock = r.releasingBlock(s);
                    if (relBlock != nullptr && !relBlock->isClear()) {
                        r.setSectionState(s, SectionState::OCCUPIED);
                    } else {
                        r.setSectionState(s, SectionState::LOCKED);
                    }
                }

                // Apply route locks to unreleased switches
                for (uint8_t s = 0; s < r.switchCount(); ++s) {
                    if (r.sectionState(s) != SectionState::RELEASED) {
                        r.switchReq(s).switchRef->addLock(SwitchLock::ROUTE_LOCKED);
                    }
                }
                continue; // Train entered, signal must stay at STOP
            }

            if (!pathClear) {
                r.resetTraversal();
                continue; // Route blocked by train ahead
            }

            // Route is aligned, locked, and clear. Derive aspect.
            Indication aspect = r.aspectCeiling();

            // ABS / Intermediate block check:
            // If approach circuit ahead is occupied, drop Clear to Approach
            if (r.approachBlock() != nullptr && !r.approachBlock()->isClear()) {
                if (aspect == Indication::CLEAR) {
                    aspect = Indication::APPROACH;
                } else if (aspect == Indication::DIVERGING_CLEAR) {
                    aspect = Indication::DIVERGING_APPROACH;
                }
            }

            // Display aspect and lock switches
            r.mast()->setHeadIndication(r.targetHeadIndex(), aspect);
            r.setCleared();
            applyRouteLocks(r);
        }

        // Apply time locking to switches on cancelled routes
        applyTimeLocks();
    }

    uint8_t routeCount() const { return routeCount_; }
    const Route& route(uint8_t idx) const { return routes_[idx]; }
    Route& route(uint8_t idx) { return routes_[idx]; }

    bool checkRouteBlocksClear(const Route& r) const {
        return checkBlocksClear(r);
    }

    const Route* activeRouteForAuthority(const SignalControl* auth) const {
        if (!auth) return nullptr;
        for (uint8_t i = 0; i < routeCount_; ++i) {
            if (routes_[i].authority() == auth && routes_[i].direction() == auth->activeDirection()) {
                if (checkSwitchesAligned(routes_[i])) {
                    return &routes_[i];
                }
            }
        }
        return nullptr;
    }

private:
    bool checkSwitchesAligned(const Route& r) const {
        for (uint8_t i = 0; i < r.switchCount(); ++i) {
            const SwitchRequirement& req = r.switchReq(i);
            if (!req.switchRef->inCorrespondence() || 
                req.switchRef->reportedPosition() != req.requiredPosition) {
                return false;
            }
        }
        return true;
    }

    bool checkBlocksClear(const Route& r) const {
        for (uint8_t i = 0; i < r.blockCount(); ++i) {
            if (!r.block(i)->isClear()) {
                return false;
            }
        }
        return true;
    }

    void applyRouteLocks(const Route& r) {
        for (uint8_t i = 0; i < r.switchCount(); ++i) {
            r.switchReq(i).switchRef->addLock(SwitchLock::ROUTE_LOCKED);
        }
    }

    void applyTimeLocks() {
        for (uint8_t i = 0; i < routeCount_; ++i) {
            Route& r = routes_[i];
            if (r.authority() != nullptr && r.authority()->isTimeLocked()) {
                if (r.direction() == r.authority()->timeLockDirection()) {
                    for (uint8_t s = 0; s < r.switchCount(); ++s) {
                        r.switchReq(s).switchRef->addLock(SwitchLock::TIME_LOCKED);
                    }
                }
            }
        }
    }

    bool evaluateEngineReturn(const Route& r) const {
        if (r.originBlock() == nullptr || r.osBlock() == nullptr) {
            return false;
        }

        if (!checkSwitchesAligned(r)) {
            return false;
        }

        if (!r.osBlock()->isClear()) {
            return false;
        }

        // Cars left behind MUST continue to occupy origin block
        if (r.originBlock()->state().value != Occupancy::OCCUPIED) {
            return false;
        }

        return true;
    }

    Route routes_[MAX_ROUTES];
    uint8_t routeCount_;
    Route dummyRoute_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_CONTROL_TABLE_H
