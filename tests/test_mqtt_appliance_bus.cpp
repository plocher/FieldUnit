#include <stdio.h>
#include <assert.h>
#include <vector>
#include <string>
#include "../src/FieldUnit.h"

using namespace FieldUnit;

// Test recording structure for MQTT publications
struct PublishedMessage {
    std::string topic;
    std::string payload;
    bool retain;
};

static std::vector<PublishedMessage> g_publishedMessages;

static bool testPublishHandler(const char* topic, const char* payload, bool retain) {
    g_publishedMessages.push_back({topic, payload, retain});
    return true;
}

void runMqttApplianceBusTests() {
    printf("====================================================\n");
    printf("   FIELDUNIT MQTT APPLIANCE BUS TEST SUITE          \n");
    printf("   Seam B: Domain Device Abstraction & MQTT Driver  \n");
    printf("====================================================\n\n");

    g_publishedMessages.clear();
    uint32_t clockMs = 1000;

    // -------------------------------------------------------------
    // Setup Plant: ControlPoint with Track, Switch, and Mast
    // -------------------------------------------------------------
    ControlPoint cp("CP_OAKLAND");
    TrackCircuit* tc1T1 = cp.addTrackCircuit("1T1");
    TrackCircuit* tc2T1 = cp.addTrackCircuit("2T1");
    Switch* sw1         = cp.addSwitch("1");
    SignalMast* mast2N  = cp.addSignalMast("2NAB", MastType::TWO_HEAD);
    SignalControl* sig2 = cp.addSignalControl("2");

    cp.bindDetectorLock("1", "1T1");

    cp.route("MAIN_EB")
      .governedBy("2", DirectionAuthority::RIGHT)
      .displays("2NAB", 0, Indication::CLEAR)
      .aligns({ {"1", SwitchPosition::NORMAL} })
      .clears({ "1T1", "2T1" })
      .entrance("1T1");

    // Initialize track circuits as VACANT
    tc1T1->update(Occupancy::VACANT, Quality::GOOD, clockMs);
    tc2T1->update(Occupancy::VACANT, Quality::GOOD, clockMs);
    sw1->updateFeedback(SwitchPosition::NORMAL);
    cp.tick(clockMs);

    // -------------------------------------------------------------
    // TEST 1: Auto-Binding from ControlPoint
    // -------------------------------------------------------------
    printf("[TEST 1] Auto-binding all appliances from ControlPoint\n");
    MqttApplianceBus bus("track");
    bus.onPack(testPublishHandler);
    bus.bind(cp);

    assert(bus.trackCircuitBindingCount() == 2);
    assert(bus.switchBindingCount() == 1);
    assert(bus.mastBindingCount() == 1);

    // Initial baseline sync (publishes initial Switch 1=CLOSED and Mast 2NAB=Stop)
    uint8_t initCount = bus.sync();
    assert(initCount == 2);
    g_publishedMessages.clear();
    printf("  -> PASS: 2 TrackCircuits, 1 Switch, 1 SignalMast auto-bound; baseline states published\n\n");

    // -------------------------------------------------------------
    // TEST 2: Ingress via onUnpack - Sensor Detection
    // -------------------------------------------------------------
    printf("[TEST 2] Ingress onUnpack: Track sensor publishes ACTIVE / INACTIVE\n");
    assert(tc1T1->isClear() == true);

    // Broker pushes sensor active message from field detector
    bool recognized = bus.onUnpack("track/sensor/1T1", "ACTIVE", clockMs);
    assert(recognized == true);
    assert(tc1T1->isClear() == false);
    assert(tc1T1->state().value == Occupancy::OCCUPIED);
    printf("  -> 'track/sensor/1T1' 'ACTIVE' parsed: TrackCircuit marked OCCUPIED\n");

    // Broker pushes sensor inactive message (train departs)
    bus.onUnpack("track/sensor/1T1", "INACTIVE", clockMs);
    cp.tick(clockMs);
    assert(tc1T1->isClear() == true);
    assert(tc1T1->state().value == Occupancy::VACANT);
    printf("  -> PASS: 'track/sensor/1T1' 'INACTIVE' parsed: TrackCircuit restored to VACANT\n\n");

    // -------------------------------------------------------------
    // TEST 3: Ingress via onUnpack - Turnout Point Feedback
    // -------------------------------------------------------------
    printf("[TEST 3] Ingress onUnpack: Turnout points report CLOSED / THROWN\n");
    assert(sw1->reportedPosition() == SwitchPosition::NORMAL);

    // Points move to Reverse / Thrown
    bus.onUnpack("track/turnout/1/state", "THROWN");
    assert(sw1->reportedPosition() == SwitchPosition::REVERSE);
    printf("  -> 'track/turnout/1/state' 'THROWN' parsed: Switch reported Reverse\n");

    // Points return to Normal / Closed
    bus.onUnpack("track/turnout/1/state", "CLOSED");
    assert(sw1->reportedPosition() == SwitchPosition::NORMAL);
    printf("  -> PASS: 'track/turnout/1/state' 'CLOSED' parsed: Switch reported Normal\n\n");

    // -------------------------------------------------------------
    // TEST 4: Egress via onPack / sync - Turnout Motor Commands
    // -------------------------------------------------------------
    printf("[TEST 4] Egress onPack/sync: Commanded switch motion publishes to turnout topic\n");
    g_publishedMessages.clear();

    // Command Switch 1 to Reverse
    sw1->throwSwitch(SwitchPosition::REVERSE, clockMs);
    uint8_t count = bus.sync();
    assert(count == 1);
    assert(g_publishedMessages.size() == 1);
    assert(g_publishedMessages[0].topic == "track/turnout/1");
    assert(g_publishedMessages[0].payload == "THROWN");
    assert(g_publishedMessages[0].retain == false);
    printf("  -> Command 'THROWN' published to 'track/turnout/1'\n");

    // Calling sync() again with no state change should be idempotent (no duplicate publish)
    count = bus.sync();
    assert(count == 0);
    assert(g_publishedMessages.size() == 1);
    printf("  -> PASS: Idempotent sync produces zero redundant publications\n\n");

    // -------------------------------------------------------------
    // TEST 5: Egress via onPack / sync - Signal Mast Aspects
    // -------------------------------------------------------------
    printf("[TEST 5] Egress onPack/sync: Signal mast aspect transitions\n");
    g_publishedMessages.clear();

    // Set mast to CLEAR
    mast2N->setIndication(Indication::CLEAR);
    count = bus.sync();
    assert(count == 1);
    assert(g_publishedMessages.size() == 1);
    assert(g_publishedMessages[0].topic == "track/signalmast/2NAB");
    assert(g_publishedMessages[0].payload == "Clear");
    assert(g_publishedMessages[0].retain == true);
    printf("  -> Mast aspect 'Clear' published to 'track/signalmast/2NAB' with retain=true\n");

    // Transition mast to STOP
    g_publishedMessages.clear();
    mast2N->forceStop();
    count = bus.sync();
    assert(count == 1);
    assert(g_publishedMessages[0].payload == "Stop");
    printf("  -> PASS: Mast aspect 'Stop' published on knockdown/stop\n\n");

    // -------------------------------------------------------------
    // TEST 6: End-to-End Interlocking over MQTT
    // -------------------------------------------------------------
    printf("[TEST 6] End-to-End Interlocking: MQTT field sensors drive vital route clearing\n");
    g_publishedMessages.clear();

    // 1. Initial State: Switch 1 reported Normal
    sw1->throwSwitch(SwitchPosition::NORMAL, clockMs);
    sw1->updateFeedback(SwitchPosition::NORMAL);
    bus.sync(); // Clear dirty switch state
    g_publishedMessages.clear();

    // 2. Dispatcher commands Signal 2 RIGHT
    ControlTransaction ctl;
    ctl.signalDemands[sig2->index()] = SignalDemand::RIGHT;
    cp.applyControlTransaction(ctl, clockMs);
    cp.tick(clockMs);

    // 3. Egress publishes Signal Mast Aspect
    bus.sync();
    assert(g_publishedMessages.size() >= 1);
    assert(g_publishedMessages.back().topic == "track/signalmast/2NAB");
    assert(g_publishedMessages.back().payload == "Clear");
    assert(mast2N->head1() == Aspect::GREEN);
    printf("  -> Route cleared: Signal mast published 'Clear'\n");

    // 4. Remote detector publishes train occupancy on 1T1 over MQTT
    bus.onUnpack("track/sensor/1T1", "ACTIVE", clockMs);
    cp.tick(clockMs);

    // Signal must knock down immediately
    assert(mast2N->head1() == Aspect::RED);
    g_publishedMessages.clear();
    bus.sync();
    assert(g_publishedMessages.size() == 1);
    assert(g_publishedMessages[0].payload == "Stop");
    printf("  -> PASS: Remote MQTT occupancy triggered vital knockdown; published 'Stop'\n\n");

    // -------------------------------------------------------------
    // TEST 7: Custom Prefix and Override Binding
    // -------------------------------------------------------------
    printf("[TEST 7] Custom Topic Prefixes and Overrides\n");
    MqttApplianceBus customBus("layout/ind");
    customBus.bindTrackCircuit(tc1T1, "my/custom/sensor/block_a");
    bool parsed = customBus.onUnpack("my/custom/sensor/block_a", "1");
    assert(parsed == true);
    assert(tc1T1->state().value == Occupancy::OCCUPIED);
    printf("  -> PASS: Custom topic override successfully matched and unpacked\n\n");

    printf("====================================================\n");
    printf("   ALL MQTT APPLIANCE BUS TESTS PASSED (100%%)       \n");
    printf("====================================================\n");
}

int main() {
    runMqttApplianceBusTests();
    return 0;
}
