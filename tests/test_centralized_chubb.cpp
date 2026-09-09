#include <stdio.h>
#include <assert.h>
#include "../src/FieldUnit.h"
#include "../src/drivers/CmriIOBus.h"

using namespace FieldUnit;

void runCentralizedChubbPatternTest() {
    printf("====================================================\n");
    printf("   CLASSIC BRUCE CHUBB CENTRALIZED C/MRI TEST       \n");
    printf("   Local Dispatcher + Central CP + Remote Image I/O \n");
    printf("====================================================\n\n");

    // 1. Centralized Control Point Instance (lives on Host computer/MCU)
    ControlPoint cp("CP_Christopher_Central");

    TrackCircuit* tc3T1  = cp.addTrackCircuit("3T1");  // SW3 OS (MT1)
    TrackCircuit* tc3BT1 = cp.addTrackCircuit("3BT1"); // SW3B OS (MT2)
    TrackCircuit* tc2SA  = cp.addTrackCircuit("2SA");  // Northbound Approach
    tc2SA->update(Occupancy::VACANT); // Downstream approach block is clear

    Switch* sw3  = cp.addSwitch("SW3");
    Switch* sw3B = cp.addSwitch("SW3B");
    sw3->pairCrossover(sw3B);

    cp.bindDetectorLock(sw3, tc3T1);
    cp.bindDetectorLock(sw3B, tc3BT1);

    SignalControl* sig2 = cp.addSignalControl("SIG2");
    SignalMast* mast2N  = cp.addSignalMast("2Nab", MastType::TWO_HEAD);

    // Control Table
    cp.route("MT2-MT2-STRAIGHT")
      .governedBy(sig2, DirectionAuthority::LEFT)
      .displays(mast2N, 0, Indication::CLEAR)
      .aligns({ {sw3B, SwitchPosition::NORMAL} })
      .clears({ tc3BT1 })
      .approaching(tc2SA);

    // 2. Remote C/MRI Field I/O Images (raw byte buffers from CMRInet transport)
    // IB[0]: Remote inputs from cpNode (detectors, microswitches)
    // OB[0]: Remote outputs to cpNode (switch motor drivers, signal LEDs)
    uint8_t CMRI_IB[4] = {0};
    uint8_t CMRI_OB[4] = {0};

    // Bind CmriIOBus directly to C/MRI byte arrays
    CmriIOBus cmriBus(CMRI_IB, sizeof(CMRI_IB), CMRI_OB, sizeof(CMRI_OB));

    // 3. Hardware Drivers mapped to C/MRI Image bytes and bits
    // Input mapping:
    //   IB[0] bit 0 = 3BT1 detector (active-low DCCOD)
    //   IB[0] bit 1 = SW3 Normal sense microswitch (active-low)
    //   IB[0] bit 2 = SW3 Reverse sense microswitch (active-low)
    IOPin pin3BT1(0, 0);
    IOPin pinSW3_N(0, 1);
    IOPin pinSW3_R(0, 2);
    TrackCircuitDriver tc3BT1_Driver(tc3BT1, pin3BT1, /*activeLow=*/true);
    SwitchDriver sw3_Driver(sw3, IOPin(0, 3) /*motor OB[0] b3*/, pinSW3_N, pinSW3_R, /*activeLow=*/true);

    // Output mapping:
    //   OB[1] bit 0 = Mast 2N Top Head RED
    //   OB[1] bit 1 = Mast 2N Top Head GREEN
    SignalMastDriver mastDriver(mast2N);
    mastDriver.addHead(IOPin(1, 0) /*Red*/, IOPin(), IOPin(1, 1) /*Green*/);

    uint32_t clockMs = 1000;

    // -------------------------------------------------------------
    // CYCLE 1: Remote inputs arrive from field over C/MRI serial poll
    // -------------------------------------------------------------
    printf("[CYCLE 1] Remote field status arrives via C/MRI IB[] array\n");
    // Remote cpNode reports:
    //   3BT1 is clear (pull-up HIGH = bit 0 is 1)
    //   SW3 Normal contact closed (pulled to GND = bit 1 is 0)
    //   SW3 Reverse contact open (pull-up HIGH = bit 2 is 1)
    CMRI_IB[0] = 0b00000101; // b0=1 (clear), b1=0 (normal closed), b2=1 (reverse open)

    // Drivers sample directly from CMRI_IB[] image
    tc3BT1_Driver.sample(cmriBus, clockMs);
    sw3_Driver.sample(cmriBus);
    sw3B->updateFeedback(SwitchPosition::NORMAL); // paired slave follows
    assert(tc3BT1->isClear());
    assert(sw3->inCorrespondence() && sw3->reportedPosition() == SwitchPosition::NORMAL);
    printf("  -> Drivers sampled CMRI_IB[]: 3BT1 is VACANT, SW3 is in NORMAL correspondence\n\n");

    // -------------------------------------------------------------
    // CYCLE 2: Dispatcher (Local to Host) sends control transaction in-memory
    // -------------------------------------------------------------
    printf("[CYCLE 2] Local Dispatcher UI issues ControlTransaction directly in-memory\n");
    ControlTransaction ctl;
    ctl.switchDemands[0] = SwitchDemand::NORMAL;
    ctl.signalDemands[0] = SignalDemand::LEFT; // Northbound authority

    // Direct in-memory application (internal CodeLine)
    cp.applyControlTransaction(ctl, clockMs);

    // Vital interlocking cycle evaluates locally
    cp.tick(clockMs);

    // Drivers push outputs directly into CMRI_OB[] image
    sw3_Driver.drive(cmriBus);
    mastDriver.drive(cmriBus, clockMs);

    // Verify CMRI_OB[] image ready for next serial transmit to cpNode:
    //   OB[0] bit 3 (SW3 motor) must be HIGH (Normal)
    assert((CMRI_OB[0] & (1 << 3)) != 0);
    //   OB[1] bit 1 (Mast 2N Top Head GREEN) must be HIGH (CLEAR)
    assert((CMRI_OB[1] & (1 << 1)) != 0);
    //   OB[1] bit 0 (Mast 2N Top Head RED) must be LOW (OFF)
    assert((CMRI_OB[1] & (1 << 0)) == 0);

    printf("  -> PASS: CMRI_OB[] updated directly: Motor pin HIGH, Signal Top Head GREEN\n");
    printf("  -> PASS: CMRI output buffer is ready for gapless serial transmission to field nodes\n\n");

    printf("====================================================\n");
    printf("   CLASSIC CHUBB CENTRALIZED PATTERN PROVEN!        \n");
    printf("====================================================\n");
}

int main() {
    runCentralizedChubbPatternTest();
    return 0;
}
