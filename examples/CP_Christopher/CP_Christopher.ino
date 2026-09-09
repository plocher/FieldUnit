/*
 * CP_Christopher - Southern Pacific Coast Line MP 81
 * Double-track mainline with high-speed facing-point crossover
 * and industry spur for Christopher Ranch.
 *
 * Wire schema:
 *   Controls (2 bytes):
 *     Byte 0: 1NW(b0), 1RW(b1), 3NW(b2), 3RW(b3), 3BNW(b4), 3BRW(b5), 5NW(b6), 5RW(b7)
 *     Byte 1: 2SG(b0), 2NG(b1), 2H(b2), MC1(b5), MC2(b6)
 *   Indications (4 bytes):
 *     Byte 0: 1NWK(b0), 1RWK(b1), 3NWK(b2), 3RWK(b3), 3BNWK(b4), 3BRWK(b5), 5NWK(b6), 5RWK(b7)
 *     Byte 1: 1T1(b0), 3T1(b1), 3BT1(b2), 5T1(b3), 1SA(b4), 2SA(b5), 1NA(b6), 2NA(b7)
 *     Byte 2: 2SGK(b0), 2NGK(b1), 2TEK(b2), MC1(b4), MC2(b5)
 *     Byte 3: IND(b0)
 */

#include <FieldUnit.h>

using namespace FieldUnit;

// Control Point Instance
ControlPoint cp("CP_Christopher");
CodeLineCodec codec(2, 4);

// Appliance handles
TrackCircuit* tc1T1;
TrackCircuit* tc3T1;
TrackCircuit* tc3BT1;
TrackCircuit* tc5T1;
TrackCircuit* tc1SA;
TrackCircuit* tc2SA;
TrackCircuit* tc1NA;
TrackCircuit* tc2NA;
TrackCircuit* tcIND;

Switch* sw1;
Switch* sw3;
Switch* sw3B;
Switch* sw5;

SignalControl* sig2;
SignalMast* mast2N;
SignalMast* mast2S;
SignalMast* mast2Nc;

