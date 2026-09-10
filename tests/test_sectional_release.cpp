#include <stdio.h>
#include <assert.h>
#include "../src/FieldUnit.h"

using namespace FieldUnit;

void runSectionalReleaseTests() {
    printf("====================================================\n");
    printf("   FIELDUNIT SECTIONAL ROUTE RELEASE TEST SUITE     \n");
    printf("   Sequential Block Progression & Progressive Locks \n");
    printf("====================================================\n\n");

    uint32_t clockMs = 1000;

    // -------------------------------------------------------------
    // Setup Plant: Two Switches (SW1, SW3), Three Blocks (1T, 2T, 3T)
    // -------------------------------------------------------------
    ControlPoint cp("CP_COMPLEX");

    // Track circuits:
    // 1T: Detector circuit over points of Switch 1 (Entrance)
    // 2T: Detector circuit over points of Switch 3
    // 3T: Exit circuit past Switch 3
    TrackCircuit* tc1 = cp.addTrackCircuit("1T");
    TrackCircuit* tc2 = cp.addTrackCircuit("2T");
    TrackCircuit* tc3 = cp.addTrackCircuit("3T");

    // Switches:
    Switch* sw1 = cp.addSwitch("1");
    Switch* sw3 = cp.addSwitch("3");

    // Bind detector locks:
    // 1T detector-locks Switch 1; 2T detector-locks Switch 3
    cp.bindDetectorLock("1", "1T");
    cp.bindDetectorLock("3", "2T");

    // Signals & Masts:
    SignalControl* sig2 = cp.addSignalControl("2");
    SignalMast* mast2E  = cp.addSignalMast("2E", MastType::ONE_HEAD);

    // Route: Eastbound Main
    // Traverses Switch 1 Normal (in 1T), then Switch 3 Normal (in 2T), exits onto 3T
    cp.route("EB_MAIN")
      .governedBy("2", DirectionAuthority::RIGHT)
      .displays("2E", Indication::CLEAR)
      .aligns({ {"1", SwitchPosition::NORMAL}, {"3", SwitchPosition::NORMAL} })
      .clears({ "1T", "2T", "3T" })
      .entrance("1T");

    // Initialize all tracks as VACANT
    tc1->update(Occupancy::VACANT, Quality::GOOD, clockMs);
    tc2->update(Occupancy::VACANT, Quality::GOOD, clockMs);
    tc3->update(Occupancy::VACANT, Quality::GOOD, clockMs);

    // Initial tick to stabilize plant
    cp.tick(clockMs);

    // -------------------------------------------------------------
    // TEST 1: Initial Idle State
    // -------------------------------------------------------------
    printf("[TEST 1] Initial State: Plant idle, switches free to throw\n");
    assert(mast2E->head1() == Aspect::RED);
    assert(sw1->isMovable());
    assert(sw3->isMovable());
    assert(sw1->activeLocks() == SwitchLock::UNLOCKED);
    assert(sw3->activeLocks() == SwitchLock::UNLOCKED);
    printf("  -> Both switches unlocked and free to move\n\n");

    // -------------------------------------------------------------
    // TEST 2: Route Cleared (Full Route Locking)
    // -------------------------------------------------------------
    printf("[TEST 2] Route Cleared: Signal 2 displays CLEAR, all switches route-locked\n");
    ControlTransaction ctl;
    ctl.signalDemands[sig2->index()] = SignalDemand::RIGHT;
    cp.applyControlTransaction(ctl, clockMs);

    cp.tick(clockMs);
    assert(mast2E->head1() == Aspect::GREEN);
    assert(sig2->activeDirection() == DirectionAuthority::RIGHT);

    // Both Switch 1 and Switch 3 must be ROUTE_LOCKED
    assert(sw1->isRouteLocked());
    assert(sw3->isRouteLocked());
    assert(!sw1->isMovable());
    assert(!sw3->isMovable());

    // Verify throw commands are rejected
    assert(sw1->throwSwitch(SwitchPosition::REVERSE, clockMs) == false);
    assert(sw3->throwSwitch(SwitchPosition::REVERSE, clockMs) == false);
    printf("  -> PASS: Signal displays CLEAR; SW1 and SW3 locked by route\n\n");

    // -------------------------------------------------------------
    // TEST 3: Train Enters Plant (Knockdown into 1T)
    // -------------------------------------------------------------
    printf("[TEST 3] Train Enters 1T: Signal knocks down; trailing SW3 REMAINS route-locked\n");
    clockMs += 100;
    tc1->update(Occupancy::OCCUPIED, Quality::GOOD, clockMs);
    cp.tick(clockMs);

    // Signal must knock down to STOP immediately
    assert(mast2E->head1() == Aspect::RED);
    assert(sig2->activeDirection() == DirectionAuthority::STOP);

    // SW1 is in 1T: has DETECTOR_LOCKED and ROUTE_LOCKED
    assert(sw1->isDetectorLocked());
    assert(sw1->isRouteLocked());

    // CRITICAL SAFETY CHECK:
    // SW3 is in 2T (which is STILL VACANT), but MUST STILL BE ROUTE_LOCKED!
    assert(!tc2->isClear() == false); // 2T is vacant
    assert(sw3->isRouteLocked() == true);
    assert(sw3->throwSwitch(SwitchPosition::REVERSE, clockMs) == false);
    printf("  -> PASS: Signal knocked down to STOP; trailing SW3 in vacant 2T remains ROUTE_LOCKED\n\n");

    // -------------------------------------------------------------
    // TEST 4: Train Spans Both Blocks (1T and 2T occupied)
    // -------------------------------------------------------------
    printf("[TEST 4] Train Spans 1T and 2T: Both switches occupied and locked\n");
    clockMs += 100;
    tc2->update(Occupancy::OCCUPIED, Quality::GOOD, clockMs);
    cp.tick(clockMs);

    assert(sw1->isRouteLocked() && sw1->isDetectorLocked());
    assert(sw3->isRouteLocked() && sw3->isDetectorLocked());
    printf("  -> PASS: Both SW1 and SW3 occupied and double-locked\n\n");

    // -------------------------------------------------------------
    // TEST 5: Sectional Route Release: Rear of Train Clears 1T
    // -------------------------------------------------------------
    printf("[TEST 5] Sectional Route Release: Rear clears 1T; SW1 releases while SW3 stays locked\n");
    clockMs += 100;
    tc1->update(Occupancy::VACANT, Quality::GOOD, clockMs);
    cp.tick(clockMs);

    // SW1 has been vacated: Sectional Release MUST free Switch 1!
    assert(sw1->isRouteLocked() == false);
    assert(sw1->isDetectorLocked() == false);
    assert(sw1->isMovable() == true);

    // SW3 is still occupied in 2T: MUST REMAIN LOCKED!
    assert(sw3->isRouteLocked() == true);
    assert(sw3->isDetectorLocked() == true);
    assert(sw3->isMovable() == false);

    // Demonstrate that SW1 can now be realigned for another move!
    assert(sw1->throwSwitch(SwitchPosition::REVERSE, clockMs) == true);
    assert(sw1->commandedPosition() == SwitchPosition::REVERSE);
    assert(sw3->throwSwitch(SwitchPosition::REVERSE, clockMs) == false);
    printf("  -> PASS: SW1 released and thrown to REVERSE while SW3 remains locked for train\n\n");

    // -------------------------------------------------------------
    // TEST 6: Train Clears 2T into 3T
    // -------------------------------------------------------------
    printf("[TEST 6] Train Clears 2T into 3T: SW3 releases progressively\n");
    clockMs += 100;
    tc3->update(Occupancy::OCCUPIED, Quality::GOOD, clockMs);
    tc2->update(Occupancy::VACANT, Quality::GOOD, clockMs);
    cp.tick(clockMs);

    // SW3 has now been vacated: Sectional Release frees Switch 3!
    assert(sw3->isRouteLocked() == false);
    assert(sw3->isDetectorLocked() == false);
    assert(sw3->isMovable() == true);
    assert(sw3->throwSwitch(SwitchPosition::REVERSE, clockMs) == true);
    printf("  -> PASS: SW3 released and thrown to REVERSE as train enters exit block 3T\n\n");

    // -------------------------------------------------------------
    // TEST 7: Train Vacates Exit Block 3T (Route Resets to IDLE)
    // -------------------------------------------------------------
    printf("[TEST 7] Train Vacates 3T: Route traversal completes and resets to IDLE\n");
    clockMs += 100;
    tc3->update(Occupancy::VACANT, Quality::GOOD, clockMs);
    cp.tick(clockMs);

    // Realign switches back to Normal for subsequent tests
    sw1->throwSwitch(SwitchPosition::NORMAL, clockMs);
    sw1->updateFeedback(SwitchPosition::NORMAL);
    sw3->throwSwitch(SwitchPosition::NORMAL, clockMs);
    sw3->updateFeedback(SwitchPosition::NORMAL);
    cp.tick(clockMs);

    assert(sw1->inCorrespondence() && sw1->reportedPosition() == SwitchPosition::NORMAL);
    assert(sw3->inCorrespondence() && sw3->reportedPosition() == SwitchPosition::NORMAL);
    assert(sw1->isMovable() && sw3->isMovable());
    printf("  -> PASS: Entire route clear and idle; plant ready for next movement\n\n");

    // -------------------------------------------------------------
    // TEST 8: Fail-Safe Fallback: Detector Locking
    // -------------------------------------------------------------
    printf("[TEST 8] Fail-Safe Fallback: Detector lock blocks switch even when route is idle\n");
    tc1->update(Occupancy::OCCUPIED, Quality::GOOD, clockMs);
    cp.tick(clockMs);
    assert(sw1->isDetectorLocked());
    assert(sw1->isMovable() == false);
    assert(sw1->throwSwitch(SwitchPosition::REVERSE, clockMs) == false);

    tc1->update(Occupancy::VACANT, Quality::GOOD, clockMs);
    cp.tick(clockMs);
    assert(sw1->isMovable() == true);
    printf("  -> PASS: Hardware detector locking independently secures switch points\n\n");

    // -------------------------------------------------------------
    // TEST 9: Explicit Releasing Block in aligns()
    // -------------------------------------------------------------
    printf("[TEST 9] Explicit Releasing Block in NamedSwitchRequirement\n");
    Route testRoute;
    testRoute.setParent(&cp);
    testRoute.aligns({
        {"1", SwitchPosition::NORMAL, "1T"},
        {"3", SwitchPosition::NORMAL, "2T"}
    });
    assert(testRoute.switchCount() == 2);
    assert(testRoute.releasingBlock(0) == tc1);
    assert(testRoute.releasingBlock(1) == tc2);
    printf("  -> PASS: Explicit releasing block successfully bound in NamedSwitchRequirement\n\n");

    printf("====================================================\n");
    printf("   ALL SECTIONAL ROUTE RELEASE TESTS PASSED (100%%)  \n");
    printf("====================================================\n");
}

int main() {
    runSectionalReleaseTests();
    return 0;
}
