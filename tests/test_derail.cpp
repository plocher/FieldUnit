#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../src/FieldUnit.h"

using namespace FieldUnit;

static void testIndependentDerailWithOs() {
    printf("[TEST] Independent derail addDerail(\"5\", \"5T1\")\n");
    InterlockingPlant cp("CP_Test");
    Switch* d5 = cp.addDerail("5", "5T1");
    assert(d5 != nullptr);
    assert(d5->isDerail());
    assert(!d5->isDependentDerail());
    assert(d5->appearsOnCodeLine());
    // Fail-safe default: derail on-rail (REVERSE)
    assert(d5->commandedPosition() == SwitchPosition::REVERSE);
    assert(d5->reportedPosition() == SwitchPosition::REVERSE);
    assert(cp.findTrackCircuit("5T1") != nullptr);
    assert(cp.findDetectorCircuitForSwitch(d5) == cp.findTrackCircuit("5T1"));
    assert(cp.findSwitch("5") == d5);
    printf("  -> PASS\n\n");
}

static void testDependentDerailBindsToBase() {
    printf("[TEST] Dependent derail addDerail(\"1D\", \"1DT1\") binds inverse to switch 1\n");
    InterlockingPlant cp("CP_Test");
    Switch* sw1 = cp.addSwitch("1", "1T1");
    Switch* d1 = cp.addDerail("1D", "1DT1");
    assert(sw1 != nullptr);
    assert(d1 != nullptr);
    assert(d1->isDerail());
    assert(d1->isDependentDerail());
    assert(!d1->appearsOnCodeLine());
    assert(sw1->dependentDerail() == d1);
    assert(d1->pairedSwitch() == sw1);

    // At rest: main NORMAL, derail on-rail REVERSE, master in combined correspondence
    assert(sw1->commandedPosition() == SwitchPosition::NORMAL);
    assert(d1->commandedPosition() == SwitchPosition::REVERSE);
    assert(d1->reportedPosition() == SwitchPosition::REVERSE);
    assert(sw1->inCorrespondence());
    assert(sw1->KR());
    assert(sw1->NWCR());
    assert(!sw1->RWCR());
    printf("  -> PASS\n\n");
}

static void testDependentMissingBaseFails() {
    printf("[TEST] addDerail(\"1D\") without base switch fails\n");
    InterlockingPlant cp("CP_Test");
    Switch* d1 = cp.addDerail("1D");
    assert(d1 == nullptr);
    assert(cp.findSwitch("1D") == nullptr);
    assert(cp.switchCount() == 0);
    printf("  -> PASS\n\n");
}

static void testInverseThrowAndCombinedKr() {
    printf("[TEST] Master throw REVERSE clears derail; KR waits for both ends\n");
    InterlockingPlant cp("CP_Test");
    Switch* sw1 = cp.addSwitch("1", "1T1");
    Switch* d1 = cp.addDerail("1D", "1DT1");
    uint32_t t = 1000;

    assert(sw1->throwSwitch(SwitchPosition::REVERSE, t));
    assert(sw1->reportedPosition() == SwitchPosition::MOVING);
    assert(d1->commandedPosition() == SwitchPosition::NORMAL);
    assert(d1->reportedPosition() == SwitchPosition::MOVING);
    assert(!sw1->inCorrespondence());
    assert(!sw1->KR());

    // Main arrives first — still dark until derail proves
    sw1->updateFeedback(SwitchPosition::REVERSE);
    assert(!sw1->KR());
    assert(!sw1->inCorrespondence());

    d1->updateFeedback(SwitchPosition::NORMAL);
    assert(sw1->inCorrespondence());
    assert(sw1->KR());
    assert(sw1->RWCR());
    assert(!sw1->NWCR());
    assert(sw1->reportedPosition() == SwitchPosition::REVERSE);

    // Back to normal main: derail must return on-rail
    assert(sw1->throwSwitch(SwitchPosition::NORMAL, t + 100));
    sw1->updateFeedback(SwitchPosition::NORMAL);
    d1->updateFeedback(SwitchPosition::REVERSE);
    assert(sw1->NWCR());
    assert(sw1->KR());
    printf("  -> PASS\n\n");
}

static void testDetectorLockFansAcrossPair() {
    printf("[TEST] Detector lock on main OS freezes derail pair\n");
    InterlockingPlant cp("CP_Test");
    Switch* sw1 = cp.addSwitch("1", "1T1");
    Switch* d1 = cp.addDerail("1D", "1DT1");
    TrackCircuit* tc = cp.findTrackCircuit("1T1");
    tc->update(Occupancy::OCCUPIED);
    cp.tick(1000);
    assert(sw1->isDetectorLocked());
    assert(d1->isDetectorLocked());
    assert(!sw1->throwSwitch(SwitchPosition::REVERSE, 1000));
    printf("  -> PASS\n\n");
}

