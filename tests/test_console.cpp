#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <vector>
#include <string>
#include "../src/FieldUnit.h"

using namespace FieldUnit;

static std::vector<std::string> g_consoleResponses;

static void consoleOutputHandler(const char* line) {
    g_consoleResponses.push_back(line);
}

void runConsoleTests() {
    printf("====================================================\n");
    printf("   FIELDUNIT CONSOLE C&C + CODELINE TEST SUITE      \n");
    printf("   CDC Serial Mux: Verbs, JSON Streaming & AAR Lines\n");
    printf("====================================================\n\n");

    g_consoleResponses.clear();
    uint32_t clockMs = 1000;

    // 1. Boot blank ControlPoint
    ControlPoint cp("Blank_Boot");
    FieldUnitConsole console(cp, consoleOutputHandler);

    assert(cp.trackCircuitCount() == 0);
    assert(cp.switchCount() == 0);
    printf("[TEST 1] Booted blank ControlPoint\n");

    // 2. Send C&C verb: "status"
    console.processLine("status", clockMs);
    assert(g_consoleResponses.size() == 1);
    assert(g_consoleResponses[0].find("\"plant\":\"Blank_Boot\"") != std::string::npos);
    assert(g_consoleResponses[0].find("\"tracks\":0") != std::string::npos);
    printf("  -> PASS: 'status' emitted JSON line: %s\n\n", g_consoleResponses[0].c_str());
    g_consoleResponses.clear();

    // 3. Send C&C verb: "load json {...}"
    printf("[TEST 2] Dynamically configure plant via 'load json' stream\n");
    const char* jsonPlant = R"json({
      "name": "CP_Dynamic",
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

    std::string loadCmd = std::string("load json ") + jsonPlant;
    console.processLine(loadCmd.c_str(), clockMs);

    assert(g_consoleResponses.size() == 1);
    assert(g_consoleResponses[0].find("\"status\":\"OK\"") != std::string::npos);
    assert(g_consoleResponses[0].find("\"name\":\"CP_Dynamic\"") != std::string::npos);
    assert(cp.trackCircuitCount() == 2);
    assert(cp.switchCount() == 1);
    assert(cp.mastCount() == 1);
    assert(cp.engine().routeCount() == 1);
    printf("  -> PASS: Plant dynamically configured in RAM: %s\n\n", g_consoleResponses[0].c_str());
    g_consoleResponses.clear();

    // 4. Send C&C verb: "dump json"
    printf("[TEST 3] Dump current configuration via 'dump json'\n");
    console.processLine("dump json", clockMs);
    assert(g_consoleResponses.size() == 1);
    assert(g_consoleResponses[0].find("\"name\": \"CP_Dynamic\"") != std::string::npos);
    assert(g_consoleResponses[0].find("\"routes\": [") != std::string::npos);
    printf("  -> PASS: 'dump json' exported valid plant JSON (%zu bytes)\n\n", g_consoleResponses[0].size());
    g_consoleResponses.clear();

    // 5. Initialize track & switch states
    cp.findTrackCircuit("1T1")->update(Occupancy::VACANT);
    cp.findTrackCircuit("2T1")->update(Occupancy::VACANT);
    cp.findSwitch("1")->updateFeedback(SwitchPosition::NORMAL);

    // 6. Send AAR CodeLine Transaction Snapshot over the same CDC link!
    printf("[TEST 4] AAR CodeLine Transaction: Dispatcher clears Signal 2 Right\n");
    console.processLine("1NWS, 2SGS", clockMs);
    assert(g_consoleResponses.size() == 1);
    const std::string& indLine = g_consoleResponses[0];
    // Must contain 1NWK (asserted) and 2SGK (asserted)
    assert(indLine.find("1NWK") != std::string::npos && indLine.find("(1NWK)") == std::string::npos);
    assert(indLine.find("2SGK") != std::string::npos && indLine.find("(2SGK)") == std::string::npos);
    assert(cp.findSignalMast("2LA")->head1() == Aspect::GREEN);
    printf("  -> PASS: CodeLine transaction executed: %s\n\n", indLine.c_str());
    g_consoleResponses.clear();

    // 7. Send C&C verb: "shunt 1T1"
    printf("[TEST 5] C&C Stimulus: Shunt track circuit 1T1 (Train enters plant)\n");
    console.processLine("shunt 1T1", clockMs);
    assert(g_consoleResponses.size() == 1);
    assert(g_consoleResponses[0].find("\"action\":\"shunted\"") != std::string::npos);
    assert(cp.findTrackCircuit("1T1")->isClear() == false);
    g_consoleResponses.clear();

    // Next cycle: signal knocks down to STOP!
    console.processLine("1NWS", clockMs);
    assert(g_consoleResponses.size() == 1);
    const std::string& indKnockdown = g_consoleResponses[0];
    assert(indKnockdown.find("1T1K") != std::string::npos && indKnockdown.find("(1T1K)") == std::string::npos);
    assert(indKnockdown.find("(2SGK)") != std::string::npos); // Signal dropped to Stop
    assert(cp.findSignalMast("2LA")->head1() == Aspect::RED);
    printf("  -> PASS: Signal knocked down to STOP; indication confirms (2SGK): %s\n\n", indKnockdown.c_str());
    g_consoleResponses.clear();

    // 8. Stream byte-by-byte testing with \n framing
    printf("[TEST 6] Byte-by-byte CDC stream framing\n");
    const char* streamData = "clear 1T1\n";
    for (size_t i = 0; i < strlen(streamData); ++i) {
        console.processByte(streamData[i], clockMs);
    }
    assert(g_consoleResponses.size() == 1);
    assert(g_consoleResponses[0].find("\"action\":\"cleared\"") != std::string::npos);
    assert(cp.findTrackCircuit("1T1")->isClear() == true);
    printf("  -> PASS: Byte-by-byte line framing parsed correctly\n\n");

    printf("====================================================\n");
    printf("   ALL CONSOLE MUX TESTS PASSED (100%%)              \n");
    printf("====================================================\n");
}

int main() {
    runConsoleTests();
    return 0;
}
