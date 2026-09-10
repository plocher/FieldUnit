/*
 * Universal_FieldUnit - Autonomous Data-Driven Bungalow Controller
 *
 * This sketch demonstrates the power of dynamic plant deserialization:
 * A single universal microcontroller firmware binary can control ANY trackside
 * interlocking plant simply by reading its topology from a local JSON file
 * (LittleFS, SPIFFS, SD card, or flash storage).
 *
 * Architecture:
 * 1. CodeLine Interface (Supervisory): Communicates plant-wide snapshots to Office.
 * 2. Vital Interlocking Core: Evaluates rules, detector locks, and routes.
 * 3. Device Interface: Automatically drives MQTT smart nodes or physical pins.
 */

#include <FieldUnit.h>

using namespace FieldUnit;

// Create blank Control Point (configured at boot from JSON)
ControlPoint cp("Universal_CP");
AarTextCodec codec;

#if defined(ESP32) || defined(ESP8266)
#include <LittleFS.h>
#endif

// Fallback embedded JSON configuration (e.g. if filesystem file not found)
const char* FALLBACK_PLANT_JSON = R"json({
  "name": "CP_Default",
  "defaultAspectPolicy": "sp1969",
  "trackCircuits": [
    {"name": "1T1", "dropoutDelayMs": 0},
    {"name": "2T1", "dropoutDelayMs": 0}
  ],
  "switches": [
    {"name": "1"}
  ],
  "signalControls": [
    {"name": "2"}
  ],
  "signalMasts": [
    {"name": "2LA", "type": "ONE_HEAD", "aspectPolicy": "sp1969"}
  ],
  "detectorLocks": [
    {"switch": "1", "trackCircuit": "1T1"}
  ],
  "routes": [
    {
      "name": "MAIN",
      "governedBy": {"signal": "2", "direction": "RIGHT"},
      "displays": {"mast": "2LA", "head": 0, "maxIndication": "CLEAR"},
      "aligns": [
        {"switch": "1", "position": "NORMAL"}
      ],
      "clears": ["1T1", "2T1"],
      "entrance": "1T1"
    }
  ]
})json";

void setup() {
#if defined(ARDUINO)
    Serial.begin(115200);
    delay(1000);
    Serial.println(F("=== FieldUnit Universal Controller Booting ==="));
#endif

    bool loadedFromFile = false;

#if defined(ESP32) || defined(ESP8266)
    if (LittleFS.begin()) {
        if (LittleFS.exists("/plant.json")) {
            File f = LittleFS.open("/plant.json", "r");
            if (f) {
                char jsonBuf[4096];
                int bytesRead = f.readBytes(jsonBuf, sizeof(jsonBuf) - 1);
                f.close();
                if (bytesRead > 0) {
                    jsonBuf[bytesRead] = '\0';
                    if (cp.deserialize(jsonBuf)) {
                        loadedFromFile = true;
#if defined(ARDUINO)
                        Serial.println(F("Loaded plant configuration from /plant.json in LittleFS"));
#endif
                    }
                }
            }
        }
    }
#endif

    if (!loadedFromFile) {
        // Fallback to embedded flash JSON
        cp.deserialize(FALLBACK_PLANT_JSON);
#if defined(ARDUINO)
        Serial.println(F("Loaded default embedded plant configuration"));
#endif
    }

#if defined(ARDUINO)
    Serial.print(F("Control Point Initialized: "));
    Serial.println(cp.name());
    Serial.print(F("Track Circuits: "));
    Serial.println(cp.trackCircuitCount());
    Serial.print(F("Switches: "));
    Serial.println(cp.switchCount());
    Serial.print(F("Signal Masts: "));
    Serial.println(cp.mastCount());
#endif
}

void loop() {
    // Invariant atomic vital scan cycle:
    // 1. Ingress: Sample field detectors & switch point contacts
    // 2. Vital Evaluation: Interlocking rules, detector locks, route locking
    // 3. Egress: Actuate switch motors and signal mast aspects
    cp.tick();
}
