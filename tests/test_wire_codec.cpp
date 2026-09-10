#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "WireCodec.h"
#include "ControlPoint.h"

using namespace FieldUnit;

void testAarTextCodecBasicOrderIndependent() {
    printf("[TEST] AarTextCodec: Order-independent decoding and strict declaration-order encoding\n");

    ControlPoint cp("CP_Test");
    Switch* sw1 = cp.addSwitch("1");
    Switch* sw3 = cp.addSwitch("3");
    SignalControl* sig2 = cp.addSignalControl("2");
    TrackCircuit* tc1T = cp.addTrackCircuit("1T");

    AarTextCodec codec;
    codec.decodeControls({
        decodeSwitch(sw1),   // 1NWS, 1RWS
        decodeSwitch(sw3),   // 3NWS, 3RWS
        decodeSignal(sig2),  // 2SGS, 2NGS, 2HS
        decodeMaintainer(0)  // MC1S
    });

    codec.encodeIndications({
        encodeSwitch(sw1),   // 1NWK, 1RWK
        encodeSwitch(sw3),   // 3NWK, 3RWK
        encodeTrack(tc1T),   // 1TK
        encodeSignal(sig2),  // 2SGK, 2NGK, 2TEK
        encodeMaintainer(0)  // MC1K
    });

    // 1. Inbound message in completely shuffled order with mixed whitespace and casing
    const char* shuffledInput = "  mc1s , (3nws), 3rws , 2NGS , 1nws, (1rws) ";
    ControlTransaction ctl;
    bool ok = codec.decodeControls(shuffledInput, ctl);
    assert(ok);
    assert(ctl.vitalValid == true);
    assert(ctl.switchDemands[0] == SwitchDemand::NORMAL);
    assert(ctl.switchDemands[1] == SwitchDemand::REVERSE);
    assert(ctl.signalDemands[0] == SignalDemand::LEFT);
    assert(ctl.maintainerCall[0] == true);
    printf("  -> PASS: Shuffled, case-insensitive, whitespace-tolerant tokens correctly decoded\n");

    // 2. Outbound indication formatting in exact declaration order
    sw1->updateFeedback(SwitchPosition::NORMAL);
    sw3->throwSwitch(SwitchPosition::REVERSE);
    sw3->updateFeedback(SwitchPosition::REVERSE);
    tc1T->update(Occupancy::OCCUPIED);
    IndicationVector ind;
    cp.exportIndicationVector(ind);
    ind.signals[0].activeAuthority = DirectionAuthority::LEFT;
    ind.maintainerCall[0] = true;

    char outBuf[256];
    size_t outLen = 0;
    bool encOk = codec.encodeIndications(ind, outBuf, sizeof(outBuf), outLen);
    assert(encOk);

    // Exact expected format:
    // 1NWK, (1RWK), (3NWK), 3RWK, 1TK, (2SGK), 2NGK, (2TEK), MC1K
    const char* expected = "1NWK, (1RWK), (3NWK), 3RWK, 1TK, (2SGK), 2NGK, (2TEK), MC1K";
    assert(strcmp(outBuf, expected) == 0);
    printf("  -> PASS: Indications formatted in exact declaration order: %s\n\n", outBuf);
}

void testAarTextCodecMandatorySuffixes() {
    printf("[TEST] AarTextCodec: Mandatory 'S' control suffix enforcement\n");

    ControlPoint cp("CP_Test");
    Switch* sw1 = cp.addSwitch("1");
    SignalControl* sig2 = cp.addSignalControl("2");

    AarTextCodec codec;
    codec.decodeControls({
        decodeSwitch(sw1),
        decodeSignal(sig2)
    });

    // Message missing 'S' suffix or using indication suffix 'K' on control line
    ControlTransaction ctl;
    codec.decodeControls("1NW, 2NGK", ctl);

    // Neither 1NW nor 2NGK end in 'S' -> must be rejected as unknown symbols
    assert(codec.unknownSymbolCount() == 2);
    assert(ctl.switchDemands[0] == SwitchDemand::NO_CHANGE);
    assert(ctl.signalDemands[0] == SignalDemand::NO_CHANGE);
    assert(ctl.vitalValid == true); // No vital conflict, just ignored non-control tokens
    printf("  -> PASS: Tokens lacking 'S' suffix rejected; unknownSymbolCount = %u\n\n",
           codec.unknownSymbolCount());
}

