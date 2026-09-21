/*
 * spcoast_ctc - SPCoast South Dispatcher cTc Desk
 * 
 * Controls 7 Control Points (Gilroy through Watsonville staging)
 * US&S Model 503 physical panel across 14 columns.
 * 
 * Select physical I/O backend:
 */
#include <FieldUnit.h>

#include "IO-I2C.h"
// #include "IO-CMRI.h"

using namespace FieldUnit;

#if defined(ARDUINO) && defined(ESP32)
#define USE_OTA
#define USE_OLED
#endif

#ifdef USE_OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 oled(128, 64, &Wire);
bool oledAlive = false;
uint32_t lastOledMs = 0;
uint8_t oledAnim = 0;
const char spinnerChars[] = "|/-\\\\";

char oledStatusLine[24] = "I2C Init...";
char oledMqttLine[24]   = "MQTT Waiting...";
char oledLastCoded[24]  = "Ready for Levers";

void updateOled() {
    if (!oledAlive) return;
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);

    // Header with live spinner
    oled.setCursor(0, 0);
    oled.printf("SPCoast cTc [%c]", spinnerChars[oledAnim % 4]);
    oled.drawFastHLine(0, 10, 128, SSD1306_WHITE);

    // Line 2 (WiFi Status / IP)
    oled.setCursor(0, 16);
    oled.print(oledStatusLine);

    // Line 3 (MQTT Status)
    oled.setCursor(0, 32);
    oled.print(oledMqttLine);

    // Line 4 (Last Coded Station)
    oled.setCursor(0, 48);
    oled.print(oledLastCoded);

    oled.display();
}
#endif

// Networking: WiFi and MQTT are mandatory for spcoast_ctc
#if __has_include("secrets.h")
#include "secrets.h"
#endif

#ifndef WIFI_SSID
#define WIFI_SSID       "your-wifi-ssid"
#define WIFI_PASSWORD   "your-wifi-password"
#define MQTT_SERVER     "************"
#define MQTT_PORT       1883
#endif

#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

#ifdef USE_OTA
#include "ota.h"
OtaManager ota;
#endif

PanelIO hardware;
cTcMachine machine(hardware);

// =============================================================================
// CodeLine Stepping & Traffic Lamp Repurposing (Column 3)
// - N-Lamp (TRACK_LAMP_1, Position 1, Blue): Inbound Indication Code
// - S-Lamp (TRACK_LAMP_2, Position 2, Blue): Outbound Control Code
// =============================================================================

static constexpr uint8_t CODELINE_COL = 3;
static constexpr uint8_t CODELINE_STEPS = 15;
static constexpr uint8_t CODELINE_ADDRESS_STEPS = 4;
static constexpr uint8_t CODELINE_TX_QUEUE_SIZE = 4;
static constexpr uint8_t CODELINE_RX_QUEUE_SIZE = 8;
static constexpr uint32_t CODELINE_LONG_PULSE_MS = 350;
static constexpr uint32_t CODELINE_SHORT_PULSE_MS = 110;
static constexpr uint32_t CODELINE_SPACE_MS = 25;

// A single 15-step US&S Form 506-style line-code display cycle.
// Long pulses represent asserted functions; short pulses represent unasserted
// functions. The last step is the execution / completion pulse.
struct CodeLineCycle {
    bool active = false;
    bool inbound = false;
    bool lampOn = false;
    uint8_t step = 0;
    bool asserted[CODELINE_STEPS] = {};
    uint32_t phaseStartMs = 0;
};

struct PendingControl {
    bool active = false;
    size_t stationIndex = 0;
    char tokens[512] = "";
};

struct PendingIndication {
    bool active = false;
    char stationName[32] = "";
    char tokens[512] = "";
};

CodeLineCycle codelineCycle;
PendingControl txQueue[CODELINE_TX_QUEUE_SIZE];
uint8_t txQueueHead = 0;
uint8_t txQueueTail = 0;
uint8_t txQueueCount = 0;
PendingControl activeTx;
uint8_t activeTxColumn = 0;

PendingIndication rxQueue[CODELINE_RX_QUEUE_SIZE];
uint8_t rxQueueHead = 0;
uint8_t rxQueueTail = 0;
uint8_t rxQueueCount = 0;
PendingIndication activeRx;
uint8_t activeRxColumn = 0;

uint32_t coldStartAlignUntilMs = 0;

