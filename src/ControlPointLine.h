#ifndef FIELDUNIT_CONTROL_POINT_LINE_H
#define FIELDUNIT_CONTROL_POINT_LINE_H

#include <initializer_list>
#include <string.h>
#include "types.h"
#include "Fact.h"
#include "SignalMast.h"
#include "SignalControl.h"
#include "ControlTable.h"

namespace FieldUnit {

class InterlockingPlant;

static constexpr uint8_t MAX_LINE_MASTS = 4;

// FieldUnit-Subdivision ADR 0013 section 14.2, the blockRules axes a control
// point's line depends on. The layout owner chooses them by railroad and era.
enum class TumbleDownTrigger : uint8_t {
    CLEARED = 0,     // SP_1942: a cleared leaving signal cuts H out
    REQUESTED = 1    // CBQ_1946, ERIE_NW_1942: the held control cuts H out
};

enum class TrafficMechanism : uint8_t {
    HD_CHAIN = 0,    // SP_1942: traffic_free is segment_clear; no F
    FIELD_FR = 1,    // CBQ_1946: a field traffic relay, F out un-bypassed
    TRAFFIC_LINE = 2 // ERIE_NW_1942: a separate traffic line, F out un-bypassed
};

enum class TrafficRelease : uint8_t {
    BLOCK_CLEAR = 0,                   // SP_1942: !cleared, !in_time, block clear
    REQUEST_NORMAL_AND_BLOCK_CLEAR = 1 // CBQ_1946, ERIE_NW_1942: !requested, !in_time, block clear
};

/**
 * ControlPointLine: what a control point produces at the seam with one
 * station-to-station block (FieldUnit ADR 0008 D5; FieldUnit-Subdivision
 * ADR 0013 section 15.4). Named for the home mast facing the block, as an
 * HD line is named (section 15.1), and read by the first location in the
 * block through the Facts <name>H, <name>D and, when the traffic mechanism
 * is not the HD chain, <name>F:
 *
 *   H out  =  governs() clear AND !in_time AND !cut
 *             cut = cleared   under TumbleDownTrigger::CLEARED
 *             cut = requested under TumbleDownTrigger::REQUESTED
 *   D out  =  home_proceed: the home mast's routes evaluate above the
 *             Restricting tier
 *   F out  =  !requested AND !cleared AND !in_time, held released once it
 *             drops until the traffic release is satisfied: block() clear
 *             and (!cleared AND !in_time | !requested AND !in_time)
 *
 * requested, cleared and in_time are the seam's outbound ports (FieldUnit
 * ADR 0007 D1): the control as held toward the block, the leaving masts'
 * HR, and a leaving control's timer running. cleared and home_proceed are
 * relay states: the previous scan's route evaluation, held as an HR front
 * contact holds it, so a stick in the block that waits for this line sees
 * H return in the scan a train clears the adjacent circuit, before a
 * leaving signal requested behind that train picks up and cuts H again
 * (a line relay is faster than a leaving signal's HR chain). No mast is
 * read; the latch is of the route equation's result.
 *
 * On another processor the same three values come from the code line
 * (section 18.2: TK, NGK/SGK, TEK) and a ControlPointLine there is bound
 * to line inputs instead; the eavesdropping shadow is this class with its
 * operands rebound.
 */
class ControlPointLine {
public:
    ControlPointLine()
        : cp_(nullptr), engine_(nullptr), homeMast_(nullptr),
          leavingCount_(0), governedCount_(0), blockCount_(0),
          tumbleDown_(TumbleDownTrigger::CLEARED),
          traffic_(TrafficMechanism::HD_CHAIN),
          release_(TrafficRelease::BLOCK_CLEAR),
          trafficDropped_(false),
          clearedLatched_(false),
          homeProceedLatched_(false),
          hOut_(this, Output::H), dOut_(this, Output::D), fOut_(this, Output::F) {
        name_[0] = '\0';
    }