void configurePlant() {
    // 1. Declare Track Circuits
    tc1T1  = cp.addTrackCircuit("1T1");  // SW1 OS
    tc3T1  = cp.addTrackCircuit("3T1");  // SW3 OS (MT1)
    tc3BT1 = cp.addTrackCircuit("3BT1"); // SW3B OS (MT2)
    tc5T1  = cp.addTrackCircuit("5T1");  // SW5 OS
    tc1SA  = cp.addTrackCircuit("1SA");  // MT1 Northbound approach
    tc2SA  = cp.addTrackCircuit("2SA");  // MT2 Southbound approach
    tc1NA  = cp.addTrackCircuit("1NA");  // MT1 Southbound exit/approach
    tc2NA  = cp.addTrackCircuit("2NA");  // MT2 Northbound exit/approach
    tcIND  = cp.addTrackCircuit("IND");  // Industry spur

    // Initial baseline: all circuits initialized clear
    tc1T1->update(Occupancy::VACANT);
    tc3T1->update(Occupancy::VACANT);
    tc3BT1->update(Occupancy::VACANT);
    tc5T1->update(Occupancy::VACANT);
    tc1SA->update(Occupancy::VACANT);
    tc2SA->update(Occupancy::VACANT);
    tc1NA->update(Occupancy::VACANT);
    tc2NA->update(Occupancy::VACANT);
    tcIND->update(Occupancy::VACANT);

    // 2. Declare Switches & Crossover Pairing
    sw1  = cp.addSwitch("SW1");
    sw3  = cp.addSwitch("SW3");
    sw3B = cp.addSwitch("SW3B");
    sw5  = cp.addSwitch("SW5");

    sw3->pairCrossover(sw3B); // SW3 and SW3B move and lock in unison

    cp.bindDetectorLock(sw1, tc1T1);
    cp.bindDetectorLock(sw3, tc3T1);
    cp.bindDetectorLock(sw3B, tc3BT1);
    cp.bindDetectorLock(sw5, tc5T1);

    // 3. Declare Signal Control & Masts
    sig2    = cp.addSignalControl("SIG2");
    mast2N  = cp.addSignalMast("2Nab", MastType::TWO_HEAD);
    mast2S  = cp.addSignalMast("2Sab", MastType::TWO_HEAD);
    mast2Nc = cp.addSignalMast("2Nc", MastType::DWARF);

    // 4. Declare Interlocking Control Table
    // Route 1: MT2-MT2 Northbound Straight (SW3B Normal) -> Top head H2NA CLEAR
    cp.route("MT2-MT2-STRAIGHT")
      .governedBy(sig2, DirectionAuthority::LEFT)
      .displays(mast2N, 0, Indication::CLEAR)
      .aligns({ {sw3B, SwitchPosition::NORMAL} })
      .clears({ tc3BT1 })
      .approaching(tc2SA);

    // Route 2: MT2-MT1 Northbound Crossover (SW3/3B Reverse) -> Lower head H2NB DIVERGING_CLEAR
    cp.route("MT2-MT1-CROSSOVER")
      .governedBy(sig2, DirectionAuthority::LEFT)
      .displays(mast2N, 1, Indication::DIVERGING_CLEAR)
      .aligns({ {sw3, SwitchPosition::REVERSE},
                {sw3B, SwitchPosition::REVERSE},
                {sw1, SwitchPosition::NORMAL} })
      .clears({ tc3BT1, tc3T1, tc1T1 })
      .approaching(tc1SA);

    // Route 3: MT1-MT1 Southbound Straight (SW1=N, SW3=N, SW5=N) -> Top head H2SA CLEAR
    cp.route("MT1-MT1-STRAIGHT")
      .governedBy(sig2, DirectionAuthority::RIGHT)
      .displays(mast2S, 0, Indication::CLEAR)
      .aligns({ {sw1, SwitchPosition::NORMAL},
                {sw3, SwitchPosition::NORMAL},
                {sw5, SwitchPosition::NORMAL} })
      .clears({ tc1T1, tc3T1, tc5T1 })
      .approaching(tc1NA);

    // Route 4: MT1-MT2 Southbound Crossover (SW1=N, SW3/3B=R) -> Lower head H2SB DIVERGING_CLEAR
    cp.route("MT1-MT2-CROSSOVER")
      .governedBy(sig2, DirectionAuthority::RIGHT)
      .displays(mast2S, 1, Indication::DIVERGING_CLEAR)
      .aligns({ {sw1, SwitchPosition::NORMAL},
                {sw3, SwitchPosition::REVERSE},
                {sw3B, SwitchPosition::REVERSE} })
      .clears({ tc1T1, tc3T1 })
      .approaching(tc2NA);

    // 5. Configure Wire Codec
    codec.mapSwitchControl(0, 0, 0, 0, 1); // SW1: b0=1NW, b1=1RW
    codec.mapSwitchControl(1, 0, 2, 0, 3); // SW3: b2=3NW, b3=3RW
    codec.mapSwitchControl(2, 0, 4, 0, 5); // SW3B: b4=3BNW, b5=3BRW
    codec.mapSwitchControl(3, 0, 6, 0, 7); // SW5: b6=5NW, b7=5RW
    codec.mapSignalControl(0, 1, 0, 1, 2); // SIG2: b0=2SG, b1=2NG, b2=2H
    codec.mapMaintainerControl(0, 1, 5);   // MC1: b5
    codec.mapMaintainerControl(1, 1, 6);   // MC2: b6

    codec.mapSwitchIndication(0, 0, 0, 0, 1); // 1NWK, 1RWK
    codec.mapSwitchIndication(1, 0, 2, 0, 3); // 3NWK, 3RWK
    codec.mapSwitchIndication(2, 0, 4, 0, 5); // 3BNWK, 3BRWK
    codec.mapSwitchIndication(3, 0, 6, 0, 7); // 5NWK, 5RWK

    codec.mapTrackIndication(0, 1, 0); // 1T1
    codec.mapTrackIndication(1, 1, 1); // 3T1
    codec.mapTrackIndication(2, 1, 2); // 3BT1
    codec.mapTrackIndication(3, 1, 3); // 5T1
    codec.mapTrackIndication(4, 1, 4); // 1SA
    codec.mapTrackIndication(5, 1, 5); // 2SA
    codec.mapTrackIndication(6, 1, 6); // 1NA
    codec.mapTrackIndication(7, 1, 7); // 2NA
    codec.mapTrackIndication(8, 3, 0); // IND

    codec.mapSignalIndication(0, 2, 0, 1, 2); // 2SGK, 2NGK, 2TEK
}

// Complete single-cycle execution: Ingress -> Vital Cycle -> Egress
void executeCycle(CodeLine& line, uint32_t nowMs) {
    uint8_t rxBuffer[16];
    size_t bytesRead = 0;

    // 1. Ingress: Poll CodeLine for incoming dispatcher control packet
    if (line.receiveControlPacket(rxBuffer, sizeof(rxBuffer), bytesRead)) {
        ControlTransaction ctl;
        if (codec.unpackControls(rxBuffer, bytesRead, ctl)) {
            cp.applyControlTransaction(ctl, nowMs);
        }
    }

    // 2. Vital Cycle: Advance timers, check locks, evaluate control table
    cp.tick(nowMs);

    // 3. Egress: Export verified plant state and transmit indications
    IndicationVector ind;
    cp.exportIndicationVector(ind);
    uint8_t txBuffer[16];
    if (codec.packIndications(ind, txBuffer, sizeof(txBuffer))) {
        line.transmitIndicationPacket(txBuffer, codec.expectedIndicationBytes());
    }
}

#ifdef ARDUINO
void setup() {
    Serial.begin(115200);
    configurePlant();
}

void loop() {
    // In Arduino environment, driven by real CodeLine transport (CMRInet or MQTT)
    // executeCycle(transport, millis());
}
#endif