void testAarTextCodecVitalConflictIsolation() {
    printf("[TEST] AarTextCodec: Vital conflict isolation ('don't poke a sleeping bear')\n");

    ControlPoint cp("CP_Test");
    Switch* sw1 = cp.addSwitch("1");
    SignalControl* sig2 = cp.addSignalControl("2");
    TrackCircuit* tc1T = cp.addTrackCircuit("1T");

    // Baseline: sw1 is Normal
    sw1->updateFeedback(SwitchPosition::NORMAL);

    AarTextCodec codec;
    codec.decodeControls({
        decodeSwitch(sw1),
        decodeSignal(sig2),
        decodeMaintainer(0)
    });

    // Inbound packet has conflicting switch demand: 1NWS AND 1RWS both asserted!
    // But maintainer call MC1S is also present and valid.
    const char* corruptInput = "1NWS, 1RWS, MC1S";
    ControlTransaction ctl;
    codec.decodeControls(corruptInput, ctl);

    // 1. Transaction must be flagged invalid for vital safety
    assert(ctl.vitalValid == false);
    assert(codec.vitalConflictCount() == 1);
    // Received raw demands are preserved for diagnostics
    assert(ctl.switchDemands[0] == SwitchDemand::REVERSE);
    // Non-vital maintainer call is parsed
    assert(ctl.maintainerCall[0] == true);
    printf("  -> PASS: Conflicting vital demand flagged (vitalValid = false, vitalConflictCount = 1)\n");

    // 2. Apply to ControlPoint: vital appliances must NOT be invoked!
    uint32_t nowMs = 1000;
    cp.applyControlTransaction(ctl, nowMs);

    // Switch points must be UNTOUCHED in Normal position
    assert(sw1->reportedPosition() == SwitchPosition::NORMAL);
    assert(sw1->commandedPosition() == SwitchPosition::NORMAL);
    // Maintainer call was non-vital and MUST be updated
    IndicationVector ind;
    cp.exportIndicationVector(ind);
    assert(ind.maintainerCall[0] == true);
    printf("  -> PASS: Vital appliances untouched; non-vital maintainer call activated\n\n");
}

void testAarTextCodecSignalConflict() {
    printf("[TEST] AarTextCodec: Conflicting signal directions\n");

    ControlPoint cp("CP_Test");
    SignalControl* sig2 = cp.addSignalControl("2");

    AarTextCodec codec;
    codec.decodeControls({
        decodeSignal(sig2)
    });

    // Inbound commands opposing directions: 2SGS and 2NGS both asserted
    ControlTransaction ctl;
    codec.decodeControls("2SGS, 2NGS", ctl);

    assert(ctl.vitalValid == false);
    assert(codec.vitalConflictCount() == 1);
    printf("  -> PASS: Opposing signal commands flagged vitalValid = false\n\n");
}

void testBitPackedCodecSequentialStream() {
    printf("[TEST] BitPackedCodec: Sequential bit streaming and byte padding\n");

    ControlPoint cp("CP_Test");
    Switch* sw1 = cp.addSwitch("1");
    Switch* sw3 = cp.addSwitch("3");
    SignalControl* sig2 = cp.addSignalControl("2");
    TrackCircuit* tc1T = cp.addTrackCircuit("1T");
    TrackCircuit* tcIND = cp.addTrackCircuit("IND");

    BitPackedCodec codec;
    codec.decodeControls({
        decodeSwitch(sw1),   // bits 0-1 (Byte 0: b0, b1)
        decodeSwitch(sw3),   // bits 2-3 (Byte 0: b2, b3)
        padToByte(),         // advance to bit 8 (Byte 1)
        decodeSignal(sig2),  // bits 8-10 (Byte 1: b0, b1, b2)
        decodeMaintainer(0)  // bit 11 (Byte 1: b3)
    });

    codec.encodeIndications({
        encodeSwitch(sw1),   // bits 0-1 (Byte 0)
        encodeSwitch(sw3),   // bits 2-3 (Byte 0)
        encodeTrack(tc1T),   // bit 4 (Byte 0)
        padToByte(),         // advance to bit 8 (Byte 1)
        encodeSignal(sig2),  // bits 8-10 (Byte 1)
        padToByte(),         // advance to bit 16 (Byte 2)
        encodeTrack(tcIND)   // bit 16 (Byte 2: b0)
    });

    assert(codec.expectedControlBytes() == 2);
    assert(codec.expectedIndicationBytes() == 3);

    // Test Control Unpack:
    // Byte 0: 1NW=1 (b0), 3RW=1 (b3) -> 0x01 | 0x08 = 0x09
    // Byte 1: 2NG=1 (b1), MC1=1 (b3) -> 0x02 | 0x08 = 0x0A
    uint8_t rx[2] = { 0x09, 0x0A };
    ControlTransaction ctl;
    bool ok = codec.unpackControls(rx, sizeof(rx), ctl);
    assert(ok);
    assert(ctl.switchDemands[0] == SwitchDemand::NORMAL);
    assert(ctl.switchDemands[1] == SwitchDemand::REVERSE);
    assert(ctl.signalDemands[0] == SignalDemand::LEFT);
    assert(ctl.maintainerCall[0] == true);
    printf("  -> PASS: Packed binary controls unpacked accurately\n");

    // Test Indication Pack:
    sw1->updateFeedback(SwitchPosition::NORMAL);
    sw3->throwSwitch(SwitchPosition::REVERSE);
    sw3->updateFeedback(SwitchPosition::REVERSE);
    tc1T->update(Occupancy::OCCUPIED);
    tcIND->update(Occupancy::VACANT);
    IndicationVector ind;
    cp.exportIndicationVector(ind);
    ind.signals[0].activeAuthority = DirectionAuthority::LEFT;

    uint8_t tx[3] = {0};
    bool packOk = codec.packIndications(ind, tx, sizeof(tx));
    assert(packOk);
    // Byte 0: 1NWK=b0(0x01), 3RWK=b3(0x08), 1TK=b4(0x10) -> 0x19
    assert(tx[0] == 0x19);
    // Byte 1: 2NGK=b1(0x02) -> 0x02
    assert(tx[1] == 0x02);
    // Byte 2: INDK=b0(0x00) -> 0x00
    assert(tx[2] == 0x00);
    printf("  -> PASS: Packed binary indications formatted with padToByte() byte alignment\n\n");
}

