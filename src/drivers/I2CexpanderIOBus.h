#ifndef FIELDUNIT_I2CEXPANDER_IOBUS_H
#define FIELDUNIT_I2CEXPANDER_IOBUS_H

#if defined(ARDUINO)
#include <Arduino.h>
#include <I2Cexpander.h>
#include "IOBus.h"

namespace FieldUnit {

// Physical IOBus adapter for the I2Cexpander library
// Binds logical IOPin(device, pin) directly to an array of I2Cexpander chips (e.g. MCP23017, cpNode-IOX)
class I2CexpanderIOBus : public IOBus {
public:
    I2CexpanderIOBus() : expanders_(nullptr), count_(0) {}

    I2CexpanderIOBus(I2Cexpander* expanders, size_t count)
        : expanders_(expanders), count_(count) {}

    void setExpanders(I2Cexpander* expanders, size_t count) {
        expanders_ = expanders;
        count_ = count;
    }

    bool readBit(IOPin pin) override {
        if (!pin.isValid() || !expanders_ || pin.device >= count_) {
            return false;
        }
        return expanders_[pin.device].digitalRead(pin.pin) != 0;
    }

    void writeBit(IOPin pin, bool value) override {
        if (!pin.isValid() || !expanders_ || pin.device >= count_) {
            return;
        }
        expanders_[pin.device].digitalWrite(pin.pin, value ? 1 : 0);
    }

private:
    I2Cexpander* expanders_;
    size_t count_;
};

} // namespace FieldUnit

#endif // ARDUINO
#endif // FIELDUNIT_I2CEXPANDER_IOBUS_H
