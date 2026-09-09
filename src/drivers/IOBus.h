#ifndef FIELDUNIT_IOBUS_H
#define FIELDUNIT_IOBUS_H

#include "IOPin.h"

namespace FieldUnit {

// Abstract physical I/O bus interface
// Implemented by I2Cexpander on Arduino, or MockIOBus on desktop
class IOBus {
public:
    virtual ~IOBus() = default;
    virtual bool readBit(IOPin pin) = 0;
    virtual void writeBit(IOPin pin, bool value) = 0;
    virtual void flush() {}
};

// Mock I/O bus for desktop unit tests and simulation
class MockIOBus : public IOBus {
public:
    static constexpr uint8_t MAX_DEVICES = 8;
    static constexpr uint8_t BITS_PER_DEVICE = 16;

    MockIOBus() {
        for (uint8_t d = 0; d < MAX_DEVICES; ++d) {
            pins_[d] = 0;
        }
    }

    bool readBit(IOPin pin) override {
        if (!pin.isValid() || pin.device >= MAX_DEVICES) return false;
        return (pins_[pin.device] & (1 << pin.pin)) != 0;
    }

    void writeBit(IOPin pin, bool value) override {
        if (!pin.isValid() || pin.device >= MAX_DEVICES) return;
        if (value) {
            pins_[pin.device] |= (1 << pin.pin);
        } else {
            pins_[pin.device] &= ~(1 << pin.pin);
        }
    }

    // Test bench helper to simulate field contacts (e.g. limit switch closing)
    void setPinState(IOPin pin, bool value) {
        writeBit(pin, value);
    }

private:
    uint16_t pins_[MAX_DEVICES];
};

} // namespace FieldUnit

#endif // FIELDUNIT_IOBUS_H
