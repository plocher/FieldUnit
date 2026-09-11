#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../src/FieldUnit.h"

using namespace FieldUnit;

void runPlantSerializerTests() {
    printf("====================================================\n");
    printf("   FIELDUNIT PLANT SERIALIZATION TEST SUITE         \n");
    printf("   Dynamic JSON Export & Boot-Time Deserialization  \n");
    printf("====================================================\n\n");

    uint32_t clockMs = 1000;

    // -------------------------------------------------------------
    // TEST 1: C++ Plant Setup -> Serialize to JSON
    // -------------------------------------------------------------
    printf("[TEST 1] Configure Plant in C++ and Serialize to JSON\n");
    ControlPoint cp1("CP_Corporal");
    cp1.setDefaultAspectPolicy(AspectPolicies::sp1969);

    // Track circuits
    cp1.addTrackCircuit("1T1");
    cp1.addTrackCircuit("3T1");
    cp1.addTrackCircuit("5T1");
    cp1.addTrackCircuit("1NAT");
    cp1.addTrackCircuit("2NAT");
    cp1.addTrackCircuit("1SAT");
    cp1.addTrackCircuit("2SAT");

    // Switches
    cp1.addSwitch("1");
    cp1.addSwitch("3");
    cp1.addSwitch("5");

    // Detector locks
    cp1.bindDetectorLock("1", "1T1");
    cp1.bindDetectorLock("3", "3T1");
    cp1.bindDetectorLock("5", "5T1");

    // Authorities and Masts
    cp1.addSignalControl("2");
    cp1.addSignalControl("4");
    cp1.addSignalMast("2NAB", MastType::TWO_HEAD);
    cp1.addSignalMast("2SA",  MastType::DWARF);
    cp1.addSignalMast("4NA",  MastType::ONE_HEAD);
    cp1.addSignalMast("4SA",  MastType::ONE_HEAD);

    // Routes
    cp1.route("MT-NB")
      .governedBy("2", DirectionAuthority::LEFT)
      .displays("2NAB", 0, Indication::CLEAR)
      .aligns({ {"1", SwitchPosition::NORMAL}, {"3", SwitchPosition::NORMAL} })
      .clears({ "3T1", "1T1", "2SAT" })
      .entrance("3T1");

    cp1.route("MT-NB-REV")
      .governedBy("2", DirectionAuthority::LEFT)
      .displays("2NAB", 1, Indication::DIVERGING_RESTRICTING)
      .aligns({ {"3", SwitchPosition::REVERSE} })
      .clears({ "3T1", "1SAT" })
      .entrance("3T1");

    cp1.route("SB-MT")
      .governedBy("2", DirectionAuthority::RIGHT)
      .displays("2SA", 0, Indication::CLEAR)
      .aligns({ {"3", SwitchPosition::REVERSE} })
      .clears({ "3T1", "1NAT" })
      .entrance("1SAT")
      .approaching("2NAT");

    cp1.route("IND-NB")
      .governedBy("4", DirectionAuthority::LEFT)
      .displays("4NA", 0, Indication::RESTRICTING)
      .aligns({ {"1", SwitchPosition::REVERSE}, {"5", SwitchPosition::REVERSE} })
      .clears({ "1T1", "5T1", "2SAT" })
      .entrance("5T1");

    cp1.route("SB-IND")
      .governedBy("4", DirectionAuthority::RIGHT)
      .displays("4SA", 0, Indication::RESTRICTING)
      .aligns({ {"1", SwitchPosition::REVERSE}, {"5", SwitchPosition::REVERSE} })
      .clears({ "1T1", "5T1" })
      .entrance("1T1");

    // Serialize to buffer
    char jsonBuf[8192];
    bool ok = cp1.serialize(jsonBuf, sizeof(jsonBuf), true /*pretty*/);
    assert(ok == true);
    assert(strlen(jsonBuf) > 0);
    assert(strstr(jsonBuf, "\"name\": \"CP_Corporal\"") != nullptr);
    assert(strstr(jsonBuf, "\"defaultAspectPolicy\": \"sp1969\"") != nullptr);
    assert(strstr(jsonBuf, "\"routes\": [") != nullptr);
    assert(strstr(jsonBuf, "\"MT-NB\"") != nullptr);
    printf("  -> PASS: Serialized CP_Corporal to JSON (%zu bytes)\n\n", strlen(jsonBuf));

    // -------------------------------------------------------------
    // TEST 2: Boot-Time Deserialization into Blank ControlPoint
    // -------------------------------------------------------------
    printf("[TEST 2] Boot-Time Deserialization from JSON into blank ControlPoint\n");
    ControlPoint cp2("Blank");
    bool loadOk = cp2.deserialize(jsonBuf);
    assert(loadOk == true);

    // Verify plant metadata
    assert(cp2.defaultAspectPolicy() == AspectPolicies::sp1969);

    // Verify track circuits
    assert(cp2.trackCircuitCount() == 7);
    assert(cp2.findTrackCircuit("1T1") != nullptr);
    assert(cp2.findTrackCircuit("3T1") != nullptr);
    assert(cp2.findTrackCircuit("5T1") != nullptr);
    assert(cp2.findTrackCircuit("1NAT") != nullptr);
    assert(cp2.findTrackCircuit("2NAT") != nullptr);
    assert(cp2.findTrackCircuit("1SAT") != nullptr);
    assert(cp2.findTrackCircuit("2SAT") != nullptr);

    // Verify switches
    assert(cp2.switchCount() == 3);
    assert(cp2.findSwitch("1") != nullptr);
    assert(cp2.findSwitch("3") != nullptr);
    assert(cp2.findSwitch("5") != nullptr);

    // Verify signal controls & masts
    assert(cp2.authorityCount() == 2);
    assert(cp2.findSignalControl("2") != nullptr);
    assert(cp2.findSignalControl("4") != nullptr);

    assert(cp2.mastCount() == 4);
    SignalMast* m2NAB = cp2.findSignalMast("2NAB");
    SignalMast* m2SA  = cp2.findSignalMast("2SA");
    assert(m2NAB != nullptr && m2NAB->type() == MastType::TWO_HEAD);
    assert(m2SA != nullptr && m2SA->type() == MastType::DWARF);

    // Verify detector locks
    assert(cp2.detectorLockCount() == 3);

    // Verify routes
    assert(cp2.engine().routeCount() == 5);
    const Route& r1 = cp2.engine().route(0);
    assert(strcmp(r1.name(), "MT-NB") == 0);
    assert(r1.authority() == cp2.findSignalControl("2"));
    assert(r1.direction() == DirectionAuthority::LEFT);
    assert(r1.mast() == m2NAB);
    assert(r1.targetHeadIndex() == 0);
    assert(r1.aspectCeiling() == Indication::CLEAR);
    assert(r1.switchCount() == 2);
    assert(r1.blockCount() == 3);
    printf("  -> PASS: All appliances, detector locks, and routes restored accurately\n\n");

    // -------------------------------------------------------------
    // TEST 3: Vital Interlocking Execution on Deserialized Plant
    // -------------------------------------------------------------
    printf("[TEST 3] Vital Interlocking Operations on Deserialized Plant\n");
    // Clear all tracks initially
    for (uint8_t i = 0; i < cp2.trackCircuitCount(); ++i) {
        cp2.trackCircuit(i)->update(Occupancy::VACANT, Quality::GOOD, clockMs);
    }
    for (uint8_t i = 0; i < cp2.switchCount(); ++i) {
        cp2.getSwitch(i)->updateFeedback(SwitchPosition::NORMAL);
    }
    cp2.tick(clockMs);

    assert(m2NAB->head1() == Aspect::RED);
    assert(m2NAB->head2() == Aspect::RED);

    // Dispatcher commands SIG2 LEFT (Route MT-NB)
    ControlTransaction ctl;
    ctl.signalDemands[cp2.findSignalControl("2")->index()] = SignalDemand::LEFT;
    cp2.applyControlTransaction(ctl, clockMs);
    cp2.tick(clockMs);

    // Signal must clear!
    assert(m2NAB->head1() == Aspect::GREEN);
    assert(m2NAB->head2() == Aspect::RED);
    assert(cp2.findSwitch("1")->isRouteLocked() == true);
    assert(cp2.findSwitch("3")->isRouteLocked() == true);
    printf("  -> Route MT-NB cleared: Mast 2NAB displays Green over Red; switches locked\n");

    // Train enters 3T1 (Signal knockdown & sectional release tracking)
    clockMs += 100;
    cp2.findTrackCircuit("3T1")->update(Occupancy::OCCUPIED, Quality::GOOD, clockMs);
    cp2.tick(clockMs);

    assert(m2NAB->head1() == Aspect::RED);
    assert(cp2.findSwitch("1")->isRouteLocked() == true); // 1T1 is still vacant; SW1 remains locked ahead!
    printf("  -> PASS: Vital route clearing and signal knockdown confirmed on deserialized plant\n\n");

    // -------------------------------------------------------------
    // TEST 4: Crossover and B&O CPL Policy Round-Trip
    // -------------------------------------------------------------
    printf("[TEST 4] Crossover & B&O CPL Aspect Policy Round-Trip\n");
    ControlPoint cpXover("CP_XOVER");
    cpXover.setDefaultAspectPolicy(AspectPolicies::boCpl);
    cpXover.addTrackCircuit("1T");
    cpXover.addTrackCircuit("2T");
    Switch* sw3A = cpXover.addSwitch("3A");
    Switch* sw3B = cpXover.addSwitch("3B");
    cpXover.addCrossover("3", sw3A, sw3B);
    cpXover.addSignalControl("2");
    cpXover.addSignalMast("2LA", MastType::ONE_HEAD, AspectPolicies::boCpl);

    cpXover.route("XOVER_REV")
      .governedBy("2", DirectionAuthority::LEFT)
      .displays("2LA", 0, Indication::MEDIUM_CLEAR)
      .aligns({ {"3", SwitchPosition::REVERSE} })
      .clears({ "1T", "2T" })
      .entrance("1T");

    char xoverJson[4096];
    bool serXover = cpXover.serialize(xoverJson, sizeof(xoverJson), false /*compact*/);
    assert(serXover == true);
    assert(strstr(xoverJson, "\"crossovers\":") != nullptr);
    assert(strstr(xoverJson, "\"switchA\": \"3A\"") != nullptr);
    assert(strstr(xoverJson, "\"boCpl\"") != nullptr);

    ControlPoint cpXoverRestored("BlankXover");
    bool desXover = cpXoverRestored.deserialize(xoverJson);
    assert(desXover == true);
    assert(cpXoverRestored.crossoverCount() == 1);
    assert(cpXoverRestored.findSwitch("3") != nullptr);
    assert(cpXoverRestored.defaultAspectPolicy() == AspectPolicies::boCpl);
    printf("  -> PASS: Crossovers and B&O CPL policies round-trip cleanly\n\n");

    printf("====================================================\n");
    printf("   ALL PLANT SERIALIZATION TESTS PASSED (100%%)     \n");
    printf("====================================================\n");
}

int main() {
    runPlantSerializerTests();
    return 0;
}
