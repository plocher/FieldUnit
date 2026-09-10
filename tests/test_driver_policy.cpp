#include <stdio.h>
#include <assert.h>
#include <vector>
#include <string>
#include "../src/FieldUnit.h"

using namespace FieldUnit;

struct MqttLog {
    std::string topic;
    std::string payload;
    bool retain;
};

static std::vector<MqttLog> g_mqttLog;

static bool loggingPublishHandler(const char* topic, const char* payload, bool retain) {
    g_mqttLog.push_back({topic, payload, retain});
    return true;
}

void runDriverPolicyTests() {
    printf("====================================================\n");
    printf("   FIELDUNIT DRIVER POLICY & SCAN ENCAPSULATION     \n");
    printf("   Plant-Wide Defaults, Hybrid Overrides & cp.tick()\n");
    printf("====================================================\n\n");

    uint32_t clockMs = 1000;
    g_mqttLog.clear();

    // -------------------------------------------------------------
    // TEST 1: Plant-Wide MQTT Driver Policy with Encapsulated cp.tick()
    // -------------------------------------------------------------
    printf("[TEST 1] Plant-Wide MQTT Policy: Zero manual driver calls in loop()\n");
    ControlPoint cpMqtt("CP_SAN_JOSE");
    TrackCircuit* tc1 = cpMqtt.addTrackCircuit("1T1");
    Switch* sw1       = cpMqtt.addSwitch("1");
    SignalMast* mast2 = cpMqtt.addSignalMast("2LA", MastType::ONE_HEAD);
    SignalControl* sig2 = cpMqtt.addSignalControl("2");

    cpMqtt.bindDetectorLock("1", "1T1");

    cpMqtt.route("MAIN_EB")
      .governedBy("2", DirectionAuthority::RIGHT)
      .displays("2LA", Indication::CLEAR)
      .aligns({ {"1", SwitchPosition::NORMAL} })
      .clears({ "1T1" })
      .entrance("1T1");

    // Initialize track & switch
    tc1->update(Occupancy::VACANT, Quality::GOOD, clockMs);
    sw1->updateFeedback(SwitchPosition::NORMAL);

    // Set up MQTT Appliance Bus & Policy
    MqttApplianceBus mqttBus("track");
    mqttBus.onPack(loggingPublishHandler);
    mqttBus.bind(cpMqtt);

    // Set plant-wide driver policy on ControlPoint!
    cpMqtt.setDefaultDriverPolicy(DriverPolicies::MQTT(mqttBus));

    // The entire loop is now a single atomic call: cp.tick(nowMs)!
    cpMqtt.tick(clockMs);
    assert(g_mqttLog.size() == 2); // Initial baseline sync: SW1=CLOSED, Mast 2LA=Stop
    g_mqttLog.clear();

    // Dispatcher commands Signal 2 Right
    ControlTransaction ctl;
    ctl.signalDemands[sig2->index()] = SignalDemand::RIGHT;
    cpMqtt.applyControlTransaction(ctl, clockMs);

    // Single cp.tick() evaluates interlocking and automatically publishes mast aspect!
    cpMqtt.tick(clockMs);
    assert(mast2->head1() == Aspect::GREEN);
    assert(g_mqttLog.size() == 1);
    assert(g_mqttLog[0].topic == "track/signalmast/2LA");
    assert(g_mqttLog[0].payload == "Clear");
    printf("  -> PASS: Single cp.tick() evaluated route and published 'Clear' over MQTT\n\n");

    // -------------------------------------------------------------
    // TEST 2: Hybrid Plant - Physical IOBit Bus with Workbench Mock Override
    // -------------------------------------------------------------
    printf("[TEST 2] Hybrid Plant: Physical I2C Bus with Mocked Turnout Override\n");
    ControlPoint cpHybrid("CP_WARM_SPRINGS");
    TrackCircuit* tcMain = cpHybrid.addTrackCircuit("1T1");
    Switch* swInstalled  = cpHybrid.addSwitch("1");  // Physically installed Tortoise
    Switch* swBench      = cpHybrid.addSwitch("3");  // Uninstalled turnout on workbench!
    SignalMast* mastHome = cpHybrid.addSignalMast("2RA", MastType::ONE_HEAD);

    cpHybrid.bindDetectorLock("1", "1T1");

    // Physical IOBus simulation (e.g. MCP23017 on cpNode-IOX)
    MockIOBus hardwareBus;
    InputBit  sensorBit(0, 0, 0, Polarity::INVERTED); // 1T1 detector pin
    OutputBit motorBit(0, 0, 3);                      // SW1 motor pin
    InputBit  normalSense(0, 0, 4, Polarity::INVERTED);
    InputBit  reverseSense(0, 0, 5, Polarity::INVERTED);
    OutputBit redPin(1, 0, 0), yelPin(1, 0, 1), grnPin(1, 0, 2);

    // Initial hardware states: 1T1 clear, SW1 points normal
    hardwareBus.setPinState(sensorBit, false /*vacant*/);
    hardwareBus.setPinState(normalSense, true /*normal contact closed*/);
    hardwareBus.setPinState(reverseSense, false);

    // 1. Configure plant-wide physical IOBit policy
    IoBitDriverPolicy ioPolicy(&hardwareBus);
    ioPolicy.addTrackCircuit(tcMain, sensorBit);
    ioPolicy.addSwitch(swInstalled, motorBit, normalSense, reverseSense);
    ioPolicy.addSignalMast(mastHome, {redPin, yelPin, grnPin});
    cpHybrid.setDefaultDriverPolicy(&ioPolicy);

    // 2. Pure by-name workbench turnout mocking (Zero pointer handles!)
    cpHybrid.mockSwitch("3", 2000 /*travelTimeMs*/);

    // Initial tick stabilizes hybrid plant
    cpHybrid.tick(clockMs);
    assert(tcMain->isClear() == true);
    assert(swInstalled->reportedPosition() == SwitchPosition::NORMAL);
    assert(swBench->reportedPosition() == SwitchPosition::NORMAL);
    printf("  -> Baseline: SW1 (Physical I2C) Normal, SW3 (Bench Mock) Normal\n");

    // 3. Command Switch 3 to Reverse
    clockMs += 100;
    swBench->throwSwitch(SwitchPosition::REVERSE, clockMs);
    assert(swBench->reportedPosition() == SwitchPosition::MOVING);
    assert(!swBench->inCorrespondence());

    // Single cp.tick() runs atomic scan cycle
    cpHybrid.tick(clockMs);
    printf("  -> SW3 commanded Reverse: Mock driver simulating point travel\n");

    // 4. Advance time by 1000ms (halfway through travel): points still in motion
    clockMs += 1000;
    cpHybrid.tick(clockMs);
    assert(swBench->reportedPosition() == SwitchPosition::MOVING);
    assert(!swBench->inCorrespondence());
    printf("  -> At t+1000ms: Points still in motion (not in correspondence)\n");

    // 5. Advance time by another 1500ms (exceeds 2000ms travel time): points complete travel
    clockMs += 1500;
    cpHybrid.tick(clockMs);
    assert(swBench->reportedPosition() == SwitchPosition::REVERSE);
    assert(swBench->inCorrespondence());
    printf("  -> PASS: Mock driver completed travel; SW3 achieved Reverse correspondence\n\n");

    // -------------------------------------------------------------
    // TEST 3: Physical Sensor Knockdown on Hybrid Plant
    // -------------------------------------------------------------
    printf("[TEST 3] Physical Hardware Sensor Shunt during Encapsulated Tick\n");
    // Shunt physical detector pin on hardwareBus
    hardwareBus.setPinState(sensorBit, true /*shunted*/);

    // Single cp.tick() samples pin, detects occupancy, and sets detector lock
    cpHybrid.tick(clockMs);
    assert(tcMain->isClear() == false);
    assert(swInstalled->isDetectorLocked() == true);

    // Physical pin clears
    hardwareBus.setPinState(sensorBit, false /*cleared*/);
    cpHybrid.tick(clockMs);
    assert(tcMain->isClear() == true);
    assert(swInstalled->isDetectorLocked() == false);
    printf("  -> PASS: Physical pin sampling and detector locking verified inside cp.tick()\n\n");

    printf("====================================================\n");
    printf("   ALL DRIVER POLICY & TICK TESTS PASSED (100%%)     \n");
    printf("====================================================\n");
}

int main() {
    runDriverPolicyTests();
    return 0;
}
