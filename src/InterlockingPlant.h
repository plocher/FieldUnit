#ifndef FIELDUNIT_INTERLOCKING_PLANT_H
#define FIELDUNIT_INTERLOCKING_PLANT_H

#include <string.h>
#include "types.h"
#include "TrackCircuit.h"
#include "Switch.h"
#include "Crossover.h"
#include "SignalMast.h"
#include "SignalControl.h"
#include "ControlTable.h"
#include "AutomaticSignal.h"
#include "ControlPointLine.h"

namespace FieldUnit {

static constexpr uint8_t MAX_APPLIANCES = 16;
static constexpr uint8_t MAX_CROSSOVERS = 8;
static constexpr uint8_t MAX_MOCK_SWITCHES = 8;

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

class InterlockingPlant;
class DriverPolicy;
class ApplianceDriver;
class MockSwitchDriver;

// FieldUnit ADR 0009 D3: what the plant records when a binding's name has
// no source. The engine never consults this; the vital answer to a gap is
// the released term (D1) or the route at Stop (D2).
enum class UnresolvedKind : uint8_t {
    IMPLICIT = 0,   // a Fact name with no source: an implicit line input was declared (D1)
    NO_SLOT  = 1,   // the same, with no line-input slot left: bound to the released fact (D1)
    MISSING  = 2,   // an appliance name with no source: the owner is incomplete, at Stop (D2)
    CONFLICT = 3    // a source was added after an implicit fact of its name existed; the
                    // implicit fact shadows it (D1's limit)
};

inline const char* unresolvedKindName(UnresolvedKind k) {
    switch (k) {
        case UnresolvedKind::IMPLICIT: return "implicit";
        case UnresolvedKind::NO_SLOT:  return "no_slot";
        case UnresolvedKind::MISSING:  return "missing";
        default:                       return "conflict";
    }
}

struct UnresolvedReference {
    char           name[MAX_APPLIANCE_NAME_LEN + 2];  // the name that did not resolve
    char           owner[MAX_ROUTE_NAME_LEN];         // the route or location that named it
    const char*    call;                              // the configuration call, a literal
    UnresolvedKind kind;
};

static constexpr uint8_t  MAX_UNRESOLVED = 16;

class InterlockingPlant {
public:
    InterlockingPlant(const char* name, AspectResolver defaultPolicy = AspectPolicies::defaultRoute)
        : defaultAspectPolicy_(defaultPolicy ? defaultPolicy : AspectPolicies::defaultRoute),
          defaultDriverPolicy_(nullptr),
          driverOverrideCount_(0),
          mockSwitchCount_(0),
          trackCircuitCount_(0),
          lineInputCount_(0),
          switchCount_(0),
          authorityCount_(0),
          mastCount_(0),
          crossoverCount_(0),
          detectorLockCouplingCount_(0),
          automaticSignalCount_(0),
          controlPointLineCount_(0),
          unresolvedCount_(0),
          unresolvedTotal_(0),
          defaultStalenessTimeoutMs_(DEFAULT_STALENESS_TIMEOUT_MS) {
        setName(name);
        for (uint8_t i = 0; i < MAX_APPLIANCES; ++i) {
            maintainerCalls_[i] = false;
        }
    }

    void setName(const char* name) {
        if (!name) { name_[0] = '\0'; return; }
        strncpy(name_, name, sizeof(name_) - 1);
        name_[sizeof(name_) - 1] = '\0';
    }

    const char* name() const { return name_; }

    void reset() {
        trackCircuitCount_ = 0;
        lineInputCount_ = 0;
        automaticSignalCount_ = 0;
        controlPointLineCount_ = 0;
        switchCount_ = 0;
        crossoverCount_ = 0;
        authorityCount_ = 0;
        mastCount_ = 0;
        for (uint8_t i = 0; i < MAX_APPLIANCES; ++i) {
            maintainerCalls_[i] = false;
        }
        detectorLockCouplingCount_ = 0;
        unresolvedCount_ = 0;
        unresolvedTotal_ = 0;
        defaultStalenessTimeoutMs_ = DEFAULT_STALENESS_TIMEOUT_MS;
        driverOverrideCount_ = 0;
        mockSwitchCount_ = 0;
        defaultDriverPolicy_ = nullptr;
        defaultAspectPolicy_ = AspectPolicies::defaultRoute;
        setName("Blank");
        engine_.clear();
    }

