#ifndef FIELDUNIT_MQTT_APPLIANCE_BUS_H
#define FIELDUNIT_MQTT_APPLIANCE_BUS_H

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include "../TrackCircuit.h"
#include "../Switch.h"
#include "../SignalMast.h"
#include "../ControlPoint.h"

namespace FieldUnit {

/**
 * MqttApplianceBus - High-Level Semantic Device Interface (JMRI MQTT / IoT Domain Topics)
 *
 * Connects FieldUnit logical appliances (TrackCircuit, Switch, SignalMast) directly
 * to discrete MQTT domain topics rather than pin bit-fields.
 *
 * Standard Topic Schemas:
 * - Track Sensors:    "{prefix}/sensor/{name}"      (payload: ACTIVE / INACTIVE, OCCUPIED / VACANT, 1 / 0)
 * - Turnout Commands: "{prefix}/turnout/{name}"     (payload: CLOSED / THROWN, NORMAL / REVERSE)
 * - Turnout Feedback: "{prefix}/turnout/{name}/state" (or "{prefix}/turnout/{name}")
 * - Signal Masts:     "{prefix}/signalmast/{name}"  (payload: "Clear", "Approach", "Stop", etc.)
 *
 * Implements the onPack / onUnpack vocabulary:
 * - onUnpack(topic, payload): Ingests MQTT subscription messages to update field inputs
 * - onPack(handler): Registers publisher callback invoked when appliance outputs change
 * - sync(): Transmits state transitions for switches and signal masts
 */
class MqttApplianceBus {
public:
    typedef bool (*PublishHandler)(const char* topic, const char* payload, bool retain);

    static constexpr uint8_t MAX_BOUND_APPLIANCES = 32;

    MqttApplianceBus(const char* prefix = "track")
        : publishHandler_(nullptr),
          trackCircuitBindingCount_(0),
          switchBindingCount_(0),
          mastBindingCount_(0) {
        setPrefix(prefix);
    }

    void setPrefix(const char* prefix) {
        if (!prefix || prefix[0] == '\0') {
            prefix_[0] = '\0';
        } else {
            snprintf(prefix_, sizeof(prefix_), "%s", prefix);
        }
    }

    const char* prefix() const { return prefix_; }

    // Register MQTT publish handler (onPack callback)
    void onPack(PublishHandler handler) {
        publishHandler_ = handler;
    }

    // -------------------------------------------------------------
    // Appliance Binding Methods
    // -------------------------------------------------------------

    // Bind a TrackCircuit to an MQTT sensor topic (default: "{prefix}/sensor/{name}")
    bool bindTrackCircuit(TrackCircuit* tc, const char* customTopic = nullptr) {
        if (!tc || trackCircuitBindingCount_ >= MAX_BOUND_APPLIANCES) return false;
        TrackCircuitBinding& b = trackCircuits_[trackCircuitBindingCount_++];
        b.circuit = tc;
        if (customTopic && customTopic[0] != '\0') {
            snprintf(b.topic, sizeof(b.topic), "%s", customTopic);
        } else {
            snprintf(b.topic, sizeof(b.topic), "%s/sensor/%s", prefix_, tc->name());
        }
        return true;
    }

    // Bind a Switch to an MQTT turnout command & feedback topic
    // default command:  "{prefix}/turnout/{name}"
    // default feedback: "{prefix}/turnout/{name}/state"
    bool bindSwitch(Switch* sw, const char* customCommandTopic = nullptr, const char* customFeedbackTopic = nullptr) {
        if (!sw || switchBindingCount_ >= MAX_BOUND_APPLIANCES) return false;
        SwitchBinding& b = switches_[switchBindingCount_++];
        b.sw = sw;
        b.lastCommanded = SwitchPosition::UNKNOWN;

        if (customCommandTopic && customCommandTopic[0] != '\0') {
            snprintf(b.commandTopic, sizeof(b.commandTopic), "%s", customCommandTopic);
        } else {
            snprintf(b.commandTopic, sizeof(b.commandTopic), "%s/turnout/%s", prefix_, sw->name());
        }

        if (customFeedbackTopic && customFeedbackTopic[0] != '\0') {
            snprintf(b.feedbackTopic, sizeof(b.feedbackTopic), "%s", customFeedbackTopic);
        } else {
            snprintf(b.feedbackTopic, sizeof(b.feedbackTopic), "%s/turnout/%s/state", prefix_, sw->name());
        }
        return true;
    }

