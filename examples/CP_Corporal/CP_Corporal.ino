/*
 * CP_Corporal - Southern Pacific Coast Line MP 83
 * Double-track to single-track convergence with Beet Loader industry spur
 * and power-operated safety derail.
 *
 * Prototype Track Diagram:
 *
 * < Railroad West / North                  MP 83                  Railroad East / South >
 *   (Toward Gilroy)                                               (Toward Sargent)
 *
 *                                DERAIL 5                  /─── IND3 ─── (Beet Loader 1)
 *                                    \ 5T1      o-| 4na   /
 *                           /─────────+─────── IND1 ─────+───── IND2 ─── (Beet Loader 2)
 *                          /                           SW7 (Hand-throw with 7WLS Lock)
 *                      1T1/                       oo-| 2nab (Two Heads)
 *   MT2 <══ 2SAT ═══][═══+═══════════════════════+══════════════════][════ 1NAT ══════ 2NAT ══> (<->)
 *   (Northbound) |-o 4sa SW1                 3T1/ SW3                     (Single Track)
 *   MT1 >══ 1SAT ═══][═════════════════════════/  [SS]
 *   (Southbound) |-o 2sa (Dwarf)
 */

#include <FieldUnit.h>

using namespace FieldUnit;

// Control Point Instance
ControlPoint cp("CP_Corporal");
AarTextCodec codec;

void configurePlant() {
    // 1. Declare Track Circuits
    cp.addTrackCircuit("1T1");  // SW1 OS
    cp.addTrackCircuit("3T1");  // SW3 OS
    cp.addTrackCircuit("5T1");  // SW5 Derail OS
    cp.addTrackCircuit("1NAT"); // Single track approach
    cp.addTrackCircuit("2NAT"); // Single track advance
    cp.addTrackCircuit("1SAT"); // MT1 Southbound approach
    cp.addTrackCircuit("2SAT"); // MT2 Northbound exit block

    // 2. Declare Switches
    cp.addSwitch("1"); // Industry lead
    cp.addSwitch("3"); // Double track merge
    cp.addSwitch("5"); // Derail

    // Bind detector locks by name
    cp.bindDetectorLock("1", "1T1");
    cp.bindDetectorLock("3", "3T1");
    cp.bindDetectorLock("5", "5T1");

    // 3. Declare Signal Authorities and Masts
    // Set plant-wide default rulebook to Southern Pacific 1969 (lunar era)
    cp.setDefaultAspectPolicy(AspectPolicies::sp1969);

    cp.addSignalControl("2");
    cp.addSignalControl("4");

    cp.addSignalMast("2NAB", MastType::TWO_HEAD);
    cp.addSignalMast("2SA",  MastType::DWARF);
    cp.addSignalMast("4NA",  MastType::ONE_HEAD);
    cp.addSignalMast("4SA",  MastType::ONE_HEAD);

    // 4. Declare Interlocking Control Table by name
    // Route 1: Northbound Single Track to MT2 right-hand running (SW1=N, SW3=N)
    cp.route("MT-NB")
      .governedBy("2", DirectionAuthority::LEFT)
      .displays("2NAB", 0 /* Top Head */, Indication::CLEAR)
      .aligns({ {"1", SwitchPosition::NORMAL}, 
                {"3", SwitchPosition::NORMAL} })
      .clears({ "3T1", "1T1", "2SAT" })
      .entrance("3T1");

    // Route 2: Northbound Single Track to MT1 reverse running (SW3=R)
    cp.route("MT-NB-REV")
      .governedBy("2", DirectionAuthority::LEFT)
      .displays("2NAB", 1 /* Lower Head */, Indication::DIVERGING_RESTRICTING)
      .aligns({ {"3", SwitchPosition::REVERSE} })
      .clears({ "3T1", "1SAT" })
      .entrance("3T1");

    // Route 3: Southbound MT1 through switch 3 onto single track (SW3=R)
    cp.route("SB-MT")
      .governedBy("2", DirectionAuthority::RIGHT)
      .displays("2SA", 0, Indication::CLEAR)
      .aligns({ {"3", SwitchPosition::REVERSE} })
      .clears({ "3T1", "1NAT" })
      .entrance("1SAT")
      .approaching("2NAT");

    // Route 4: Northbound Industry track onto MT2 (SW1=R, Derail SW5=R)
    cp.route("IND-NB")
      .governedBy("4", DirectionAuthority::LEFT)
      .displays("4NA", 0, Indication::RESTRICTING)
      .aligns({ {"1", SwitchPosition::REVERSE},
                {"5", SwitchPosition::REVERSE} })
      .clears({ "1T1", "5T1", "2SAT" })
      .entrance("5T1");

    // Route 5: Southbound on MT2 diverging into Industry lead (SW1=R, Derail SW5=R)
    cp.route("SB-IND")
      .governedBy("4", DirectionAuthority::RIGHT)
      .displays("4SA", 0, Indication::RESTRICTING)
      .aligns({ {"1", SwitchPosition::REVERSE},
                {"5", SwitchPosition::REVERSE} })
      .clears({ "1T1", "5T1" })
      .entrance("1T1");

    // 5. Configure Wire Codec
    codec.decodeControls({
        decodeSwitch(cp.findSwitch("1")),
        decodeSwitch(cp.findSwitch("3")),
        decodeSwitch(cp.findSwitch("5")),
        decodeSignal(cp.findSignalControl("2")),
        decodeSignal(cp.findSignalControl("4")),
        decodeMaintainer(0)
    });

    codec.encodeIndications({
        encodeSwitch(cp.findSwitch("1")),
        encodeSwitch(cp.findSwitch("3")),
        encodeSwitch(cp.findSwitch("5")),
        encodeTrack(cp.findTrackCircuit("1T1")),
        encodeTrack(cp.findTrackCircuit("3T1")),
        encodeTrack(cp.findTrackCircuit("5T1")),
        encodeTrack(cp.findTrackCircuit("1NAT")),
        encodeTrack(cp.findTrackCircuit("2NAT")),
        encodeTrack(cp.findTrackCircuit("1SAT")),
        encodeTrack(cp.findTrackCircuit("2SAT")),
        encodeSignal(cp.findSignalControl("2")),
        encodeSignal(cp.findSignalControl("4")),
        encodeMaintainer(0)
    });
}