    void setDefaultDriverPolicy(DriverPolicy* policy) {
        defaultDriverPolicy_ = policy;
    }

    DriverPolicy* defaultDriverPolicy() const { return defaultDriverPolicy_; }

    void overrideDriver(const char* applianceName, ApplianceDriver* driver);
    void mockSwitch(const char* applianceName, uint32_t travelTimeMs = 2000);

    uint8_t trackCircuitCount() const { return trackCircuitCount_; }
    const TrackCircuit* trackCircuit(uint8_t idx) const { return (idx < trackCircuitCount_) ? &trackCircuits_[idx] : nullptr; }
    TrackCircuit* trackCircuit(uint8_t idx) { return (idx < trackCircuitCount_) ? &trackCircuits_[idx] : nullptr; }

    uint8_t switchCount() const { return switchCount_; }
    const Switch* getSwitch(uint8_t idx) const { return (idx < switchCount_) ? &switches_[idx] : nullptr; }
    Switch* getSwitch(uint8_t idx) { return (idx < switchCount_) ? &switches_[idx] : nullptr; }

    uint8_t mastCount() const { return mastCount_; }
    const SignalMast* mast(uint8_t idx) const { return (idx < mastCount_) ? &masts_[idx] : nullptr; }
    SignalMast* mast(uint8_t idx) { return (idx < mastCount_) ? &masts_[idx] : nullptr; }

    uint8_t authorityCount() const { return authorityCount_; }
    const SignalControl* authority(uint8_t idx) const { return (idx < authorityCount_) ? &authorities_[idx] : nullptr; }
    SignalControl* authority(uint8_t idx) { return (idx < authorityCount_) ? &authorities_[idx] : nullptr; }

    uint8_t crossoverCount() const { return crossoverCount_; }
    const Crossover* crossover(uint8_t idx) const { return (idx < crossoverCount_) ? &crossovers_[idx] : nullptr; }
    Crossover* crossover(uint8_t idx) { return (idx < crossoverCount_) ? &crossovers_[idx] : nullptr; }

    uint8_t detectorLockCount() const { return detectorLockCouplingCount_; }
    Switch* detectorLockSwitch(uint8_t idx) const { return (idx < detectorLockCouplingCount_) ? detectorLocks_[idx].sw : nullptr; }
    TrackCircuit* detectorLockTrackCircuit(uint8_t idx) const { return (idx < detectorLockCouplingCount_) ? detectorLocks_[idx].tc : nullptr; }

    InterlockingEngine& engine() { return engine_; }
    const InterlockingEngine& engine() const { return engine_; }

    // Serialization & Deserialization
    bool serialize(char* buffer, size_t maxLen, bool pretty = true) const;
    bool deserialize(const char* json);

    void setDefaultAspectPolicy(AspectResolver policy) {
        defaultAspectPolicy_ = policy ? policy : AspectPolicies::defaultRoute;
    }

    AspectResolver defaultAspectPolicy() const { return defaultAspectPolicy_; }

    TrackCircuit* addTrackCircuit(const char* name, uint32_t dropoutDelayMs = 0) {
        if (trackCircuitCount_ >= MAX_APPLIANCES) return nullptr;
        noteConflict(name, "addTrackCircuit");
        trackCircuits_[trackCircuitCount_] = TrackCircuit(name, dropoutDelayMs);
        trackCircuits_[trackCircuitCount_].setIndex(trackCircuitCount_);
        trackCircuits_[trackCircuitCount_].setStalenessTimeout(defaultStalenessTimeoutMs_);
        return &trackCircuits_[trackCircuitCount_++];
    }

    Switch* addSwitch(const char* name) {
        if (switchCount_ >= MAX_APPLIANCES) return nullptr;
        switches_[switchCount_] = Switch(name);
        switches_[switchCount_].setIndex(switchCount_);
        return &switches_[switchCount_++];
    }

