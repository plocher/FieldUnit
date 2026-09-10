#ifndef FIELDUNIT_DRIVER_POLICY_H
#define FIELDUNIT_DRIVER_POLICY_H

#include <stdint.h>
#include <string.h>
#include "../types.h"
#include "../Switch.h"
#include "../TrackCircuit.h"
#include "../SignalMast.h"
#include "IOBus.h"
#include "TrackCircuitDriver.h"
#include "SwitchDriver.h"
#include "SignalMastDriver.h"
#include "MqttApplianceBus.h"

namespace FieldUnit {

/**
 * Abstract base for individual appliance driver delegates
 */
class ApplianceDriver {
public:
    virtual ~ApplianceDriver() = default;
    virtual void setAppliance(void* appliance) {}
    virtual void sample(uint32_t nowMs) {}
    virtual void drive(uint32_t nowMs) {}
};

/**
 * MockSwitchDriver - Simulates non-blocking point travel and contact closure for
 * bench testing uninstalled turnouts before physical layout wiring.
 */
class MockSwitchDriver : public ApplianceDriver {
public:
    MockSwitchDriver(uint32_t travelTimeMs = 2000)
        : sw_(nullptr), travelTimeMs_(travelTimeMs), moveStartMs_(0), inMotion_(false) {}

    MockSwitchDriver(Switch* sw, uint32_t travelTimeMs = 2000)
        : sw_(sw), travelTimeMs_(travelTimeMs), moveStartMs_(0), inMotion_(false) {}

    void setAppliance(void* appliance) override {
        sw_ = static_cast<Switch*>(appliance);
    }

    void setSwitch(Switch* sw) { sw_ = sw; }

    void drive(uint32_t nowMs) override {
        if (!sw_) return;
        if (sw_->reportedPosition() != sw_->commandedPosition() && !inMotion_) {
            inMotion_ = true;
            moveStartMs_ = nowMs;
        }
    }

    void sample(uint32_t nowMs) override {
        if (!sw_ || !inMotion_) return;
        if (nowMs - moveStartMs_ >= travelTimeMs_) {
            sw_->updateFeedback(sw_->commandedPosition());
            inMotion_ = false;
        }
    }

    bool inMotion() const { return inMotion_; }

private:
    Switch* sw_;
    uint32_t travelTimeMs_;
    uint32_t moveStartMs_;
    bool inMotion_;
};

/**
 * DriverPolicy - Contract for plant-wide hardware driver execution
 * Evaluated atomically inside ControlPoint::tick()
 */
class DriverPolicy {
public:
    virtual ~DriverPolicy() = default;
    virtual void sampleAll(uint32_t nowMs) = 0;
    virtual void driveAll(uint32_t nowMs) = 0;
};

inline void ControlPoint::sampleInputs(uint32_t nowMs) {
    if (defaultDriverPolicy_) {
        defaultDriverPolicy_->sampleAll(nowMs);
    }
    for (uint8_t i = 0; i < driverOverrideCount_; ++i) {
        if (driverOverrides_[i].driver) {
            driverOverrides_[i].driver->sample(nowMs);
        }
    }
}

inline void ControlPoint::driveOutputs(uint32_t nowMs) {
    if (defaultDriverPolicy_) {
        defaultDriverPolicy_->driveAll(nowMs);
    }
    for (uint8_t i = 0; i < driverOverrideCount_; ++i) {
        if (driverOverrides_[i].driver) {
            driverOverrides_[i].driver->drive(nowMs);
        }
    }
}

inline void ControlPoint::overrideDriver(const char* applianceName, ApplianceDriver* driver) {
    if (driverOverrideCount_ < MAX_APPLIANCES && applianceName && driver) {
        Switch* sw = findSwitch(applianceName);
        if (sw) {
            driver->setAppliance(sw);
        } else {
            TrackCircuit* tc = findTrackCircuit(applianceName);
            if (tc) {
                driver->setAppliance(tc);
            } else {
                SignalMast* mast = findSignalMast(applianceName);
                if (mast) {
                    driver->setAppliance(mast);
                }
            }
        }
        driverOverrides_[driverOverrideCount_++] = { applianceName, driver };
    }
}

inline void ControlPoint::mockSwitch(const char* applianceName, uint32_t travelTimeMs) {
    static MockSwitchDriver s_pool[MAX_APPLIANCES];
    if (mockSwitchCount_ < MAX_APPLIANCES) {
        s_pool[mockSwitchCount_] = MockSwitchDriver(nullptr, travelTimeMs);
        mockSwitches_[mockSwitchCount_] = &s_pool[mockSwitchCount_];
        overrideDriver(applianceName, mockSwitches_[mockSwitchCount_++]);
    }
}

/**
 * MqttDriverPolicy - High-Level Semantic Device Interface
 * Drives smart trackside nodes over MQTT domain topics (e.g. JMRI MQTT schemas).
 */
class MqttDriverPolicy : public DriverPolicy {
public:
    MqttDriverPolicy(MqttApplianceBus* bus = nullptr) : bus_(bus) {}

