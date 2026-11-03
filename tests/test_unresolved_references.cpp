#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../src/FieldUnit.h"

using namespace FieldUnit;

// FieldUnit ADR 0009: an incomplete plant. A Fact-typed name with no source
// becomes an implicit line input, released until a ghost drives it (D1). An
// appliance-typed name with no source makes its route or location incomplete,
// which is at Stop (D2). The plant reports its gaps and isComplete() (D3).
// Implicit facts are derived, not written (D4). A ghost drives them through
// update() and touch() (D5).

static const UnresolvedReference* findUnresolved(const InterlockingPlant& cp, const char* name, UnresolvedKind kind) {
    for (uint8_t i = 0; i < cp.unresolvedCount(); ++i) {
        const UnresolvedReference* u = cp.unresolved(i);
        if (strcmp(u->name, name) == 0 && u->kind == kind) return u;
    }
    return nullptr;
}

static void requestRight(InterlockingPlant& cp, const char* control, uint32_t nowMs) {
    ControlTransaction ctl;
    ctl.signalDemands[cp.findSignalControl(control)->index()] = SignalDemand::RIGHT;
    cp.applyControlTransaction(ctl, nowMs);
}

int main() {
    printf("====================================================\n");
    printf("   FIELDUNIT UNRESOLVED REFERENCES TEST SUITE       \n");
    printf("   An incomplete plant (FieldUnit ADR 0009)         \n");
    printf("====================================================\n\n");

    // ---------------------------------------------------------
    printf("[TEST 1] A complete plant has an empty report\n");
    {
        InterlockingPlant cp("COMPLETE");
        cp.addTrackCircuit("1T");
        cp.addLineInput("H_IN");
        cp.addSignalControl("2");
        cp.addSignalMast("2E", MastType::ONE_HEAD);
        cp.route("R").governedBy("2", DirectionAuthority::RIGHT).displays("2E", Indication::CLEAR)
          .clears({ "1T" }).home({ "H_IN" }).entrance("1T");
        assert(cp.isComplete());
        assert(cp.unresolvedCount() == 0);
        assert(cp.unresolvedTotal() == 0);
        assert(cp.lineInputCount() == 1);
        assert(!cp.lineInput(0)->isImplicit());
    }
    printf("   PASS\n\n");

    // ---------------------------------------------------------
    printf("[TEST 2] home() with no source: implicit line input, Stop until a ghost drives it (D1, D5)\n");
    {
        uint32_t clockMs = 1000;
        InterlockingPlant cp("GHOSTED");
        cp.addTrackCircuit("1T");
        cp.addSignalControl("2");
        cp.addSignalMast("2E", MastType::ONE_HEAD);
        cp.route("R").governedBy("2", DirectionAuthority::RIGHT).displays("2E", Indication::CLEAR)
          .clears({ "1T" }).home({ "SEGMENT_CLEAR" }).entrance("1T");

        assert(!cp.isComplete());
        assert(cp.unresolvedTotal() == 1);
        const UnresolvedReference* u = findUnresolved(cp, "SEGMENT_CLEAR", UnresolvedKind::IMPLICIT);
        assert(u != nullptr);
        assert(strcmp(u->call, "home") == 0);
        assert(strcmp(u->owner, "R") == 0);
        assert(strcmp(unresolvedKindName(u->kind), "implicit") == 0);

        LineInput* ghost = cp.findLineInput("SEGMENT_CLEAR");
        assert(ghost != nullptr);
        assert(ghost->isImplicit());
        assert(ghost->stalenessTimeout() == DEFAULT_STALENESS_TIMEOUT_MS);
        assert(static_cast<const Route&>(cp.engine().route(0)).homeFact(0) == ghost);

        TrackCircuit* tc = cp.findTrackCircuit("1T");
        SignalMast* mast = cp.findSignalMast("2E");
        tc->update(Occupancy::VACANT, Quality::GOOD, clockMs);
        requestRight(cp, "2", clockMs);
        cp.tick(clockMs);
        assert(mast->currentIndication() == Indication::STOP);      // released until driven

        ghost->update(true, clockMs);                                 // the ghost says clear
        cp.tick(clockMs);
        assert(mast->currentIndication() == Indication::CLEAR);

        clockMs += DEFAULT_STALENESS_TIMEOUT_MS + 1000;               // the ghost stops
        tc->update(Occupancy::VACANT, Quality::GOOD, clockMs);
        cp.tick(clockMs);
        assert(ghost->isStale());
        assert(mast->currentIndication() == Indication::STOP);

        ghost->touch(clockMs);                                        // the ghost is back, no change
        cp.tick(clockMs);
        assert(mast->currentIndication() == Indication::CLEAR);
    }
    printf("   PASS\n\n");

    // ---------------------------------------------------------
    printf("[TEST 3] proceed(), governs(), approach(), block() and reads() with no source (D1)\n");
    {
        uint32_t clockMs = 1000;
        InterlockingPlant cp("TERMS");
        cp.addTrackCircuit("1T");
        cp.addSignalControl("2");
        cp.addSignalMast("2E", MastType::ONE_HEAD);
        cp.route("R").governedBy("2", DirectionAuthority::RIGHT).displays("2E", Indication::CLEAR)
          .clears({ "1T" }).proceed("NEXT_PROCEED").entrance("1T");
        assert(findUnresolved(cp, "NEXT_PROCEED", UnresolvedKind::IMPLICIT) != nullptr);

        TrackCircuit* tc = cp.findTrackCircuit("1T");
        SignalMast* mast = cp.findSignalMast("2E");
        tc->update(Occupancy::VACANT, Quality::GOOD, clockMs);
        requestRight(cp, "2", clockMs);
        cp.tick(clockMs);
        assert(mast->currentIndication() == Indication::APPROACH);   // the cap holds while released
        cp.findLineInput("NEXT_PROCEED")->update(true, clockMs);
        cp.tick(clockMs);
        assert(mast->currentIndication() == Indication::CLEAR);

        // An automatic signal whose governed block, approach block and line
        // from ahead all lack a source, and a control point line likewise.
        cp.addSignalMast("830NAB", MastType::TWO_HEAD);
        AutomaticSignal& a = cp.addAutomaticSignal("832SA", MastType::ONE_HEAD);
        a.governs({ "832T" }).approach({ "830T1" }).reads("834SAH", "834SAD");
        ControlPointLine& l = cp.addControlPointLine("830NAB");
        l.governs({ "830T2" }).block({ "830T2", "832T" });

        assert(findUnresolved(cp, "832T",   UnresolvedKind::IMPLICIT) != nullptr);
        assert(findUnresolved(cp, "830T1",  UnresolvedKind::IMPLICIT) != nullptr);
        assert(findUnresolved(cp, "834SAH", UnresolvedKind::IMPLICIT) != nullptr);
        assert(findUnresolved(cp, "834SAD", UnresolvedKind::IMPLICIT) != nullptr);
        assert(findUnresolved(cp, "830T2",  UnresolvedKind::IMPLICIT) != nullptr);
        assert(cp.findLineInput("832T")->isImplicit());
        // The second reference to 832T found the implicit input: one declaration, one report.
        assert(cp.unresolvedTotal() == 6);

        cp.tick(clockMs);
        assert(a.mast()->currentIndication() == Indication::STOP);
        assert(!a.hOut()->asserted());
        assert(!a.dOut()->asserted());
        assert(!l.hOut()->asserted());

        // A ghost drives every implicit fact clear: the location reads Clear.
        for (uint8_t i = 0; i < cp.lineInputCount(); ++i) {
            if (cp.lineInput(i)->isImplicit()) cp.lineInput(i)->update(true, clockMs);
        }
        cp.tick(clockMs);
        assert(a.mast()->currentIndication() == Indication::CLEAR);
        assert(a.hOut()->asserted());
        assert(a.dOut()->asserted());
        assert(l.hOut()->asserted());
    }
    printf("   PASS\n\n");

    // ---------------------------------------------------------
    printf("[TEST 4] clears() naming a circuit the plant lacks: the route is at Stop (D2)\n");
    {
        uint32_t clockMs = 1000;
        InterlockingPlant cp("MISSING");
        cp.addTrackCircuit("1T");
        cp.addSignalControl("2");
        cp.addSignalMast("2E", MastType::ONE_HEAD);
        cp.route("R").governedBy("2", DirectionAuthority::RIGHT).displays("2E", Indication::CLEAR)
          .clears({ "1T", "2T" }).entrance("1T");

        const UnresolvedReference* u = findUnresolved(cp, "2T", UnresolvedKind::MISSING);
        assert(u != nullptr);
        assert(strcmp(u->call, "clears") == 0);
        assert(strcmp(u->owner, "R") == 0);
        assert(cp.engine().route(0).isIncomplete());
        assert(cp.lineInputCount() == 0);                             // nothing invented

        cp.findTrackCircuit("1T")->update(Occupancy::VACANT, Quality::GOOD, clockMs);
        requestRight(cp, "2", clockMs);
        cp.tick(clockMs);
        assert(cp.findSignalControl("2")->HSR());                     // the control is stored
        assert(cp.findSignalMast("2E")->currentIndication() == Indication::STOP);
        assert(cp.engine().evaluateRouteIndication(cp.engine().route(0)) == Indication::STOP);
        assert(cp.engine().evaluateMastIndication(cp.findSignalMast("2E")) == Indication::STOP);
    }
    printf("   PASS\n\n");

    // ---------------------------------------------------------
    printf("[TEST 5] Every appliance-typed binding reports missing and marks its owner\n");
    {
        InterlockingPlant cp("APPLIANCES");
        cp.addTrackCircuit("1T");
        cp.addSignalMast("2E", MastType::ONE_HEAD);
        cp.route("R1").governedBy("NOCTL", DirectionAuthority::RIGHT).displays("2E", Indication::CLEAR).clears({ "1T" });
        cp.route("R2").governedBy("NOCTL", DirectionAuthority::RIGHT).displays("NOMAST", Indication::CLEAR).clears({ "1T" });
        cp.route("R3").displays("2E", Indication::CLEAR).clears({ "1T" }).entrance("NOT").approaching("NOA");
        cp.route("R4").displays("2E", Indication::CLEAR).clears({ "1T" }).aligns({ { "NOSW", SwitchPosition::NORMAL } });
        cp.route("R5").displays("2E", Indication::CLEAR).clears({ "1T" }).engineReturn("NOS", "NOI");
        AutomaticSignal& a = cp.addAutomaticSignal("832SA", MastType::ONE_HEAD);
        a.governs({ "1T" }).opposing("NOAUTO");
        ControlPointLine& l = cp.addControlPointLine("NOHOME");       // no mast of that name
        l.leaving({ "NOLEAVE" }).home("NOHOME2");

        assert(findUnresolved(cp, "NOCTL",   UnresolvedKind::MISSING) != nullptr);
        assert(findUnresolved(cp, "NOMAST",  UnresolvedKind::MISSING) != nullptr);
        assert(findUnresolved(cp, "NOT",     UnresolvedKind::MISSING) != nullptr);
        assert(findUnresolved(cp, "NOA",     UnresolvedKind::MISSING) != nullptr);
        assert(findUnresolved(cp, "NOSW",    UnresolvedKind::MISSING) != nullptr);
        assert(findUnresolved(cp, "NOS",     UnresolvedKind::MISSING) != nullptr);
        assert(findUnresolved(cp, "NOI",     UnresolvedKind::MISSING) != nullptr);
        assert(findUnresolved(cp, "NOAUTO",  UnresolvedKind::MISSING) != nullptr);
        assert(findUnresolved(cp, "NOHOME",  UnresolvedKind::MISSING) != nullptr);
        assert(findUnresolved(cp, "NOLEAVE", UnresolvedKind::MISSING) != nullptr);
        assert(findUnresolved(cp, "NOHOME2", UnresolvedKind::MISSING) != nullptr);
        for (uint8_t i = 0; i < cp.engine().routeCount(); ++i) {
            if (cp.engine().route(i).isAutomatic()) continue;
            assert(cp.engine().route(i).isIncomplete());
        }
        assert(a.isIncomplete());
        assert(l.isIncomplete());
        assert(cp.lineInputCount() == 0);                             // no fact stands in for an appliance

        cp.findTrackCircuit("1T")->update(Occupancy::VACANT, Quality::GOOD, 1000);
        cp.tick(1000);
        assert(!a.hOut()->asserted());                                // incomplete: released
        assert(!a.dOut()->asserted());
        assert(a.mast()->currentIndication() == Indication::STOP);
        assert(!l.hOut()->asserted());
        assert(!l.dOut()->asserted());
    }
    printf("   PASS\n\n");

    // ---------------------------------------------------------
    printf("[TEST 6] Seventeen unknown fact names: the seventeenth binds to the released fact (D1, no slot)\n");
    {
        InterlockingPlant cp("OVERFLOW");
        cp.addTrackCircuit("1T");
        cp.addSignalMast("2E", MastType::ONE_HEAD);
        Route& r = cp.route("R");
        r.displays("2E", Indication::CLEAR).clears({ "1T" });
        char name[16];
        for (int i = 0; i < MAX_APPLIANCES + 1; ++i) {
            snprintf(name, sizeof(name), "F%d", i);
            r.homeFact(name);
        }
        assert(cp.lineInputCount() == MAX_APPLIANCES);
        assert(cp.unresolvedTotal() == MAX_APPLIANCES + 1);
        assert(cp.unresolvedCount() == MAX_UNRESOLVED);               // stored up to the limit
        assert(findUnresolved(cp, "F16", UnresolvedKind::NO_SLOT) == nullptr);   // the 17th is counted, not stored
        // The eighth home fact onward is beyond MAX_ROUTE_FACTS; what matters is
        // that the released fact is what an overflow resolves to.
        assert(cp.findLineInput("F16") == nullptr);
        assert(!releasedFact()->asserted());
        assert(strcmp(releasedFact()->name(), "(unsourced)") == 0);
    }
    printf("   PASS\n\n");

    // ---------------------------------------------------------
    printf("[TEST 7] A source added after an implicit fact of its name is a conflict (D1's limit)\n");
    {
        InterlockingPlant cp("CONFLICT");
        cp.addSignalMast("2E", MastType::ONE_HEAD);
        Route& r = cp.route("R");
        r.displays("2E", Indication::CLEAR).home({ "832T", "834SAH" });
        TrackCircuit* late = cp.addTrackCircuit("832T");               // too late
        cp.addAutomaticSignal("834SA", MastType::ONE_HEAD);           // produces 834SAH: too late
        assert(late != nullptr);
        assert(findUnresolved(cp, "832T",   UnresolvedKind::CONFLICT) != nullptr);
        assert(findUnresolved(cp, "834SAH", UnresolvedKind::CONFLICT) != nullptr);
        assert(static_cast<const Route&>(r).homeFact(0) == cp.findLineInput("832T"));   // still the implicit input
        assert(static_cast<const Route&>(r).homeFact(0) != late);
    }
    printf("   PASS\n\n");

    // ---------------------------------------------------------
    printf("[TEST 8] The serializer writes given line inputs only and re-derives implicit ones (D4)\n");
    {
        InterlockingPlant cp("ROUNDTRIP");
        cp.setDefaultStalenessTimeout(12000);
        cp.addTrackCircuit("1T");
        cp.addLineInput("GIVEN");                                     // takes the plant default
        cp.addLineInput("GIVEN2", 3000);
        cp.addSignalControl("2");
        cp.addSignalMast("2E", MastType::ONE_HEAD);
        cp.route("R").governedBy("2", DirectionAuthority::RIGHT).displays("2E", Indication::CLEAR)
          .clears({ "1T" }).home({ "GIVEN", "GIVEN2", "IMPLIED" }).entrance("1T");
        assert(cp.findLineInput("GIVEN")->stalenessTimeout() == 12000);
        assert(cp.findLineInput("IMPLIED")->isImplicit());
        assert(cp.findLineInput("IMPLIED")->stalenessTimeout() == 12000);

        static char json[8192];
        assert(cp.serialize(json, sizeof(json), true));
        assert(strstr(json, "\"defaultStalenessTimeoutMs\": 12000") != nullptr);
        assert(strstr(json, "\"GIVEN\"") != nullptr);
        assert(strstr(json, "\"name\": \"IMPLIED\"") == nullptr);          // not written as a line input
        assert(strstr(json, "\"home\": [\"GIVEN\", \"GIVEN2\", \"IMPLIED\"]") != nullptr);

        InterlockingPlant twin("TWIN");
        assert(twin.deserialize(json));
        assert(twin.defaultStalenessTimeout() == 12000);
        assert(twin.lineInputCount() == 3);
        assert(!twin.findLineInput("GIVEN")->isImplicit());
        assert(twin.findLineInput("GIVEN")->stalenessTimeout() == 12000);
        assert(twin.findLineInput("GIVEN2")->stalenessTimeout() == 3000);
        assert(twin.findLineInput("IMPLIED") != nullptr);
        assert(twin.findLineInput("IMPLIED")->isImplicit());
        assert(twin.findLineInput("IMPLIED")->stalenessTimeout() == 12000);
        assert(!twin.isComplete());
        assert(findUnresolved(twin, "IMPLIED", UnresolvedKind::IMPLICIT) != nullptr);
        assert(twin.engine().route(0).homeFact(2) == twin.findLineInput("IMPLIED"));

        // A second round trip is the same file.
        static char json2[8192];
        assert(twin.serialize(json2, sizeof(json2), true));
        assert(strcmp(json, json2) == 0);
    }
    printf("   PASS\n\n");

    // ---------------------------------------------------------
    printf("[TEST 9] reset() clears the report and the default timeout\n");
    {
        InterlockingPlant cp("RESET");
        cp.setDefaultStalenessTimeout(1);
        cp.addSignalMast("2E", MastType::ONE_HEAD);
        cp.route("R").displays("2E", Indication::CLEAR).home({ "X" });
        assert(!cp.isComplete());
        cp.reset();
        assert(cp.isComplete());
        assert(cp.unresolvedCount() == 0);
        assert(cp.lineInputCount() == 0);
        assert(cp.defaultStalenessTimeout() == DEFAULT_STALENESS_TIMEOUT_MS);
    }
    printf("   PASS\n\n");

    printf("====================================================\n");
    printf("   ALL UNRESOLVED REFERENCE TESTS PASSED            \n");
    printf("====================================================\n");
    return 0;
}
