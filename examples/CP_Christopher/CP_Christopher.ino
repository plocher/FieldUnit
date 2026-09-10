/*
 * CP_Christopher - Southern Pacific Coast Line MP 81
 * Double-track mainline with high-speed facing-point crossover
 * and industry spur for Christopher Ranch.
 *
 * Prototype Track Diagram:
 *
 * < Railroad West/North               MP 81                    Railroad East/South >
 *   (Carneros/Luchessa)                                        (Corporal)
 *
 *                                                        oo=| 2Nab
 *  MT2 <══ 2SA ══════][═════════════════════════+═══════════][════ 2NA ════< MT2
 *        (Exit)       |-o 2Sc             3BT1 /                 (Approach)
 *                     (Dwarf)                 /
 *                                            /  3T1      oo-| 2Nc
 *  MT1 >══ 1SA ══════][════════+════════][══+══════][═══+═══][════ 1NA ════> MT1
 *      (Approach)     |-oo 2Sab \ 1T1              5T1 /          (Exit)
 *                                \═][══════════════][═/
 *                                       IND (Spur)
 *
 * Wire schema:
 *   Controls:
 *     1NW, 1RW, 3NW, 3RW, 3BNW, 3BRW, 5NW, 5RW
 *     2SG, 2NG, 2H,  MC1, MC2
 *   Indications (4 bytes):
 *     1NWK, 1RWK, 3NWK, 3RWK, 3BNWK, 3BRWK, 5NWK, 5RWK
 *     1T1,  3T1,  3BT1, 5T1,  1SA,   2SA,   1NA,  2NA
 *     2SGK, 2NGK, 2TEK, MC1,  MC2,   IND
 */

#include <FieldUnit.h>

using namespace FieldUnit;

// Control Point Instance
ControlPoint cp("CP_Christopher");
AarTextCodec codec;

void configurePlant() {
    // 1. Declare Track Circuits
    cp.addTrackCircuit("1T1");  // SW1 OS
    cp.addTrackCircuit("3T1");  // SW3 OS (MT1)
    cp.addTrackCircuit("3BT1"); // SW3B OS (MT2)
    cp.addTrackCircuit("5T1");  // SW5 OS
    cp.addTrackCircuit("1SA");  // MT1 Northbound approach
    cp.addTrackCircuit("2SA");  // MT2 Southbound approach
    cp.addTrackCircuit("1NA");  // MT1 Southbound exit/approach
    cp.addTrackCircuit("2NA");  // MT2 Northbound exit/approach
    cp.addTrackCircuit("IND");  // Industry spur

    // 2. Declare Switches & Crossover
    cp.addSwitch("1");
    cp.addSwitch("3");
    cp.addSwitch("3B");
    cp.addSwitch("5");
    cp.addCrossover("3", "3", "3B"); // Logical crossover wrapping machines 3 and 3B

    cp.bindDetectorLock("1", "1T1");
    cp.bindDetectorLock("3", "3T1");
    cp.bindDetectorLock("3B", "3BT1");
    cp.bindDetectorLock("5", "5T1");

    // 3. Declare Signal Control & Masts
    // Set plant-wide default rulebook to Southern Pacific 1969 (lunar era)
    cp.setDefaultAspectPolicy(AspectPolicies::sp1969);

    cp.addSignalControl("2");
    cp.addSignalMast("2Nab", MastType::TWO_HEAD);
    cp.addSignalMast("2Sab", MastType::TWO_HEAD);
    cp.addSignalMast("2Nc",  MastType::DWARF);

    // 4. Declare Interlocking Control Table by name
    // Route 1: MT2-MT2 Northbound Straight (Crossover 3 Normal) -> Top head H2NA CLEAR
    cp.route("MT2-MT2-STRAIGHT")
      .governedBy("2", DirectionAuthority::LEFT)
      .displays("2Nab", 0, Indication::CLEAR)
      .aligns({ {"3", SwitchPosition::NORMAL} })
      .clears({ "3BT1" })
      .entrance("3BT1")
      .approaching("2SA");

    // Route 2: MT2-MT1 Northbound Crossover (Crossover 3 Reverse) -> Lower head H2NB DIVERGING_CLEAR
    cp.route("MT2-MT1-CROSSOVER")
      .governedBy("2", DirectionAuthority::LEFT)
      .displays("2Nab", 1, Indication::DIVERGING_CLEAR)
      .aligns({ {"3", SwitchPosition::REVERSE},
                {"1", SwitchPosition::NORMAL} })
      .clears({ "3BT1", "3T1", "1T1" })
      .entrance("3BT1")
      .approaching("1SA");

    // Route 3: MT1-MT1 Southbound Straight (SW1=N, Crossover 3=N, SW5=N) -> Top head H2SA CLEAR
    cp.route("MT1-MT1-STRAIGHT")
      .governedBy("2", DirectionAuthority::RIGHT)
      .displays("2Sab", 0, Indication::CLEAR)
      .aligns({ {"1", SwitchPosition::NORMAL},
                {"3", SwitchPosition::NORMAL},
                {"5", SwitchPosition::NORMAL} })
      .clears({ "1T1", "3T1", "5T1" })
      .entrance("1T1")
      .approaching("1NA");

    // Route 4: MT1-MT2 Southbound Crossover (SW1=N, Crossover 3=R) -> Lower head H2SB DIVERGING_CLEAR
    cp.route("MT1-MT2-CROSSOVER")
      .governedBy("2", DirectionAuthority::RIGHT)
      .displays("2Sab", 1, Indication::DIVERGING_CLEAR)
      .aligns({ {"1", SwitchPosition::NORMAL},
                {"3", SwitchPosition::REVERSE} })
      .clears({ "1T1", "3T1" })
      .entrance("1T1")
      .approaching("2NA");

    // 5. Configure Wire Codec
    codec.decodeControls({
        decodeSwitch(cp.findSwitch("1")),
        decodeSwitch(cp.findSwitch("3")),
        decodeSwitch(cp.findSwitch("3B")),
        decodeSwitch(cp.findSwitch("5")),
        decodeSignal(cp.findSignalControl("2")),
        decodeMaintainer(0)
    });

    codec.encodeIndications({
        encodeSwitch(cp.findSwitch("1")),
        encodeSwitch(cp.findSwitch("3")),
        encodeSwitch(cp.findSwitch("3B")),
        encodeSwitch(cp.findSwitch("5")),
        encodeTrack(cp.findTrackCircuit("1T1")),
        encodeTrack(cp.findTrackCircuit("3T1")),
        encodeTrack(cp.findTrackCircuit("3BT1")),
        encodeTrack(cp.findTrackCircuit("5T1")),
        encodeTrack(cp.findTrackCircuit("1SA")),
        encodeTrack(cp.findTrackCircuit("2SA")),
        encodeTrack(cp.findTrackCircuit("1NA")),
        encodeTrack(cp.findTrackCircuit("2NA")),
        encodeSignal(cp.findSignalControl("2")),
        encodeTrack(cp.findTrackCircuit("IND")),
        encodeMaintainer(0)
    });
}

