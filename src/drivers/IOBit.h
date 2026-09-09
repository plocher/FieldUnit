#ifndef FIELDUNIT_IOBIT_H
#define FIELDUNIT_IOBIT_H

#include <stdint.h>
#include <stdbool.h>

namespace FieldUnit {

// Electrical polarity of a physical or logical bit
enum class Polarity : uint8_t {
    NORMAL   = 0, // Active-High (1 = active/true)
    INVERTED = 1  // Active-Low  (0 = active/true, e.g. grounded switch, DCCOD)
};

// 3-coordinate hardware and network addressing tuple:
//   device:   Expander chip index (0..7), C/MRI Node UA, or MCU port ID
//   offset:   Port index (Port A=0, Port B=1), or byte offset in packet/buffer
//   bitIndex: Bit position within the byte/port (0 to 7)
// Note: named 'bitIndex' to avoid collision with Arduino.h's legacy macro #define bit(b)
struct IOBit {
    uint8_t  device;
    uint8_t  offset;
    uint8_t  bitIndex;
    Polarity polarity;

    constexpr IOBit()
        : device(0xFF), offset(0xFF), bitIndex(0xFF), polarity(Polarity::NORMAL) {}

    constexpr IOBit(uint8_t d, uint8_t off, uint8_t b, Polarity pol = Polarity::NORMAL)
        : device(d), offset(off), bitIndex(b), polarity(pol) {}

    bool isValid() const {
        return device != 0xFF && offset != 0xFF && bitIndex != 0xFF;
    }
};

// Strongly-typed direction wrappers to enforce compile-time wiring safety:
// An Output cannot be accidentally passed to a sensor input, and vice versa.
struct InputBit : public IOBit {
    constexpr InputBit() : IOBit() {}
    constexpr InputBit(uint8_t d, uint8_t off, uint8_t b, Polarity pol = Polarity::NORMAL)
        : IOBit(d, off, b, pol) {}
};

struct OutputBit : public IOBit {
    constexpr OutputBit() : IOBit() {}
    constexpr OutputBit(uint8_t d, uint8_t off, uint8_t b, Polarity pol = Polarity::NORMAL)
        : IOBit(d, off, b, pol) {}
};

} // namespace FieldUnit

#endif // FIELDUNIT_IOBIT_H
