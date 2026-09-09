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

    bool readBit(InputBit b) override {
        if (!b.isValid() || !expanders_ || b.device >= count_) {
            return false;
        }
        uint8_t pinIndex = (b.offset * 8) + b.bitIndex;
        bool raw = expanders_[b.device].digitalRead(pinIndex) != 0;
        return (b.polarity == Polarity::INVERTED) ? !raw : raw;
    }

    void writeBit(OutputBit b, bool value) override {
        if (!b.isValid() || !expanders_ || b.device >= count_) {
            return;
        }
        uint8_t pinIndex = (b.offset * 8) + b.bitIndex;
        bool actual = (b.polarity == Polarity::INVERTED) ? !value : value;
        expanders_[b.device].digitalWrite(pinIndex, actual ? 1 : 0);
    }

private:
    I2Cexpander* expanders_;
    size_t count_;
};

} // namespace FieldUnit

#endif // ARDUINO
#endif // FIELDUNIT_I2CEXPANDER_IOBUS_H
