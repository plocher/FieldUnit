#ifndef FIELDUNIT_IOBUS_H
#define FIELDUNIT_IOBUS_H

#include "IOBit.h"

namespace FieldUnit {

// Abstract physical I/O bus interface with typed input/output directionality
class IOBus {
public:
    virtual ~IOBus() = default;
    virtual bool readBit(InputBit bit) = 0;
    virtual void writeBit(OutputBit bit, bool value) = 0;
    virtual void flush() {}
};

// Mock I/O bus for desktop unit tests and simulation
class MockIOBus : public IOBus {
public:
    static constexpr uint8_t MAX_DEVICES = 8;
    static constexpr uint8_t MAX_OFFSETS = 8; // e.g. Ports A-H or Bytes 0-7

    MockIOBus() {
        for (uint8_t d = 0; d < MAX_DEVICES; ++d) {
            for (uint8_t off = 0; off < MAX_OFFSETS; ++off) {
                pins_[d][off] = 0;
            }
        }
    }

    bool readBit(InputBit bit) override {
        if (!bit.isValid() || bit.device >= MAX_DEVICES || bit.offset >= MAX_OFFSETS) {
            return false;
        }
        bool raw = (pins_[bit.device][bit.offset] & (1 << bit.bitIndex)) != 0;
        return (bit.polarity == Polarity::INVERTED) ? !raw : raw;
    }

    void writeBit(OutputBit bit, bool value) override {
        if (!bit.isValid() || bit.device >= MAX_DEVICES || bit.offset >= MAX_OFFSETS) {
            return;
        }
        bool actual = (bit.polarity == Polarity::INVERTED) ? !value : value;
        if (actual) {
            pins_[bit.device][bit.offset] |= (1 << bit.bitIndex);
        } else {
            pins_[bit.device][bit.offset] &= ~(1 << bit.bitIndex);
        }
    }

    // Test bench helper to simulate field contacts (e.g. limit switch closing)
    void setPinState(InputBit bit, bool physicalActive) {
        if (!bit.isValid() || bit.device >= MAX_DEVICES || bit.offset >= MAX_OFFSETS) {
            return;
        }
        bool raw = (bit.polarity == Polarity::INVERTED) ? !physicalActive : physicalActive;
        if (raw) {
            pins_[bit.device][bit.offset] |= (1 << bit.bitIndex);
        } else {
            pins_[bit.device][bit.offset] &= ~(1 << bit.bitIndex);
        }
    }

    bool readOutputRaw(OutputBit bit) const {
        if (!bit.isValid() || bit.device >= MAX_DEVICES || bit.offset >= MAX_OFFSETS) {
            return false;
        }
        bool raw = (pins_[bit.device][bit.offset] & (1 << bit.bitIndex)) != 0;
        return (bit.polarity == Polarity::INVERTED) ? !raw : raw;
    }

private:
    uint8_t pins_[MAX_DEVICES][MAX_OFFSETS];
};

} // namespace FieldUnit

#endif // FIELDUNIT_IOBUS_H
