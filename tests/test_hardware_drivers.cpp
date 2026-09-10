#include <stdio.h>
#include <assert.h>
#include "../src/FieldUnit.h"

using namespace FieldUnit;

void runHardwareDriverTests() {
    printf("====================================================\n");
    printf("   FIELDUNIT V2 HARDWARE DRIVERS TEST SUITE         \n");
    printf("   3-Coordinate (device, offset, bit) Mapping Layer \n");
    printf("====================================================\n\n");

    MockIOBus io;
    uint32_t clockMs = 1000;

    // -------------------------------------------------------------
    // TEST 1: TrackCircuitDriver (DCCOD Active-Low Occupancy)
    // -------------------------------------------------------------
    printf("[TEST 1] TrackCircuitDriver with active-low DCCOD detector pin\n");
    TrackCircuit tc1("1T1");
    // Standard cpNode mapping: Expander 0, Port A (offset 0), bit 2 = 1T1 (active-low)
    InputBit bit1T1(0, 0, 2, Polarity::INVERTED);
    TrackCircuitDriver tcDriver(&tc1, bit1T1);

    // Initial state: Physical contact open (pull-up HIGH, no train) -> VACANT
    io.setPinState(bit1T1, false /*not shunted*/);
    tcDriver.sample(io, clockMs);
    assert(tc1.isClear());
    assert(tc1.state().value == Occupancy::VACANT);
    printf("  -> Contact open (un-shunted): TrackCircuit reports VACANT\n");

    // Train arrives: DCCOD sensor pulls pin LOW to GND -> OCCUPIED
    io.setPinState(bit1T1, true /*shunted*/);
    tcDriver.sample(io, clockMs);
    assert(!tc1.isClear());
    assert(tc1.state().value == Occupancy::OCCUPIED);
    printf("  -> PASS: Sensor shunted: TrackCircuit reports OCCUPIED\n\n");

    // -------------------------------------------------------------
    // TEST 2: SwitchDriver (Tortoise Stall Motor + Microswitches)
    // -------------------------------------------------------------
    printf("[TEST 2] SwitchDriver: Motor driving and point contact sampling\n");
    Switch sw1("SW1");
    // Standard cpNode-IOX mapping for Switch 1:
    // Device 0, Port A (offset 0):
    //   bit 3 = T1 (Motor Output)
    //   bit 1 = 1NW (Normal sense contact, active-low)
    //   bit 0 = 1RW (Reverse sense contact, active-low)
    OutputBit bitMotor(0, 0, 3);
    InputBit  bitNormalSense(0, 0, 1, Polarity::INVERTED);
    InputBit  bitReverseSense(0, 0, 0, Polarity::INVERTED);
    SwitchDriver swDriver(&sw1, bitMotor, bitNormalSense, bitReverseSense);

    // 1. Initially Normal contact is closed, Reverse contact is open
    io.setPinState(bitNormalSense, true /*closed*/);
    io.setPinState(bitReverseSense, false /*open*/);
    swDriver.sample(io);
    assert(sw1.reportedPosition() == SwitchPosition::NORMAL);
    assert(sw1.inCorrespondence());

    // Drive motor output
    swDriver.drive(io);
    assert(io.readOutputRaw(bitMotor) == true && "Motor bit must be HIGH for Normal");
    printf("  -> Normal contact closed: Switch in Normal correspondence, motor bit HIGH\n");

    // 2. Command Switch to Reverse
    sw1.throwSwitch(SwitchPosition::REVERSE, clockMs);
    swDriver.drive(io);
    assert(io.readOutputRaw(bitMotor) == false && "Motor bit must be LOW for Reverse");
    printf("  -> Switch commanded Reverse: Motor bit driven LOW\n");

    // During travel, Normal contact breaks -> neither contact closed -> MOVING
    io.setPinState(bitNormalSense, false);
    io.setPinState(bitReverseSense, false);
    swDriver.sample(io);
    assert(sw1.reportedPosition() == SwitchPosition::MOVING);
    assert(!sw1.inCorrespondence());
    printf("  -> In-flight: Neither contact closed -> Switch reports MOVING\n");

    // Points reach Reverse stop -> Reverse contact makes
    io.setPinState(bitReverseSense, true);
    swDriver.sample(io);
    assert(sw1.reportedPosition() == SwitchPosition::REVERSE);
    assert(sw1.inCorrespondence());
    printf("  -> PASS: Reverse contact closed -> Switch in Reverse correspondence\n\n");

    // -------------------------------------------------------------
    // TEST 3: SignalMastDriver (Color-Light 2-Head LED Driving)
    // -------------------------------------------------------------
    printf("[TEST 3] SignalMastDriver: Multi-head LED pin driving\n");
    SignalMast mast2N("2Nab", MastType::TWO_HEAD);
    SignalMastDriver mastDriver(&mast2N);

    // Head 0 (Top head H2NA): Device 2, Port A (offset 0): Red=b0, Yellow=b1, Green=b2
    // Head 1 (Lower head H2NB): Device 2, Port A (offset 0): Red=b3, Yellow=b4, Green=b5
    OutputBit h0R(2, 0, 0), h0Y(2, 0, 1), h0G(2, 0, 2);
    OutputBit h1R(2, 0, 3), h1Y(2, 0, 4), h1G(2, 0, 5);
    mastDriver.addHead(h0R, h0Y, h0G);
    mastDriver.addHead(h1R, h1Y, h1G);

    // 1. Initial State: STOP (Red over Red)
    mastDriver.drive(io, clockMs);
    assert(io.readOutputRaw(h0R) == true && io.readOutputRaw(h0G) == false);
    assert(io.readOutputRaw(h1R) == true && io.readOutputRaw(h1G) == false);
    printf("  -> Mast at STOP: Pin drivers assert Red over Red (h0R=1, h1R=1)\n");

    // 2. Mainline Route Cleared: CLEAR (Green over Red)
    mast2N.setHeadIndication(0, Indication::CLEAR);
    mastDriver.drive(io, clockMs);
    assert(io.readOutputRaw(h0G) == true && io.readOutputRaw(h0R) == false); // Top head GREEN
    assert(io.readOutputRaw(h1R) == true && io.readOutputRaw(h1G) == false); // Lower head RED
    printf("  -> Mast at CLEAR: Top head GREEN (h0G=1), Lower head RED (h1R=1)\n");

    // 3. Diverging Route Cleared: DIVERGING_CLEAR (Red over Green)
    mast2N.setHeadIndication(1, Indication::DIVERGING_CLEAR);
    mastDriver.drive(io, clockMs);
    assert(io.readOutputRaw(h0R) == true && io.readOutputRaw(h0G) == false); // Top head RED
    assert(io.readOutputRaw(h1G) == true && io.readOutputRaw(h1R) == false); // Lower head GREEN
    printf("  -> PASS: Mast at DIVERGING_CLEAR: Top head RED (h0R=1), Lower head GREEN (h1G=1)\n\n");

    // -------------------------------------------------------------
    // TEST 4: Pluggable Aspect Policies (SP 1969 Lunar vs SP 1985 Flashing Red)
    // -------------------------------------------------------------
    printf("[TEST 4] Pluggable Aspect Policies: SP 1969 (Lunar) vs SP 1985 (Flashing Red)\n");

    // Add Lunar pin to Head 1 for SP 1969 testing
    OutputBit h1L(2, 0, 6);
    SignalMast mastSP("SP_Mast", MastType::TWO_HEAD);
    SignalMastDriver mastSPDriver(&mastSP);
    mastSPDriver.addHead(h0R, h0Y, h0G);
    mastSPDriver.addHead(h1R, h1Y, h1G, h1L);

    // Part A: SP 1969 Rulebook (Older era: Red over Lunar for Diverging Restricting)
    mastSP.setAspectPolicy(AspectPolicies::sp1969);
    mastSP.setIndication(Indication::DIVERGING_RESTRICTING);
    assert(mastSP.head1() == Aspect::RED);
    assert(mastSP.head2() == Aspect::LUNAR);
    assert(mastSP.compositeAspect() == Aspect::RED_OVER_LUNAR);

    mastSPDriver.drive(io, clockMs);
    assert(io.readOutputRaw(h0R) == true);
    assert(io.readOutputRaw(h1L) == true);
    assert(io.readOutputRaw(h1R) == false);
    printf("  -> PASS: SP 1969 Policy resolves Rule 290 Restricting to Red over Lunar (h0R=1, h1L=1)\n");

    // Part B: SP 1985 Rulebook (Later era: Red over Flashing Red for Diverging Restricting)
    mastSP.setAspectPolicy(AspectPolicies::sp1985);
    mastSP.setIndication(Indication::DIVERGING_RESTRICTING);
    assert(mastSP.head1() == Aspect::RED);
    assert(mastSP.head2() == Aspect::FLASHING_RED);
    assert(mastSP.compositeAspect() == Aspect::RED_OVER_FLASHING_RED);

    // Drive at t=1200 ms (flashPhase ON: 200ms < 500ms)
    mastSPDriver.drive(io, 1200);
    assert(io.readOutputRaw(h0R) == true); // Top head steady RED
    assert(io.readOutputRaw(h1R) == true); // Lower head flashing RED (ON phase)
    assert(io.readOutputRaw(h1L) == false); // Lunar OFF

    // Drive at t=1700 ms (flashPhase OFF: 700ms >= 500ms)
    mastSPDriver.drive(io, 1700);
    assert(io.readOutputRaw(h0R) == true);  // Top head steady RED
    assert(io.readOutputRaw(h1R) == false); // Lower head flashing RED (OFF phase)
    printf("  -> PASS: SP 1985 Policy resolves Rule 290 Restricting to Red over Flashing Red at 1 Hz\n");

    // Part C: SP 1985 Dwarf (Flashing Red)
    SignalMast dwarfSP("SP_Dwarf", MastType::DWARF);
    dwarfSP.setAspectPolicy(AspectPolicies::sp1985);
    dwarfSP.setIndication(Indication::RESTRICTING);
    assert(dwarfSP.head1() == Aspect::FLASHING_RED);
    printf("  -> PASS: SP 1985 Dwarf resolves Restricting to single Flashing Red\n\n");

    // -------------------------------------------------------------
    // TEST 5: SemaphoreDriver (Servo-Actuated Mechanical Blades)
    // -------------------------------------------------------------
    printf("[TEST 5] SemaphoreDriver: Multi-blade servo angle driving\n");
    SignalMast semMast("SEM_2R", MastType::TWO_HEAD);
    semMast.setAspectPolicy(AspectPolicies::upperQuadrantSemaphore);

    SemaphoreDriver semDriver(&semMast);
    // Arm 0 (Top blade): Device 0, Servo channel 0 (Stop=0 deg, Approach=45 deg, Clear=90 deg)
    // Arm 1 (Lower blade): Device 0, Servo channel 1 (Stop=0 deg, Approach=45 deg, Clear=90 deg)
    semDriver.addArm(0, 0, 0, 45, 90);
    semDriver.addArm(0, 1, 0, 45, 90);

    // 1. Initial State: STOP (Both blades horizontal at 0 deg)
    semMast.forceStop();
    semDriver.drive(io);
    assert(io.readAngle(0, 0) == 0 && "Top blade must be horizontal (0 deg)");
    assert(io.readAngle(0, 1) == 0 && "Lower blade must be horizontal (0 deg)");
    printf("  -> At STOP: Both blades horizontal at 0 deg (Ch0=0, Ch1=0)\n");

    // 2. Mainline Route Cleared: CLEAR (Top blade vertical 90 deg, lower blade 0 deg)
    semMast.setIndication(Indication::CLEAR);
    semDriver.drive(io);
    assert(io.readAngle(0, 0) == 90);
    assert(io.readAngle(0, 1) == 0);
    printf("  -> At CLEAR: Top blade vertical (90 deg), Lower blade horizontal (0 deg)\n");

    // 3. Diverging Route Cleared: DIVERGING_CLEAR (Top blade 0 deg, lower blade vertical 90 deg)
    semMast.setIndication(Indication::DIVERGING_CLEAR);
    semDriver.drive(io);
    assert(io.readAngle(0, 0) == 0);
    assert(io.readAngle(0, 1) == 90);
    printf("  -> At DIVERGING_CLEAR: Top blade 0 deg, Lower blade vertical 90 deg\n");

    // 4. Approach: APPROACH (Top blade diagonal 45 deg, lower blade 0 deg)
    semMast.setIndication(Indication::APPROACH);
    semDriver.drive(io);
    assert(io.readAngle(0, 0) == 45);
    assert(io.readAngle(0, 1) == 0);
    printf("  -> PASS: SemaphoreDriver correctly outputs servo PWM angles for mechanical arms\n\n");

    // -------------------------------------------------------------
    // TEST 6: Eastern 3-Head Speed Signaling (New York Central Policy)
    // -------------------------------------------------------------
    printf("[TEST 6] Eastern 3-Head Speed Signaling: New York Central Policy\n");
    SignalMast nycMast("NYC_CP", MastType::THREE_HEAD);
    nycMast.setAspectPolicy(AspectPolicies::nycSpeed);

    // High Speed: CLEAR -> Green over Red over Red
    nycMast.setIndication(Indication::CLEAR);
    assert(nycMast.head1() == Aspect::GREEN && nycMast.head2() == Aspect::RED && nycMast.head3() == Aspect::RED);
    printf("  -> CLEAR: Green over Red over Red (Rule 281)\n");

    // Approach Medium: Yellow over Green over Red
    nycMast.setIndication(Indication::APPROACH_MEDIUM);
    assert(nycMast.head1() == Aspect::YELLOW && nycMast.head2() == Aspect::GREEN && nycMast.head3() == Aspect::RED);
    printf("  -> APPROACH_MEDIUM: Yellow over Green over Red (Rule 282)\n");

    // Medium Speed: MEDIUM_CLEAR -> Red over Green over Red
    nycMast.setIndication(Indication::MEDIUM_CLEAR);
    assert(nycMast.head1() == Aspect::RED && nycMast.head2() == Aspect::GREEN && nycMast.head3() == Aspect::RED);
    printf("  -> MEDIUM_CLEAR: Red over Green over Red (Rule 283)\n");

    // Slow Speed: SLOW_CLEAR -> Red over Red over Green
    nycMast.setIndication(Indication::SLOW_CLEAR);
    assert(nycMast.head1() == Aspect::RED && nycMast.head2() == Aspect::RED && nycMast.head3() == Aspect::GREEN);
    printf("  -> SLOW_CLEAR: Red over Red over Green (Rule 287)\n");

    // Restricting: Red over Red over Yellow
    nycMast.setIndication(Indication::RESTRICTING);
    assert(nycMast.head1() == Aspect::RED && nycMast.head2() == Aspect::RED && nycMast.head3() == Aspect::YELLOW);
    printf("  -> PASS: NYC 3-Head speed signaling resolves all 3 speed brackets accurately\n\n");

    // -------------------------------------------------------------
    // TEST 7: Pennsylvania Railroad (PRR) Position-Light Policy
    // -------------------------------------------------------------
    printf("[TEST 7] Pennsylvania Railroad (PRR) Position-Light Policy\n");
    SignalMast prrMast("PRR_CP", MastType::TWO_HEAD);
    prrMast.setAspectPolicy(AspectPolicies::prrPositionLight);

    // Clear: Vertical over Dark
    prrMast.setIndication(Indication::CLEAR);
    assert(prrMast.head1() == Aspect::GREEN && prrMast.head2() == Aspect::DARK);
    printf("  -> CLEAR: Vertical over Dark\n");

    // Medium Clear: Horizontal over Vertical
    prrMast.setIndication(Indication::MEDIUM_CLEAR);
    assert(prrMast.head1() == Aspect::RED && prrMast.head2() == Aspect::GREEN);
    printf("  -> MEDIUM_CLEAR: Horizontal over Vertical\n");

    // Stop: Horizontal over Dark
    prrMast.forceStop();
    assert(prrMast.head1() == Aspect::RED && prrMast.head2() == Aspect::DARK);
    printf("  -> PASS: PRR Position-Light Policy correctly resolves amber lamp rows\n\n");

    printf("====================================================\n");
    printf("   ALL HARDWARE DRIVER TESTS PASSED!                \n");
    printf("====================================================\n");
}

int main() {
    runHardwareDriverTests();
    return 0;
}