void executeCycle(CodeLine& line, uint32_t nowMs) {
    char rxBuffer[256];
    size_t bytesRead = 0;

    // 1. Ingress
    if (line.receiveControlPacket(reinterpret_cast<uint8_t*>(rxBuffer), sizeof(rxBuffer) - 1, bytesRead)) {
        rxBuffer[bytesRead] = '\0';
        ControlTransaction ctl;
        if (codec.decodeControls(rxBuffer, ctl)) {
            // Safety Derail Interlock: SW5 derail tracks SW1 inversely
            if (ctl.switchDemands[0] == SwitchDemand::NORMAL) {
                ctl.switchDemands[2] = SwitchDemand::NORMAL; // Derail is derailing (closed)
            } else if (ctl.switchDemands[0] == SwitchDemand::REVERSE) {
                ctl.switchDemands[2] = SwitchDemand::REVERSE; // Derail is clear (open)
            }
            cp.applyControlTransaction(ctl, nowMs);
        }
    }

    // 2. Vital Cycle
    cp.tick(nowMs);

    // 3. Egress
    IndicationVector ind;
    cp.exportIndicationVector(ind);
    char txBuffer[256];
    size_t txLen = 0;
    if (codec.encodeIndications(ind, txBuffer, sizeof(txBuffer), txLen)) {
        line.transmitIndicationPacket(reinterpret_cast<const uint8_t*>(txBuffer), txLen);
    }
}

#ifdef ARDUINO
#include <Wire.h>
#include <I2Cexpander.h>
#include "drivers/I2CexpanderIOBus.h"

// Physical I2C Expanders (MCP23017 on cpNode-IOX)
I2Cexpander expanders[3];
I2CexpanderIOBus hardwareBus(expanders, 3);

// Hardware Drivers
TrackCircuitDriver tc1T1_drv;
TrackCircuitDriver tc3T1_drv;
TrackCircuitDriver tc5T1_drv;