    void setBus(MqttApplianceBus* bus) { bus_ = bus; }
    MqttApplianceBus* bus() const { return bus_; }

    void sampleAll(uint32_t nowMs) override {
        // In MQTT, inbound sensor/turnout messages are pushed asynchronously into onUnpack.
        // No polling loop required.
    }

    void driveAll(uint32_t nowMs) override {
        if (bus_) {
            bus_->sync();
        }
    }

private:
    MqttApplianceBus* bus_;
};

/**
 * IoBitDriverPolicy - Low-Level Electrical Hardware Device Interface
 * Drives physical pins, I2C port expanders, and PWM servos over IOBus.
 */
class IoBitDriverPolicy : public DriverPolicy {
public:
    static constexpr uint8_t MAX_POLICY_DRIVERS = 32;

    IoBitDriverPolicy(IOBus* io = nullptr)
        : io_(io),
          tcCount_(0),
          swCount_(0),
          mastCount_(0) {}

    void setIOBus(IOBus* io) { io_ = io; }
    IOBus* io() const { return io_; }

    void addTrackCircuit(TrackCircuit* tc, InputBit sensorBit) {
        if (tcCount_ < MAX_POLICY_DRIVERS) {
            tcDrivers_[tcCount_++] = TrackCircuitDriver(tc, sensorBit);
        }
    }

    void addSwitch(Switch* sw, OutputBit motorBit, InputBit normalSense = InputBit(), InputBit reverseSense = InputBit()) {
        if (swCount_ < MAX_POLICY_DRIVERS) {
            swDrivers_[swCount_++] = SwitchDriver(sw, motorBit, normalSense, reverseSense);
        }
    }

    void addSignalMast(SignalMast* mast, const HeadPins& h0, const HeadPins& h1 = HeadPins(), const HeadPins& h2 = HeadPins()) {
        if (mastCount_ < MAX_POLICY_DRIVERS) {
            mastDrivers_[mastCount_] = SignalMastDriver(mast);
            mastDrivers_[mastCount_].addHead(h0.redPin, h0.yellowPin, h0.greenPin, h0.lunarPin);
            if (h1.redPin.isValid() || h1.greenPin.isValid()) {
                mastDrivers_[mastCount_].addHead(h1.redPin, h1.yellowPin, h1.greenPin, h1.lunarPin);
            }
            if (h2.redPin.isValid() || h2.greenPin.isValid()) {
                mastDrivers_[mastCount_].addHead(h2.redPin, h2.yellowPin, h2.greenPin, h2.lunarPin);
            }
            mastCount_++;
        }
    }

    void sampleAll(uint32_t nowMs) override {
        if (!io_) return;
        for (uint8_t i = 0; i < tcCount_; ++i) {
            tcDrivers_[i].sample(*io_, nowMs);
        }
        for (uint8_t i = 0; i < swCount_; ++i) {
            swDrivers_[i].sample(*io_);
        }
    }

    void driveAll(uint32_t nowMs) override {
        if (!io_) return;
        for (uint8_t i = 0; i < swCount_; ++i) {
            swDrivers_[i].drive(*io_);
        }
        for (uint8_t i = 0; i < mastCount_; ++i) {
            mastDrivers_[i].drive(*io_, nowMs);
        }
        io_->flush();
    }

private:
    IOBus* io_;
    TrackCircuitDriver tcDrivers_[MAX_POLICY_DRIVERS];
    uint8_t tcCount_;
    SwitchDriver swDrivers_[MAX_POLICY_DRIVERS];
    uint8_t swCount_;
    SignalMastDriver mastDrivers_[MAX_POLICY_DRIVERS];
    uint8_t mastCount_;
};

// Fluent policy factory helpers
namespace DriverPolicies {

inline MqttDriverPolicy* MQTT(MqttApplianceBus& bus) {
    static MqttDriverPolicy s_mqttPolicy(&bus);
    s_mqttPolicy.setBus(&bus);
    return &s_mqttPolicy;
}

inline IoBitDriverPolicy* IOBit(IOBus& bus) {
    static IoBitDriverPolicy s_ioBitPolicy(&bus);
    s_ioBitPolicy.setIOBus(&bus);
    return &s_ioBitPolicy;
}

} // namespace DriverPolicies

} // namespace FieldUnit

#endif // FIELDUNIT_DRIVER_POLICY_H