bool tokenIsAsserted(const char* tokenText, const char* expectedToken) {
    if (!tokenText || !expectedToken) return false;
    const char* p = tokenText;

    while (*p) {
        while (*p == ',' || *p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
        if (!*p) break;

        bool asserted = true;
        if (*p == '(') {
            asserted = false;
            p++;
        }

        char token[24];
        uint8_t n = 0;
        while (*p && *p != ',' && *p != ')' && n + 1 < sizeof(token)) {
            if (*p != ' ' && *p != '\t') token[n++] = *p;
            p++;
        }
        token[n] = '\0';
        if (*p == ')') p++;

        if (strcmp(token, expectedToken) == 0) return asserted;
        while (*p && *p != ',') p++;
    }
    return false;
}

void setCodeLineLamp(bool inbound, bool on) {
    hardware.write(
        CODELINE_COL,
        inbound ? FieldUnit::PanelOutput::TRACK_LAMP_1 : FieldUnit::PanelOutput::TRACK_LAMP_2,
        on
    );
}

void setCodeLineFunctionBit(
    uint8_t step,
    const char* appliance,
    const char* suffix,
    const char* tokens
) {
    if (!appliance || !suffix || !tokens || step >= CODELINE_STEPS) return;
    char expected[24];
    snprintf(expected, sizeof(expected), "%s%s", appliance, suffix);
    codelineCycle.asserted[step] = tokenIsAsserted(tokens, expected);
}

void buildCodeLineCycle(
    bool inbound,
    const PanelColumn& column,
    const char* tokens,
    uint32_t nowMs
) {
    codelineCycle = CodeLineCycle();
    codelineCycle.active = true;
    codelineCycle.inbound = inbound;
    codelineCycle.lampOn = true;
    codelineCycle.phaseStartMs = nowMs;

    // Step 1: line clear / synchronization; Steps 2-5: 4-bit physical panel-column address.
    codelineCycle.asserted[0] = true;
    uint8_t columnAddress = column.columnNumber();
    for (uint8_t bit = 0; bit < CODELINE_ADDRESS_STEPS; ++bit) {
        codelineCycle.asserted[1 + bit] = (columnAddress & (1 << (CODELINE_ADDRESS_STEPS - 1 - bit))) != 0;
    }

    // Steps 6-14: actual control or indication functions wired on this panel column.
    if (column.hasSwitch()) {
        setCodeLineFunctionBit(5, column.switchNum(), inbound ? "NWK" : "NWS", tokens);
        setCodeLineFunctionBit(6, column.switchNum(), inbound ? "RWK" : "RWS", tokens);
    }
    if (column.hasSignal()) {
        setCodeLineFunctionBit(7, column.signalNum(), inbound ? "NGK" : "NGS", tokens);
        setCodeLineFunctionBit(8, column.signalNum(), inbound ? "TEK" : "HS", tokens);
        setCodeLineFunctionBit(9, column.signalNum(), inbound ? "SGK" : "SGS", tokens);
    }
    if (inbound) {
        for (uint8_t i = 0; i < column.trackCount() && i < 3; ++i) {
            setCodeLineFunctionBit(10 + i, column.trackName(i), "K", tokens);
        }
    }
    if (column.hasMaintainerCall()) {
        setCodeLineFunctionBit(13, column.maintainerNum(), inbound ? "K" : "S", tokens);
    }

    // Step 15: execution pulse.
    codelineCycle.asserted[14] = true;
    setCodeLineLamp(inbound, true);
}

bool enqueueControl(size_t stationIndex, const char* tokens) {
    if (!tokens || txQueueCount >= CODELINE_TX_QUEUE_SIZE) return false;
    PendingControl& pending = txQueue[txQueueTail];
    pending.active = true;
    pending.stationIndex = stationIndex;
    strncpy(pending.tokens, tokens, sizeof(pending.tokens) - 1);
    pending.tokens[sizeof(pending.tokens) - 1] = '\0';
    txQueueTail = (txQueueTail + 1) % CODELINE_TX_QUEUE_SIZE;
    txQueueCount++;
    return true;
}

bool enqueueIndication(const char* stationName, const char* tokens) {
    if (!stationName || !tokens || rxQueueCount >= CODELINE_RX_QUEUE_SIZE) return false;
    PendingIndication& pending = rxQueue[rxQueueTail];
    pending.active = true;
    strncpy(pending.stationName, stationName, sizeof(pending.stationName) - 1);
    pending.stationName[sizeof(pending.stationName) - 1] = '\0';
    strncpy(pending.tokens, tokens, sizeof(pending.tokens) - 1);
    pending.tokens[sizeof(pending.tokens) - 1] = '\0';
    rxQueueTail = (rxQueueTail + 1) % CODELINE_RX_QUEUE_SIZE;
    rxQueueCount++;
    return true;
}

void startNextControlCycle(uint32_t nowMs) {
    if (codelineCycle.active || activeTx.active) return;
    if (txQueueCount == 0) return;

    activeTx = txQueue[txQueueHead];
    txQueue[txQueueHead].active = false;
    txQueueHead = (txQueueHead + 1) % CODELINE_TX_QUEUE_SIZE;
    txQueueCount--;
    activeTxColumn = 0;

    const CtcStation& station = machine.station(activeTx.stationIndex);
    buildCodeLineCycle(false, station.column(activeTxColumn), activeTx.tokens, nowMs);
    Serial.printf("CONTROL STEPPING [%s, Column %u/%u]...\n",
                  station.name(), activeTxColumn + 1, station.columnCount());
}

void startNextIndicationCycle(uint32_t nowMs) {
    if (codelineCycle.active || activeRx.active) return;
    if (rxQueueCount == 0) return;

    activeRx = rxQueue[rxQueueHead];
    rxQueue[rxQueueHead].active = false;
    rxQueueHead = (rxQueueHead + 1) % CODELINE_RX_QUEUE_SIZE;
    rxQueueCount--;
    activeRxColumn = 0;

    CtcStation* station = machine.findStation(activeRx.stationName);
    if (!station) {
        Serial.printf("INDICATION DROPPED: unknown station %s\n", activeRx.stationName);
        activeRx.active = false;
        return;
    }

    buildCodeLineCycle(true, station->column(activeRxColumn), activeRx.tokens, nowMs);
    Serial.printf("INDICATION STEPPING [%s, Column %u/%u]...\n",
                  station->name(), activeRxColumn + 1, station->columnCount());
}

void completeCodeLineCycle(uint32_t nowMs) {
    bool inbound = codelineCycle.inbound;
    setCodeLineLamp(inbound, false);
    codelineCycle.active = false;

    if (!inbound) {
        const CtcStation& station = machine.station(activeTx.stationIndex);
        activeTxColumn++;
        if (activeTxColumn < station.columnCount()) {
            buildCodeLineCycle(false, station.column(activeTxColumn), activeTx.tokens, nowMs);
            Serial.printf("CONTROL STEPPING [%s, Column %u/%u]...\n",
                          station.name(), activeTxColumn + 1, station.columnCount());
            return;
        }

        char topic[128];
        snprintf(topic, sizeof(topic), "ctc/SPCoast/codeline/%s/controls", station.name());
        if (mqtt.connected()) {
            mqtt.publish(topic, activeTx.tokens);
            Serial.printf("CONTROL TX COMPLETE [%s]: %s\n", station.name(), activeTx.tokens);
        } else {
            Serial.printf("CONTROL TX FAILED [%s]: MQTT disconnected\n", station.name());
        }
        activeTx.active = false;
        startNextIndicationCycle(nowMs);
        startNextControlCycle(nowMs);
        return;
    }

    CtcStation* station = machine.findStation(activeRx.stationName);
    if (!station) {
        activeRx.active = false;
        startNextIndicationCycle(nowMs);
        return;
    }

    activeRxColumn++;
    if (activeRxColumn < station->columnCount()) {
        buildCodeLineCycle(true, station->column(activeRxColumn), activeRx.tokens, nowMs);
        Serial.printf("INDICATION STEPPING [%s, Column %u/%u]...\n",
                      station->name(), activeRxColumn + 1, station->columnCount());
        return;
    }

    bool ok = machine.applyIndications(activeRx.stationName, activeRx.tokens);
    Serial.printf(ok ? "INDICATION: %s: %s\n" : "INDICATION REJECTED %s: %s\n",
                  activeRx.stationName, activeRx.tokens);
    activeRx.active = false;
    startNextIndicationCycle(nowMs);
    startNextControlCycle(nowMs);
}

void updateCodeLine(uint32_t nowMs) {
    if (!codelineCycle.active) {
        // Prototype priority: receive indications before office control traffic.
        startNextIndicationCycle(nowMs);
        if (!codelineCycle.active) startNextControlCycle(nowMs);
        return;
    }

    uint32_t duration = codelineCycle.lampOn
        ? (codelineCycle.asserted[codelineCycle.step] ? CODELINE_LONG_PULSE_MS : CODELINE_SHORT_PULSE_MS)
        : CODELINE_SPACE_MS;
    if (nowMs - codelineCycle.phaseStartMs < duration) return;

    codelineCycle.phaseStartMs = nowMs;
    if (codelineCycle.lampOn) {
        codelineCycle.lampOn = false;
        setCodeLineLamp(codelineCycle.inbound, false);
        return;
    }

    codelineCycle.step++;
    if (codelineCycle.step >= CODELINE_STEPS) {
        completeCodeLineCycle(nowMs);
        return;
    }

    codelineCycle.lampOn = true;
    setCodeLineLamp(codelineCycle.inbound, true);
}

void configureDesk() {
    // Column 1..2: CP_GilroyCaltrain (Yard terminal: No MC)
    machine.addStation("CP_GilroyCaltrain")
        .inColumn(1).withSwitch("1").withTrackLamps({ "1T1", "EA1" })
        .inColumn(2).withSwitch("3").withTrackLamps({ "TK1", "TK2", "TK3" }).withCodeButton();

    // Column 3..4: CP_GilroyInterchange (MC1 on Col 3, MC2 on Col 4)
    // Note: Column 3 traffic lamps (Position 1 & 2) repurposed for CodeLine Control/Indication pulses!
    machine.addStation("CP_GilroyInterchange")
        .inColumn(3).withSwitch("1").withMaintainerCall("1")
        .inColumn(4).withSwitch("3").withTrackLamps({ "1T1", "3T1", "TK1" }).withMaintainerCall("2").withCodeButton();

    // Column 5..7: CP_Luchessa (Signal 2 on Col 5, MC1 on Col 6)
    machine.addStation("CP_Luchessa")
        .inColumn(5).withSwitch("1").withSignal("2").withTrackLamps({ "1T1" })
        .inColumn(6).withSwitch("3").withTrackLamps({ "3T1" }).withMaintainerCall("1")
        .inColumn(7).withSwitch("5").withCodeButton();

    // Column 8..10: CP_Christopher (MC1 on Col 8, MC2 on Col 10)
    machine.addStation("CP_Christopher")
        .inColumn(8).withSwitch("1").withTrackLamps({ "1T1", "1WA", "2WA" }).withMaintainerCall("1")
        .inColumn(9).withSwitch("3").withSignal("2").withTrackLamps({ "3T1", "3BT1", "5T1" })
        .inColumn(10).withSwitch("5").withTrackLamps({ "1EA", "2EA" }).withMaintainerCall("2").withCodeButton();

    // Column 11..12: CP_Corporal (MC1 on Col 11)
    machine.addStation("CP_Corporal")
        .inColumn(11).withSwitch("1").withSignal("2").withTrackLamps({ "1EA", "1T1", "3T1" }).withMaintainerCall("1")
        .inColumn(12).withSwitch("3").withTrackLamps({ "SDT", "TL", "TR" }).withCodeButton();

    // Column 13: CP_Sargent (MC1 on Col 13)
    machine.addStation("CP_Sargent")
        .inColumn(13).withSwitch("1").withTrackLamps({ "1T1", "HBD" }).withMaintainerCall("1").withCodeButton();

    // Column 14: CP_Watsonville (Staging yard: No MC)
    machine.addStation("CP_Watsonville")
        .inColumn(14).withSwitch("1").withSignal("2").withTrackLamps({ "ALT", "EAT", "SAT" }).withCodeButton();
}

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
    // Topic: ctc/SPCoast/codeline/<stationName>/indications
    const char* prefix = "codeline/";
    const char* p = strstr(topic, prefix);
    if (!p) return;
    p += strlen(prefix);
    const char* slash = strchr(p, '/');
    if (!slash) return;

    char stationName[32];
    size_t stLen = slash - p;
    if (stLen >= sizeof(stationName)) return;
    memcpy(stationName, p, stLen);
    stationName[stLen] = '\0';

    char msgBuf[512];
    size_t copyLen = (length < sizeof(msgBuf) - 1) ? length : sizeof(msgBuf) - 1;
    memcpy(msgBuf, payload, copyLen);
    msgBuf[copyLen] = '\0';

    uint32_t now = millis();
    // Cold-start align: apply immediately during initial connect window so all 7 CPs align instantly
    if (now < coldStartAlignUntilMs) {
        bool ok = machine.applyIndications(stationName, msgBuf);
        if (ok) {
            Serial.printf("COLD-START ALIGN [%s]: %s\n", stationName, msgBuf);
        }
        return;
    }

    // Normal operation: queue full indication truth for a per-column 15-step receive cycle.
    if (!enqueueIndication(stationName, msgBuf)) {
        Serial.printf("INDICATION DROPPED [%s]: receive queue full\n", stationName);
    }
}

