#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../src/cTcMachine.h"

using namespace FieldUnit;

class MockPanelHardware : public PanelHardware {
public:
    MockPanelHardware() {
        memset(inputs_, 0, sizeof(inputs_));
        memset(outputs_, 0, sizeof(outputs_));
    }

    bool read(uint8_t column, PanelInput fn) override {
        if (column < 16 && static_cast<uint8_t>(fn) < 8) {
            return inputs_[column][static_cast<uint8_t>(fn)];
        }
        return false;
    }

    void write(uint8_t column, PanelOutput fn, bool state) override {
        if (column < 16 && static_cast<uint8_t>(fn) < 16) {
            outputs_[column][static_cast<uint8_t>(fn)] = state;
        }
    }

    void setInput(uint8_t column, PanelInput fn, bool val) {
        if (column < 16 && static_cast<uint8_t>(fn) < 8) {
            inputs_[column][static_cast<uint8_t>(fn)] = val;
        }
    }

    bool getOutput(uint8_t column, PanelOutput fn) const {
        if (column < 16 && static_cast<uint8_t>(fn) < 16) {
            return outputs_[column][static_cast<uint8_t>(fn)];
        }
        return false;
    }

private:
    bool inputs_[16][8];
    bool outputs_[16][16];
};

void testCtcMachineAssemblyAndCodeButton() {
    printf("--- Running testCtcMachineAssemblyAndCodeButton ---\n");
    MockPanelHardware hw;
    cTcMachine machine(hw);

    // CP Christopher: Columns 8, 9, 10 fully chained with zero throwaway variables
    machine.addStation("CP_Christopher")
        .inColumn(8).withSwitch("1").withTrackLamps({ "1T1", "1WA", "2WA" })
        .inColumn(9).withSwitch("3").withSignal("2").withTrackLamps({ "3T1", "3BT1", "5T1" })
        .inColumn(10).withSwitch("5").withTrackLamps({ "1EA", "2EA" }).withCodeButton();

    // CP Corporal: Columns 11, 12 fully chained
    machine.addStation("CP_Corporal")
        .inColumn(11).withSwitch("1").withSignal("2").withTrackLamps({ "1EA", "1T1", "3T1" })
        .inColumn(12).withSwitch("3").withTrackLamps({ "SDT", "TL", "TR" }).withCodeButton();

    assert(machine.stationCount() == 2);
    assert(strcmp(machine.station(0).name(), "CP_Christopher") == 0);
    assert(strcmp(machine.station(1).name(), "CP_Corporal") == 0);

    // 1. When no code button is pressed, pollCode returns false
    size_t stIdx = 999;
    char tokens[256];
    assert(!machine.pollCode(stIdx, tokens, sizeof(tokens)));

    // 2. Set levers on Christopher:
    // Switch 1: NORMAL
    hw.setInput(8, PanelInput::SW_NORMAL, true);
    hw.setInput(8, PanelInput::SW_REVERSE, false);

    // Switch 3: REVERSE
    hw.setInput(9, PanelInput::SW_NORMAL, false);
    hw.setInput(9, PanelInput::SW_REVERSE, true);

    // Signal 2: LEFT
    hw.setInput(9, PanelInput::SIG_LEFT, true);
    hw.setInput(9, PanelInput::SIG_RIGHT, false);
    hw.setInput(9, PanelInput::SIG_STOP, false);

    // Switch 5: NORMAL
    hw.setInput(10, PanelInput::SW_NORMAL, true);
    hw.setInput(10, PanelInput::SW_REVERSE, false);

    // Press CODE10 on Column 10
    hw.setInput(10, PanelInput::CODE_BUTTON, true);

    // 3. Poll code should succeed for station 0 (CP_Christopher)
    assert(machine.pollCode(stIdx, tokens, sizeof(tokens)));
    assert(stIdx == 0);
    printf("  -> Compiled Tokens: %s\n", tokens);

    // Verify demands gathered across all three columns
    assert(strstr(tokens, "1NWS, (1RWS)") != nullptr);
    assert(strstr(tokens, "(3NWS), 3RWS") != nullptr);
    assert(strstr(tokens, "(2SGS), 2NGS, (2HS)") != nullptr);
    assert(strstr(tokens, "5NWS, (5RWS)") != nullptr);

    printf("  -> PASS: All column demands harvested correctly into single token packet.\n");
}

void testCtcMachineIndicationFanOut() {
    printf("--- Running testCtcMachineIndicationFanOut ---\n");
    MockPanelHardware hw;
    cTcMachine machine(hw);

    auto& christopher = machine.addStation("CP_Christopher");
    christopher.inColumn(8).withSwitch("1").withTrackLamps({ "1T1", "1WA", "2WA" });
    christopher.inColumn(9).withSwitch("3").withSignal("2").withTrackLamps({ "3T1", "3BT1", "5T1" });
    christopher.inColumn(10).withSwitch("5").withTrackLamps({ "1EA", "2EA" }).withCodeButton();

    // Field publishes indication string:
    // Switch 1: NORMAL (1NWK)
    // Switch 3: REVERSE (3RWK)
    // Switch 5: NORMAL (5NWK)
    // Track 1T1: OCCUPIED (1T1K)
    // Track 3T1: VACANT ((3T1K))
    // Track 1EA: OCCUPIED (1EAK)
    // Signal 2: LEFT (2NGK)
    const char* indPayload = "1NWK, (1RWK), (3NWK), 3RWK, 5NWK, (5RWK), 1T1K, (3T1K), 1EAK, (2SGK), 2NGK, (2TEK)";

    bool ok = machine.applyIndications("CP_Christopher", indPayload);
    assert(ok);

    // Verify Column 8 lamps
    assert(hw.getOutput(8, PanelOutput::SW_NORMAL_LAMP) == true);
    assert(hw.getOutput(8, PanelOutput::SW_REVERSE_LAMP) == false);
    assert(hw.getOutput(8, PanelOutput::TRACK_LAMP_1) == true); // 1T1 occupied

    // Verify Column 9 lamps
    assert(hw.getOutput(9, PanelOutput::SW_NORMAL_LAMP) == false);
    assert(hw.getOutput(9, PanelOutput::SW_REVERSE_LAMP) == true);
    assert(hw.getOutput(9, PanelOutput::TRACK_LAMP_1) == false); // 3T1 vacant
    assert(hw.getOutput(9, PanelOutput::SIG_LEFT_LAMP) == true);  // 2NGK
    assert(hw.getOutput(9, PanelOutput::SIG_RIGHT_LAMP) == false);
    assert(hw.getOutput(9, PanelOutput::SIG_STOP_LAMP) == false);

    // Verify Column 10 lamps
    assert(hw.getOutput(10, PanelOutput::SW_NORMAL_LAMP) == true);
    assert(hw.getOutput(10, PanelOutput::TRACK_LAMP_1) == true); // 1EA occupied

    printf("  -> PASS: All panel lamps updated accurately from AAR indication tokens.\n");
}

int main() {
    printf("====================================================\n");
    printf("   CTC MACHINE UNIT TESTS                           \n");
    printf("====================================================\n");

    testCtcMachineAssemblyAndCodeButton();
    testCtcMachineIndicationFanOut();

    printf("\nALL CTC MACHINE TESTS PASSED!\n");
    return 0;
}