    /**
     * Declare a switch and bind its OS island track circuit for detector locking.
     * Creates the track circuit if it does not already exist.
     */
    Switch* addSwitch(const char* name, const char* osTrackCircuitName) {
        Switch* sw = addSwitch(name);
        if (!sw || !osTrackCircuitName || osTrackCircuitName[0] == '\0') {
            return sw;
        }
        TrackCircuit* tc = findTrackCircuit(osTrackCircuitName);
        if (!tc) {
            tc = addTrackCircuit(osTrackCircuitName);
        }
        if (tc) {
            bindDetectorLock(sw, tc);
        }
        return sw;
    }

    /**
     * Declare a derail appliance.
     *
     * Naming rules:
     * - "5" (unique id) => dispatcher-controlled derail (CodeLine citizen).
     * - "1D" when switch "1" already exists => dependent derail paired to "1" (same position),
     *   hidden from CodeLine as its own lever/indication step.
     * - "1D" when switch "1" does not exist => configuration error (nullptr).
     *   A *D name never falls through to an independent derail.
     *
     * Optional osTrackCircuitName creates/binds the derail island for detector lock.
     *
     * Position vocabulary (prototype): NORMAL = derailing (on the rail), REVERSE = clear.
     * Independent and dependent derails rest fail-safe in NORMAL (derailing).
     */
    Switch* addDerail(const char* name, const char* osTrackCircuitName = nullptr) {
        if (!name || name[0] == '\0') return nullptr;

        // Detect <baseId>D dependent form (e.g. "1D", "3D", "777D")
        const size_t len = strlen(name);
        Switch* base = nullptr;
        bool dependentForm = false;
        if (len >= 2 && (name[len - 1] == 'D' || name[len - 1] == 'd')) {
            char baseName[MAX_APPLIANCE_NAME_LEN];
            if (len < sizeof(baseName)) {
                memcpy(baseName, name, len - 1);
                baseName[len - 1] = '\0';
                // Require a non-empty base id that is not itself only punctuation
                if (baseName[0] != '\0') {
                    dependentForm = true;
                    base = findSwitchExact(baseName);
                    if (!base) {
                        return nullptr; // missing base: hard error, never independent
                    }
                }
            }
        }

        if (switchCount_ >= MAX_APPLIANCES) return nullptr;
        switches_[switchCount_] = Switch(name);
        switches_[switchCount_].setIndex(switchCount_);
        Switch* derail = &switches_[switchCount_++];
        derail->configureAsDerail();

        if (dependentForm && base) {
            base->pairDependentDerail(derail);
        }

        if (osTrackCircuitName && osTrackCircuitName[0] != '\0') {
            TrackCircuit* tc = findTrackCircuit(osTrackCircuitName);
            if (!tc) {
                tc = addTrackCircuit(osTrackCircuitName);
            }
            if (tc) {
                bindDetectorLock(derail, tc);
            }
        }
        return derail;
    }

