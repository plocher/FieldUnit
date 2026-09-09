#include <stdio.h>
#include <assert.h>
#include "../src/FieldUnit.h"

using namespace FieldUnit;

void runTracerBulletTests() {
    printf("====================================================\n");
    printf("   FIELDUNIT V2 TRACER BULLET POC TEST SUITE        \n");
    printf("====================================================\n\n");

    // 1. Instantiate Control Point
    ControlPoint cp("CP_End_Of_Siding");

    // 2. Add Appliances
    TrackCircuit* tcOS   = cp.addTrackCircuit("1T_OS");       // Over turnout points
    TrackCircuit* tcAppr = cp.addTrackCircuit("1A_APPROACH"); // Mainline block ahead
    TrackCircuit* tcMain = cp.addTrackCircuit("1M_MAIN");     // Main track
    TrackCircuit* tcSide = cp.addTrackCircuit("1S_SIDING");   // Siding track

    Switch* sw1 = cp.addSwitch("SW1");
    cp.bindDetectorLock(sw1, tcOS); // 1T detector-locks SW1

    SignalControl* auth2 = cp.addSignalControl("SIG2");

    SignalMast* mast2R = cp.addSignalMast("2R", MastType::TWO_HEAD);
    SignalMast* mast2L = cp.addSignalMast("2L", MastType::DWARF);

    // Initial state: all track circuits vacant
    tcOS->update(Occupancy::VACANT);
    tcAppr->update(Occupancy::VACANT);
    tcMain->update(Occupancy::VACANT);
    tcSide->update(Occupancy::VACANT);

    // 3. Define Control Table
    // Route 1: Rightward into Main Track (Normal switch)
    cp.route("MAIN_NORMAL")
      .governedBy(auth2, DirectionAuthority::RIGHT)
      .displays(mast2R, 0, Indication::CLEAR)
      .aligns({ {sw1, SwitchPosition::NORMAL} })
      .clears({ tcOS })
      .approaching(tcAppr);

    // Route 2: Rightward into Siding (Reverse switch)
    cp.route("SIDING_REVERSE")
      .governedBy(auth2, DirectionAuthority::RIGHT)
      .displays(mast2R, 1, Indication::DIVERGING_APPROACH)
      .aligns({ {sw1, SwitchPosition::REVERSE} })
      .clears({ tcOS });

    // Route 3: Engine Return from dark track onto cars standing on 1A
    cp.route("ENGINE_RETURN")
      .engineReturn(tcAppr, tcOS)
      .displays(mast2L, 0, Indication::RESTRICTING)
      .aligns({ {sw1, SwitchPosition::REVERSE} });

    uint32_t clockMs = 1000;

    // -------------------------------------------------------------
    // TEST 1: Baseline Check & Route Alignment
    // -------------------------------------------------------------
    printf("[TEST 1] Dispatcher commands SIG2 RIGHT with SW1 NORMAL\n");
    ControlTransaction ctl1;
    ctl1.switchDemands[0] = SwitchDemand::NORMAL;
    ctl1.signalDemands[0] = SignalDemand::RIGHT; // No fleet

    cp.applyControlTransaction(ctl1, clockMs);
    cp.tick(clockMs);
    assert(mast2R->currentIndication() == Indication::CLEAR);
    assert(mast2R->head1() == Aspect::GREEN);
    assert(mast2R->head2() == Aspect::RED);
    assert(sw1->activeLocks() == SwitchLock::ROUTE_LOCKED);
    printf("  -> PASS: Mast 2R displays CLEAR (Green over Red); SW1 is Route-Locked\n\n");

    // -------------------------------------------------------------
    // TEST 2: Intermediate Block Check (Approach Downgrade)
    // -------------------------------------------------------------
    printf("[TEST 2] Downstream block 1A becomes OCCUPIED\n");
    tcAppr->update(Occupancy::OCCUPIED);
    cp.tick(clockMs);

    assert(mast2R->currentIndication() == Indication::APPROACH);
    assert(mast2R->head1() == Aspect::YELLOW);
    assert(mast2R->head2() == Aspect::RED);
    printf("  -> PASS: Mast 2R drops to APPROACH (Yellow over Red) due to occupied block ahead\n\n");

    // -------------------------------------------------------------
    // TEST 3: Train Enters (Knockdown)
    // -------------------------------------------------------------
    printf("[TEST 3] Train accepts signal and enters OS block (1T shunts)\n");
    tcOS->update(Occupancy::OCCUPIED);
    cp.tick(clockMs);

    assert(mast2R->currentIndication() == Indication::STOP);
    assert(mast2R->head1() == Aspect::RED);
    assert(mast2R->head2() == Aspect::RED);
    assert(auth2->activeDirection() == DirectionAuthority::STOP);
    printf("  -> PASS: Signal 2R immediately knocks down to STOP\n\n");

    // -------------------------------------------------------------
    // TEST 4: Detector Locking (Rejection of Unsafe Switch Demand)
    // -------------------------------------------------------------
    printf("[TEST 4] Dispatcher attempts to throw SW1 while OS track is occupied\n");
    ControlTransaction ctlUnsafe;
    ctlUnsafe.switchDemands[0] = SwitchDemand::REVERSE;

    cp.applyControlTransaction(ctlUnsafe, clockMs);
    cp.tick(clockMs);

    IndicationVector ind4;
    cp.exportIndicationVector(ind4);
    assert(ind4.switches[0].position == SwitchPosition::NORMAL && "Switch MUST NOT move under train");
    assert((ind4.switches[0].locks & SwitchLock::DETECTOR_LOCKED) == SwitchLock::DETECTOR_LOCKED);
    printf("  -> PASS: Indication confirms switch did not move; points remain locked in Normal\n\n");

    // -------------------------------------------------------------
    // TEST 5: Standard Stick (No Fleeting)
    // -------------------------------------------------------------
    printf("[TEST 5] Train completely clears OS block (Standard stick)\n");
    tcOS->update(Occupancy::VACANT);
    cp.tick(clockMs);

    // Because fleetMode was false, authority must remain dead at STOP
    assert(mast2R->currentIndication() == Indication::STOP);
    assert(auth2->activeDirection() == DirectionAuthority::STOP);
    printf("  -> PASS: Signal stays at STOP; stick broke and requires new code button press\n\n");

    // -------------------------------------------------------------
    // TEST 6: Fleeting Mode (Automatic Re-clear)
    // -------------------------------------------------------------
    printf("[TEST 6] Dispatcher commands SIG2 RIGHT with FLEETING enabled\n");
    ControlTransaction ctlFleet;
    ctlFleet.signalDemands[0] = SignalDemand::RIGHT;
    ctlFleet.fleetDemands[0] = true; // FLEET = TRUE

    cp.applyControlTransaction(ctlFleet, clockMs);
    tcAppr->update(Occupancy::VACANT); // Clear down the line
    cp.tick(clockMs);

    assert(mast2R->currentIndication() == Indication::CLEAR);
    printf("  -> Signal 2R is CLEAR for Train 1\n");

    printf("  -> Train 1 shunts 1T (Knockdown)\n");
    tcOS->update(Occupancy::OCCUPIED);
    cp.tick(clockMs);
    assert(mast2R->currentIndication() == Indication::STOP);

    printf("  -> Train 1 leaves 1T into next block\n");
    tcOS->update(Occupancy::VACANT);
    cp.tick(clockMs);

    // Because fleeting was on, signal automatically re-clears for Train 2!
    assert(mast2R->currentIndication() == Indication::CLEAR);
    assert(mast2R->head1() == Aspect::GREEN);
    printf("  -> PASS: Signal 2R automatically re-cleared for Train 2 without dispatcher intervention\n\n");

    // -------------------------------------------------------------
    // TEST 7: Engine Return (Dark Industry Track Return)
    // -------------------------------------------------------------
    printf("[TEST 7] Engine Return Scenario\n");
    // Cancel route on 2R
    ControlTransaction ctlStop;
    ctlStop.signalDemands[0] = SignalDemand::STOP;
    cp.applyControlTransaction(ctlStop, clockMs);
    clockMs += 40000; // Let time lock expire
    cp.tick(clockMs);

    // Engine diverges into siding/spur: SW1 thrown REVERSE
    ControlTransaction ctlRev;
    ctlRev.switchDemands[0] = SwitchDemand::REVERSE;
    cp.applyControlTransaction(ctlRev, clockMs);
    sw1->updateFeedback(SwitchPosition::REVERSE); // Points complete travel

    // Train cars left standing on main/approach block 1A
    tcAppr->update(Occupancy::OCCUPIED);
    // OS block is clear (engine has pulled inside the dark siding)
    tcOS->update(Occupancy::VACANT);

    cp.tick(clockMs);

    // Dwarf signal 2L should now automatically display RESTRICTING!
    assert(mast2L->currentIndication() == Indication::RESTRICTING);
    assert(mast2L->head1() == Aspect::LUNAR);
    printf("  -> PASS: Dwarf 2L automatically displays RESTRICTING to let engine return to cars\n");

    printf("  -> Another train pulls cars away from 1A (1A becomes VACANT)\n");
    tcAppr->update(Occupancy::VACANT);
    cp.tick(clockMs);

    assert(mast2L->currentIndication() == Indication::STOP);
    assert(mast2L->head1() == Aspect::RED);
    printf("  -> PASS: Dwarf 2L immediately drops to STOP fail-safe when cars are no longer present\n\n");

    // -------------------------------------------------------------
    // TEST 8: Injected Time & Non-Blocking Switch Travel Timeout
    // -------------------------------------------------------------
    printf("[TEST 8] Injected Time & Non-Blocking Switch Travel Timeout\n");
    // Dispatcher commands SW1 Normal
    ControlTransaction ctlNorm;
    ctlNorm.switchDemands[0] = SwitchDemand::NORMAL;
    cp.applyControlTransaction(ctlNorm, clockMs);
    assert(sw1->reportedPosition() == SwitchPosition::MOVING);

    // Advance simulated time by 2000 ms (less than 5000 ms timeout)
    clockMs += 2000;
    cp.tick(clockMs);
    assert(sw1->reportedPosition() == SwitchPosition::MOVING);
    printf("  -> At t+2000ms: Switch is still MOVING\n");

    // Advance simulated time past 5000 ms timeout without contacts making
    clockMs += 4000; // total 6000 ms
    cp.tick(clockMs);
    assert(sw1->reportedPosition() == SwitchPosition::OUT_OF_CORRESPONDENCE);
    assert(!sw1->inCorrespondence());
    printf("  -> At t+6000ms: Switch timed out to OUT_OF_CORRESPONDENCE\n");

    // Dispatcher attempts to clear signal over jammed switch
    ControlTransaction ctlClear;
    ctlClear.signalDemands[0] = SignalDemand::RIGHT;
    cp.applyControlTransaction(ctlClear, clockMs);
    cp.tick(clockMs);
    assert(mast2R->currentIndication() == Indication::STOP);
    printf("  -> PASS: Signal refused to clear over out-of-correspondence switch\n\n");

    printf("====================================================\n");
    printf("   ALL TRACER BULLET TESTS PASSED SUCCESSFULLY!    \n");
    printf("====================================================\n");
}

int main() {
    runTracerBulletTests();
    return 0;
}
