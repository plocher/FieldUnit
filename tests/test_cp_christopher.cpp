#include <stdio.h>
#include <assert.h>
#include "../src/FieldUnit.h"

using namespace FieldUnit;

void runCPChristopherTests() {
    printf("====================================================\n");
    printf("   CP CHRISTOPHER PROTOTYPE INTERLOCKING TEST       \n");
    printf("   SP Coast Line MP 81 (Double Track + Crossover)   \n");
    printf("====================================================\n\n");

    ControlPoint cp("CP_Christopher");

    // -------------------------------------------------------------
    // 1. Declare Track Circuits
    // -------------------------------------------------------------
    TrackCircuit* tc1T1  = cp.addTrackCircuit("1T1");  // SW1 OS
    TrackCircuit* tc3T1  = cp.addTrackCircuit("3T1");  // SW3 OS (MT1)
    TrackCircuit* tc3BT1 = cp.addTrackCircuit("3BT1"); // SW3B OS (MT2)
    TrackCircuit* tc5T1  = cp.addTrackCircuit("5T1");  // SW5 OS
    TrackCircuit* tc1SA  = cp.addTrackCircuit("1SA");  // MT1 Approach from North
    TrackCircuit* tc2SA  = cp.addTrackCircuit("2SA");  // MT2 Approach from North
    TrackCircuit* tc1NA  = cp.addTrackCircuit("1NA");  // MT1 Exit/Approach South
    TrackCircuit* tc2NA  = cp.addTrackCircuit("2NA");  // MT2 Exit/Approach South
    TrackCircuit* tcIND  = cp.addTrackCircuit("IND");  // Christopher Ranch industry spur

    // Initial state: all track circuits vacant
    tc1T1->update(Occupancy::VACANT);
    tc3T1->update(Occupancy::VACANT);
    tc3BT1->update(Occupancy::VACANT);
    tc5T1->update(Occupancy::VACANT);
    tc1SA->update(Occupancy::VACANT);
    tc2SA->update(Occupancy::VACANT);
    tc1NA->update(Occupancy::VACANT);
    tc2NA->update(Occupancy::VACANT);
    tcIND->update(Occupancy::VACANT);

    // -------------------------------------------------------------
    // 2. Declare Switches & Crossover
    // -------------------------------------------------------------
    Switch* sw1  = cp.addSwitch("SW1");
    Switch* sw3  = cp.addSwitch("SW3");
    Switch* sw3B = cp.addSwitch("SW3B");
    Switch* sw5  = cp.addSwitch("SW5");

    // SW3 and SW3B form the facing-point crossover; driven in unison
    sw3->pairCrossover(sw3B);

    // Bind detector locking to point track circuits
    cp.bindDetectorLock(sw1, tc1T1);
    cp.bindDetectorLock(sw3, tc3T1);
    cp.bindDetectorLock(sw3B, tc3BT1);
    cp.bindDetectorLock(sw5, tc5T1);

    // -------------------------------------------------------------
    // 3. Declare Signal Control & Masts
    // -------------------------------------------------------------
    SignalControl* sig2 = cp.addSignalControl("SIG2");

    // 2Nab: Governs Northbound movement on MT2 (2 heads: H2NA top, H2NB lower)
    SignalMast* mast2N = cp.addSignalMast("2Nab", MastType::TWO_HEAD);

    // 2Sab: Governs Southbound movement on MT1 (2 heads: H2SA top, H2SB lower)
    SignalMast* mast2S = cp.addSignalMast("2Sab", MastType::TWO_HEAD);

    // 2Nc: Reverse-running Northbound entrance on MT1
    SignalMast* mast2Nc = cp.addSignalMast("2Nc", MastType::DWARF);
    (void)mast2Nc; // Available for reverse moves

    // -------------------------------------------------------------
    // 4. Define Interlocking Control Table (Routes)
    // -------------------------------------------------------------

    // Route 1: Northbound Straight on MT2 (SW3B Normal) -> Top Head H2NA CLEAR
    cp.addRoute({
        .name              = "MT2-MT2-STRAIGHT",
        .authority          = sig2,
        .direction         = DirectionAuthority::LEFT, // Northbound
        .mast              = mast2N,
        .targetHeadIndex   = 0,                        // Top head (H2NA)
        .aspectCeiling     = Indication::CLEAR,
        .switchCount       = 1,
        .switches          = { {sw3B, SwitchPosition::NORMAL} },
        .blockCount        = 1,
        .blocks            = { tc3BT1 },
        .approachBlock     = tc2SA,                    // Block ahead on MT2
        .isEngineReturn    = false,
        .originBlock       = nullptr,
        .osBlock           = nullptr
    });

    // Route 2: Northbound Crossover MT2 -> MT1 (SW3/3B Reverse) -> Lower Head H2NB DIVERGING_CLEAR
    cp.addRoute({
        .name              = "MT2-MT1-CROSSOVER",
        .authority          = sig2,
        .direction         = DirectionAuthority::LEFT, // Northbound
        .mast              = mast2N,
        .targetHeadIndex   = 1,                        // Lower head (H2NB)
        .aspectCeiling     = Indication::DIVERGING_CLEAR,
        .switchCount       = 3,
        .switches          = { {sw3, SwitchPosition::REVERSE},
                               {sw3B, SwitchPosition::REVERSE},
                               {sw1, SwitchPosition::NORMAL} },
        .blockCount        = 3,
        .blocks            = { tc3BT1, tc3T1, tc1T1 },
        .approachBlock     = tc1SA,                    // Block ahead on MT1
        .isEngineReturn    = false,
        .originBlock       = nullptr,
        .osBlock           = nullptr
    });

    // Route 3: Southbound Straight on MT1 (SW1=N, SW3=N, SW5=N) -> Top Head H2SA CLEAR
    cp.addRoute({
        .name              = "MT1-MT1-STRAIGHT",
        .authority          = sig2,
        .direction         = DirectionAuthority::RIGHT, // Southbound
        .mast              = mast2S,
        .targetHeadIndex   = 0,                         // Top head (H2SA)
        .aspectCeiling     = Indication::CLEAR,
        .switchCount       = 3,
        .switches          = { {sw1, SwitchPosition::NORMAL},
                               {sw3, SwitchPosition::NORMAL},
                               {sw5, SwitchPosition::NORMAL} },
        .blockCount        = 3,
        .blocks            = { tc1T1, tc3T1, tc5T1 },
        .approachBlock     = tc1NA,                     // Block ahead on MT1
        .isEngineReturn    = false,
        .originBlock       = nullptr,
        .osBlock           = nullptr
    });

    // Route 4: Southbound Crossover MT1 -> MT2 (SW1=N, SW3/3B=R) -> Lower Head H2SB DIVERGING_CLEAR
    cp.addRoute({
        .name              = "MT1-MT2-CROSSOVER",
        .authority          = sig2,
        .direction         = DirectionAuthority::RIGHT, // Southbound
        .mast              = mast2S,
        .targetHeadIndex   = 1,                         // Lower head (H2SB)
        .aspectCeiling     = Indication::DIVERGING_CLEAR,
        .switchCount       = 3,
        .switches          = { {sw1, SwitchPosition::NORMAL},
                               {sw3, SwitchPosition::REVERSE},
                               {sw3B, SwitchPosition::REVERSE} },
        .blockCount        = 2,
        .blocks            = { tc1T1, tc3T1 },
        .approachBlock     = tc2NA,                     // Block ahead on MT2
        .isEngineReturn    = false,
        .originBlock       = nullptr,
        .osBlock           = nullptr
    });

    uint32_t clockMs = 1000;

    // -------------------------------------------------------------
    // TEST 1: Northbound Mainline Straight Move (MT2-MT2)
    // -------------------------------------------------------------
    printf("[TEST 1] Dispatcher sends complete control transaction: SIG2 LEFT with all switches Normal\n");
    ControlTransaction ctl1;
    ctl1.switchDemands[0] = SwitchDemand::NORMAL; // 1NW
    ctl1.switchDemands[1] = SwitchDemand::NORMAL; // 3NW
    ctl1.switchDemands[2] = SwitchDemand::NORMAL; // 3BNW
    ctl1.switchDemands[3] = SwitchDemand::NORMAL; // 5NW
    ctl1.signalDemands[0] = SignalDemand::LEFT;   // 2NG

    cp.applyControlTransaction(ctl1, clockMs);
    cp.tick(clockMs);

    // Mast 2Nab should show Green over Red (Clear on MT2)
    assert(mast2N->currentIndication() == Indication::CLEAR);
    assert(mast2N->head1() == Aspect::GREEN); // H2NA
    assert(mast2N->head2() == Aspect::RED);   // H2NB
    printf("  -> PASS: Mast 2Nab shows CLEAR (Green over Red on top head H2NA)\n\n");

    // -------------------------------------------------------------
    // TEST 2: Crossover Operation (SW3 pairs with SW3B)
    // -------------------------------------------------------------
    printf("[TEST 2] Line crossover: Command SW3 REVERSE -> SW3B must also throw\n");
    // Drop signal first via complete transaction
    ControlTransaction ctlStop;
    ctlStop.signalDemands[0] = SignalDemand::STOP; // 2H
    cp.applyControlTransaction(ctlStop, clockMs);
    clockMs += 35000; // Let time-lock expire
    cp.tick(clockMs);

    // Command SW3 Reverse in complete transaction
    ControlTransaction ctlXover;
    ctlXover.switchDemands[0] = SwitchDemand::NORMAL;  // SW1 stays Normal
    ctlXover.switchDemands[1] = SwitchDemand::REVERSE; // SW3 Reverse
    ctlXover.switchDemands[3] = SwitchDemand::NORMAL;  // SW5 stays Normal
    cp.applyControlTransaction(ctlXover, clockMs);

    // Verify SW3 and SW3B both started moving
    assert(sw3->reportedPosition() == SwitchPosition::MOVING);
    assert(sw3B->reportedPosition() == SwitchPosition::MOVING);
    printf("  -> Both SW3 and SW3B started travel in unison\n");

    // Simulate points settling into Reverse
    sw3->updateFeedback(SwitchPosition::REVERSE);
    sw3B->updateFeedback(SwitchPosition::REVERSE);
    assert(sw3->inCorrespondence());
    assert(sw3B->inCorrespondence());
    printf("  -> PASS: Crossover SW3/3B in full Reverse correspondence\n\n");

    // -------------------------------------------------------------
    // TEST 3: Diverging Route across Crossover (MT2 -> MT1)
    // -------------------------------------------------------------
    printf("[TEST 3] Clear SIG2 LEFT across Crossover (MT2 -> MT1)\n");
    ControlTransaction ctlCrossoverSig;
    ctlCrossoverSig.signalDemands[0] = SignalDemand::LEFT; // 2NG
    cp.applyControlTransaction(ctlCrossoverSig, clockMs);
    cp.tick(clockMs);

    // Diverging route targets LOWER head H2NB -> Red over Green!
    assert(mast2N->currentIndication() == Indication::DIVERGING_CLEAR);
    assert(mast2N->head1() == Aspect::RED);   // Top head H2NA is RED marker
    assert(mast2N->head2() == Aspect::GREEN); // Lower head H2NB is GREEN
    assert(sw3->activeLocks() == SwitchLock::ROUTE_LOCKED);
    assert(sw3B->activeLocks() == SwitchLock::ROUTE_LOCKED);
    assert(sw1->activeLocks() == SwitchLock::ROUTE_LOCKED);
    printf("  -> PASS: Mast 2Nab shows DIVERGING_CLEAR (Red over Green on lower head H2NB)\n");
    printf("  -> PASS: SW3, SW3B, and SW1 are all Route-Locked\n\n");

    // -------------------------------------------------------------
    // TEST 4: Opposing Move Prevention (Interlocking Mutex)
    // -------------------------------------------------------------
    printf("[TEST 4] Dispatcher attempts to clear opposing Southbound SIG2 RIGHT\n");
    ControlTransaction ctlOpposing;
    ctlOpposing.signalDemands[0] = SignalDemand::RIGHT; // 2SG (Opposing!)
    cp.applyControlTransaction(ctlOpposing, clockMs);
    cp.tick(clockMs);

    // Since Signal 2 direction flipped while cleared, approach locking tripped
    // and opposing Southbound signals stay strictly at STOP!
    assert(mast2S->currentIndication() == Indication::STOP);
    assert(mast2S->head1() == Aspect::RED);
    assert(mast2S->head2() == Aspect::RED);
    printf("  -> PASS: Opposing Southbound mast 2Sab held strictly at STOP (Red over Red)\n\n");

    // -------------------------------------------------------------
    // TEST 5: Dual-Track Fouling Protection
    // -------------------------------------------------------------
    printf("[TEST 5] Train occupies crossover track 3BT1 on MT2\n");
    tc3BT1->update(Occupancy::OCCUPIED);
    cp.tick(clockMs);

    assert(sw3->activeLocks() == SwitchLock::DETECTOR_LOCKED);
    assert(sw3B->activeLocks() == SwitchLock::DETECTOR_LOCKED);
    assert(!sw3->isMovable());
    assert(!sw3B->isMovable());

    // Dispatcher attempts to throw crossover while occupied -> Plant remains in existing position!
    ControlTransaction ctlUnsafeXover;
    ctlUnsafeXover.switchDemands[1] = SwitchDemand::NORMAL; // Attempt to throw back Normal
    cp.applyControlTransaction(ctlUnsafeXover, clockMs);
    cp.tick(clockMs);

    IndicationVector ind5;
    cp.exportIndicationVector(ind5);
    // Switch 3 was not thrown; still reported in Reverse!
    assert(ind5.switches[1].position == SwitchPosition::REVERSE);
    assert((ind5.switches[1].locks & SwitchLock::DETECTOR_LOCKED) == SwitchLock::DETECTOR_LOCKED);
    printf("  -> PASS: Indication confirms crossover did not move; points remain locked in Reverse\n\n");

    // -------------------------------------------------------------
    // TEST 6: CodeLine Wire Codec Bit-Level Pack/Unpack (AAR Wire Mapping)
    // -------------------------------------------------------------
    printf("[TEST 6] CodeLine Wire Codec: Unpack raw bytes and pack indications\n");
    // Christopher wire layout from CP_Christopher.xml:
    // Controls: 2 bytes
    // Byte 0: 1NW(b0), 1RW(b1), 3NW(b2), 3RW(b3), 3BNW(b4), 3BRW(b5), 5NW(b6), 5RW(b7)
    // Byte 1: 2SG(b0), 2NG(b1), 2H(b2), MC1(b5), MC2(b6)
    // Indications: 4 bytes
    // Byte 0: 1NWK(b0), 1RWK(b1), 3NWK(b2), 3RWK(b3), 3BNWK(b4), 3BRWK(b5), 5NWK(b6), 5RWK(b7)
    // Byte 1: 1T1(b0), 3T1(b1), 3BT1(b2), 5T1(b3), 1SA(b4), 2SA(b5), 1NA(b6), 2NA(b7)
    // Byte 2: 2SGK(b0), 2NGK(b1), 2TEK(b2), MC1(b4), MC2(b5)
    // Byte 3: IND(b0)
    CodeLineCodec codec(2, 4);
    codec.mapSwitchControl(0, 0, 0, 0, 1); // SW1: b0=1NW, b1=1RW
    codec.mapSwitchControl(1, 0, 2, 0, 3); // SW3: b2=3NW, b3=3RW
    codec.mapSwitchControl(2, 0, 4, 0, 5); // SW3B: b4=3BNW, b5=3BRW
    codec.mapSwitchControl(3, 0, 6, 0, 7); // SW5: b6=5NW, b7=5RW
    codec.mapSignalControl(0, 1, 0, 1, 2); // SIG2: b0=2SG, b1=2NG, b2=2H
    codec.mapMaintainerControl(0, 1, 5);   // MC1: b5

    codec.mapSwitchIndication(0, 0, 0, 0, 1); // 1NWK, 1RWK
    codec.mapSwitchIndication(1, 0, 2, 0, 3); // 3NWK, 3RWK
    codec.mapSwitchIndication(2, 0, 4, 0, 5); // 3BNWK, 3BRWK
    codec.mapSwitchIndication(3, 0, 6, 0, 7); // 5NWK, 5RWK
    codec.mapTrackIndication(0, 1, 0);        // 1T1
    codec.mapTrackIndication(2, 1, 2);        // 3BT1
    codec.mapSignalIndication(0, 2, 0, 1, 2); // 2SGK, 2NGK, 2TEK

    // Unpack raw wire packet:
    // Byte 0 = 0x55 (01010101 binary: 1NW=1, 3NW=1, 3BNW=1, 5NW=1 -> All Normal!)
    // Byte 1 = 0x02 (00000010 binary: 2NG=1 -> Northbound Left authority)
    uint8_t rawControlBytes[2] = { 0x55, 0x02 };
    ControlTransaction wireCtl;
    bool unpackedOk = codec.unpackControls(rawControlBytes, sizeof(rawControlBytes), wireCtl);
    assert(unpackedOk);
    assert(wireCtl.switchDemands[0] == SwitchDemand::NORMAL);
    assert(wireCtl.switchDemands[1] == SwitchDemand::NORMAL);
    assert(wireCtl.signalDemands[0] == SignalDemand::LEFT);

    // Clear track occupancy from Test 5 and advance clock past time-lock to clear detector locks
    tc3BT1->update(Occupancy::VACANT);
    clockMs += 35000; // Let approach time lock from Test 4 expire
    cp.tick(clockMs);

    cp.applyControlTransaction(wireCtl, clockMs);
    cp.tick(clockMs);

    // While switches are in motion, indication bits must be DARK (0)
    IndicationVector movingInd;
    cp.exportIndicationVector(movingInd);
    uint8_t movingIndBytes[4] = {0};
    codec.packIndications(movingInd, movingIndBytes, sizeof(movingIndBytes));
    assert((movingIndBytes[0] & 0x0C) == 0 && "Crossover NWK bits must be dark while moving");
    printf("  -> Switch points moving: indication bits are dark (out of correspondence)\n");

    // Points complete travel and lock in Normal position
    sw3->updateFeedback(SwitchPosition::NORMAL);
    sw3B->updateFeedback(SwitchPosition::NORMAL);
    sw1->updateFeedback(SwitchPosition::NORMAL);
    sw5->updateFeedback(SwitchPosition::NORMAL);
    cp.tick(clockMs);

    // Export settled indications and pack into raw bytes
    IndicationVector settledInd;
    cp.exportIndicationVector(settledInd);
    uint8_t rawIndicationBytes[4] = {0};
    bool packedOk = codec.packIndications(settledInd, rawIndicationBytes, sizeof(rawIndicationBytes));
    assert(packedOk);

    // Byte 0 must have all Normal correspondence bits set (0x55)
    assert(rawIndicationBytes[0] == 0x55);
    // Byte 2 must have 2NGK bit set (bit 1 = 0x02)
    assert((rawIndicationBytes[2] & 0x02) != 0);
    printf("  -> PASS: Raw wire byte decoding matches XML schema; Indication packet correctly formatted\n\n");

    printf("====================================================\n");
    printf("   CP CHRISTOPHER ALL PROTO TESTS PASSED!           \n");
    printf("====================================================\n");
}

int main() {
    runCPChristopherTests();
    return 0;
}
