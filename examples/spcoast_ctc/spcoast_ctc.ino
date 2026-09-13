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

// Diagnostics: Uncomment to run raw direct lever-to-lamp mirror (no 1-shot, no MQTT)
// #define TEST_DIRECT_MIRROR

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

#ifdef USE_OTA
#include "ota.h"

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

OtaManager ota;
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);
#endif

PanelIO hardware;
cTcMachine machine(hardware);

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

#ifdef USE_OTA
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

    machine.applyIndications(stationName, msgBuf);
}

void reconnectMqtt(uint32_t nowMs) {
    static uint32_t lastReconnectMs = 0;
    if (nowMs - lastReconnectMs < 5000) return;
    lastReconnectMs = nowMs;

    if (mqtt.connect("ctc-desk-south", "ctc/SPCoast/telemetry", 1, true, "OFFLINE")) {
        mqtt.publish("ctc/SPCoast/telemetry", "ONLINE", true);
        mqtt.subscribe("ctc/SPCoast/codeline/+/indications");
        Serial.println("MQTT connected. Subscribed to plant indications.");
#ifdef USE_OLED
        snprintf(oledMqttLine, sizeof(oledMqttLine), "MQTT: Connected");
#endif
    }
}
#endif

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

    // Visual lamp check: all ON for 2s, all OFF, then column-by-column chase
    hardware.runLampTest();

    configureDesk();
    machine.begin(); // Preallocates Strategy B exact buffers and builds canonical AAR schemas

#ifdef USE_OTA
    ota.begin("spcoast-ctc", WIFI_SSID, WIFI_PASSWORD);
    mqtt.setServer(MQTT_SERVER, MQTT_PORT);
    mqtt.setCallback(onMqttMessage);
#endif

    Serial.println("SPCoast CTC Machine initialized.");
}

void loop() {
    uint32_t nowMs = millis();

#ifdef TEST_DIRECT_MIRROR
    static uint32_t lastReportMs = 0;
    static uint32_t loopCount = 0;
    loopCount++;

    uint32_t t0 = micros();
    hardware.directMirrorLoop();
    uint32_t dt = micros() - t0;

    if (nowMs - lastReportMs >= 1000) {
        lastReportMs = nowMs;
        Serial.printf("[BENCH] 14-col direct mirror: %u us/scan (%u loops/sec)\n", dt, loopCount);
        loopCount = 0;
    }
    return;
#endif

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
    if (WiFi.status() == WL_CONNECTED) {
        if (!mqtt.connected()) {
            reconnectMqtt(nowMs);
        } else {
            mqtt.loop();
        }
    }
#endif

    hardware.syncInputs();

    size_t stIdx = 0;
    char txTokens[256];
    if (machine.pollCode(stIdx, txTokens, sizeof(txTokens))) {
        const char* targetCp = machine.station(stIdx).name();
        Serial.printf("CODED [%s]: %s\n", targetCp, txTokens);

#ifdef USE_OLED
        snprintf(oledLastCoded, sizeof(oledLastCoded), "CODED: %s", targetCp);
        updateOled();
#endif

#ifdef USE_OTA
        if (mqtt.connected()) {
            char topic[128];
            snprintf(topic, sizeof(topic), "ctc/SPCoast/codeline/%s/controls", targetCp);
            mqtt.publish(topic, txTokens);
        }
#endif
    }

    hardware.syncOutputs();
}
#endif
