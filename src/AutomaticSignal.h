#ifndef FIELDUNIT_AUTOMATIC_SIGNAL_H
#define FIELDUNIT_AUTOMATIC_SIGNAL_H

#include <initializer_list>
#include <stdio.h>
#include <string.h>
#include "types.h"
#include "Fact.h"
#include "SignalMast.h"
#include "ControlTable.h"

namespace FieldUnit {

class InterlockingPlant;

static constexpr uint8_t MAX_BLOCK_FACTS = 4;

/**
 * AutomaticSignal: one signal location, one mast, one direction.
 *
 * FieldUnit ADR 0008. A block signal controlled by track circuits and HD
 * lines alone (GLOSSARY "automatic signal"; FieldUnit-Subdivision ADR 0013
 * section 15.3). Mast M stands at location L, governs into the block
 * T(L->W) toward the next location W, and sends its HD line back to the
 * previous location U:
 *
 *   display       least permissive of
 *                   T(L->W) clear      -> CLEAR else STOP      governs()
 *                   H from W asserted  -> CLEAR else STOP      reads()
 *                   D from W asserted  -> CLEAR else APPROACH  reads()
 *                   opposing HR, SR at L down -> CLEAR else STOP   opposing()
 *   HR            display above the Restricting tier
 *   D to U        HR
 *   H to U        T(U->L) clear AND (H from W OR SR)           approach()
 *   SR            sets when T(L->W) goes occupied while HR was up on the
 *                 previous scan; holds while H from W is released
 *
 * The display is a control-table Route with no signal control, marked
 * automatic (ADR 0008 D1). The two outputs are Facts named <mast>H and
 * <mast>D (D3), computed from inputs and the stick when read; nothing is
 * stored but the stick and the previous scan's HR (D4). Every operand is a
 * Fact (D2): a track circuit on this processor, a line input carrying a
 * neighbour's value, or another local location's output.
 *
 * The mast is an output. Nothing reads it.
 *
 * Names bind when the call is made, as for a Route: add every track
 * circuit, line input and automatic signal before a binding names it.
 */
class AutomaticSignal {
public:
    AutomaticSignal()
        : cp_(nullptr), engine_(nullptr), route_(nullptr), mast_(nullptr),
          hIn_(nullptr), dIn_(nullptr), opposing_(nullptr),
          governedCount_(0), approachCount_(0),
          sr_(false), hrWasUp_(false), busy_(false),
          hOut_(this, Output::H), dOut_(this, Output::D), cut_(this) {
        name_[0] = '\0';
    }

    void bind(const char* name, InterlockingPlant* cp, InterlockingEngine* engine,
              Route* route, SignalMast* mast) {
        setName(name);
        cp_ = cp;
        engine_ = engine;
        route_ = route;
        mast_ = mast;
        hOut_.setName(name_, 'H');
        dOut_.setName(name_, 'D');
        cut_.setName(name_);
    }

    void setName(const char* name) {
        if (!name) { name_[0] = '\0'; return; }
        strncpy(name_, name, sizeof(name_) - 1);
        name_[sizeof(name_) - 1] = '\0';
    }

    const char* name() const { return name_; }
    SignalMast* mast() const { return mast_; }
    Route* route() const { return route_; }

    // --- configuration (names resolve through the plant; see InterlockingPlant.h) ---

    // T(L->W): the block the signal governs into. Also the route's track term.
    AutomaticSignal& governs(std::initializer_list<const char*> names);
    AutomaticSignal& governs(const Fact* f) {
        if (f && governedCount_ < MAX_BLOCK_FACTS) {
            governed_[governedCount_++] = f;
            if (route_) route_->homeFact(f);
        }
        return *this;
    }

    // T(U->L): the block behind the signal, reported to U in H.
    AutomaticSignal& approach(std::initializer_list<const char*> names);
    AutomaticSignal& approach(const Fact* f) {
        if (f && approachCount_ < MAX_BLOCK_FACTS) {
            approach_[approachCount_++] = f;
        }
        return *this;
    }

    // The HD line from W.
    AutomaticSignal& reads(const char* hName, const char* dName);
    AutomaticSignal& reads(const Fact* h, const Fact* d) {
        hIn_ = h;
        dIn_ = d;
        if (route_) {
            if (h) route_->homeFact(h);
            route_->proceed(d);
        }
        return *this;
    }

    // sameLocationOpposingCut (ADR 0013 section 14.2): the opposing mast at
    // this location holds this one at Stop while its HR or SR is up. Reads the
    // opposing relays' held state, as a back contact does.
    AutomaticSignal& opposing(const char* mastName);
    AutomaticSignal& opposing(AutomaticSignal* other) {
        opposing_ = other;
        if (route_ && other) route_->homeFact(&cut_);
        return *this;
    }

