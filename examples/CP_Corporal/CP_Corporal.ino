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
CodeLineCodec codec(2, 4);

// Appliance handles
TrackCircuit* tc1T1;
TrackCircuit* tc3T1;
TrackCircuit* tc5T1;
TrackCircuit* tc1NAT;
TrackCircuit* tc2NAT;
TrackCircuit* tc1SAT;
TrackCircuit* tc2SAT;

Switch* sw1; // North industry lead
Switch* sw3; // Double-track convergence
Switch* sw5; // Industry safety derail (slaved inversely to SW1)

SignalControl* sig2;
SignalControl* sig4;

SignalMast* mast2NAB; // Northbound entry from single track to double track (2-Head)
SignalMast* mast2SA;  // Southbound exit from MT1 to single track (Dwarf)
SignalMast* mast4NA;  // Northbound exit from industry lead (1-Head)
SignalMast* mast4SA;  // Southbound reverse move into industry (1-Head)

void configurePlant() {
    // 1. Declare Track Circuits
    tc1T1  = cp.addTrackCircuit("1T1");  // SW1 OS
    tc3T1  = cp.addTrackCircuit("3T1");  // SW3 OS
    tc5T1  = cp.addTrackCircuit("5T1");  // SW5 Derail OS
    tc1NAT = cp.addTrackCircuit("1NAT"); // Single track approach
    tc2NAT = cp.addTrackCircuit("2NAT"); // Single track advance
    tc1SAT = cp.addTrackCircuit("1SAT"); // MT1 Southbound approach
    tc2SAT = cp.addTrackCircuit("2SAT"); // MT2 Northbound exit block

    // Initial baseline: all circuits initialized clear
    tc1T1->update(Occupancy::VACANT);
    tc3T1->update(Occupancy::VACANT);
    tc5T1->update(Occupancy::VACANT);
    tc1NAT->update(Occupancy::VACANT);
    tc2NAT->update(Occupancy::VACANT);
    tc1SAT->update(Occupancy::VACANT);
    tc2SAT->update(Occupancy::VACANT);

    // 2. Declare Switches
    sw1 = cp.addSwitch("SW1"); // Industry lead
    sw3 = cp.addSwitch("SW3"); // Double track merge
    sw5 = cp.addSwitch("SW5"); // Derail

    // Bind detector locks
    cp.bindDetectorLock(sw1, tc1T1);
    cp.bindDetectorLock(sw3, tc3T1);
    cp.bindDetectorLock(sw5, tc5T1);

    // 3. Declare Signal Authorities and Masts
    sig2 = cp.addSignalControl("SIG2");
    sig4 = cp.addSignalControl("SIG4");

    mast2NAB = cp.addSignalMast("S2NAB", MastType::TWO_HEAD);
    mast2SA  = cp.addSignalMast("S2SA",  MastType::DWARF);
    mast4NA  = cp.addSignalMast("S4NA",  MastType::ONE_HEAD);
    mast4SA  = cp.addSignalMast("S4SA",  MastType::ONE_HEAD);

    // 4. Declare Interlocking Control Table
    // Route 1: Northbound Single Track to MT2 right-hand running (SW1=N, SW3=N)
    cp.route("MT-NB")
      .governedBy(sig2, DirectionAuthority::LEFT)
      .displays(mast2NAB, 0 /* Top Head */, Indication::CLEAR)
      .aligns({ {sw1, SwitchPosition::NORMAL}, 
                {sw3, SwitchPosition::NORMAL} })
      .clears({ tc1T1, tc3T1, tc2SAT });

    // Route 2: Northbound Single Track to MT1 reverse running (SW3=R)
    cp.route("MT-SB")
      .governedBy(sig2, DirectionAuthority::LEFT)
      .displays(mast2NAB, 1 /* Lower Head */, Indication::DIVERGING_RESTRICTING)
      .aligns({ {sw3, SwitchPosition::REVERSE} })
      .clears({ tc3T1, tc1SAT });

    // Route 3: Southbound MT1 through switch 3 onto single track (SW3=R)
    cp.route("SB-MT")
      .governedBy(sig2, DirectionAuthority::RIGHT)
      .displays(mast2SA, 0, Indication::CLEAR)
      .aligns({ {sw3, SwitchPosition::REVERSE} })
      .clears({ tc3T1, tc1NAT })
      .approaching(tc2NAT);

    // Route 4: Northbound Industry track onto MT2 (SW1=R)
    cp.route("IND-NB")
      .governedBy(sig4, DirectionAuthority::LEFT)
      .displays(mast4NA, 0, Indication::RESTRICTING)
      .aligns({ {sw1, SwitchPosition::REVERSE} })
      .clears({ tc1T1, tc5T1, tc1SAT });

    // Route 5: Southbound on MT2 diverging into Industry lead (SW1=R)
    cp.route("SB-IND")
      .governedBy(sig4, DirectionAuthority::RIGHT)
      .displays(mast4SA, 0, Indication::RESTRICTING)
      .aligns({ {sw1, SwitchPosition::REVERSE} })
      .clears({ tc1T1, tc5T1 });

    // 5. Configure Wire Codec
    codec.mapSwitchControl(0, 0, 0, 0, 1); // SW1: b0=1NW, b1=1RW
    codec.mapSwitchControl(1, 0, 2, 0, 3); // SW3: b2=3NW, b3=3RW
    codec.mapSwitchControl(2, 0, 4, 0, 5); // SW5: b4=5NW, b5=5RW
    codec.mapSignalControl(0, 1, 0, 1, 2); // SIG2: b0=2SG, b1=2NG, b2=2H
    codec.mapSignalControl(1, 1, 4, 5, 6); // SIG4: b4=4SG, b5=4NG, b6=4H
    codec.mapMaintainerControl(0, 1, 7);   // MC1: b7

    codec.mapSwitchIndication(0, 0, 0, 0, 1); // 1NWK, 1RWK
    codec.mapSwitchIndication(1, 0, 2, 0, 3); // 3NWK, 3RWK
    codec.mapSwitchIndication(2, 0, 4, 0, 5); // 5NWK, 5RWK

    codec.mapTrackIndication(0, 1, 0); // 1T1
    codec.mapTrackIndication(1, 1, 2); // 3T1
    codec.mapTrackIndication(2, 1, 4); // 5T1
    codec.mapTrackIndication(3, 3, 0); // 1NAT
    codec.mapTrackIndication(4, 3, 1); // 2NAT
    codec.mapTrackIndication(5, 3, 2); // 1SAT
    codec.mapTrackIndication(6, 3, 3); // 2SAT

    codec.mapSignalIndication(0, 2, 0, 1, 2); // 2SGK, 2NGK, 2TEK
    codec.mapSignalIndication(1, 2, 4, 5, 6); // 4SGK, 4NGK, 4TEK
}

void executeCycle(CodeLine& line, uint32_t nowMs) {
    uint8_t rxBuffer[16];
    size_t bytesRead = 0;

    // 1. Ingress
    if (line.receiveControlPacket(rxBuffer, sizeof(rxBuffer), bytesRead)) {
        ControlTransaction ctl;
        if (codec.unpackControls(rxBuffer, bytesRead, ctl)) {
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
    // executeCycle(transport, millis());
}
#endif