// Complete single-cycle execution: Ingress -> Vital Cycle -> Egress
void executeCycle(CodeLine& line, uint32_t nowMs) {
    char rxBuffer[256];
    size_t bytesRead = 0;

    // 1. Ingress: Poll CodeLine for incoming dispatcher control snapshot
    if (line.receiveControlPacket(reinterpret_cast<uint8_t*>(rxBuffer), sizeof(rxBuffer) - 1, bytesRead)) {
        rxBuffer[bytesRead] = '\0';
        ControlTransaction ctl;
        if (codec.decodeControls(rxBuffer, ctl)) {
            cp.applyControlTransaction(ctl, nowMs);
        }
    }

    // 2. Vital Cycle: Advance timers, check locks, evaluate control table
    cp.tick(nowMs);

    // 3. Egress: Export verified plant state and transmit indications
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
#include <drivers/I2CexpanderIOBus.h>

// Physical I2C Expanders (MCP23017 on cpNode-IOX)
// Dev 0: 0x20 (Switches 1 & 5)
// Dev 1: 0x21 (Switches 3 & 3B Crossover)
// Dev 2: 0x22 (Signal Mast 2Nab and 2Sab Heads)
// Dev 3: 0x23 (Signal Masts 2Nc and 2Sc Dwarfs)
// Dev 4: 0x24 (Approach & Aux Inputs)
I2Cexpander expanders[5];
I2CexpanderIOBus hardwareBus(expanders, 5);

// Physical Hardware Drivers
TrackCircuitDriver tc1T1_drv;
TrackCircuitDriver tc3T1_drv;
TrackCircuitDriver tc3BT1_drv;
TrackCircuitDriver tc5T1_drv;

SwitchDriver sw1_drv;
SwitchDriver sw3_drv;
SwitchDriver sw3B_drv;
SwitchDriver sw5_drv;

SignalMastDriver mast2N_drv;
SignalMastDriver mast2S_drv;

void configureHardwareDrivers() {
    // Track Circuits (DCCOD active-low detectors on Port A = offset 0)
    tc1T1_drv  = TrackCircuitDriver(cp.findTrackCircuit("1T1"),  InputBit(0, 0, 2, Polarity::INVERTED));
    tc3T1_drv  = TrackCircuitDriver(cp.findTrackCircuit("3T1"),  InputBit(1, 0, 2, Polarity::INVERTED));
    tc3BT1_drv = TrackCircuitDriver(cp.findTrackCircuit("3BT1"), InputBit(1, 0, 6, Polarity::INVERTED));
    tc5T1_drv  = TrackCircuitDriver(cp.findTrackCircuit("5T1"),  InputBit(0, 0, 6, Polarity::INVERTED));

    // Switches (Tortoise motor + Normal & Reverse feedback microswitches)
    sw1_drv  = SwitchDriver(cp.findSwitch("1"),  OutputBit(0, 0, 3), InputBit(0, 0, 1, Polarity::INVERTED), InputBit(0, 0, 0, Polarity::INVERTED));
    sw5_drv  = SwitchDriver(cp.findSwitch("5"),  OutputBit(0, 0, 7), InputBit(0, 0, 5, Polarity::INVERTED), InputBit(0, 0, 4, Polarity::INVERTED));
    sw3_drv  = SwitchDriver(cp.findSwitch("3"),  OutputBit(1, 0, 3), InputBit(1, 0, 1, Polarity::INVERTED), InputBit(1, 0, 0, Polarity::INVERTED));
    sw3B_drv = SwitchDriver(cp.findSwitch("3B"), OutputBit(1, 0, 7), InputBit(1, 0, 5, Polarity::INVERTED), InputBit(1, 0, 4, Polarity::INVERTED));

    // Signal Masts (Color-Light 2-Head LED Driving on Expander 2)
    mast2N_drv = SignalMastDriver(cp.findSignalMast("2Nab"));
    mast2N_drv.addHead(OutputBit(2, 0, 0) /*H2NA Red*/, OutputBit(2, 0, 1) /*Yellow*/, OutputBit(2, 0, 2) /*Green*/);
    mast2N_drv.addHead(OutputBit(2, 0, 3) /*H2NB Red*/, OutputBit(2, 0, 4) /*Yellow*/, OutputBit(2, 0, 5) /*Green*/);

    mast2S_drv = SignalMastDriver(cp.findSignalMast("2Sab"));
    mast2S_drv.addHead(OutputBit(2, 0, 6) /*H2SA Red*/, OutputBit(2, 0, 7) /*Yellow*/, OutputBit(2, 1, 0) /*Green*/);
    mast2S_drv.addHead(OutputBit(2, 1, 1) /*H2SB Red*/, OutputBit(2, 1, 2) /*Yellow*/, OutputBit(2, 1, 3) /*Green*/);
}

void samplePhysicalInputs(IOBus& bus, uint32_t nowMs) {
    tc1T1_drv.sample(bus, nowMs);
    tc3T1_drv.sample(bus, nowMs);
    tc3BT1_drv.sample(bus, nowMs);
    tc5T1_drv.sample(bus, nowMs);

    sw1_drv.sample(bus);
    sw3_drv.sample(bus);
    sw3B_drv.sample(bus);
    sw5_drv.sample(bus);
}

void drivePhysicalOutputs(IOBus& bus, uint32_t nowMs) {
    sw1_drv.drive(bus);
    sw3_drv.drive(bus);
    sw3B_drv.drive(bus);
    sw5_drv.drive(bus);

    mast2N_drv.drive(bus, nowMs);
    mast2S_drv.drive(bus, nowMs);
}

// Stand-in MockCodeLine for Arduino serial debugging until network driver attached
MockCodeLine localDebugLine;

void setup() {
    Serial.begin(115200);
    Wire.begin();

    // Initialize MCP23017 expanders (addresses 0x20 .. 0x24)
    for (uint8_t i = 0; i < 5; ++i) {
        expanders[i].init(i, I2Cexpander::MCP23017, 0xFFFF /* pull-ups */);
    }

    configurePlant();
    configureHardwareDrivers();

    Serial.println(F("CP Christopher FieldUnit Initialized and Running"));
}

void loop() {
    uint32_t nowMs = millis();

    // 1. Sample physical track detectors and limit switches
    samplePhysicalInputs(hardwareBus, nowMs);

    // 2. Ingress & Vital Interlocking Cycle
    executeCycle(localDebugLine, nowMs);

    // 3. Drive physical switch motors and signal LED pins
    drivePhysicalOutputs(hardwareBus, nowMs);

    delay(20); // 50 Hz non-blocking scan rate
}
#endif