    void bind(const char* name, InterlockingPlant* cp, InterlockingEngine* engine) {
        setName(name);
        cp_ = cp;
        engine_ = engine;
        hOut_.setName(name_, 'H');
        dOut_.setName(name_, 'D');
        fOut_.setName(name_, 'F');
    }

    void setName(const char* name) {
        if (!name) { name_[0] = '\0'; return; }
        strncpy(name_, name, sizeof(name_) - 1);
        name_[sizeof(name_) - 1] = '\0';
    }

    const char* name() const { return name_; }

    // --- configuration ---

    // The block circuit(s) adjacent to this end, ANDed into H out.
    ControlPointLine& governs(std::initializer_list<const char*> names);
    ControlPointLine& governs(const Fact* f) {
        if (f && governedCount_ < MAX_BLOCK_FACTS) governed_[governedCount_++] = f;
        return *this;
    }

    // Every circuit of the block, for the traffic release. Local circuits or
    // line inputs carrying the far end's TK.
    ControlPointLine& block(std::initializer_list<const char*> names);
    ControlPointLine& block(const Fact* f) {
        if (f && blockCount_ < MAX_ROUTE_BLOCKS) block_[blockCount_++] = f;
        return *this;
    }

    // The leaving masts into the block. Their routes give cleared; their
    // signal controls and route directions give requested and in_time.
    ControlPointLine& leaving(std::initializer_list<const char*> mastNames);
    ControlPointLine& leaving(SignalMast* mast) {
        if (mast && leavingCount_ < MAX_LINE_MASTS) leaving_[leavingCount_++] = mast;
        return *this;
    }

    // The home mast facing the block (defaults to the line's name).
    ControlPointLine& home(const char* mastName);
    ControlPointLine& home(SignalMast* mast) { homeMast_ = mast; return *this; }

    ControlPointLine& tumbleDown(TumbleDownTrigger t) { tumbleDown_ = t; return *this; }
    ControlPointLine& traffic(TrafficMechanism m) { traffic_ = m; return *this; }
    ControlPointLine& trafficRelease(TrafficRelease r) { release_ = r; return *this; }

    // --- outputs ---

    const Fact* hOut() const { return &hOut_; }
    const Fact* dOut() const { return &dOut_; }
    const Fact* fOut() const { return &fOut_; }
    bool producesF() const { return traffic_ != TrafficMechanism::HD_CHAIN; }

    // --- seam ports (FieldUnit ADR 0007 D1) ---

    bool requested() const {
        for (uint8_t i = 0; i < leavingCount_; ++i) {
            if (routeHeld(leaving_[i])) return true;
        }
        return false;
    }

    // HR of a leaving signal, as held from the previous scan.
    bool cleared() const { return clearedLatched_; }

    bool inTime() const {
        for (uint8_t i = 0; i < leavingCount_; ++i) {
            if (controlInTime(leaving_[i])) return true;
        }
        return false;
    }

    // HR of the home mast facing the block, as held from the previous scan.
    bool homeProceed() const { return homeProceedLatched_; }

    // --- accessors ---

    SignalMast* homeMast() const { return homeMast_; }
    uint8_t leavingCount() const { return leavingCount_; }
    const SignalMast* leavingMast(uint8_t i) const { return (i < leavingCount_) ? leaving_[i] : nullptr; }
    uint8_t governedCount() const { return governedCount_; }
    const Fact* governed(uint8_t i) const { return (i < governedCount_) ? governed_[i] : nullptr; }
    uint8_t blockCount() const { return blockCount_; }
    const Fact* blockFact(uint8_t i) const { return (i < blockCount_) ? block_[i] : nullptr; }
    TumbleDownTrigger tumbleDown() const { return tumbleDown_; }
    TrafficMechanism traffic() const { return traffic_; }
    TrafficRelease trafficRelease() const { return release_; }
    bool trafficDropped() const { return trafficDropped_; }

    // --- scan (called by InterlockingPlant::evaluateVitalRules after the control table) ---