    Crossover* addCrossover(const char* name, Switch* swA, Switch* swB) {
        if (crossoverCount_ >= MAX_CROSSOVERS) return nullptr;
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
        if (!sw || !tc) return;
        for (uint8_t i = 0; i < detectorLockCouplingCount_; ++i) {
            if (detectorLocks_[i].sw == sw && detectorLocks_[i].tc == tc) {
                return; // already bound
            }
        }
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

    // -------------------------------------------------------------
    // Line inputs (FieldUnit ADR 0007 D2): the consumer end of a line
    // circuit, a named Fact that a route's home() or proceed() term requires.
    // -------------------------------------------------------------
    LineInput* addLineInput(const char* name, uint32_t stalenessTimeoutMs) {
        noteConflict(name, "addLineInput");
        return declareLineInput(name, stalenessTimeoutMs);
    }

    // With the plant's default staleness timeout (FieldUnit ADR 0009:
    // a per-interlocking setting, DEFAULT_STALENESS_TIMEOUT_MS unless set).
    LineInput* addLineInput(const char* name) {
        return addLineInput(name, defaultStalenessTimeoutMs_);
    }

    void setDefaultStalenessTimeout(uint32_t timeoutMs) { defaultStalenessTimeoutMs_ = timeoutMs; }
    uint32_t defaultStalenessTimeout() const { return defaultStalenessTimeoutMs_; }

    // -------------------------------------------------------------
    // Unresolved references (FieldUnit ADR 0009 D1 to D3). A binding whose
    // name has no source calls one of these; the plant records the gap and
    // answers with something that fails safe.
    // -------------------------------------------------------------

    // A Fact-typed name (home, proceed, governs, approach, block, reads).
    // Null or empty is an absent term, not a reference. A name with no
    // source becomes an implicit line input, released until driven; with
    // no slot left, the shared released fact.
    const Fact* resolveFact(const char* name, const char* call, const char* owner) {
        if (!name || name[0] == '\0') return nullptr;
        const Fact* f = findFact(name);
        if (f) return f;
        LineInput* li = declareLineInput(name, defaultStalenessTimeoutMs_);
        if (li) {
            li->setImplicit(true);
            noteUnresolved(name, call, owner, UnresolvedKind::IMPLICIT);
            return li;
        }
        noteUnresolved(name, call, owner, UnresolvedKind::NO_SLOT);
        return releasedFact();
    }

    // An appliance-typed name that did not resolve: recorded as missing.
    // The caller marks its route or location incomplete.
    void noteMissing(const char* name, const char* call, const char* owner) {
        if (!name || name[0] == '\0') return;
        noteUnresolved(name, call, owner, UnresolvedKind::MISSING);
    }

    uint8_t  unresolvedCount() const { return unresolvedCount_; }   // stored (at most MAX_UNRESOLVED)
    uint16_t unresolvedTotal() const { return unresolvedTotal_; }   // counted, including those not stored
    const UnresolvedReference* unresolved(uint8_t idx) const {
        return (idx < unresolvedCount_) ? &unresolved_[idx] : nullptr;
    }
    bool isComplete() const { return unresolvedTotal_ == 0; }

    LineInput* findLineInput(const char* name) {
        if (!name) return nullptr;
        for (uint8_t i = 0; i < lineInputCount_; ++i) {
            if (strcmp(lineInputs_[i].name(), name) == 0) return &lineInputs_[i];
        }
        return nullptr;
    }

    // A fact by name (FieldUnit ADR 0008 D2, D3): a line input, the H or D
    // an automatic signal produces, or a track circuit (its TR).
    const Fact* findFact(const char* name) {
        if (!name) return nullptr;
        const Fact* f = findLineInput(name);
        if (f) return f;
        for (uint8_t i = 0; i < automaticSignalCount_; ++i) {
            if (strcmp(automaticSignals_[i].hOut()->name(), name) == 0) return automaticSignals_[i].hOut();
            if (strcmp(automaticSignals_[i].dOut()->name(), name) == 0) return automaticSignals_[i].dOut();
        }
        for (uint8_t i = 0; i < controlPointLineCount_; ++i) {
            const ControlPointLine& l = controlPointLines_[i];
            if (strcmp(l.hOut()->name(), name) == 0) return l.hOut();
            if (strcmp(l.dOut()->name(), name) == 0) return l.dOut();
            if (l.producesF() && strcmp(l.fOut()->name(), name) == 0) return l.fOut();
        }
        return findTrackCircuit(name);
    }

    // Control point line (FieldUnit ADR 0008 D5): what this plant produces at
    // the seam with one block, named for the home mast facing the block.
    ControlPointLine& addControlPointLine(const char* name) {
        if (controlPointLineCount_ >= MAX_APPLIANCES) {
            static ControlPointLine overflow;
            return overflow;
        }
        noteConflict(name, 'H', "addControlPointLine");
        noteConflict(name, 'D', "addControlPointLine");
        noteConflict(name, 'F', "addControlPointLine");
        ControlPointLine& l = controlPointLines_[controlPointLineCount_++];
        l.bind(name, this, &engine_);
        SignalMast* home = findSignalMast(name);
        if (home == nullptr) {
            noteMissing(name, "addControlPointLine", name);
            l.markIncomplete();
        }
        l.home(home);
        return l;
    }

    ControlPointLine* findControlPointLine(const char* name) {
        if (!name) return nullptr;
        for (uint8_t i = 0; i < controlPointLineCount_; ++i) {
            if (strcmp(controlPointLines_[i].name(), name) == 0) return &controlPointLines_[i];
        }
        return nullptr;
    }

    uint8_t controlPointLineCount() const { return controlPointLineCount_; }
    const ControlPointLine* controlPointLine(uint8_t idx) const { return (idx < controlPointLineCount_) ? &controlPointLines_[idx] : nullptr; }

    // Automatic signal (FieldUnit ADR 0008 D1): one mast, one direction,
    // its own route with no signal control, its stick and its HD line out.
    AutomaticSignal& addAutomaticSignal(const char* name, MastType type, AspectResolver policy = nullptr) {
        if (automaticSignalCount_ >= MAX_APPLIANCES) {
            static AutomaticSignal overflow;
            return overflow;
        }
        noteConflict(name, 'H', "addAutomaticSignal");
        noteConflict(name, 'D', "addAutomaticSignal");
        SignalMast* mast = addSignalMast(name, type, policy);
        Route& r = route(name);
        r.displays(name, Indication::CLEAR).automatic();
        AutomaticSignal& a = automaticSignals_[automaticSignalCount_++];
        a.bind(name, this, &engine_, &r, mast);
        return a;
    }

    AutomaticSignal* findAutomaticSignal(const char* name) {
        if (!name) return nullptr;
        for (uint8_t i = 0; i < automaticSignalCount_; ++i) {
            if (strcmp(automaticSignals_[i].name(), name) == 0) return &automaticSignals_[i];
        }
        return nullptr;
    }

    uint8_t automaticSignalCount() const { return automaticSignalCount_; }
    const AutomaticSignal* automaticSignal(uint8_t idx) const { return (idx < automaticSignalCount_) ? &automaticSignals_[idx] : nullptr; }
    AutomaticSignal* automaticSignal(uint8_t idx) { return (idx < automaticSignalCount_) ? &automaticSignals_[idx] : nullptr; }
    bool isAutomaticMast(const SignalMast* mast) const {
        for (uint8_t i = 0; i < automaticSignalCount_; ++i) {
            if (automaticSignals_[i].mast() == mast) return true;
        }
        return false;
    }

    uint8_t lineInputCount() const { return lineInputCount_; }
    const LineInput* lineInput(uint8_t idx) const { return (idx < lineInputCount_) ? &lineInputs_[idx] : nullptr; }
    LineInput* lineInput(uint8_t idx) { return (idx < lineInputCount_) ? &lineInputs_[idx] : nullptr; }

    /** Resolve a physical switch or derail by exact name (not logical crossovers). */
    Switch* findSwitchExact(const char* name) {
        if (!name) return nullptr;
        for (uint8_t i = 0; i < switchCount_; ++i) {
            if (strcmp(switches_[i].name(), name) == 0) return &switches_[i];
        }
        return nullptr;
    }

    Switch* findSwitch(const char* name) {
        if (!name) return nullptr;
        Switch* exact = findSwitchExact(name);
        if (exact) return exact;
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
        // Prefer a direct binding for this appliance
        for (uint8_t i = 0; i < detectorLockCouplingCount_; ++i) {
            if (detectorLocks_[i].sw == sw) {
                return detectorLocks_[i].tc;
            }
        }
        // Fall back to a paired machine only when this appliance has no OS of its own
        if (sw->pairedSwitch() != nullptr) {
            for (uint8_t i = 0; i < detectorLockCouplingCount_; ++i) {
                if (detectorLocks_[i].sw == sw->pairedSwitch()) {
                    return detectorLocks_[i].tc;
                }
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
                // Dependent derails are driven only via their base switch pair
                if (switches_[i].isDependentDerail()) {
                    continue;
                }
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
        for (uint8_t i = 0; i < MAX_APPLIANCES; ++i) {
            maintainerCalls_[i] = ctl.maintainerCall[i];
        }
    }

    void sampleInputs(uint32_t nowMs);
    void driveOutputs(uint32_t nowMs);

    // Vital Scan Cycle: Periodic atomic scan period
    // 1. Ingress: Sample physical pins / MQTT subscriptions
    // 2. Vital Interlocking: Locks, correspondence, route evaluation
    // 3. Egress: Actuate switch motors, PWM servos, signal lamps
    void tick(uint32_t nowMs = 0) {
#if defined(ARDUINO)
        if (nowMs == 0) nowMs = millis();
#endif
        // 1. Ingress
        sampleInputs(nowMs);

        // 2. Vital Evaluation
        evaluateVitalRules(nowMs);

        // 3. Egress
        driveOutputs(nowMs);
    }

    void evaluateVitalRules(uint32_t nowMs) {
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
        for (uint8_t i = 0; i < lineInputCount_; ++i) {
            lineInputs_[i].tick(nowMs);
        }

        // C. Evaluate signal authority stick / fleet logic
        for (uint8_t i = 0; i < authorityCount_; ++i) {
            bool routeClear = true;
            DirectionAuthority cmdDir = authorities_[i].commandedDirection();
            if (cmdDir != DirectionAuthority::STOP) {
                for (uint8_t rIdx = 0; rIdx < engine_.routeCount(); ++rIdx) {
                    const Route& r = engine_.route(rIdx);
                    if (r.authority() == &authorities_[i] && r.direction() == cmdDir) {
                        if (!engine_.checkRouteBlocksClear(r) || !engine_.checkRouteHomeFacts(r)) {
                            routeClear = false;
                            break;
                        }
                    }
                }
            }
            authorities_[i].evaluate(routeClear, nowMs);
        }

        // D. Automatic signal sticks advance before the control table runs
        //    (FieldUnit ADR 0008 D4): each stick reads the previous scan's HR.
        for (uint8_t i = 0; i < automaticSignalCount_; ++i) {
            automaticSignals_[i].updateStick();
        }

        // E. Interlocking Control Table vital route evaluation
        engine_.evaluate();

        // F. Remember this scan's HR for the next scan's stick; latch the
        //    control point lines' traffic sticks on this scan's ports
        for (uint8_t i = 0; i < automaticSignalCount_; ++i) {
            automaticSignals_[i].latchHR();
        }
        for (uint8_t i = 0; i < controlPointLineCount_; ++i) {
            controlPointLines_[i].latch();
        }
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
            ind.maintainerCall[i] = maintainerCalls_[i];
        }
    }

    bool maintainerCall(uint8_t idx = 0) const { return (idx < MAX_APPLIANCES) ? maintainerCalls_[idx] : false; }
    void setMaintainerCall(uint8_t idx, bool val) { if (idx < MAX_APPLIANCES) maintainerCalls_[idx] = val; }
    bool maintainerCallActive() const { return maintainerCall(0); }
    void setMaintainerCall(bool val) { setMaintainerCall(0, val); }

private:
    struct DetectorBinding {
        Switch* sw;
        TrackCircuit* tc;
    };

    LineInput* declareLineInput(const char* name, uint32_t stalenessTimeoutMs) {
        if (lineInputCount_ >= MAX_APPLIANCES) return nullptr;
        lineInputs_[lineInputCount_] = LineInput(name, stalenessTimeoutMs);
        lineInputs_[lineInputCount_].setIndex(lineInputCount_);
        return &lineInputs_[lineInputCount_++];
    }

    void noteUnresolved(const char* name, const char* call, const char* owner, UnresolvedKind kind) {
        if (unresolvedTotal_ < 0xFFFF) ++unresolvedTotal_;
        if (unresolvedCount_ >= MAX_UNRESOLVED) return;
        UnresolvedReference& u = unresolved_[unresolvedCount_++];
        strncpy(u.name, name ? name : "", sizeof(u.name) - 1);
        u.name[sizeof(u.name) - 1] = '\0';
        strncpy(u.owner, owner ? owner : "", sizeof(u.owner) - 1);
        u.owner[sizeof(u.owner) - 1] = '\0';
        u.call = call ? call : "";
        u.kind = kind;
    }

    // A source added after an implicit fact of its name exists cannot be
    // bound to; findFact() finds the implicit line input first.
    void noteConflict(const char* name, const char* call) {
        LineInput* li = findLineInput(name);
        if (li && li->isImplicit()) {
            noteUnresolved(name, call, name, UnresolvedKind::CONFLICT);
        }
    }

    void noteConflict(const char* base, char suffix, const char* call) {
        if (!base) return;
        char n[MAX_APPLIANCE_NAME_LEN + 2];
        size_t len = strlen(base);
        if (len > sizeof(n) - 2) len = sizeof(n) - 2;
        memcpy(n, base, len);
        n[len] = suffix;
        n[len + 1] = '\0';
        noteConflict(n, call);
    }

    struct DriverOverride {
        const char* name;
        ApplianceDriver* driver;
    };

    char name_[MAX_ROUTE_NAME_LEN];
    AspectResolver defaultAspectPolicy_;
    DriverPolicy* defaultDriverPolicy_;
    DriverOverride driverOverrides_[MAX_APPLIANCES];
    uint8_t driverOverrideCount_;
    MockSwitchDriver* mockSwitches_[MAX_MOCK_SWITCHES];
    uint8_t mockSwitchCount_;
    TrackCircuit trackCircuits_[MAX_APPLIANCES];
    uint8_t trackCircuitCount_;

    LineInput lineInputs_[MAX_APPLIANCES];
    uint8_t lineInputCount_;

    Switch switches_[MAX_APPLIANCES];
    uint8_t switchCount_;

    Crossover crossovers_[MAX_CROSSOVERS];
    uint8_t crossoverCount_;

    SignalControl authorities_[MAX_APPLIANCES];
    uint8_t authorityCount_;

    SignalMast masts_[MAX_APPLIANCES];
    uint8_t mastCount_;

    bool maintainerCalls_[MAX_APPLIANCES];

    DetectorBinding detectorLocks_[MAX_APPLIANCES];
    uint8_t detectorLockCouplingCount_;

    AutomaticSignal automaticSignals_[MAX_APPLIANCES];
    uint8_t automaticSignalCount_;

    ControlPointLine controlPointLines_[MAX_APPLIANCES];
    uint8_t controlPointLineCount_;

    UnresolvedReference unresolved_[MAX_UNRESOLVED];
    uint8_t  unresolvedCount_;
    uint16_t unresolvedTotal_;
    uint32_t defaultStalenessTimeoutMs_;

    InterlockingEngine engine_;
};

// -------------------------------------------------------------
// Route String-Based Configuration Implementations
// Resolves string names once during setup to retain O(1) runtime pointer execution.
// -------------------------------------------------------------

// An appliance-typed name with no source (FieldUnit ADR 0009 D2): the plant
// records it and the route is incomplete, which evaluates to Stop.
inline Route& Route::governedBy(const char* signalName, DirectionAuthority dir) {
    authority_ = cp_ ? cp_->findSignalControl(signalName) : nullptr;
    direction_ = dir;
    if (cp_ && authority_ == nullptr) { cp_->noteMissing(signalName, "governedBy", name_); markIncomplete(); }
    return *this;
}

inline Route& Route::displays(const char* mastName, Indication maxIndication) {
    mast_ = cp_ ? cp_->findSignalMast(mastName) : nullptr;
    aspectCeiling_ = maxIndication;
    if (cp_ && mast_ == nullptr) { cp_->noteMissing(mastName, "displays", name_); markIncomplete(); }
    return *this;
}

inline Route& Route::aligns(std::initializer_list<NamedSwitchRequirement> swList) {
    switchCount_ = 0;
    for (const auto& s : swList) {
        if (switchCount_ < MAX_ROUTE_SWITCHES) {
            align(s.switchName, s.requiredPosition, s.releasingBlockName);
        }
    }
    return *this;
}

inline Route& Route::align(const char* swName, SwitchPosition pos, const char* relName) {
    if (switchCount_ < MAX_ROUTE_SWITCHES && cp_) {
        Switch* sw = cp_->findSwitch(swName);
        TrackCircuit* rel = relName ? cp_->findTrackCircuit(relName) : nullptr;
        if (sw) {
            switches_[switchCount_++] = { sw, pos, rel };
        } else {
            cp_->noteMissing(swName, "aligns", name_);
            markIncomplete();
        }
        if (relName && rel == nullptr) {
            cp_->noteMissing(relName, "aligns", name_);
            markIncomplete();
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
            clearBlock(tcName);
        }
    }
    return *this;
}

inline Route& Route::clearBlock(const char* tcName) {
    if (blockCount_ < MAX_ROUTE_BLOCKS && cp_) {
        TrackCircuit* tc = cp_->findTrackCircuit(tcName);
        if (tc) {
            blocks_[blockCount_++] = tc;
        } else {
            cp_->noteMissing(tcName, "clears", name_);
            markIncomplete();
        }
    }
    return *this;
}

inline Route& Route::entrance(const char* tcName) {
    entranceBlock_ = cp_ ? cp_->findTrackCircuit(tcName) : nullptr;
    if (cp_ && entranceBlock_ == nullptr) { cp_->noteMissing(tcName, "entrance", name_); markIncomplete(); }
    return *this;
}

inline Route& Route::approaching(const char* tcName) {
    approachBlock_ = cp_ ? cp_->findTrackCircuit(tcName) : nullptr;
    if (cp_ && approachBlock_ == nullptr) { cp_->noteMissing(tcName, "approaching", name_); markIncomplete(); }
    return *this;
}

// A Fact-typed name with no source (FieldUnit ADR 0009 D1): the plant
// declares an implicit line input, released until a ghost drives it.
inline Route& Route::home(std::initializer_list<const char*> factNames) {
    homeFactCount_ = 0;
    for (const char* factName : factNames) {
        homeFact(factName);
    }
    return *this;
}

inline Route& Route::homeFact(const char* factName) {
    const Fact* f = cp_ ? cp_->resolveFact(factName, "home", name_) : nullptr;
    return homeFact(f);
}

inline Route& Route::proceed(const char* factName) {
    proceedFact_ = cp_ ? cp_->resolveFact(factName, "proceed", name_) : nullptr;
    return *this;
}

// -------------------------------------------------------------
// AutomaticSignal String-Based Configuration Implementations
// -------------------------------------------------------------

inline AutomaticSignal& AutomaticSignal::governs(std::initializer_list<const char*> names) {
    for (const char* n : names) {
        governs(cp_ ? cp_->resolveFact(n, "governs", name_) : nullptr);
    }
    return *this;
}

inline AutomaticSignal& AutomaticSignal::approach(std::initializer_list<const char*> names) {
    for (const char* n : names) {
        approach(cp_ ? cp_->resolveFact(n, "approach", name_) : nullptr);
    }
    return *this;
}

inline AutomaticSignal& AutomaticSignal::reads(const char* hName, const char* dName) {
    const Fact* h = cp_ ? cp_->resolveFact(hName, "reads", name_) : nullptr;
    const Fact* d = cp_ ? cp_->resolveFact(dName, "reads", name_) : nullptr;
    return reads(h, d);
}

inline AutomaticSignal& AutomaticSignal::opposing(const char* mastName) {
    AutomaticSignal* other = cp_ ? cp_->findAutomaticSignal(mastName) : nullptr;
    if (cp_ && other == nullptr) { cp_->noteMissing(mastName, "opposing", name_); markIncomplete(); }
    return opposing(other);
}

// -------------------------------------------------------------
// ControlPointLine String-Based Configuration Implementations
// -------------------------------------------------------------

inline ControlPointLine& ControlPointLine::governs(std::initializer_list<const char*> names) {
    for (const char* n : names) governs(cp_ ? cp_->resolveFact(n, "governs", name_) : nullptr);
    return *this;
}

inline ControlPointLine& ControlPointLine::block(std::initializer_list<const char*> names) {
    for (const char* n : names) block(cp_ ? cp_->resolveFact(n, "block", name_) : nullptr);
    return *this;
}

inline ControlPointLine& ControlPointLine::leaving(std::initializer_list<const char*> mastNames) {
    for (const char* n : mastNames) {
        SignalMast* m = cp_ ? cp_->findSignalMast(n) : nullptr;
        if (cp_ && m == nullptr) { cp_->noteMissing(n, "leaving", name_); markIncomplete(); }
        leaving(m);
    }
    return *this;
}

inline ControlPointLine& ControlPointLine::home(const char* mastName) {
    SignalMast* m = cp_ ? cp_->findSignalMast(mastName) : nullptr;
    if (cp_ && m == nullptr) { cp_->noteMissing(mastName, "home", name_); markIncomplete(); }
    return home(m);
}

inline Route& Route::engineReturn(const char* standingCarsName, const char* islandName) {
    TrackCircuit* standing = cp_ ? cp_->findTrackCircuit(standingCarsName) : nullptr;
    TrackCircuit* island = cp_ ? cp_->findTrackCircuit(islandName) : nullptr;
    if (cp_ && standing == nullptr) { cp_->noteMissing(standingCarsName, "engineReturn", name_); markIncomplete(); }
    if (cp_ && island == nullptr)   { cp_->noteMissing(islandName, "engineReturn", name_); markIncomplete(); }
    return engineReturn(standing, island);
}

} // namespace FieldUnit

#endif // FIELDUNIT_INTERLOCKING_PLANT_H
