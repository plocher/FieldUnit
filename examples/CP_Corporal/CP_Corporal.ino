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

    // 2. Declare Switches
    sw1 = cp.addSwitch("1"); // Industry lead
    sw3 = cp.addSwitch("3"); // Double track merge
    sw5 = cp.addSwitch("5"); // Derail

    // Bind detector locks
    cp.bindDetectorLock(sw1, tc1T1);
    cp.bindDetectorLock(sw3, tc3T1);
    cp.bindDetectorLock(sw5, tc5T1);

    // 3. Declare Signal Authorities and Masts
    sig2 = cp.addSignalControl("2");
    sig4 = cp.addSignalControl("4");

    mast2NAB = cp.addSignalMast("2NAB", MastType::TWO_HEAD);
    mast2SA  = cp.addSignalMast("2SA",  MastType::DWARF);
    mast4NA  = cp.addSignalMast("4NA",  MastType::ONE_HEAD);
    mast4SA  = cp.addSignalMast("4SA",  MastType::ONE_HEAD);

    // 4. Declare Interlocking Control Table
    // Route 1: Northbound Single Track to MT2 right-hand running (SW1=N, SW3=N)
    cp.route("MT-NB")
      .governedBy(sig2, DirectionAuthority::LEFT)
      .displays(mast2NAB, 0 /* Top Head */, Indication::CLEAR)
      .aligns({ {sw1, SwitchPosition::NORMAL}, 
                {sw3, SwitchPosition::NORMAL} })
      .clears({ tc3T1, tc1T1, tc2SAT });

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

    // 5. Configure Wire Codec (AAR Symbolic CodeLine)
    codec.decodeControls({
        decodeSwitch(sw1),   // 1NWS, 1RWS
        decodeSwitch(sw3),   // 3NWS, 3RWS
        decodeSwitch(sw5),   // 5NWS, 5RWS
        decodeSignal(sig2),  // 2SGS, 2NGS, 2HS
        decodeSignal(sig4),  // 4SGS, 4NGS, 4HS
        decodeMaintainer(0)  // MC1S
    });

    codec.encodeIndications({
        encodeSwitch(sw1),   // 1NWK, 1RWK
        encodeSwitch(sw3),   // 3NWK, 3RWK
        encodeSwitch(sw5),   // 5NWK, 5RWK
        encodeTrack(tc1T1),  // 1T1K
        encodeTrack(tc3T1),  // 3T1K
        encodeTrack(tc5T1),  // 5T1K
        encodeTrack(tc1NAT), // 1NATK
        encodeTrack(tc2NAT), // 2NATK
        encodeTrack(tc1SAT), // 1SATK
        encodeTrack(tc2SAT), // 2SATK
        encodeSignal(sig2),  // 2SGK, 2NGK, 2TEK
        encodeSignal(sig4),  // 4SGK, 4NGK, 4TEK
        encodeMaintainer(0)  // MC1K
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
void setup() {
    Serial.begin(115200);
    configurePlant();
}

void loop() {
    // executeCycle(transport, millis());
}
#endif
