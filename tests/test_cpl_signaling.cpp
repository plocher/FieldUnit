#include <stdio.h>
#include <assert.h>
#include "../src/FieldUnit.h"
#include "../src/drivers/CplMastDriver.h"

using namespace FieldUnit;

void runCplSignalingTests() {
    printf("====================================================\n");
    printf("   FIELDUNIT B&O CPL SIGNALING TEST SUITE           \n");
    printf("   Orbital Markers & Central Disk Aspect Resolution \n");
    printf("====================================================\n\n");

    MockIOBus io;
    uint32_t clockMs = 1000;

    // -------------------------------------------------------------
    // TEST 1: boCpl Aspect Resolver Policy - High Signal Mast
    // -------------------------------------------------------------
    printf("[TEST 1] boCpl Aspect Resolver Policy - High Signal Mast\n");

    // High signal mast (single head, not dwarf)
    uint8_t headCount = 1;
    bool isDwarf = false;

    // Rule 292: Stop -> Red horizontal pair, no markers
    MastAspects aStop = AspectPolicies::boCpl(Indication::STOP, headCount, isDwarf);
    assert(aStop.head1 == Aspect::RED);
    assert(aStop.markers == 0);
    printf("  -> Rule 292 Stop: Red horizontal pair, zero markers\n");

    // Rule 281: Clear -> Green vertical pair + Top (12 o'clock) marker
    MastAspects aClear = AspectPolicies::boCpl(Indication::CLEAR, headCount, isDwarf);
    assert(aClear.head1 == Aspect::GREEN);
    assert(aClear.markers == static_cast<uint8_t>(CplMarker::TOP_12));
    printf("  -> Rule 281 Clear: Vertical Green + Top (12 o'clock) marker\n");

    // Rule 281A: Cab Speed -> Green vertical pair + Upper Left (10 o'clock) marker
    MastAspects aCab = AspectPolicies::boCpl(Indication::CAB_SPEED, headCount, isDwarf);
    assert(aCab.head1 == Aspect::GREEN);
    assert(aCab.markers == static_cast<uint8_t>(CplMarker::UPPER_L_10));
    printf("  -> Rule 281A Cab Speed: Vertical Green + Upper Left (10 o'clock) marker\n");

    // Rule 283: Medium Clear / Diverging Clear -> Green vertical pair + Upper Right (2 o'clock) marker
    MastAspects aMedClear = AspectPolicies::boCpl(Indication::MEDIUM_CLEAR, headCount, isDwarf);
    assert(aMedClear.head1 == Aspect::GREEN);
    assert(aMedClear.markers == static_cast<uint8_t>(CplMarker::UPPER_R_2));

    MastAspects aDivClear = AspectPolicies::boCpl(Indication::DIVERGING_CLEAR, headCount, isDwarf);
    assert(aDivClear.head1 == Aspect::GREEN);
    assert(aDivClear.markers == static_cast<uint8_t>(CplMarker::UPPER_R_2));
    printf("  -> Rule 283 Medium / Diverging Clear: Vertical Green + Upper Right (2 o'clock) marker\n");

    // Rule 287: Slow Clear -> Green vertical pair + Bottom (6 o'clock) marker
    MastAspects aSlowClear = AspectPolicies::boCpl(Indication::SLOW_CLEAR, headCount, isDwarf);
    assert(aSlowClear.head1 == Aspect::GREEN);
    assert(aSlowClear.markers == static_cast<uint8_t>(CplMarker::BOTTOM_6));
    printf("  -> Rule 287 Slow Clear: Vertical Green + Bottom (6 o'clock) marker\n");

    // Rule 285: Approach -> Diagonal Yellow + Top (12 o'clock) marker
    MastAspects aApp = AspectPolicies::boCpl(Indication::APPROACH, headCount, isDwarf);
    assert(aApp.head1 == Aspect::YELLOW);
    assert(aApp.markers == static_cast<uint8_t>(CplMarker::TOP_12));
    printf("  -> Rule 285 Approach: Diagonal Yellow + Top (12 o'clock) marker\n");

    // Rule 282: Approach Medium -> Diagonal Yellow + Upper Right (2 o'clock) marker
    MastAspects aAppMed = AspectPolicies::boCpl(Indication::APPROACH_MEDIUM, headCount, isDwarf);
    assert(aAppMed.head1 == Aspect::YELLOW);
    assert(aAppMed.markers == static_cast<uint8_t>(CplMarker::UPPER_R_2));
    printf("  -> Rule 282 Approach Medium: Diagonal Yellow + Upper Right (2 o'clock) marker\n");

    // Rule 284: Approach Slow -> Diagonal Yellow + Bottom (6 o'clock) marker
    MastAspects aAppSlow = AspectPolicies::boCpl(Indication::APPROACH_SLOW, headCount, isDwarf);
    assert(aAppSlow.head1 == Aspect::YELLOW);
    assert(aAppSlow.markers == static_cast<uint8_t>(CplMarker::BOTTOM_6));
    printf("  -> Rule 284 Approach Slow: Diagonal Yellow + Bottom (6 o'clock) marker\n");

    // Rule 282A: Advance Approach -> Flashing Diagonal Yellow + Top (12 o'clock) marker
    MastAspects aAdvApp = AspectPolicies::boCpl(Indication::ADVANCE_APPROACH, headCount, isDwarf);
    assert(aAdvApp.head1 == Aspect::FLASHING_YELLOW);
    assert(aAdvApp.markers == static_cast<uint8_t>(CplMarker::TOP_12));
    printf("  -> Rule 282A Advance Approach: Flashing Diagonal Yellow + Top (12 o'clock) marker\n");

    // Rule 290: Restricting -> Diagonal Lunar, zero markers
    MastAspects aRest = AspectPolicies::boCpl(Indication::RESTRICTING, headCount, isDwarf);
    assert(aRest.head1 == Aspect::LUNAR);
    assert(aRest.markers == 0);
    printf("  -> Rule 290 Restricting: Diagonal Lunar, zero markers\n");

    // Rule 290A: Diverging Restricting -> Diagonal Lunar + Bottom (6 o'clock) marker
    MastAspects aDivRest = AspectPolicies::boCpl(Indication::DIVERGING_RESTRICTING, headCount, isDwarf);
    assert(aDivRest.head1 == Aspect::LUNAR);
    assert(aDivRest.markers == static_cast<uint8_t>(CplMarker::BOTTOM_6));
    printf("  -> Rule 290A Diverging Restricting: Diagonal Lunar + Bottom (6 o'clock) marker\n\n");

    // -------------------------------------------------------------
    // TEST 2: boCpl Aspect Resolver Policy - Dwarf Signal
    // -------------------------------------------------------------
    printf("[TEST 2] boCpl Aspect Resolver Policy - Dwarf Signal\n");
    bool dwarf = true;

    assert(AspectPolicies::boCpl(Indication::STOP, 1, dwarf).head1 == Aspect::RED);
    assert(AspectPolicies::boCpl(Indication::STOP, 1, dwarf).markers == 0);

    assert(AspectPolicies::boCpl(Indication::CLEAR, 1, dwarf).head1 == Aspect::GREEN);
    assert(AspectPolicies::boCpl(Indication::CLEAR, 1, dwarf).markers == 0);

    assert(AspectPolicies::boCpl(Indication::APPROACH, 1, dwarf).head1 == Aspect::YELLOW);
    assert(AspectPolicies::boCpl(Indication::APPROACH, 1, dwarf).markers == 0);

    assert(AspectPolicies::boCpl(Indication::RESTRICTING, 1, dwarf).head1 == Aspect::LUNAR);
    assert(AspectPolicies::boCpl(Indication::RESTRICTING, 1, dwarf).markers == 0);
    printf("  -> PASS: Dwarf signals suppress all orbital markers\n\n");

    // -------------------------------------------------------------
    // TEST 3: SignalMast Integration with B&O Policy
    // -------------------------------------------------------------
    printf("[TEST 3] SignalMast with boCpl Policy\n");
    SignalMast cplMast("2LA", MastType::ONE_HEAD, AspectPolicies::boCpl);

    cplMast.setIndication(Indication::CLEAR);
    assert(cplMast.head1() == Aspect::GREEN);
    assert(cplMast.hasMarker(CplMarker::TOP_12));
    assert(!cplMast.hasMarker(CplMarker::BOTTOM_6));

    cplMast.setIndication(Indication::MEDIUM_CLEAR);
    assert(cplMast.head1() == Aspect::GREEN);
    assert(cplMast.hasMarker(CplMarker::UPPER_R_2));
    assert(!cplMast.hasMarker(CplMarker::TOP_12));

    cplMast.forceStop();
    assert(cplMast.head1() == Aspect::RED);
    assert(cplMast.markers() == 0);
    printf("  -> PASS: SignalMast reflects central aspect and orbital markers\n\n");

    // -------------------------------------------------------------
    // TEST 4: CplMastDriver Hardware Actuation & Flasher
    // -------------------------------------------------------------
    printf("[TEST 4] CplMastDriver Hardware Actuation & 1 Hz Flasher\n");
    OutputBit diskRed(1, 0, 0);
    OutputBit diskYel(1, 0, 1);
    OutputBit diskGrn(1, 0, 2);
    OutputBit diskLun(1, 0, 3);

    OutputBit mark12(1, 0, 4);
    OutputBit mark2(1, 0, 5);
    OutputBit mark4(1, 0, 6);
    OutputBit mark6(1, 0, 7);

    CplMastDriver driver(&cplMast);
    driver.setDiskPins(diskRed, diskYel, diskGrn, diskLun);
    driver.setMarkerPins(mark12, mark2, mark4, mark6);

    // 1. At STOP
    cplMast.forceStop();
    driver.drive(io, 1000);
    assert(io.readOutputRaw(diskRed) == true);
    assert(io.readOutputRaw(diskGrn) == false);
    assert(io.readOutputRaw(mark12) == false);
    assert(io.readOutputRaw(mark2) == false);
    printf("  -> At STOP: diskRed=1, all markers=0\n");

    // 2. Clear (Normal Speed) -> Green disk + mark12
    cplMast.setIndication(Indication::CLEAR);
    driver.drive(io, 1000);
    assert(io.readOutputRaw(diskGrn) == true);
    assert(io.readOutputRaw(diskRed) == false);
    assert(io.readOutputRaw(mark12) == true);
    assert(io.readOutputRaw(mark2) == false);
    printf("  -> At CLEAR: diskGrn=1, mark12=1, other markers=0\n");

    // 3. Medium Clear -> Green disk + mark2
    cplMast.setIndication(Indication::MEDIUM_CLEAR);
    driver.drive(io, 1000);
    assert(io.readOutputRaw(diskGrn) == true);
    assert(io.readOutputRaw(mark12) == false);
    assert(io.readOutputRaw(mark2) == true);
    printf("  -> At MEDIUM_CLEAR: diskGrn=1, mark2=1\n");

    // 4. Advance Approach (Flashing Yellow disk + mark12)
    cplMast.setIndication(Indication::ADVANCE_APPROACH);

    // t=1200ms -> flashPhase ON (200ms < 500ms)
    driver.drive(io, 1200);
    assert(io.readOutputRaw(diskYel) == true);
    assert(io.readOutputRaw(mark12) == true);

    // t=1700ms -> flashPhase OFF (700ms >= 500ms)
    driver.drive(io, 1700);
    assert(io.readOutputRaw(diskYel) == false);
    assert(io.readOutputRaw(mark12) == true); // Marker stays steady
    printf("  -> PASS: Advance Approach flashes yellow disk at 1 Hz with steady top marker\n\n");

    // -------------------------------------------------------------
    // TEST 5: ControlPoint Plant-Wide IndicationVector Export
    // -------------------------------------------------------------
    printf("[TEST 5] ControlPoint Plant-Wide IndicationVector Export\n");
    ControlPoint cp("CP_HARPERS_FERRY", AspectPolicies::boCpl);
    SignalMast* homeMast = cp.addSignalMast("4LA", MastType::ONE_HEAD);
    homeMast->setIndication(Indication::SLOW_CLEAR);

    IndicationVector ind;
    cp.exportIndicationVector(ind);
    assert(ind.mastCount == 1);
    assert(ind.masts[0].rulebookIndication == Indication::SLOW_CLEAR);
    assert(ind.masts[0].displayedAspect == Aspect::GREEN);
    assert(ind.masts[0].markers == static_cast<uint8_t>(CplMarker::BOTTOM_6));
    printf("  -> PASS: Plant indication vector correctly reports CPL orbital markers (BOTTOM_6)\n\n");

    printf("====================================================\n");
    printf("   ALL B&O CPL SIGNALING TESTS PASSED (100%%)       \n");
    printf("====================================================\n");
}

int main() {
    runCplSignalingTests();
    return 0;
}
