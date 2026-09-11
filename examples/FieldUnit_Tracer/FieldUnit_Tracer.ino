/*
 * FieldUnit_Tracer - Interactive USB CDC Test Jig & Universal Bungalow Firmware
 *
 * Inspired by CMRInet's tracerHost/tracerNode testbed:
 * - Uses USB CDC Serial as a multiplexed Command & Control (C&C) + CodeLine link.
 * - Boots unconfigured into neutral listening mode.
 * - Ingests dynamic plant configurations over the wire: 'load json <payload>'
 * - Emits plant state: 'dump json' or 'status'
 * - Injects track & switch stimulus: 'shunt <tc>', 'clear <tc>', 'throw <sw> <pos>'
 * - Transacts AAR CodeLine snapshots: '1NWS, 2NGS' <-> '1NWK, 2NGK...'
 */

#include <FieldUnit.h>
#include "FieldUnitConsole.h"

#ifdef ARDUINO
#include <Wire.h>
#include <I2Cexpander.h>
#include "drivers/I2CexpanderIOBus.h"

// Physical MCP23017 at I2C address 0x20
I2Cexpander expander;
FieldUnit::I2CexpanderIOBus hardwareBus(&expander, 1);
#endif

using namespace FieldUnit;

// Create unconfigured Control Point
ControlPoint cp("Tracer_CP");

void serialOutput(const char* line) {
    Serial.println(line);
}

FieldUnitConsole console(cp, serialOutput);

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000) {
        delay(10);
    }
#ifdef ARDUINO
    Wire.begin();
    // MCP23017 at 0x20: Port A (pins 0..7) = IN (0x00FF), Port B (pins 8..15) = OUT
    expander.init(0, I2Cexpander::MCP23017, 0x00FF);
    console.setIOBus(&hardwareBus);
#endif
    Serial.println(F("=== FieldUnit Tracer Test Jig Ready ==="));
    Serial.println(F("Send 'help' for C&C verbs or stream AAR CodeLine snapshots."));
}

void loop() {
    uint32_t nowMs = millis();

    // 1. Process inbound bytes from USB CDC Serial
    while (Serial.available() > 0) {
        int c = Serial.read();
        if (c >= 0) {
            console.processByte(static_cast<char>(c), nowMs);
        }
    }

    // 2. Periodic vital interlocking scan
    cp.tick(nowMs);
}