void reconnectMqtt(uint32_t nowMs) {
    static uint32_t lastReconnectMs = 0;
    if (nowMs - lastReconnectMs < 5000) return;
    lastReconnectMs = nowMs;

    if (mqtt.connect("ctc-desk-south", "ctc/SPCoast/telemetry", 1, true, "OFFLINE")) {
        coldStartAlignUntilMs = nowMs + 2000; // Allow 2.0s for initial retained indications to align instantly
        mqtt.publish("ctc/SPCoast/telemetry", "ONLINE", true);
        mqtt.subscribe("ctc/SPCoast/codeline/+/indications");
        Serial.println("MQTT connected. Subscribed to plant indications.");
#ifdef USE_OLED
        snprintf(oledMqttLine, sizeof(oledMqttLine), "MQTT: Connected");
#endif
    }
}

#ifdef ARDUINO
void setup() {
    Serial.begin(115200);
    Wire.begin();
    Wire.setClock(800000UL); // 800 kHz Fast-Mode Plus

#ifdef USE_OLED
    Wire.beginTransmission(0x3C);
    if (Wire.endTransmission() == 0) {
        oledAlive = oled.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false);
        if (oledAlive) {
            oled.clearDisplay();
            snprintf(oledStatusLine, sizeof(oledStatusLine), "I2C 14 Devs OK");
            updateOled();
        }
    }
