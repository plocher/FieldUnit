#ifndef FIELDUNIT_SWITCH_DRIVER_H
#define FIELDUNIT_SWITCH_DRIVER_H

#include "IOBus.h"
#include "../Switch.h"

namespace FieldUnit {

// Standard Tortoise / motor switch machine driver
// Controls 1 motor output pin (Normal=HIGH/Reverse=LOW)
// Reads 2 contact sense pins (Normal closed, Reverse closed)
class SwitchDriver {
public:
    SwitchDriver()
        : sw_(nullptr), motorPin_{}, normalSensePin_{}, reverseSensePin_{}, activeLowSense_(true) {}

    SwitchDriver(Switch* sw, IOPin motorPin, IOPin normalSense, IOPin reverseSense, bool activeLowSense = true)
        : sw_(sw),
          motorPin_(motorPin),
          normalSensePin_(normalSense),
          reverseSensePin_(reverseSense),
          activeLowSense_(activeLowSense) {}

    // Sample physical limit switches / point detector contacts
    void sample(IOBus& io) {
        if (!sw_) return;

        bool nClosed = false;
        bool rClosed = false;

        if (normalSensePin_.isValid()) {
            bool rawN = io.readBit(normalSensePin_);
            nClosed = activeLowSense_ ? !rawN : rawN;
        }

        if (reverseSensePin_.isValid()) {
            bool rawR = io.readBit(reverseSensePin_);
            rClosed = activeLowSense_ ? !rawR : rawR;
        }

        if (nClosed && !rClosed) {
            sw_->updateFeedback(SwitchPosition::NORMAL);
        } else if (!nClosed && rClosed) {
            sw_->updateFeedback(SwitchPosition::REVERSE);
        } else if (nClosed && rClosed) {
            // Both contacts closed simultaneously indicates a mechanical or electrical fault
            sw_->updateFeedback(SwitchPosition::OUT_OF_CORRESPONDENCE);
        } else {
            // Neither contact closed: points are in flight / moving
            sw_->updateFeedback(SwitchPosition::MOVING);
        }
    }

    // Drive physical motor pin
    void drive(IOBus& io) {
        if (!sw_ || !motorPin_.isValid()) return;

        bool motorVal = (sw_->commandedPosition() == SwitchPosition::NORMAL);
        io.writeBit(motorPin_, motorVal);
    }

private:
    Switch* sw_;
    IOPin motorPin_;
    IOPin normalSensePin_;
    IOPin reverseSensePin_;
    bool  activeLowSense_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_SWITCH_DRIVER_H
