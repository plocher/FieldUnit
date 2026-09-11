#include <stdio.h>
#include <assert.h>
#include <string.h>

// Undefine ARDUINO so CP_Corporal.ino provides configurePlant() and executeCycle()
#undef ARDUINO

#include "../examples/CP_Corporal/CP_Corporal.ino"

void runCorporalSketchIntegrationTest() {
    printf("====================================================\n");
    printf("   CP CORPORAL END-TO-END SKETCH TEST BENCH         \n");
    printf("   Driven entirely through Mock CodeLine Packets    \n");
    printf("====================================================\n\n");

    // 1. Initialize the Sketch
    configurePlant();
    MockCodeLine mockLine;
    uint32_t clockMs = 1000;

    // Resolve appliances by name from ControlPoint
    Switch* sw1 = cp.findSwitch("1");
    Switch* sw3 = cp.findSwitch("3");
    Switch* sw5 = cp.findSwitch("5");

    TrackCircuit* tc1T1 = cp.findTrackCircuit("1T1");
    TrackCircuit* tc3T1 = cp.findTrackCircuit("3T1");
    TrackCircuit* tc5T1 = cp.findTrackCircuit("5T1");
    TrackCircuit* tc1NAT = cp.findTrackCircuit("1NAT");
    TrackCircuit* tc2NAT = cp.findTrackCircuit("2NAT");
    TrackCircuit* tc1SAT = cp.findTrackCircuit("1SAT");
    TrackCircuit* tc2SAT = cp.findTrackCircuit("2SAT");

    SignalMast* mast2NAB = cp.findSignalMast("2NAB");
    SignalMast* mast4SA  = cp.findSignalMast("4SA");

    // Initially, all switches settle in Normal correspondence
    sw1->updateFeedback(SwitchPosition::NORMAL);
    sw3->updateFeedback(SwitchPosition::NORMAL);
    sw5->updateFeedback(SwitchPosition::NORMAL);

    // Initially, all track circuits are clear/vacant
    tc1T1->update(Occupancy::VACANT);
    tc3T1->update(Occupancy::VACANT);
    tc5T1->update(Occupancy::VACANT);
    tc1NAT->update(Occupancy::VACANT);
    tc2NAT->update(Occupancy::VACANT);
    tc1SAT->update(Occupancy::VACANT);
    tc2SAT->update(Occupancy::VACANT);

    // Initial cycle with no incoming packets: exports baseline indications
    executeCycle(mockLine, clockMs);
    assert(mockLine.hasOutboundPacket());
    const char* outText = mockLine.outboundText();
    // Verify all switches in Normal correspondence
    assert(strstr(outText, "1NWK") != nullptr && strstr(outText, "(1NWK)") == nullptr);
    assert(strstr(outText, "3NWK") != nullptr && strstr(outText, "(3NWK)") == nullptr);
    assert(strstr(outText, "5NWK") != nullptr && strstr(outText, "(5NWK)") == nullptr);
    assert(strstr(outText, "(1RWK)") != nullptr);
    assert(strstr(outText, "(3RWK)") != nullptr);
    assert(strstr(outText, "(5RWK)") != nullptr);
    // All track circuits vacant
    assert(strstr(outText, "(1T1K)") != nullptr);
    assert(strstr(outText, "(3T1K)") != nullptr);
    assert(strstr(outText, "(5T1K)") != nullptr);
    // All signals at STOP
    assert(strstr(outText, "(2SGK)") != nullptr);
    assert(strstr(outText, "(2NGK)") != nullptr);
    assert(strstr(outText, "(4SGK)") != nullptr);
    assert(strstr(outText, "(4NGK)") != nullptr);
    printf("[CYCLE 1] Plant at rest: Baseline indications verified\n  -> Outbound: %s\n\n", outText);
    mockLine.clearOutbound();

    // -------------------------------------------------------------
    // SCENARIO 1: Dispatcher clears Northbound Single-Track to MT2 (SIG2 LEFT)
    // -------------------------------------------------------------
    printf("[SCENARIO 1] Dispatcher clears Route MT-NB: SIG2 LEFT with SW1=N, SW3=N\n");
    mockLine.injectControlText("1NWS, (1RWS), 3NWS, (3RWS), (5NWS), (5RWS), (2SGS), 2NGS, (2HS), (4SGS), (4NGS), (4HS), (MC1S)");

    executeCycle(mockLine, clockMs);

    assert(mockLine.hasOutboundPacket());
    outText = mockLine.outboundText();
    // Must show 2NGK lit (asserted / unparenthesized)
    assert(strstr(outText, "2NGK") != nullptr && strstr(outText, "(2NGK)") == nullptr);
    // Mast 2NAB top head must be GREEN (Clear)
    assert(mast2NAB->head1() == Aspect::GREEN);
    assert(mast2NAB->head2() == Aspect::RED);
    printf("  -> PASS: Indication confirms 2NGK lit: %s\n", outText);
    printf("  -> PASS: Mast 2NAB displays Green over Red (Clear)\n\n");
    mockLine.clearOutbound();

    // -------------------------------------------------------------
    // SCENARIO 2: Train enters OS block 3T1 -> Signal knocks down
    // -------------------------------------------------------------
    printf("[SCENARIO 2] Train shunts switch 3 detector block 3T1\n");
    tc3T1->update(Occupancy::OCCUPIED);

    executeCycle(mockLine, clockMs);

    // Signal immediately knocks down to STOP!
    assert(mast2NAB->head1() == Aspect::RED);
    assert(mast2NAB->head2() == Aspect::RED);
    outText = mockLine.outboundText();
    assert(strstr(outText, "3T1K") != nullptr && strstr(outText, "(3T1K)") == nullptr);
    assert(strstr(outText, "(2NGK)") != nullptr);
    printf("  -> PASS: Mast 2NAB knocked down to Red over Red (Stop)\n");
    printf("  -> PASS: Indication confirms 3T1K occupied and 2NGK dropped: %s\n\n", outText);
    mockLine.clearOutbound();

    // -------------------------------------------------------------
    // SCENARIO 3: Train clears; Dispatcher commands industry move with Safety Derail
    // -------------------------------------------------------------
    printf("[SCENARIO 3] Dispatcher lines into Industry Spur (SW1=R, Derail SW5 follows)\n");
    tc3T1->update(Occupancy::VACANT);
    clockMs += 35000; // Let approach time lock expire
    executeCycle(mockLine, clockMs);
    mockLine.clearOutbound();

    // Dispatcher commands SW1 REVERSE and clears SIG4 RIGHT (into industry)
    mockLine.injectControlText("(1NWS), 1RWS, (3NWS), (3RWS), (5NWS), (5RWS), (2SGS), (2NGS), (2HS), 4SGS, (4NGS), (4HS), (MC1S)");

    executeCycle(mockLine, clockMs);

    // The sketch safety derail interlock must automatically slave derail SW5 to REVERSE (open/clear)
    assert(sw1->commandedPosition() == SwitchPosition::REVERSE);
    assert(sw5->commandedPosition() == SwitchPosition::REVERSE);
    printf("  -> PASS: Safety Derail SW5 automatically slaved to Reverse (clear)\n");

    // Both switch and derail complete travel
    sw1->updateFeedback(SwitchPosition::REVERSE);
    sw5->updateFeedback(SwitchPosition::REVERSE);

    executeCycle(mockLine, clockMs);

    outText = mockLine.outboundText();
    // 1RWK and 5RWK are in correspondence
    assert(strstr(outText, "1RWK") != nullptr && strstr(outText, "(1RWK)") == nullptr);
    assert(strstr(outText, "5RWK") != nullptr && strstr(outText, "(5RWK)") == nullptr);
    // Signal 4 displays RESTRICTING into industry
    assert(strstr(outText, "4SGK") != nullptr && strstr(outText, "(4SGK)") == nullptr);
    assert(mast4SA->currentIndication() == Indication::RESTRICTING);
    printf("  -> PASS: Switch 1 and Derail 5 in Reverse correspondence: %s\n", outText);
    printf("  -> PASS: Mast 4SA displays Restricting into industry spur\n\n");

    printf("====================================================\n");
    printf("   CP CORPORAL ALL INTEGRATION TESTS PASSED!        \n");
    printf("====================================================\n");
}

int main() {
    runCorporalSketchIntegrationTest();
    return 0;
}
