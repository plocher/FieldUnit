#include <stdio.h>
#include <assert.h>

// Undefine ARDUINO so CP_Christopher.ino provides configurePlant() and executeCycle()
#undef ARDUINO

#include "../examples/CP_Christopher/CP_Christopher.ino"

void runChristopherSketchIntegrationTest() {
    printf("====================================================\n");
    printf("   CP CHRISTOPHER END-TO-END SKETCH TEST BENCH      \n");
    printf("   Driven entirely through Mock CodeLine Packets    \n");
    printf("====================================================\n\n");

    // 1. Initialize the Sketch
    configurePlant();
    MockCodeLine mockLine;
    uint32_t clockMs = 1000;

    // Resolve appliances by name from ControlPoint
    Switch* sw1  = cp.findSwitch("1");
    Switch* sw3  = cp.findSwitch("3");
    Switch* sw3B = cp.findSwitch("3B");
    Switch* sw5  = cp.findSwitch("5");

    TrackCircuit* tc1T1  = cp.findTrackCircuit("1T1");
    TrackCircuit* tc3T1  = cp.findTrackCircuit("3T1");
    TrackCircuit* tc3BT1 = cp.findTrackCircuit("3BT1");
    TrackCircuit* tc5T1  = cp.findTrackCircuit("5T1");
    TrackCircuit* tc1SA  = cp.findTrackCircuit("1SA");
    TrackCircuit* tc2SA  = cp.findTrackCircuit("2SA");
    TrackCircuit* tc1NA  = cp.findTrackCircuit("1NA");
    TrackCircuit* tc2NA  = cp.findTrackCircuit("2NA");
    TrackCircuit* tcIND  = cp.findTrackCircuit("IND");

    SignalMast* mast2N = cp.findSignalMast("2Nab");

    // Initially, all switches settle in Normal correspondence
    sw1->updateFeedback(SwitchPosition::NORMAL);
    sw3->updateFeedback(SwitchPosition::NORMAL);
    sw3B->updateFeedback(SwitchPosition::NORMAL);
    sw5->updateFeedback(SwitchPosition::NORMAL);

    // Initially, all track circuits are clear/vacant
    tc1T1->update(Occupancy::VACANT);
    tc3T1->update(Occupancy::VACANT);
    tc3BT1->update(Occupancy::VACANT);
    tc5T1->update(Occupancy::VACANT);
    tc1SA->update(Occupancy::VACANT);
    tc2SA->update(Occupancy::VACANT);
    tc1NA->update(Occupancy::VACANT);
    tc2NA->update(Occupancy::VACANT);
    tcIND->update(Occupancy::VACANT);

    // Initial cycle with no incoming packets: exports baseline indications
    executeCycle(mockLine, clockMs);
    assert(mockLine.hasOutboundPacket());
    const char* outText = mockLine.outboundText();
    // Verify all switches in Normal correspondence
    assert(strstr(outText, "1NWK") != nullptr && strstr(outText, "(1NWK)") == nullptr);
    assert(strstr(outText, "3NWK") != nullptr && strstr(outText, "(3NWK)") == nullptr);
    assert(strstr(outText, "3BNWK") != nullptr && strstr(outText, "(3BNWK)") == nullptr);
    assert(strstr(outText, "5NWK") != nullptr && strstr(outText, "(5NWK)") == nullptr);
    // Reverse indications must be dropped (parenthesized)
    assert(strstr(outText, "(1RWK)") != nullptr);
    assert(strstr(outText, "(3RWK)") != nullptr);
    // Signal 2 is at STOP -> 2SGK and 2NGK are dropped (parenthesized)
    assert(strstr(outText, "(2SGK)") != nullptr);
    assert(strstr(outText, "(2NGK)") != nullptr);
    printf("[CYCLE 1] Plant at rest: Baseline indications verified\n  -> Outbound: %s\n\n", outText);
    mockLine.clearOutbound();

    // -------------------------------------------------------------
    // SCENARIO 1: Dispatcher clears Northbound MT2 straight (SIG2 LEFT)
    // -------------------------------------------------------------
    printf("[SCENARIO 1] Dispatcher transmits Control Packet: SIG2 LEFT with switches Normal\n");
    mockLine.injectControlText("1NWS, (1RWS), 3NWS, (3RWS), 3BNWS, (3BRWS), 5NWS, (5RWS), (2SGS), 2NGS, (2HS), (MC1S)");

    executeCycle(mockLine, clockMs);

    assert(mockLine.hasOutboundPacket());
    outText = mockLine.outboundText();
    // Must show 2NGK lit (asserted / unparenthesized)
    assert(strstr(outText, "2NGK") != nullptr && strstr(outText, "(2NGK)") == nullptr);
    // Mast 2Nab top head H2NA must be GREEN
    assert(mast2N->head1() == Aspect::GREEN);
    assert(mast2N->head2() == Aspect::RED);
    printf("  -> PASS: Indication confirms 2NGK lit: %s\n", outText);
    printf("  -> PASS: Mast 2Nab physically displays Green over Red (Clear)\n\n");
    mockLine.clearOutbound();

    // -------------------------------------------------------------
    // SCENARIO 2: Train accepts signal, enters 3BT1 OS block
    // -------------------------------------------------------------
    printf("[SCENARIO 2] Train shunts crossover detector block 3BT1\n");
    tc3BT1->update(Occupancy::OCCUPIED);

    executeCycle(mockLine, clockMs);

    // Signal immediately knocks down to STOP!
    assert(mast2N->head1() == Aspect::RED);
    assert(mast2N->head2() == Aspect::RED);
    outText = mockLine.outboundText();
    // Indication reflects 3BT1 occupied (unparenthesized)
    assert(strstr(outText, "3BT1K") != nullptr && strstr(outText, "(3BT1K)") == nullptr);
    // Signal indication light 2NGK drops to dark (parenthesized)
    assert(strstr(outText, "(2NGK)") != nullptr);
    printf("  -> PASS: Signal knocked down to Red over Red\n");
    printf("  -> PASS: Indication confirms 3BT1K occupied and 2NGK dropped: %s\n\n", outText);
    mockLine.clearOutbound();

    // -------------------------------------------------------------
    // SCENARIO 3: Dispatcher attempts to throw crossover while train on 3BT1
    // -------------------------------------------------------------
    printf("[SCENARIO 3] Dispatcher attempts to throw Crossover SW3/3B while 3BT1 occupied\n");
    mockLine.injectControlText("(1NWS), (1RWS), (3NWS), 3RWS, (3BNWS), 3BRWS, (5NWS), (5RWS), (2SGS), (2NGS), (2HS), (MC1S)");

    executeCycle(mockLine, clockMs);

    // Switch points did not move; still in Normal position and detector-locked!
    assert(sw3->reportedPosition() == SwitchPosition::NORMAL);
    assert((sw3->activeLocks() & SwitchLock::DETECTOR_LOCKED) == SwitchLock::DETECTOR_LOCKED);
    outText = mockLine.outboundText();
    // Indications still show 3NWK and 3BNWK in Normal correspondence
    assert(strstr(outText, "3NWK") != nullptr && strstr(outText, "(3NWK)") == nullptr);
    assert(strstr(outText, "3BNWK") != nullptr && strstr(outText, "(3BNWK)") == nullptr);
    printf("  -> PASS: Unsafe crossover throw rejected; points held in Normal\n");
    printf("  -> PASS: Indications confirm switches remain in Normal correspondence: %s\n\n", outText);
    mockLine.clearOutbound();

    // -------------------------------------------------------------
    // SCENARIO 4: Train clears; Line crossover and clear diverging route
    // -------------------------------------------------------------
    printf("[SCENARIO 4] Train clears plant; Dispatcher throws Crossover and clears SIG2 LEFT\n");
    tc3BT1->update(Occupancy::VACANT);
    clockMs += 35000; // Let approach time lock expire
    executeCycle(mockLine, clockMs);
    mockLine.clearOutbound();

    // Command SW1 Normal, SW3/SW3B Reverse, SW5 Normal, SIG2 LEFT
    mockLine.injectControlText("1NWS, (1RWS), (3NWS), 3RWS, (3BNWS), 3BRWS, 5NWS, (5RWS), (2SGS), 2NGS, (2HS), (MC1S)");

    executeCycle(mockLine, clockMs);

    outText = mockLine.outboundText();
    // Switches started moving in unison -> both NWK and RWK dark (parenthesized)
    assert(strstr(outText, "(3NWK)") != nullptr);
    assert(strstr(outText, "(3RWK)") != nullptr);
    assert(strstr(outText, "(3BNWK)") != nullptr);
    assert(strstr(outText, "(3BRWK)") != nullptr);
    printf("  -> Points moving: Indications confirm crossover is out of correspondence\n");
    mockLine.clearOutbound();

    // Points complete travel
    sw3->updateFeedback(SwitchPosition::REVERSE);
    sw3B->updateFeedback(SwitchPosition::REVERSE);

    executeCycle(mockLine, clockMs);

    outText = mockLine.outboundText();
    // Indications now show 3RWK and 3BRWK (unparenthesized)
    assert(strstr(outText, "3RWK") != nullptr && strstr(outText, "(3RWK)") == nullptr);
    assert(strstr(outText, "3BRWK") != nullptr && strstr(outText, "(3BRWK)") == nullptr);
    // Mast 2Nab lower head H2NB displays GREEN (Red over Green)
    assert(mast2N->head1() == Aspect::RED);
    assert(mast2N->head2() == Aspect::GREEN);
    // Indication shows 2NGK lit
    assert(strstr(outText, "2NGK") != nullptr && strstr(outText, "(2NGK)") == nullptr);
    printf("  -> PASS: Crossover Reverse correspondence confirmed: %s\n", outText);
    printf("  -> PASS: Mast 2Nab displays DIVERGING_CLEAR (Red over Green on lower head)\n\n");

    printf("====================================================\n");
    printf("   CP CHRISTOPHER ALL INTEGRATION TESTS PASSED!     \n");
    printf("====================================================\n");
}

int main() {
    runChristopherSketchIntegrationTest();
    return 0;
}
