#ifndef FIELDUNIT_IMAGE_IOBUS_H
#define FIELDUNIT_IMAGE_IOBUS_H

#include <stddef.h>
#include "IOBus.h"

namespace FieldUnit {

// IOBus adapter backed by raw memory byte buffers (CMRI IB[] and OB[] images)
// Enables centralized interlocking compute with remote I/O nodes (classic Bruce Chubb C/MRI model)
class ImageIOBus : public IOBus {
public:
    ImageIOBus() : ib_(nullptr), ibLen_(0), ob_(nullptr), obLen_(0) {}

    ImageIOBus(const uint8_t* inputBytes, size_t inputLen,
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

    // Read an input bit (from CMRI IB[] array)
    // pin.device maps to byte index; pin.pin maps to bit index (0..7)
    bool readBit(IOPin pin) override {
        if (!pin.isValid() || !ib_ || pin.device >= ibLen_) {
            return false;
        }
        return (ib_[pin.device] & (1 << pin.pin)) != 0;
    }

    // Write an output bit (to CMRI OB[] array)
    // pin.device maps to byte index; pin.pin maps to bit index (0..7)
    void writeBit(IOPin pin, bool value) override {
        if (!pin.isValid() || !ob_ || pin.device >= obLen_) {
            return;
        }
        if (value) {
            ob_[pin.device] |= (1 << pin.pin);
        } else {
            ob_[pin.device] &= ~(1 << pin.pin);
        }
    }

private:
    const uint8_t* ib_;
    size_t         ibLen_;
    uint8_t*       ob_;
    size_t         obLen_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_IMAGE_IOBUS_H
