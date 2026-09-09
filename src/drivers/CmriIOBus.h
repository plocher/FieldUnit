#ifndef FIELDUNIT_CMRI_IOBUS_H
#define FIELDUNIT_CMRI_IOBUS_H

#include <stddef.h>
#include "IOBus.h"

namespace FieldUnit {

// IOBus adapter backed by raw memory byte buffers (C/MRI IB[] and OB[] arrays)
// Enables centralized interlocking compute with remote I/O nodes (classic Bruce Chubb C/MRI model)
class CmriIOBus : public IOBus {
public:
    CmriIOBus() : ib_(nullptr), ibLen_(0), ob_(nullptr), obLen_(0) {}

    CmriIOBus(const uint8_t* inputBytes, size_t inputLen,
              uint8_t* outputBytes, size_t outputLen)
        : ib_(inputBytes), ibLen_(inputLen),
          ob_(outputBytes), obLen_(outputLen) {}

    void setBuffers(const uint8_t* inputBytes, size_t inputLen,
                    uint8_t* outputBytes, size_t outputLen) {
        ib_ = inputBytes;
        ibLen_ = inputLen;
        ob_ = outputBytes;
        obLen_ = outputLen;
    }

    // Read an input bit (from C/MRI IB[] array)
    // b.offset maps to byte index in IB[]; b.bitIndex maps to bit index (0..7)
    bool readBit(InputBit b) override {
        if (!b.isValid() || !ib_ || b.offset >= ibLen_) {
            return false;
        }
        bool raw = (ib_[b.offset] & (1 << b.bitIndex)) != 0;
        return (b.polarity == Polarity::INVERTED) ? !raw : raw;
    }

    // Write an output bit (to C/MRI OB[] array)
    // b.offset maps to byte index in OB[]; b.bitIndex maps to bit index (0..7)
    void writeBit(OutputBit b, bool value) override {
        if (!b.isValid() || !ob_ || b.offset >= obLen_) {
            return;
        }
        bool actual = (b.polarity == Polarity::INVERTED) ? !value : value;
        if (actual) {
            ob_[b.offset] |= (1 << b.bitIndex);
        } else {
            ob_[b.offset] &= ~(1 << b.bitIndex);
        }
    }

private:
    const uint8_t* ib_;
    size_t         ibLen_;
    uint8_t*       ob_;
    size_t         obLen_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_CMRI_IOBUS_H