    // Bind a SignalMast to an MQTT signalmast topic (default: "{prefix}/signalmast/{name}")
    bool bindSignalMast(SignalMast* mast, const char* customTopic = nullptr) {
        if (!mast || mastBindingCount_ >= MAX_BOUND_APPLIANCES) return false;
        SignalMastBinding& b = masts_[mastBindingCount_++];
        b.mast = mast;
        b.lastIndication = Indication::STOP;
        b.lastAspect = Aspect::DARK;
        b.initialized = false;

        if (customTopic && customTopic[0] != '\0') {
            snprintf(b.topic, sizeof(b.topic), "%s", customTopic);
        } else {
            snprintf(b.topic, sizeof(b.topic), "%s/signalmast/%s", prefix_, mast->name());
        }
        return true;
    }

    // Auto-bind all appliances declared in a ControlPoint
    void bind(ControlPoint& cp) {
        for (uint8_t i = 0; i < cp.trackCircuitCount(); ++i) {
            bindTrackCircuit(cp.trackCircuit(i));
        }
        for (uint8_t i = 0; i < cp.switchCount(); ++i) {
            bindSwitch(cp.getSwitch(i));
        }
        for (uint8_t i = 0; i < cp.mastCount(); ++i) {
            bindSignalMast(cp.mast(i));
        }
    }

    // -------------------------------------------------------------
    // Ingress: onUnpack (Subscription Message Handler)
    // -------------------------------------------------------------
    // Ingests MQTT message arrived on subscribed topic and unpacks to appliance states.
    // Returns true if topic was recognized and matched an appliance.
    bool onUnpack(const char* topic, const char* payload, uint32_t nowMs = 0) {
        if (!topic || !payload) return false;

        // 1. Check TrackCircuit bindings
        for (uint8_t i = 0; i < trackCircuitBindingCount_; ++i) {
            if (strcmp(trackCircuits_[i].topic, topic) == 0) {
                Occupancy occ = parseSensorPayload(payload);
                trackCircuits_[i].circuit->update(occ, Quality::GOOD, nowMs);
                return true;
            }
        }

        // 2. Check Switch bindings (matches feedback topic OR command topic)
        for (uint8_t i = 0; i < switchBindingCount_; ++i) {
            if (strcmp(switches_[i].feedbackTopic, topic) == 0 ||
                strcmp(switches_[i].commandTopic, topic) == 0) {
                SwitchPosition pos = parseTurnoutPayload(payload);
                if (pos != SwitchPosition::UNKNOWN) {
                    switches_[i].sw->updateFeedback(pos);
                    return true;
                }
            }
        }

        return false;
    }

    // -------------------------------------------------------------
    // Egress: onPack / sync (Publish Appliance State Transitions)
    // -------------------------------------------------------------
    // Scans bound appliances and publishes changed output states (turnout commands, mast aspects).
    // Returns count of messages published.
    uint8_t sync(bool forceAll = false) {
        if (!publishHandler_) return 0;
        uint8_t pubCount = 0;

        // 1. Publish switch command changes
        for (uint8_t i = 0; i < switchBindingCount_; ++i) {
            SwitchBinding& b = switches_[i];
            SwitchPosition currentCmd = b.sw->commandedPosition();
            if (forceAll || currentCmd != b.lastCommanded) {
                const char* payload = (currentCmd == SwitchPosition::REVERSE) ? "THROWN" : "CLOSED";
                if (publishHandler_(b.commandTopic, payload, false /*not retain*/)) {
                    b.lastCommanded = currentCmd;
                    pubCount++;
                }
            }
        }

        // 2. Publish signal mast aspect changes
        for (uint8_t i = 0; i < mastBindingCount_; ++i) {
            SignalMastBinding& b = masts_[i];
            Indication currentInd = b.mast->currentIndication();
            Aspect currentAspect = b.mast->compositeAspect();

            if (forceAll || !b.initialized || currentInd != b.lastIndication || currentAspect != b.lastAspect) {
                const char* aspectStr = aspectName(currentInd, currentAspect);
                if (publishHandler_(b.topic, aspectStr, true /*retain*/)) {
                    b.lastIndication = currentInd;
                    b.lastAspect = currentAspect;
                    b.initialized = true;
                    pubCount++;
                }
            }
        }

        return pubCount;
    }

