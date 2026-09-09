#ifndef FIELDUNIT_SWITCH_DRIVER_H
#define FIELDUNIT_SWITCH_DRIVER_H

#include "IOBus.h"
#include "../Switch.h"

namespace FieldUnit {

// Standard Tortoise / motor switch machine driver
// Controls 1 motor output bit (Normal=true, Reverse=false)
// Reads 2 contact sense input bits (Normal closed, Reverse closed)
class SwitchDriver {
public:
    SwitchDriver()
        : sw_(nullptr), motor_{}, normalSense_{}, reverseSense_{} {}

    SwitchDriver(Switch* sw, OutputBit motor, InputBit normalSense, InputBit reverseSense)
        : sw_(sw),
          motor_(motor),
          normalSense_(normalSense),
          reverseSense_(reverseSense) {}

    // Sample physical limit switches / point detector contacts
    void sample(IOBus& io) {
        if (!sw_) return;

        bool nClosed = false;
        bool rClosed = false;

        if (normalSense_.isValid()) {
            nClosed = io.readBit(normalSense_);
        }

        if (reverseSense_.isValid()) {
            rClosed = io.readBit(reverseSense_);
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

    // Drive physical motor bit
    void drive(IOBus& io) {
        if (!sw_ || !motor_.isValid()) return;

        bool motorVal = (sw_->commandedPosition() == SwitchPosition::NORMAL);
        io.writeBit(motor_, motorVal);
    }

private:
    Switch*   sw_;
    OutputBit motor_;
    InputBit  normalSense_;
    InputBit  reverseSense_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_SWITCH_DRIVER_H
