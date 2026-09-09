#include <stdio.h>
#include <assert.h>
#include "../src/FieldUnit.h"

using namespace FieldUnit;

void runHardwareDriverTests() {
    printf("====================================================\n");
    printf("   FIELDUNIT V2 HARDWARE DRIVERS TEST SUITE         \n");
    printf("   I2Cexpander (device, bit) Thin Mapping Layer     \n");
    printf("====================================================\n\n");

    MockIOBus io;
    uint32_t clockMs = 1000;

    // -------------------------------------------------------------
    // TEST 1: TrackCircuitDriver (DCCOD Active-Low Occupancy)
    // -------------------------------------------------------------
    printf("[TEST 1] TrackCircuitDriver with active-low DCCOD detector pin\n");
    TrackCircuit tc1("1T1");
    // Standard cpNode mapping: Expander 0, bit 2 = 1T1 (active-low open collector)
    IOPin pin1T1(0, 2);
    TrackCircuitDriver tcDriver(&tc1, pin1T1, /*activeLow=*/true);

    // Initial state: Pin is HIGH (pull-up resistor, no train) -> VACANT
    io.setPinState(pin1T1, true);
    tcDriver.sample(io, clockMs);
    assert(tc1.isClear());
    assert(tc1.state().value == Occupancy::VACANT);
    printf("  -> Pin HIGH (un-shunted): TrackCircuit reports VACANT\n");

    // Train arrives: DCCOD sensor pulls pin LOW to GND -> OCCUPIED
    io.setPinState(pin1T1, false);
    tcDriver.sample(io, clockMs);
    assert(!tc1.isClear());
    assert(tc1.state().value == Occupancy::OCCUPIED);
    printf("  -> PASS: Pin pulled LOW (shunted): TrackCircuit reports OCCUPIED\n\n");

    // -------------------------------------------------------------
    // TEST 2: SwitchDriver (Tortoise Stall Motor + Microswitches)
    // -------------------------------------------------------------
    printf("[TEST 2] SwitchDriver: Motor driving and point contact sampling\n");
    Switch sw1("SW1");
    // Standard cpNode-IOX mapping for Switch 1:
    // Device 0, bit 3 = T1 (Motor Output)
    // Device 0, bit 1 = 1NW (Normal sense contact, active-low)
    // Device 0, bit 0 = 1RW (Reverse sense contact, active-low)
    IOPin pinMotor(0, 3);
    IOPin pinNormalSense(0, 1);
    IOPin pinReverseSense(0, 0);
    SwitchDriver swDriver(&sw1, pinMotor, pinNormalSense, pinReverseSense, /*activeLowSense=*/true);

    // 1. Initially Normal contact is closed (pulls 1NW LOW), Reverse contact is open (HIGH)
    io.setPinState(pinNormalSense, false);
    io.setPinState(pinReverseSense, true);
    swDriver.sample(io);
    assert(sw1.reportedPosition() == SwitchPosition::NORMAL);
    assert(sw1.inCorrespondence());

    // Drive motor output
    swDriver.drive(io);
    assert(io.readBit(pinMotor) == true && "Motor pin must be HIGH for Normal");
    printf("  -> Normal contact closed: Switch in Normal correspondence, motor pin HIGH\n");

    // 2. Command Switch to Reverse
    sw1.throwSwitch(SwitchPosition::REVERSE, clockMs);
    swDriver.drive(io);
    assert(io.readBit(pinMotor) == false && "Motor pin must be LOW for Reverse");
    printf("  -> Switch commanded Reverse: Motor pin driven LOW\n");

    // During travel, Normal contact breaks -> neither contact closed -> MOVING
    io.setPinState(pinNormalSense, true);
    io.setPinState(pinReverseSense, true);
    swDriver.sample(io);
    assert(sw1.reportedPosition() == SwitchPosition::MOVING);
    assert(!sw1.inCorrespondence());
    printf("  -> In-flight: Neither contact closed -> Switch reports MOVING\n");

    // Points reach Reverse stop -> Reverse contact makes (pulls 1RW LOW)
    io.setPinState(pinReverseSense, false);
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

    // Head 0 (Top head H2NA): Device 2, Red=b0, Yellow=b1, Green=b2
    // Head 1 (Lower head H2NB): Device 2, Red=b3, Yellow=b4, Green=b5
    IOPin h0R(2, 0), h0Y(2, 1), h0G(2, 2);
    IOPin h1R(2, 3), h1Y(2, 4), h1G(2, 5);
    mastDriver.addHead(h0R, h0Y, h0G);
    mastDriver.addHead(h1R, h1Y, h1G);

    // 1. Initial State: STOP (Red over Red)
    mastDriver.drive(io, clockMs);
    assert(io.readBit(h0R) == true && io.readBit(h0G) == false);
    assert(io.readBit(h1R) == true && io.readBit(h1G) == false);
    printf("  -> Mast at STOP: Pin drivers assert Red over Red (h0R=1, h1R=1)\n");

    // 2. Mainline Route Cleared: CLEAR (Green over Red)
    mast2N.setHeadIndication(0, Indication::CLEAR);
    mastDriver.drive(io, clockMs);
    assert(io.readBit(h0G) == true && io.readBit(h0R) == false); // Top head GREEN
    assert(io.readBit(h1R) == true && io.readBit(h1G) == false); // Lower head RED
    printf("  -> Mast at CLEAR: Top head GREEN (h0G=1), Lower head RED (h1R=1)\n");

    // 3. Diverging Route Cleared: DIVERGING_CLEAR (Red over Green)
    mast2N.setHeadIndication(1, Indication::DIVERGING_CLEAR);
    mastDriver.drive(io, clockMs);
    assert(io.readBit(h0R) == true && io.readBit(h0G) == false); // Top head RED
    assert(io.readBit(h1G) == true && io.readBit(h1R) == false); // Lower head GREEN
    printf("  -> PASS: Mast at DIVERGING_CLEAR: Top head RED (h0R=1), Lower head GREEN (h1G=1)\n\n");

    printf("====================================================\n");
    printf("   ALL HARDWARE DRIVER TESTS PASSED!                \n");
    printf("====================================================\n");
}

int main() {
    runHardwareDriverTests();
    return 0;
}
