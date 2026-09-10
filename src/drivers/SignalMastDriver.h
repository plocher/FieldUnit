#ifndef FIELDUNIT_SIGNAL_MAST_DRIVER_H
#define FIELDUNIT_SIGNAL_MAST_DRIVER_H

#include "IOBus.h"
#include "../SignalMast.h"

namespace FieldUnit {

struct HeadPins {
    OutputBit redPin;
    OutputBit yellowPin;
    OutputBit greenPin;
    OutputBit lunarPin;
};

// Driver for multi-head color-light signal masts
class SignalMastDriver {
public:
    SignalMastDriver() : mast_(nullptr), headCount_(0) {}

    SignalMastDriver(SignalMast* mast) : mast_(mast), headCount_(0) {}

    void addHead(OutputBit red, OutputBit yellow, OutputBit green, OutputBit lunar = OutputBit()) {
        if (headCount_ < 3) {
            heads_[headCount_++] = {red, yellow, green, lunar};
        }
    }

    void drive(IOBus& io, uint32_t nowMs) {
        if (!mast_) return;

        bool flashPhase = (nowMs % 1000) < 500; // 1 Hz flash: 500ms ON / 500ms OFF

        if (headCount_ >= 1) driveHead(io, heads_[0], mast_->head1(), flashPhase);
        if (headCount_ >= 2) driveHead(io, heads_[1], mast_->head2(), flashPhase);
        if (headCount_ >= 3) driveHead(io, heads_[2], mast_->head3(), flashPhase);
    }

private:
    void driveHead(IOBus& io, const HeadPins& pins, Aspect aspect, bool flashPhase) {
        bool r = false, y = false, g = false, l = false;

        switch (aspect) {
            case Aspect::GREEN:
            case Aspect::RED_OVER_GREEN:
                g = true;
                break;
            case Aspect::FLASHING_GREEN:
                g = flashPhase;
                break;
            case Aspect::YELLOW:
            case Aspect::RED_OVER_YELLOW:
            case Aspect::YELLOW_OVER_RED:
            case Aspect::YELLOW_OVER_YELLOW:
                y = true;
                break;
            case Aspect::FLASHING_YELLOW:
            case Aspect::YELLOW_OVER_GREEN:
            case Aspect::RED_OVER_FLASHING_YELLOW:
                y = flashPhase;
                break;
            case Aspect::LUNAR:
            case Aspect::RED_OVER_LUNAR:
                l = true;
                break;
            case Aspect::FLASHING_LUNAR:
                l = flashPhase;
                break;
            case Aspect::FLASHING_RED:
            case Aspect::RED_OVER_FLASHING_RED:
            case Aspect::FLASHING_RED_OVER_RED:
                r = flashPhase;
                break;
            case Aspect::RED:
            case Aspect::RED_OVER_RED:
            default:
                r = true;
                break;
            case Aspect::DARK:
                break;
        }

        if (pins.redPin.isValid())    io.writeBit(pins.redPin, r);
        if (pins.yellowPin.isValid()) io.writeBit(pins.yellowPin, y);
        if (pins.greenPin.isValid())  io.writeBit(pins.greenPin, g);
        if (pins.lunarPin.isValid())  io.writeBit(pins.lunarPin, l);
    }

    SignalMast* mast_;
    HeadPins heads_[3];
    uint8_t headCount_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_SIGNAL_MAST_DRIVER_H
