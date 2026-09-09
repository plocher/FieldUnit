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

    printf("====================================================\n");
    printf("   ALL HARDWARE DRIVER TESTS PASSED!                \n");
    printf("====================================================\n");
}

int main() {
    runHardwareDriverTests();
    return 0;
}
