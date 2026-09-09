#ifndef FIELDUNIT_IOPIN_H
#define FIELDUNIT_IOPIN_H

#include <stdint.h>
#include <stdbool.h>

namespace FieldUnit {

// Physical pin or expander bit reference
// Adopts the standard (device, pin) convention matching I2Cexpander and cpNode-IOX
// Note: named 'pin' rather than 'bit' to avoid Arduino.h's legacy macro #define bit(b)
struct IOPin {
    uint8_t device; // 0..N device/expander index, or 0xFF if unassigned
    uint8_t pin;    // 0..15 pin / bit index on that device

    constexpr IOPin() : device(0xFF), pin(0xFF) {}
    constexpr IOPin(uint8_t d, uint8_t p) : device(d), pin(p) {}

    bool isValid() const {
        return device != 0xFF && pin != 0xFF;
    }
};

} // namespace FieldUnit

#endif // FIELDUNIT_IOPIN_H