static void testSwitchOsOptional() {
    printf("[TEST] addSwitch without OS has no detector lock\n");
    InterlockingPlant cp("CP_Test");
    Switch* sw7 = cp.addSwitch("7");
    assert(sw7 != nullptr);
    assert(cp.findDetectorCircuitForSwitch(sw7) == nullptr);
    assert(cp.trackCircuitCount() == 0);
    printf("  -> PASS\n\n");
}

static void testDependentDerailNotDirectlyCommandable() {
    printf("[TEST] Dependent derail rejects direct CodeLine demands; master drives pair\n");
    InterlockingPlant cp("CP_Test");
    Switch* sw1 = cp.addSwitch("1", "1T1");
    Switch* d1 = cp.addDerail("1D", "1DT1");
    sw1->updateFeedback(SwitchPosition::NORMAL);
    d1->updateFeedback(SwitchPosition::REVERSE);

    // Direct demand on dependent derail index must be ignored
    ControlTransaction rogue;
    rogue.switchDemands[d1->index()] = SwitchDemand::NORMAL;
    cp.applyControlTransaction(rogue, 1500);
    assert(d1->commandedPosition() == SwitchPosition::REVERSE);

    assert(!d1->appearsOnCodeLine());
    assert(sw1->appearsOnCodeLine());
    printf("  -> PASS\n\n");
}

static void testApplyTransactionDrivesDependentDerail() {
    printf("[TEST] ControlTransaction on switch 1 drives dependent derail inverse\n");
    InterlockingPlant cp("CP_Test");
    Switch* sw1 = cp.addSwitch("1", "1T1");
    Switch* d1 = cp.addDerail("1D", "1DT1");
    sw1->updateFeedback(SwitchPosition::NORMAL);
    d1->updateFeedback(SwitchPosition::REVERSE);

    ControlTransaction ctl;
    ctl.switchDemands[sw1->index()] = SwitchDemand::REVERSE;
    cp.applyControlTransaction(ctl, 2000);
    assert(sw1->commandedPosition() == SwitchPosition::REVERSE);
    assert(d1->commandedPosition() == SwitchPosition::NORMAL);
    printf("  -> PASS\n\n");
}

static void testSerializerRoundTripDerails() {
    printf("[TEST] PlantSerializer round-trip preserves derails and OS\n");
    InterlockingPlant cp1("CP_D");
    cp1.addSwitch("1", "1T1");
    cp1.addDerail("1D", "1DT1");
    cp1.addDerail("5", "5T1");
    cp1.addSwitch("3"); // no OS

    char buf[4096];
    assert(cp1.serialize(buf, sizeof(buf), true));
    assert(strstr(buf, "\"1D\"") != nullptr);
    assert(strstr(buf, "\"5\"") != nullptr);
    assert(strstr(buf, "\"os\"") != nullptr);

    InterlockingPlant cp2("Blank");
    assert(cp2.deserialize(buf));
    Switch* sw1 = cp2.findSwitch("1");
    Switch* d1 = cp2.findSwitch("1D");
    Switch* d5 = cp2.findSwitch("5");
    assert(sw1 && d1 && d5);
    assert(d1->isDependentDerail());
    assert(d5->isDerail() && !d5->isDependentDerail());
    assert(cp2.findDetectorCircuitForSwitch(sw1) == cp2.findTrackCircuit("1T1"));
    assert(cp2.findDetectorCircuitForSwitch(d1) == cp2.findTrackCircuit("1DT1"));
    assert(sw1->dependentDerail() == d1);
    printf("  -> PASS\n\n");
}

int main() {
    printf("====================================================\n");
    printf("   DERAIL & OS-BIND UNIT TESTS\n");
    printf("====================================================\n\n");
    testIndependentDerailWithOs();
    testDependentDerailBindsToBase();
    testDependentMissingBaseFails();
    testInverseThrowAndCombinedKr();
    testDetectorLockFansAcrossPair();
    testSwitchOsOptional();
    testDependentDerailNotDirectlyCommandable();
    testApplyTransactionDrivesDependentDerail();
    testSerializerRoundTripDerails();
    printf("====================================================\n");
    printf("   ALL DERAIL TESTS PASSED\n");
    printf("====================================================\n");
    return 0;
}
