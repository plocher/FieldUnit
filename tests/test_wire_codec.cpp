#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "WireCodec.h"
#include "ControlPoint.h"

using namespace FieldUnit;

void testAarTextCodecSequentialStepping() {
    printf("[TEST] AarTextCodec: Strict sequential step decoding and declaration-order encoding\n");

    ControlPoint cp("CP_Test");
    Switch* sw1 = cp.addSwitch("1");
    Switch* sw3 = cp.addSwitch("3");
    SignalControl* sig2 = cp.addSignalControl("2");
    TrackCircuit* tc1T = cp.addTrackCircuit("1T");

    AarTextCodec codec;
    codec.decodeControls({
        decodeSwitch(sw1),   // Step 0: 1NW, Step 1: 1RW
        decodeSwitch(sw3),   // Step 2: 3NW, Step 3: 3RW
        decodeSignal(sig2),  // Step 4: 2SG, Step 5: 2NG, Step 6: 2H
        decodeMaintainer(0)  // Step 7: MC1
    });

    codec.encodeIndications({
        encodeSwitch(sw1),   // 1NWK, 1RWK
        encodeSwitch(sw3),   // 3NWK, 3RWK
        encodeTrack(tc1T),   // 1TK
        encodeSignal(sig2),  // 2SGK, 2NGK, 2TEK
        encodeMaintainer(0)  // MC1K
    });

    // 1. Inbound message in exact sequential order (Step 0 to Step 7)
    const char* validInput = "1nws, (1rws), (3nws), 3rws, (2sgs), 2NGS, (2hs), mc1s";
    ControlTransaction ctl;
    bool ok = codec.decodeControls(validInput, ctl);
    assert(ok);
    assert(ctl.vitalValid == true);
    assert(ctl.switchDemands[0] == SwitchDemand::NORMAL);
    assert(ctl.switchDemands[1] == SwitchDemand::REVERSE);
    assert(ctl.signalDemands[0] == SignalDemand::LEFT);
    assert(ctl.maintainerCall[0] == true);
    printf("  -> PASS: Strict sequential step tokens correctly decoded\n");

    // 2. Out-of-sequence message: MC1S sent first -> must be rejected immediately!
    const char* outOfOrder = "mc1s, 1nws, (1rws), (3nws), 3rws, (2sgs), 2NGS, (2hs)";
    ControlTransaction ctlBadSeq;
    bool badSeqOk = codec.decodeControls(outOfOrder, ctlBadSeq);
    assert(badSeqOk == false);
    assert(ctlBadSeq.vitalValid == false);
    printf("  -> PASS: Out-of-sequence transmission rejected immediately\n");

    // 3. Truncated message: Omitted switches and signals -> must be rejected immediately!
    const char* truncated = "1nws, (1rws)";
    ControlTransaction ctlTrunc;
    bool truncOk = codec.decodeControls(truncated, ctlTrunc);
    assert(truncOk == false);
    assert(ctlTrunc.vitalValid == false);
    printf("  -> PASS: Truncated transmission rejected immediately (Gate 1 failure)\n");

    // 4. Missing term for Switch 1: Only 1NWS sent, 1RWS omitted -> must be rejected!
    const char* missingTerm = "1nws, (3nws), 3rws, (2sgs), 2NGS, (2hs), mc1s";
    ControlTransaction ctlMissing;
    bool missOk = codec.decodeControls(missingTerm, ctlMissing);
    assert(missOk == false);
    assert(ctlMissing.vitalValid == false);
    printf("  -> PASS: Transmission missing required step term rejected immediately\n");

    // 5. Outbound indication formatting in exact declaration order
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

    // First token 1NW lacks 'S' -> rejected immediately on Step 0!
    assert(codec.unknownSymbolCount() == 1);
    assert(ctl.switchDemands[0] == SwitchDemand::NO_CHANGE);
    assert(ctl.signalDemands[0] == SignalDemand::NO_CHANGE);
    assert(ctl.vitalValid == false); // Truncated / malformed step
    printf("  -> PASS: Malformed step token lacking 'S' suffix rejected immediately; unknownSymbolCount = %u\n\n",
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
    const char* corruptInput = "1NWS, 1RWS, (2SGS), (2NGS), (2HS), MC1S";
    ControlTransaction ctl;
    codec.decodeControls(corruptInput, ctl);

    // 1. Transaction must be flagged invalid for vital safety
    assert(ctl.vitalValid == false);
    assert(codec.vitalConflictCount() == 1);
    // Demands remain safe at NO_CHANGE
    assert(ctl.switchDemands[0] == SwitchDemand::NO_CHANGE);
    printf("  -> PASS: Conflicting vital demand flagged (vitalValid = false, vitalConflictCount = 1)\n");

    // 2. Apply to ControlPoint: vital appliances must NOT be invoked!
    uint32_t nowMs = 1000;
    cp.applyControlTransaction(ctl, nowMs);

    // Switch points must be UNTOUCHED in Normal position
    assert(sw1->reportedPosition() == SwitchPosition::NORMAL);
    assert(sw1->commandedPosition() == SwitchPosition::NORMAL);
    printf("  -> PASS: Vital appliances untouched on corrupted transmission\n\n");
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
    codec.decodeControls("2SGS, 2NGS, (2HS)", ctl);

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

void testAarTextCodecSymmetricalOfficeAndSizing() {
    printf("[TEST] AarTextCodec: Symmetrical Office Operations, Worst-Case Sizing, and Preallocation (Strategy B)\n");

    ControlPoint cp("CP_Christopher");
    Switch* sw1 = cp.addSwitch("1");
    Switch* sw3 = cp.addSwitch("3");
    SignalControl* sig2 = cp.addSignalControl("2");
    TrackCircuit* tc1T = cp.addTrackCircuit("1T1");

    AarTextCodec codec;
    codec.decodeControls({
        decodeSwitch(sw1),
        decodeSwitch(sw3),
        decodeSignal(sig2)
    });
    codec.encodeIndications({
        encodeSwitch(sw1),
        encodeSwitch(sw3),
        encodeTrack(tc1T),
        encodeSignal(sig2)
    });

    // 1. Verify Worst-Case Sizing:
    // Controls:
    // - SW1: 1NWS (4) + (1RWS) (6) = 10
    // - SW3: 3NWS (4) + (3RWS) (6) = 10
    // - SIG2: 2SGS (4) + (2NGS) (6) + (2HS) (5) = 15
    // Total tokens: 7. Delimiters: 6 * 2 = 12. Null: 1. Total = 10 + 10 + 15 + 12 + 1 = 48.
    size_t maxCtl = codec.maxControlPayloadSize();
    assert(maxCtl >= 48);
    printf("  -> Calculated max control payload size: %zu bytes\n", maxCtl);

    size_t maxInd = codec.maxIndicationPayloadSize();
    assert(maxInd > 0);
    printf("  -> Calculated max indication payload size: %zu bytes\n", maxInd);

    // 2. Preallocate exact buffers (Strategy B):
    codec.preallocateBuffers();

    // 3. Office encodes controls:
    ControlTransaction ctl;
    ctl.switchDemands[0] = SwitchDemand::NORMAL;   // 1NWS, (1RWS)
    ctl.switchDemands[1] = SwitchDemand::REVERSE;  // (3NWS), 3RWS
    ctl.signalDemands[0] = SignalDemand::RIGHT;    // 2SGS, (2NGS), (2HS)

    const char* encodedCtl = codec.encodeControls(ctl);
    assert(encodedCtl != nullptr);
    assert(strcmp(encodedCtl, "1NWS, (1RWS), (3NWS), 3RWS, 2SGS, (2NGS), (2HS)") == 0);
    printf("  -> PASS: Preallocated encodeControls produced: %s\n", encodedCtl);

    // 4. Fail-fast capacity check on undersized buffer:
    char tinyBuf[10];
    size_t written = 0;
    bool fits = codec.encodeControls(ctl, tinyBuf, sizeof(tinyBuf), written);
    assert(!fits);
    assert(written == 0);
    printf("  -> PASS: Undersized buffer failed fast without corruption.\n");

    // 5. Office decodes indications from field:
    const char* fieldIndText = "1NWK, (1RWK), (3NWK), 3RWK, 1T1K, (2SGK), 2NGK, (2TEK)";
    IndicationVector ind;
    bool decOk = codec.decodeIndications(fieldIndText, ind);
    assert(decOk);
    assert(ind.switches[0].position == SwitchPosition::NORMAL);
    assert(ind.switches[0].inCorrespondence == true);
    assert(ind.switches[1].position == SwitchPosition::REVERSE);
    assert(ind.switches[1].inCorrespondence == true);
    assert(ind.trackCircuits[0].occupancy == Occupancy::OCCUPIED);
    assert(ind.signals[0].activeAuthority == DirectionAuthority::LEFT);
    assert(ind.signals[0].timeLocked == false);
    printf("  -> PASS: decodeIndications accurately reconstructed IndicationVector.\n\n");
}

int main() {
    printf("====================================================\n");
    printf("   FIELDUNIT WIRE CODEC COMPREHENSIVE TEST SUITE    \n");
    printf("====================================================\n\n");

    testAarTextCodecSequentialStepping();
    testAarTextCodecMandatorySuffixes();
    testAarTextCodecVitalConflictIsolation();
    testAarTextCodecSignalConflict();
    testAarTextCodecSymmetricalOfficeAndSizing();
    testBitPackedCodecSequentialStream();
    testElectricLockCodec();

    printf("====================================================\n");
    printf("   ALL WIRE CODEC TESTS PASSED SUCCESSFULLY!        \n");
    printf("====================================================\n");
    return 0;
}
