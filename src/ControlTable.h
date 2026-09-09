#ifndef FIELDUNIT_CONTROL_TABLE_H
#define FIELDUNIT_CONTROL_TABLE_H

#include <initializer_list>
#include "types.h"
#include "TrackCircuit.h"
#include "Switch.h"
#include "SignalMast.h"
#include "SignalControl.h"

namespace FieldUnit {

static constexpr uint8_t MAX_ROUTE_SWITCHES = 8;
static constexpr uint8_t MAX_ROUTE_BLOCKS   = 8;
static constexpr uint8_t MAX_ROUTES         = 32;

struct SwitchRequirement {
    Switch* switchRef;
    SwitchPosition requiredPosition;
};

// Fluent Route Definition in the Interlocking Control Table
class Route {
public:
    Route()
        : name_(""),
          authority_(nullptr),
          direction_(DirectionAuthority::STOP),
          mast_(nullptr),
          targetHeadIndex_(0),
          aspectCeiling_(Indication::STOP),
          switchCount_(0),
          blockCount_(0),
          approachBlock_(nullptr),
          isEngineReturn_(false),
          originBlock_(nullptr),
          osBlock_(nullptr) {}

    Route& name(const char* n) {
        name_ = n;
        return *this;
    }

    Route& governedBy(SignalControl* auth, DirectionAuthority dir) {
        authority_ = auth;
        direction_ = dir;
        return *this;
    }

    Route& displays(SignalMast* mast, uint8_t headIndex, Indication maxIndication) {
        mast_ = mast;
        targetHeadIndex_ = headIndex;
        aspectCeiling_ = maxIndication;
        return *this;
    }

    Route& displays(SignalMast* mast, Indication maxIndication) {
        return displays(mast, 0, maxIndication);
    }

    Route& aligns(std::initializer_list<SwitchRequirement> swList) {
        switchCount_ = 0;
        for (const auto& s : swList) {
            if (switchCount_ < MAX_ROUTE_SWITCHES) {
                switches_[switchCount_++] = s;
            }
        }
        return *this;
    }

    Route& clears(std::initializer_list<TrackCircuit*> tcList) {
        blockCount_ = 0;
        for (auto tc : tcList) {
            if (blockCount_ < MAX_ROUTE_BLOCKS) {
                blocks_[blockCount_++] = tc;
            }
        }
        return *this;
    }

    Route& approaching(TrackCircuit* tc) {
        approachBlock_ = tc;
        return *this;
    }

    // Engine Return: Permits Restricting back onto cars standing on originBlock
    Route& engineReturn(TrackCircuit* standingCarsBlock, TrackCircuit* islandBlock) {
        isEngineReturn_ = true;
        originBlock_ = standingCarsBlock;
        osBlock_ = islandBlock;
        return *this;
    }

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
    TrackCircuit* approachBlock() const { return approachBlock_; }

    bool isEngineReturn() const { return isEngineReturn_; }
    TrackCircuit* originBlock() const { return originBlock_; }
    TrackCircuit* osBlock() const { return osBlock_; }

private:
    const char*        name_;
    SignalControl*     authority_;
    DirectionAuthority direction_;
    SignalMast*        mast_;
    uint8_t            targetHeadIndex_;
    Indication         aspectCeiling_;

    uint8_t            switchCount_;
    SwitchRequirement  switches_[MAX_ROUTE_SWITCHES];

    uint8_t            blockCount_;
    TrackCircuit*      blocks_[MAX_ROUTE_BLOCKS];

    TrackCircuit*      approachBlock_;

    bool               isEngineReturn_;
    TrackCircuit*      originBlock_;
    TrackCircuit*      osBlock_;
};

class InterlockingEngine {
public:
    InterlockingEngine() : routeCount_(0) {}

    Route& addRoute(const char* name) {
        if (routeCount_ >= MAX_ROUTES) {
            return dummyRoute_;
        }
        Route& r = routes_[routeCount_++];
        r = Route();
        r.name(name);
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
                continue;
            }

            // Check if dispatcher granted authority in this direction
            if (r.authority()->activeDirection() != r.direction()) {
                continue;
            }

            // Check switch alignment and correspondence
            if (!checkSwitchesAligned(r)) {
                continue;
            }

            // Check route track circuits
            bool pathClear = checkBlocksClear(r);

            // Check if train has entered the plant (knockdown)
            if (r.blockCount() > 0 && !r.block(0)->isClear()) {
                r.authority()->knockdown();
                continue; // Train entered, signal must stay at STOP
            }

            if (!pathClear) {
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
            applyRouteLocks(r);
        }
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
