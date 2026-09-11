#include <stdio.h>
#include <assert.h>
#include <string.h>

// Include the sketch directly as a compilation unit
#include "../examples/Universal_FieldUnit/Universal_FieldUnit.ino"

void runUniversalSketchTest() {
    printf("====================================================\n");
    printf("   UNIVERSAL FIELDUNIT SKETCH TEST BENCH            \n");
    printf("   Boot-Time JSON Deserialization & Vital Execution \n");
    printf("====================================================\n\n");

    // 1. Run setup() - loads embedded JSON fallback into cp
    setup();

    assert(strcmp(cp.name(), "CP_Default") == 0);
    assert(cp.trackCircuitCount() == 2);
    assert(cp.switchCount() == 1);
    assert(cp.mastCount() == 1);
    assert(cp.engine().routeCount() == 1);
    printf("  -> PASS: setup() initialized plant from JSON: %s\n", cp.name());

    // 2. Initialize track state
    cp.findTrackCircuit("1T1")->update(Occupancy::VACANT);
    cp.findTrackCircuit("2T1")->update(Occupancy::VACANT);
    cp.findSwitch("1")->updateFeedback(SwitchPosition::NORMAL);

    // 3. Clear route MAIN
    ControlTransaction ctl;
    ctl.signalDemands[cp.findSignalControl("2")->index()] = SignalDemand::RIGHT;
    cp.applyControlTransaction(ctl, 1000);

    // Run loop()
    loop();

    SignalMast* mast2LA = cp.findSignalMast("2LA");
    assert(mast2LA->head1() == Aspect::GREEN);
    assert(cp.findSwitch("1")->isRouteLocked() == true);
    printf("  -> PASS: loop() cleared route MAIN: Signal 2LA displays Green; Switch 1 route-locked\n");

    // 4. Train enters entrance block 1T1 (Knockdown)
    cp.findTrackCircuit("1T1")->update(Occupancy::OCCUPIED);
    loop();

    assert(mast2LA->head1() == Aspect::RED);
    assert(cp.findSwitch("1")->isRouteLocked() == true);
    assert(cp.findSwitch("1")->isDetectorLocked() == true);
    printf("  -> PASS: loop() knocked down signal to Red; switch points double-locked\n\n");

    printf("====================================================\n");
    printf("   UNIVERSAL FIELDUNIT ALL TESTS PASSED (100%%)      \n");
    printf("====================================================\n");
}

int main() {
    runUniversalSketchTest();
    return 0;
}