    // Convert Indication/Aspect into standard JMRI aspect name string
    static const char* aspectName(Indication ind, Aspect asp) {
        switch (ind) {
            case Indication::CLEAR:
                return "Clear";
            case Indication::CAB_SPEED:
                return "Cab Speed";
            case Indication::APPROACH:
                return "Approach";
            case Indication::ADVANCE_APPROACH:
                return "Advance Approach";
            case Indication::MEDIUM_CLEAR:
                return "Medium Clear";
            case Indication::DIVERGING_CLEAR:
                return "Diverging Clear";
            case Indication::MEDIUM_APPROACH:
                return "Medium Approach";
            case Indication::DIVERGING_APPROACH:
                return "Diverging Approach";
            case Indication::APPROACH_MEDIUM:
                return "Approach Medium";
            case Indication::APPROACH_SLOW:
                return "Approach Slow";
            case Indication::APPROACH_DIVERGING:
                return "Approach Diverging";
            case Indication::SLOW_CLEAR:
                return "Slow Clear";
            case Indication::SLOW_APPROACH:
                return "Slow Approach";
            case Indication::RESTRICTING:
                return "Restricting";
            case Indication::DIVERGING_RESTRICTING:
                return "Diverging Restricting";
            case Indication::APPROACH_RESTRICTING:
                return "Approach Restricting";
            case Indication::STOP:
            default:
                return "Stop";
        }
    }

    // Helper: case-insensitive sensor payload parsing
    static Occupancy parseSensorPayload(const char* payload) {
        if (!payload) return Occupancy::VACANT;
        while (*payload == ' ' || *payload == '\t') payload++;

        if (strcasecmp(payload, "ACTIVE") == 0 ||
            strcasecmp(payload, "OCCUPIED") == 0 ||
            strcasecmp(payload, "ON") == 0 ||
            strcasecmp(payload, "1") == 0 ||
            strcasecmp(payload, "TRUE") == 0) {
            return Occupancy::OCCUPIED;
        }
        return Occupancy::VACANT;
    }

    // Helper: case-insensitive turnout payload parsing
    static SwitchPosition parseTurnoutPayload(const char* payload) {
        if (!payload) return SwitchPosition::UNKNOWN;
        while (*payload == ' ' || *payload == '\t') payload++;

        if (strcasecmp(payload, "CLOSED") == 0 ||
            strcasecmp(payload, "NORMAL") == 0 ||
            strcasecmp(payload, "1") == 0 ||
            strcasecmp(payload, "N") == 0) {
            return SwitchPosition::NORMAL;
        }
        if (strcasecmp(payload, "THROWN") == 0 ||
            strcasecmp(payload, "REVERSE") == 0 ||
            strcasecmp(payload, "2") == 0 ||
            strcasecmp(payload, "R") == 0) {
            return SwitchPosition::REVERSE;
        }
        if (strcasecmp(payload, "MOVING") == 0 ||
            strcasecmp(payload, "3") == 0) {
            return SwitchPosition::MOVING;
        }
        return SwitchPosition::UNKNOWN;
    }

    uint8_t trackCircuitBindingCount() const { return trackCircuitBindingCount_; }
    uint8_t switchBindingCount() const { return switchBindingCount_; }
    uint8_t mastBindingCount() const { return mastBindingCount_; }

private:
    struct TrackCircuitBinding {
        TrackCircuit* circuit;
        char topic[64];
    };

    struct SwitchBinding {
        Switch* sw;
        char commandTopic[64];
        char feedbackTopic[64];
        SwitchPosition lastCommanded;
    };

    struct SignalMastBinding {
        SignalMast* mast;
        char topic[64];
        Indication lastIndication;
        Aspect lastAspect;
        bool initialized;
    };

    PublishHandler publishHandler_;
    char prefix_[32];

    TrackCircuitBinding trackCircuits_[MAX_BOUND_APPLIANCES];
    uint8_t trackCircuitBindingCount_;

    SwitchBinding switches_[MAX_BOUND_APPLIANCES];
    uint8_t switchBindingCount_;

    SignalMastBinding masts_[MAX_BOUND_APPLIANCES];
    uint8_t mastBindingCount_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_MQTT_APPLIANCE_BUS_H
