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

    // Initially, all switches settle in Normal correspondence
    sw1->updateFeedback(SwitchPosition::NORMAL);
    sw3->updateFeedback(SwitchPosition::NORMAL);
    sw3B->updateFeedback(SwitchPosition::NORMAL);
    sw5->updateFeedback(SwitchPosition::NORMAL);

    // Initial cycle with no incoming packets: exports baseline indications
    executeCycle(mockLine, clockMs);
    assert(mockLine.hasOutboundPacket());
    assert(mockLine.outboundLength() == 4);
    // Byte 0: All Normal correspondence bits (1NWK=b0, 3NWK=b2, 3BNWK=b4, 5NWK=b6) -> 0x55!
    assert(mockLine.outboundPacket()[0] == 0x55);
    // Byte 2: Signal 2 is at STOP -> SGK and NGK are 0
    assert((mockLine.outboundPacket()[2] & 0x03) == 0);
    printf("[CYCLE 1] Plant at rest: Baseline indications verified (Byte 0 = 0x55)\n\n");
    mockLine.clearOutbound();

    // -------------------------------------------------------------
    // SCENARIO 1: Dispatcher clears Northbound MT2 straight (SIG2 LEFT)
    // -------------------------------------------------------------
    printf("[SCENARIO 1] Dispatcher transmits Control Packet: SIG2 LEFT with switches Normal\n");
    // Byte 0 = 0x55 (all switches Normal)
    // Byte 1 = 0x02 (2NG = 1 -> Left/Northbound)
    uint8_t packet1[2] = { 0x55, 0x02 };
    mockLine.injectControlPacket(packet1, sizeof(packet1));

    executeCycle(mockLine, clockMs);

    assert(mockLine.hasOutboundPacket());
    // Byte 2 must show 2NGK = 1 (bit 1 active)
    assert((mockLine.outboundPacket()[2] & 0x02) != 0);
    // Mast 2Nab top head H2NA must be GREEN
    assert(mast2N->head1() == Aspect::GREEN);
    assert(mast2N->head2() == Aspect::RED);
    printf("  -> PASS: Indication packet confirms 2NGK lit (Byte 2 = 0x02)\n");
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
    // Indication packet reflects 3BT1 occupied (Byte 1, bit 2 = 0x04)
    assert((mockLine.outboundPacket()[1] & 0x04) != 0);
    // Signal indication light 2NGK drops to dark
    assert((mockLine.outboundPacket()[2] & 0x02) == 0);
    printf("  -> PASS: Signal knocked down to Red over Red\n");
    printf("  -> PASS: Indication packet reports 3BT1 occupied (Byte 1 bit 2)\n\n");
    mockLine.clearOutbound();

    // -------------------------------------------------------------
    // SCENARIO 3: Dispatcher attempts to throw crossover while train on 3BT1
    // -------------------------------------------------------------
    printf("[SCENARIO 3] Dispatcher attempts to throw Crossover SW3/3B while 3BT1 occupied\n");
    // Byte 0 = 0x65 (demands SW3 Reverse: bit 2=0, bit 3=1)
    uint8_t packetUnsafe[2] = { 0x69, 0x00 };
    mockLine.injectControlPacket(packetUnsafe, clockMs);

    executeCycle(mockLine, clockMs);

    // Switch points did not move; still in Normal position and detector-locked!
    assert(sw3->reportedPosition() == SwitchPosition::NORMAL);
    assert((sw3->activeLocks() & SwitchLock::DETECTOR_LOCKED) == SwitchLock::DETECTOR_LOCKED);
    // Indication Byte 0 still shows 1NWK and 3NWK (Normal correspondence)
    assert((mockLine.outboundPacket()[0] & 0x05) == 0x05);
    printf("  -> PASS: Unsafe crossover throw rejected; points held in Normal\n");
    printf("  -> PASS: Indication Byte 0 confirms switches remain in Normal correspondence\n\n");
    mockLine.clearOutbound();

    // -------------------------------------------------------------
    // SCENARIO 4: Train clears; Line crossover and clear diverging route
    // -------------------------------------------------------------
    printf("[SCENARIO 4] Train clears plant; Dispatcher throws Crossover and clears SIG2 LEFT\n");
    tc3BT1->update(Occupancy::VACANT);
    clockMs += 35000; // Let approach time lock expire
    executeCycle(mockLine, clockMs);
    mockLine.clearOutbound();

    // Command SW1 Normal, SW3/SW3B Reverse, SW5 Normal
    // Byte 0: b0(1NW)=1, b1=0, b2=0, b3(3RW)=1, b4=0, b5(3BRW)=1, b6(5NW)=1, b7=0 -> 0x69
    // SIG2 LEFT (Byte 1: 0x02)
    uint8_t packetDiverge[2] = { 0x69, 0x02 };
    mockLine.injectControlPacket(packetDiverge, sizeof(packetDiverge));

    executeCycle(mockLine, clockMs);

    // Switches started moving in unison -> indications dark
    assert((mockLine.outboundPacket()[0] & 0x3C) == 0); // SW3/3B NWK and RWK are dark
    printf("  -> Points moving: Indication Byte 0 confirms crossover is out of correspondence\n");
    mockLine.clearOutbound();

    // Points complete travel
    sw3->updateFeedback(SwitchPosition::REVERSE);
    sw3B->updateFeedback(SwitchPosition::REVERSE);

    executeCycle(mockLine, clockMs);

    // Indication Byte 0 now shows 3RWK (bit 3) and 3BRWK (bit 5) -> (0x08 | 0x20 = 0x28)
    assert((mockLine.outboundPacket()[0] & 0x28) == 0x28);
    // Mast 2Nab lower head H2NB displays GREEN (Red over Green)
    assert(mast2N->head1() == Aspect::RED);
    assert(mast2N->head2() == Aspect::GREEN);
    // Indication Byte 2 shows 2NGK lit
    assert((mockLine.outboundPacket()[2] & 0x02) != 0);
    printf("  -> PASS: Crossover Reverse correspondence confirmed on Byte 0\n");
    printf("  -> PASS: Mast 2Nab displays DIVERGING_CLEAR (Red over Green on lower head)\n\n");

    printf("====================================================\n");
    printf("   CP CHRISTOPHER ALL INTEGRATION TESTS PASSED!     \n");
    printf("====================================================\n");
}

int main() {
    runChristopherSketchIntegrationTest();
    return 0;
}