    // --- outputs ---

    const Fact* hOut() const { return &hOut_; }
    const Fact* dOut() const { return &dOut_; }

    // --- state ---

    bool SR() const { return sr_; }
    bool HR() const { return computeD(); }
    const Fact* hIn() const { return hIn_; }
    const Fact* dIn() const { return dIn_; }
    const AutomaticSignal* opposingSignal() const { return opposing_; }
    uint8_t governedCount() const { return governedCount_; }
    const Fact* governed(uint8_t i) const { return (i < governedCount_) ? governed_[i] : nullptr; }
    uint8_t approachCount() const { return approachCount_; }
    const Fact* approachFact(uint8_t i) const { return (i < approachCount_) ? approach_[i] : nullptr; }

    // --- scan (called by InterlockingPlant::evaluateVitalRules) ---

    // Before the control table evaluates: advance the stick.
    void updateStick() {
        bool governedOccupied = !allAsserted(governed_, governedCount_);
        if (hrWasUp_ && governedOccupied) {
            sr_ = true;
        } else if (sr_ && hInAsserted()) {
            sr_ = false;
        }
    }

    // After the control table evaluates: remember this scan's HR. This is
    // the slow-release HR of ADR 0013 section 15.3, one scan long.
    void latchHR() {
        hrWasUp_ = computeD();
    }

private:
    class Output : public Fact {
    public:
        enum Kind : uint8_t { H, D };
        Output(AutomaticSignal* owner, Kind kind) : owner_(owner), kind_(kind) { name_[0] = '\0'; }
        void setName(const char* base, char suffix) {
            size_t n = strlen(base);
            if (n > sizeof(name_) - 2) n = sizeof(name_) - 2;
            memcpy(name_, base, n);
            name_[n] = suffix;
            name_[n + 1] = '\0';
        }
        const char* name() const override { return name_; }
        bool asserted() const override {
            return kind_ == H ? owner_->computeH() : owner_->computeD();
        }
    private:
        AutomaticSignal* owner_;
        Kind kind_;
        char name_[MAX_APPLIANCE_NAME_LEN + 2];
    };

    class OpposingCut : public Fact {
    public:
        explicit OpposingCut(AutomaticSignal* owner) : owner_(owner) { name_[0] = '\0'; }
        void setName(const char* base) {
            snprintf(name_, sizeof(name_), "%sCUT", base);
        }
        const char* name() const override { return name_; }
        bool asserted() const override {
            const AutomaticSignal* o = owner_->opposing_;
            return o == nullptr || (!o->hrWasUp_ && !o->sr_);
        }
    private:
        AutomaticSignal* owner_;
        char name_[MAX_APPLIANCE_NAME_LEN + 4];
    };

    static bool allAsserted(const Fact* const* facts, uint8_t count) {
        for (uint8_t i = 0; i < count; ++i) {
            if (facts[i] == nullptr || !facts[i]->asserted()) return false;
        }
        return true;
    }

    // An absent line term is an absent term, as in the route equation.
    bool hInAsserted() const {
        return hIn_ == nullptr || hIn_->asserted();
    }

    // H to U = T(U->L) clear AND (H from W OR SR). Re-entry (a loop of
    // locations) reads as released.
    bool computeH() const {
        if (busy_) return false;
        busy_ = true;
        bool h = allAsserted(approach_, approachCount_) && (hInAsserted() || sr_);
        busy_ = false;
        return h;
    }

    // D to U = HR = the route equation above the Restricting tier.
    bool computeD() const {
        if (busy_ || engine_ == nullptr || route_ == nullptr) return false;
        busy_ = true;
        Indication ind = engine_->evaluateRouteIndication(*route_);
        busy_ = false;
        return Route::permissivenessRank(ind) > 1;
    }

    char               name_[MAX_APPLIANCE_NAME_LEN];
    InterlockingPlant* cp_;
    InterlockingEngine* engine_;
    Route*             route_;
    SignalMast*        mast_;
    const Fact*        hIn_;
    const Fact*        dIn_;
    AutomaticSignal*   opposing_;
    const Fact*        governed_[MAX_BLOCK_FACTS];
    uint8_t            governedCount_;
    const Fact*        approach_[MAX_BLOCK_FACTS];
    uint8_t            approachCount_;
    bool               sr_;
    bool               hrWasUp_;
    mutable bool       busy_;
    Output             hOut_;
    Output             dOut_;
    OpposingCut        cut_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_AUTOMATIC_SIGNAL_H
