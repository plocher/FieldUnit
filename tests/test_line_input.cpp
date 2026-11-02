#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../src/FieldUnit.h"

using namespace FieldUnit;

// FieldUnit ADR 0007 D2: a leaving route's home() terms (segment_clear, traffic_free)
// are ANDed into its HR condition; its proceed() term (next_proceed) caps the
// route at Approach while released. A line input is released until first set
// and when stale.

static void buildLeavingPlant(InterlockingPlant& cp) {
    cp.addTrackCircuit("1T");      // OS / entrance
    cp.addTrackCircuit("2T");      // exit circuit inside the plant
    cp.addLineInput("H_IN");       // segment_clear from the first intermediate
    cp.addLineInput("F_IN");       // traffic_free from the far end
    cp.addLineInput("D_IN");       // next_proceed from the first intermediate
    cp.addSignalControl("2");
    cp.addSignalMast("2E", MastType::ONE_HEAD);

    cp.route("LEAVING")
      .governedBy("2", DirectionAuthority::RIGHT)
      .displays("2E", Indication::CLEAR)
      .clears({ "1T", "2T" })
      .home({ "H_IN", "F_IN" })
      .proceed("D_IN")
      .entrance("1T");
}

static void requestRight(InterlockingPlant& cp, uint32_t nowMs) {
    ControlTransaction ctl;
    ctl.signalDemands[cp.findSignalControl("2")->index()] = SignalDemand::RIGHT;
    cp.applyControlTransaction(ctl, nowMs);
}