SwitchDriver sw1_drv;
SwitchDriver sw3_drv;
SwitchDriver sw5_drv;

SignalMastDriver mast2NAB_drv;
SignalMastDriver mast2SA_drv;
SignalMastDriver mast4NA_drv;
SignalMastDriver mast4SA_drv;

void configureHardwareDrivers() {
    tc1T1_drv = TrackCircuitDriver(cp.findTrackCircuit("1T1"), InputBit(0, 0, 0, Polarity::INVERTED));
    tc3T1_drv = TrackCircuitDriver(cp.findTrackCircuit("3T1"), InputBit(0, 0, 1, Polarity::INVERTED));
    tc5T1_drv = TrackCircuitDriver(cp.findTrackCircuit("5T1"), InputBit(0, 0, 2, Polarity::INVERTED));

    sw1_drv = SwitchDriver(cp.findSwitch("1"), OutputBit(0, 0, 3), InputBit(0, 0, 4, Polarity::INVERTED), InputBit(0, 0, 5, Polarity::INVERTED));
    sw3_drv = SwitchDriver(cp.findSwitch("3"), OutputBit(0, 0, 6), InputBit(0, 0, 7, Polarity::INVERTED), InputBit(0, 1, 0, Polarity::INVERTED));
    sw5_drv = SwitchDriver(cp.findSwitch("5"), OutputBit(0, 1, 1), InputBit(0, 1, 2, Polarity::INVERTED), InputBit(0, 1, 3, Polarity::INVERTED));

    mast2NAB_drv = SignalMastDriver(cp.findSignalMast("2NAB"));
    mast2NAB_drv.addHead(OutputBit(1, 0, 0), OutputBit(1, 0, 1), OutputBit(1, 0, 2)); // Head 0
    mast2NAB_drv.addHead(OutputBit(1, 0, 3), OutputBit(1, 0, 4), OutputBit(1, 0, 5)); // Head 1

    mast2SA_drv = SignalMastDriver(cp.findSignalMast("2SA"));
    mast2SA_drv.addHead(OutputBit(1, 0, 6), OutputBit(), OutputBit(1, 0, 7)); // Dwarf

    mast4NA_drv = SignalMastDriver(cp.findSignalMast("4NA"));
    mast4NA_drv.addHead(OutputBit(1, 1, 0), OutputBit(), OutputBit(1, 1, 1));

    mast4SA_drv = SignalMastDriver(cp.findSignalMast("4SA"));
    mast4SA_drv.addHead(OutputBit(1, 1, 2), OutputBit(), OutputBit(1, 1, 3));
}

void samplePhysicalInputs(IOBus& bus, uint32_t nowMs) {
    tc1T1_drv.sample(bus, nowMs);
    tc3T1_drv.sample(bus, nowMs);
    tc5T1_drv.sample(bus, nowMs);

    sw1_drv.sample(bus);
    sw3_drv.sample(bus);
    sw5_drv.sample(bus);
}

void drivePhysicalOutputs(IOBus& bus, uint32_t nowMs) {
    sw1_drv.drive(bus);
    sw3_drv.drive(bus);
    sw5_drv.drive(bus);

    mast2NAB_drv.drive(bus, nowMs);
    mast2SA_drv.drive(bus, nowMs);
    mast4NA_drv.drive(bus, nowMs);
    mast4SA_drv.drive(bus, nowMs);
}

MockCodeLine localDebugLine;

void setup() {
    Serial.begin(115200);
    Wire.begin();
    for (uint8_t i = 0; i < 3; ++i) {
        expanders[i].init(i, I2Cexpander::MCP23017, 0xFFFF);
    }
    configurePlant();
    configureHardwareDrivers();
    Serial.println(F("CP Corporal FieldUnit Initialized"));
}

void loop() {
    uint32_t nowMs = millis();
    samplePhysicalInputs(hardwareBus, nowMs);
    executeCycle(localDebugLine, nowMs);
    drivePhysicalOutputs(hardwareBus, nowMs);
    delay(20);
}
#endif