    void latch() {
        clearedLatched_ = false;
        for (uint8_t i = 0; i < leavingCount_; ++i) {
            if (mastProceed(leaving_[i])) { clearedLatched_ = true; break; }
        }
        homeProceedLatched_ = mastProceed(homeMast_);
        if (!producesF()) { trafficDropped_ = false; return; }
        if (requested() || cleared() || inTime()) {
            trafficDropped_ = true;
        } else if (trafficDropped_ && releaseSatisfied()) {
            trafficDropped_ = false;
        }
    }

private:
    class Output : public Fact {
    public:
        enum Kind : uint8_t { H, D, F };
        Output(ControlPointLine* owner, Kind kind) : owner_(owner), kind_(kind) { name_[0] = '\0'; }
        void setName(const char* base, char suffix) {
            size_t n = strlen(base);
            if (n > sizeof(name_) - 2) n = sizeof(name_) - 2;
            memcpy(name_, base, n);
            name_[n] = suffix;
            name_[n + 1] = '\0';
        }
        const char* name() const override { return name_; }
        bool asserted() const override {
            switch (kind_) {
                case H: return owner_->computeH();
                case D: return owner_->homeProceed();
                default: return owner_->computeF();
            }
        }
    private:
        ControlPointLine* owner_;
        Kind kind_;
        char name_[MAX_APPLIANCE_NAME_LEN + 2];
    };

    static bool allAsserted(const Fact* const* facts, uint8_t count) {
        for (uint8_t i = 0; i < count; ++i) {
            if (facts[i] == nullptr || !facts[i]->asserted()) return false;
        }
        return true;
    }

    bool computeH() const {
        bool cut = (tumbleDown_ == TumbleDownTrigger::CLEARED) ? cleared() : requested();
        return allAsserted(governed_, governedCount_) && !inTime() && !cut;
    }

    bool computeF() const {
        if (!producesF()) return false;
        return !trafficDropped_ && !requested() && !cleared() && !inTime();
    }

    bool releaseSatisfied() const {
        if (!allAsserted(block_, blockCount_)) return false;
        if (release_ == TrafficRelease::BLOCK_CLEAR) return !cleared() && !inTime();
        return !requested() && !inTime();
    }

    // The mast's routes evaluate above the Restricting tier.
    bool mastProceed(const SignalMast* mast) const {
        if (mast == nullptr || engine_ == nullptr) return false;
        return Route::permissivenessRank(engine_->evaluateMastIndication(mast)) > 1;
    }

    // Some route displaying the mast has its control held in its direction.
    bool routeHeld(const SignalMast* mast) const {
        if (mast == nullptr || engine_ == nullptr) return false;
        for (uint8_t i = 0; i < engine_->routeCount(); ++i) {
            const Route& r = engine_->route(i);
            if (r.mast() != mast || r.authority() == nullptr) continue;
            if (r.authority()->commandedDirection() == r.direction()) return true;
        }
        return false;
    }

    bool controlInTime(const SignalMast* mast) const {
        if (mast == nullptr || engine_ == nullptr) return false;
        for (uint8_t i = 0; i < engine_->routeCount(); ++i) {
            const Route& r = engine_->route(i);
            if (r.mast() != mast || r.authority() == nullptr) continue;
            if (r.authority()->isTimeLocked()) return true;
        }
        return false;
    }

    char               name_[MAX_APPLIANCE_NAME_LEN];
    InterlockingPlant* cp_;
    InterlockingEngine* engine_;
    SignalMast*        homeMast_;
    SignalMast*        leaving_[MAX_LINE_MASTS];
    uint8_t            leavingCount_;
    const Fact*        governed_[MAX_BLOCK_FACTS];
    uint8_t            governedCount_;
    const Fact*        block_[MAX_ROUTE_BLOCKS];
    uint8_t            blockCount_;
    TumbleDownTrigger  tumbleDown_;
    TrafficMechanism   traffic_;
    TrafficRelease     release_;
    bool               trafficDropped_;
    bool               clearedLatched_;
    bool               homeProceedLatched_;
    Output             hOut_;
    Output             dOut_;
    Output             fOut_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_CONTROL_POINT_LINE_H