void runLineInputTests() {
    printf("====================================================\n");
    printf("   FIELDUNIT LINE INPUT TEST SUITE                  \n");
    printf("   Route home() and proceed() terms (ADR 0007 D2)   \n");
    printf("====================================================\n\n");

    uint32_t clockMs = 1000;
    InterlockingPlant cp("CP_LEAVING");
    buildLeavingPlant(cp);

    TrackCircuit* tc1 = cp.findTrackCircuit("1T");
    TrackCircuit* tc2 = cp.findTrackCircuit("2T");
    LineInput* hIn = cp.findLineInput("H_IN");
    LineInput* fIn = cp.findLineInput("F_IN");
    LineInput* dIn = cp.findLineInput("D_IN");
    SignalMast* mast = cp.findSignalMast("2E");
    const Route& r = cp.engine().route(0);

    assert(hIn && fIn && dIn && mast);
    assert(r.homeFactCount() == 2);
    assert(r.proceedFact() == dIn);
    assert(strcmp(r.homeFact(0)->name(), "H_IN") == 0);

    tc1->update(Occupancy::VACANT, Quality::GOOD, clockMs);
    tc2->update(Occupancy::VACANT, Quality::GOOD, clockMs);
    cp.tick(clockMs);

    // ---------------------------------------------------------
    printf("[TEST 1] Never-updated line inputs hold the leaving signal at STOP\n");
    requestRight(cp, clockMs);
    cp.tick(clockMs);
    assert(!hIn->asserted());
    assert(cp.findSignalControl("2")->HSR());          // the control is stored
    assert(mast->currentIndication() == Indication::STOP);
    printf("   PASS: control stored, HR term released, mast at STOP\n\n");

    // ---------------------------------------------------------
    printf("[TEST 2] H and F asserted, D released: APPROACH\n");
    hIn->update(true, clockMs);
    fIn->update(true, clockMs);
    dIn->update(false, clockMs);
    cp.tick(clockMs);
    assert(mast->currentIndication() == Indication::APPROACH);
    printf("   PASS: Clear capped at Approach by the proceed term\n\n");

    // ---------------------------------------------------------
    printf("[TEST 3] D asserted: CLEAR\n");
    dIn->update(true, clockMs);
    cp.tick(clockMs);
    assert(mast->currentIndication() == Indication::CLEAR);
    printf("   PASS\n\n");

    // ---------------------------------------------------------
    printf("[TEST 4] F released alone drops the signal to STOP (I1, traffic_free is an HR term)\n");
    fIn->update(false, clockMs);
    cp.tick(clockMs);
    assert(mast->currentIndication() == Indication::STOP);
    fIn->update(true, clockMs);
    cp.tick(clockMs);
    assert(mast->currentIndication() == Indication::CLEAR);
    printf("   PASS\n\n");

    // ---------------------------------------------------------
    printf("[TEST 5] A stale H drops the signal; a fresh update restores it\n");
    clockMs += 4000;
    tc1->update(Occupancy::VACANT, Quality::GOOD, clockMs);   // keep the local circuits fresh;
    tc2->update(Occupancy::VACANT, Quality::GOOD, clockMs);   // only the line inputs age here
    cp.tick(clockMs);
    assert(hIn->asserted());                            // within the 5000 ms timeout
    assert(mast->currentIndication() == Indication::CLEAR);
    clockMs += 2000;                                    // 6000 ms since the line inputs' last update
    tc1->update(Occupancy::VACANT, Quality::GOOD, clockMs);
    tc2->update(Occupancy::VACANT, Quality::GOOD, clockMs);
    cp.tick(clockMs);
    assert(hIn->isStale());
    assert(!hIn->asserted());
    assert(mast->currentIndication() == Indication::STOP);
    hIn->update(true, clockMs);
    fIn->update(true, clockMs);
    dIn->update(true, clockMs);
    cp.tick(clockMs);
    assert(mast->currentIndication() == Indication::CLEAR);
    printf("   PASS\n\n");

    // ---------------------------------------------------------
    printf("[TEST 6] Fleeting does not re-clear while a home term is released\n");
    {
        ControlTransaction fleet;
        fleet.signalDemands[cp.findSignalControl("2")->index()] = SignalDemand::RIGHT;
        fleet.fleetDemands[cp.findSignalControl("2")->index()] = true;
        cp.applyControlTransaction(fleet, clockMs);
        cp.tick(clockMs);
        assert(mast->currentIndication() == Indication::CLEAR);

        // Train enters: knockdown.
        tc1->update(Occupancy::OCCUPIED, Quality::GOOD, clockMs);
        cp.tick(clockMs);
        assert(cp.findSignalControl("2")->activeDirection() == DirectionAuthority::STOP);

        // Train leaves the plant, but the block ahead reports H released.
        tc1->update(Occupancy::VACANT, Quality::GOOD, clockMs);
        hIn->update(false, clockMs);
        cp.tick(clockMs);
        assert(cp.findSignalControl("2")->activeDirection() == DirectionAuthority::STOP);
        assert(mast->currentIndication() == Indication::STOP);

        // H returns: the fleet re-clears.
        hIn->update(true, clockMs);
        cp.tick(clockMs);
        assert(cp.findSignalControl("2")->activeDirection() == DirectionAuthority::RIGHT);
        assert(mast->currentIndication() == Indication::CLEAR);
    }
    printf("   PASS\n\n");

    // ---------------------------------------------------------
    printf("[TEST 7] A staleness timeout of 0 means an in-memory fact that never goes stale\n");
    {
        InterlockingPlant local("LOCAL");
        LineInput* mem = local.addLineInput("MEM", 0);
        mem->update(true, 10);
        mem->tick(10 + 1000000);
        assert(mem->asserted());
    }
    printf("   PASS\n\n");

    // ---------------------------------------------------------
    printf("[TEST 8] PlantSerializer carries lineInputs, home and proceed\n");
    {
        static char json[8192];
        assert(cp.serialize(json, sizeof(json), true));
        assert(strstr(json, "\"lineInputs\"") != nullptr);
        assert(strstr(json, "\"home\": [\"H_IN\", \"F_IN\"]") != nullptr);
        assert(strstr(json, "\"proceed\": \"D_IN\"") != nullptr);

        InterlockingPlant twin("TWIN");
        assert(twin.deserialize(json));
        assert(twin.lineInputCount() == 3);
        assert(twin.findLineInput("D_IN") != nullptr);
        const Route& tr = twin.engine().route(0);
        assert(tr.homeFactCount() == 2);
        assert(tr.homeFact(1) == twin.findLineInput("F_IN"));
        assert(tr.proceedFact() == twin.findLineInput("D_IN"));
    }
    printf("   PASS\n\n");

    printf("====================================================\n");
    printf("   ALL LINE INPUT TESTS PASSED                      \n");
    printf("====================================================\n");
}

int main() {
    runLineInputTests();
    return 0;
}