void testElectricLockCodec() {
    printf("[TEST] ElectricLock Codec: WLS control decode and WLK indication encode\n");
    ControlPoint cp("CP_Test");
    Switch* sw7 = cp.addSwitch("7");
    sw7->addLock(SwitchLock::HAND_LOCKED); // Initially locked

    AarTextCodec codec;
    codec.decodeControls({
        decodeElectricLock(sw7) // 7WLS
    });
    codec.encodeIndications({
        encodeElectricLock(sw7) // 7WLK
    });

    // 1. Initial State: locked -> reports (7WLK)
    IndicationVector ind;
    cp.exportIndicationVector(ind);
    char buf[128];
    codec.encodeIndications(ind, buf, sizeof(buf));
    assert(strcmp(buf, "(7WLK)") == 0);
    printf("  -> Initially locked: Indication reports (7WLK)\n");

    // 2. Dispatcher transmits 7WLS (Unlock demand)
    ControlTransaction ctl;
    codec.decodeControls("7WLS", ctl);
    assert(ctl.lockDemands[0] == ElectricLockDemand::UNLOCK);

    // Apply to ControlPoint: releases lock
    cp.applyControlTransaction(ctl, 1000);
    assert((sw7->activeLocks() & SwitchLock::HAND_LOCKED) == SwitchLock::UNLOCKED);

    // 3. Outbound indication now reports 7WLK (asserted / unlocked)
    cp.exportIndicationVector(ind);
    codec.encodeIndications(ind, buf, sizeof(buf));
    assert(strcmp(buf, "7WLK") == 0);
    printf("  -> Dispatcher unlocked: Indication reports 7WLK\n");

    // 4. Dispatcher sends (7WLS) to relock
    codec.decodeControls("(7WLS)", ctl);
    assert(ctl.lockDemands[0] == ElectricLockDemand::LOCK);
    cp.applyControlTransaction(ctl, 1000);
    assert((sw7->activeLocks() & SwitchLock::HAND_LOCKED) == SwitchLock::HAND_LOCKED);
    cp.exportIndicationVector(ind);
    codec.encodeIndications(ind, buf, sizeof(buf));
    assert(strcmp(buf, "(7WLK)") == 0);
    printf("  -> PASS: Electric Switch Lock WLS / WLK cycle verified!\n\n");
}

int main() {
    printf("====================================================\n");
    printf("   FIELDUNIT WIRE CODEC COMPREHENSIVE TEST SUITE    \n");
    printf("====================================================\n\n");

    testAarTextCodecBasicOrderIndependent();
    testAarTextCodecMandatorySuffixes();
    testAarTextCodecVitalConflictIsolation();
    testAarTextCodecSignalConflict();
    testBitPackedCodecSequentialStream();
    testElectricLockCodec();

    printf("====================================================\n");
    printf("   ALL WIRE CODEC TESTS PASSED SUCCESSFULLY!        \n");
    printf("====================================================\n");
    return 0;
}
