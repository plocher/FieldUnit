#ifndef FIELDUNIT_CONTROL_TABLE_H
#define FIELDUNIT_CONTROL_TABLE_H

#include "types.h"
#include "TrackCircuit.h"
#include "Turnout.h"
#include "SignalMast.h"
#include "SignalAuthority.h"

namespace FieldUnit {

// Maximum limits for an individual route (no dynamic allocation)
static constexpr uint8_t MAX_ROUTE_SWITCHES = 8;
static constexpr uint8_t MAX_ROUTE_BLOCKS   = 8;
static constexpr uint8_t MAX_ROUTES         = 32;

struct SwitchRequirement {
    Turnout* switchRef;
    TurnoutPosition requiredPosition;
};

// Route Definition: One row in the Interlocking Control Table
struct RouteDef {
    const char*        name;
    SignalAuthority*   authority;
    DirectionAuthority direction;
    SignalMast*        mast;
    Indication         aspectCeiling;

    // Physical alignment
    uint8_t            switchCount;
    SwitchRequirement  switches[MAX_ROUTE_SWITCHES];

    // Track circuits on the route path (must be clear for normal moves)
    uint8_t            blockCount;
    TrackCircuit*      blocks[MAX_ROUTE_BLOCKS];

    // Downstream approach circuit (if occupied, downgrades CLEAR to APPROACH)
    TrackCircuit*      approachBlock;

    // Engine Return: If true, permits Restricting into occupied originBlock
    bool               isEngineReturn;
    TrackCircuit*      originBlock;    // Must continue to be occupied
    TrackCircuit*      osBlock;        // Interlocking points (must be clear)
};

class InterlockingEngine {
public:
    InterlockingEngine() : routeCount_(0) {}

    bool addRoute(const RouteDef& route) {
        if (routeCount_ >= MAX_ROUTES) {
            return false;
        }
        routes_[routeCount_++] = route;
        return true;
    }

    // Main vital evaluation cycle (called once per control point tick)
    void evaluate() {
        // Reset all signal masts to STOP initially
        // The active matching route will elevate the governing mast
        for (uint8_t i = 0; i < routeCount_; ++i) {
            routes_[i].mast->forceStop();
        }

        // 1. Evaluate each route in priority order
        for (uint8_t i = 0; i < routeCount_; ++i) {
            const RouteDef& r = routes_[i];

            // A. Check if Engine Return applies
            if (r.isEngineReturn) {
                if (evaluateEngineReturn(r)) {
                    r.mast->setIndication(Indication::RESTRICTING);
                    applyRouteLocks(r);
                    continue; // Engine return route satisfied
                }
            }

            // B. Standard Dispatcher-Governed Route
            if (r.authority == nullptr) {
                continue;
            }

            // Check if dispatcher granted authority in this direction
            if (r.authority->activeDirection() != r.direction) {
                continue;
            }

            // Check switch alignment and correspondence
            if (!checkSwitchesAligned(r)) {
                continue;
            }

            // Check route track circuits
            bool pathClear = checkBlocksClear(r);

            // Check if train has entered the plant (knockdown)
            if (!r.blocks[0]->isClear()) {
                r.authority->knockdown();
                continue; // Train entered, signal must stay at STOP
            }

            if (!pathClear) {
                continue; // Route blocked by train ahead
            }

            // Route is aligned, locked, and clear. Derive aspect.
            Indication aspect = r.aspectCeiling;

            // ABS / Intermediate block check:
            // If approach circuit ahead is occupied, drop Clear to Approach
            if (r.approachBlock != nullptr && !r.approachBlock->isClear()) {
                if (aspect == Indication::CLEAR) {
                    aspect = Indication::APPROACH;
                } else if (aspect == Indication::DIVERGING_CLEAR) {
                    aspect = Indication::DIVERGING_APPROACH;
                }
            }

            // Display aspect and lock turnouts
            r.mast->setIndication(aspect);
            applyRouteLocks(r);
        }
    }

private:
    bool checkSwitchesAligned(const RouteDef& r) const {
        for (uint8_t i = 0; i < r.switchCount; ++i) {
            const SwitchRequirement& req = r.switches[i];
            if (!req.switchRef->inCorrespondence() || 
                req.switchRef->reportedPosition() != req.requiredPosition) {
                return false;
            }
        }
        return true;
    }

    bool checkBlocksClear(const RouteDef& r) const {
        for (uint8_t i = 0; i < r.blockCount; ++i) {
            if (!r.blocks[i]->isClear()) {
                return false;
            }
        }
        return true;
    }

    void applyRouteLocks(const RouteDef& r) {
        for (uint8_t i = 0; i < r.switchCount; ++i) {
            r.switches[i].switchRef->addLock(TurnoutLock::ROUTE_LOCKED);
        }
    }

    // Engine Return: Automatic Restricting back onto left-behind cars
    bool evaluateEngineReturn(const RouteDef& r) const {
        if (r.originBlock == nullptr || r.osBlock == nullptr) {
            return false;
        }

        // Turnout must be in correspondence
        if (!checkSwitchesAligned(r)) {
            return false;
        }

        // OS circuit over points must be clear
        if (!r.osBlock->isClear()) {
            return false;
        }

        // Cars left behind MUST continue to occupy origin block!
        // If cars departed, origin is vacant -> ER drops to STOP fail-safe
        if (r.originBlock->state().value != Occupancy::OCCUPIED) {
            return false;
        }

        return true;
    }

    RouteDef routes_[MAX_ROUTES];
    uint8_t routeCount_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_CONTROL_TABLE_H