#endif

    hardware.begin();

    configureDesk();
    machine.begin(); // Preallocates Strategy B exact buffers and builds canonical AAR schemas

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    wifiClient.setConnectionTimeout(300);
    mqtt.setServer(MQTT_SERVER, MQTT_PORT);
    mqtt.setCallback(onMqttMessage);

#ifdef USE_OTA
    ota.begin("spcoast-ctc", WIFI_SSID, WIFI_PASSWORD);
#endif

    Serial.println("SPCoast CTC Machine initialized.");
}

void loop() {
    uint32_t nowMs = millis();

#ifdef USE_OLED
    // Throttle OLED refresh: only once per second for heartbeat/IP, or immediately on events
    if (nowMs - lastOledMs >= 1000) {
        lastOledMs = nowMs;
        oledAnim++;
        if (WiFi.status() == WL_CONNECTED) {
            snprintf(oledStatusLine, sizeof(oledStatusLine), "%s", WiFi.localIP().toString().c_str());
        } else {
            snprintf(oledStatusLine, sizeof(oledStatusLine), "WiFi Connecting...");
        }
        updateOled();
    }
#endif

#ifdef USE_OTA
    ota.poll();
#endif

    if (WiFi.status() == WL_CONNECTED) {
        if (!mqtt.connected()) {
            reconnectMqtt(nowMs);
        } else {
            mqtt.loop();
        }
    }

    hardware.syncInputs();

    // Strategy B: tokens point at station preallocated buffer (sized in machine.begin())
    size_t stIdx = 0;
    const char* txTokens = nullptr;
    if (machine.pollCode(stIdx, txTokens)) {
        const char* targetCp = machine.station(stIdx).name();
        Serial.printf("CONTROL QUEUED [%s]: %s\n", targetCp, txTokens);

#ifdef USE_OLED
        snprintf(oledLastCoded, sizeof(oledLastCoded), "CODING: %s", targetCp);
        updateOled();
#endif

        if (!enqueueControl(stIdx, txTokens)) {
            Serial.printf("CONTROL DROPPED [%s]: transmit queue full\n", targetCp);
        }
    }

    // Use a fresh timestamp because mqtt.loop() can enqueue an indication cycle.
    updateCodeLine(millis());

    hardware.syncOutputs();
}
#endif
