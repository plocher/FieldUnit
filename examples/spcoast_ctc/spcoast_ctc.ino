/*
 * spcoast_ctc - SPCoast South Dispatcher cTc Desk
 * 
 * Controls 7 Control Points (Gilroy through Watsonville staging)
 * US&S Model 503 physical panel across 14 columns.
 * 
 * Select physical I/O backend:
 */
#include "IO-I2C.h"
// #include "IO-CMRI.h"

#include <FieldUnit.h>

using namespace FieldUnit;

PanelIO hardware;
cTcMachine machine(hardware);
AarTextCodec codec;
MqttCodeLine codeLine("************", 1883, "SPCoast", "ctc-office-south");

void configureDesk() {
    // Column 1..2: CP_GilroyCaltrain
    machine.addStation("CP_GilroyCaltrain")
        .inColumn(1).withSwitch("1").withTrackLamps({ "1T1", "EA1" })
        .inColumn(2).withSwitch("3").withTrackLamps({ "TK1", "TK2", "TK3" }).withCodeButton();

    // Column 3..4: CP_GilroyInterchange
    machine.addStation("CP_GilroyInterchange")
        .inColumn(3).withSwitch("1").withTrackLamps({ "1T1", "3T1", "TK1" })
        .inColumn(4).withSwitch("3").withTrackLamps({ "EA1", "TL", "TR" }).withCodeButton();

    // Column 5..7: CP_Luchessa
    machine.addStation("CP_Luchessa")
        .inColumn(5).withSwitch("1").withTrackLamps({ "1T1" })
        .inColumn(6).withSwitch("3").withSignal("2").withTrackLamps({ "3T1" })
        .inColumn(7).withSwitch("5").withCodeButton();

    // Column 8..10: CP_Christopher
    machine.addStation("CP_Christopher")
        .inColumn(8).withSwitch("1").withTrackLamps({ "1T1", "1WA", "2WA" })
        .inColumn(9).withSwitch("3").withSignal("2").withTrackLamps({ "3T1", "3BT1", "5T1" })
        .inColumn(10).withSwitch("5").withTrackLamps({ "1EA", "2EA" }).withCodeButton();

    // Column 11..12: CP_Corporal
    machine.addStation("CP_Corporal")
        .inColumn(11).withSwitch("1").withSignal("2").withTrackLamps({ "1EA", "1T1", "3T1" })
        .inColumn(12).withSwitch("3").withTrackLamps({ "SDT", "TL", "TR" }).withCodeButton();

    // Column 13: CP_Sargent
    machine.addStation("CP_Sargent")
        .inColumn(13).withSwitch("1").withTrackLamps({ "1T1", "HBD" }).withCodeButton();

    // Column 14: CP_Watsonville
    machine.addStation("CP_Watsonville")
        .inColumn(14).withSwitch("1").withSignal("2").withTrackLamps({ "ALT", "EAT", "SAT" });
}

#ifdef ARDUINO
void setup() {
    hardware.begin();
    configureDesk();
    codeLine.begin();
    codeLine.subscribeIndications();
}

void loop() {
    uint32_t nowMs = millis();
    codeLine.tick(nowMs);

    // 1. Ingress: Update panel lamps from field indications
    char cpName[32];
    char rxPayload[256];
    size_t rxLen = 0;
    while (codeLine.receiveIndication(cpName, sizeof(cpName), rxPayload, sizeof(rxPayload), rxLen)) {
        machine.applyIndications(cpName, rxPayload);
    }

    // 2. Egress: Check code buttons across all stations
    hardware.syncInputs();

    size_t stIdx = 0;
    char txTokens[256];
    if (machine.pollCode(stIdx, txTokens, sizeof(txTokens))) {
        const char* targetCp = machine.station(stIdx).name();
        codeLine.transmitControls(targetCp, txTokens, strlen(txTokens));
    }

    hardware.syncOutputs();
}
#endif
