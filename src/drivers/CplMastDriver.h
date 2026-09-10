#ifndef FIELDUNIT_CPL_MAST_DRIVER_H
#define FIELDUNIT_CPL_MAST_DRIVER_H

#include "IOBus.h"
#include "../SignalMast.h"
#include "../types.h"

namespace FieldUnit {

/**
 * Physical pin coordinates for B&O CPL central cluster lamp pairs
 */
struct CplDiskPins {
    OutputBit redPin;    // Horizontal lamp pair (Stop)
    OutputBit yellowPin; // 45 deg diagonal lamp pair (Approach)
    OutputBit greenPin;  // Vertical lamp pair (Clear)
    OutputBit lunarPin;  // 135 deg diagonal lamp pair (Restricting)
};

/**
 * Physical pin coordinates for B&O CPL orbital marker lamps
 */
struct CplMarkerPins {
    OutputBit top12;     // 12 o'clock: Normal Speed route (High Speed)
    OutputBit upperR2;   // 2 o'clock: Medium Speed route
    OutputBit lowerR4;   // 4 o'clock: Limited Speed route
    OutputBit bottom6;   // 6 o'clock: Slow Speed route / Stop & Proceed
    OutputBit upperL10;  // 10 o'clock: Cab Speed / Advance
    OutputBit lowerL8;   // 8 o'clock: Auxiliary / Restricting (low marker)
};

/**
 * Driver for Baltimore & Ohio (B&O) Color-Position-Light (CPL) signal masts.
 * Drives central cluster lamp pairs and up to six perimeter orbital markers.
 */
class CplMastDriver {
public:
    CplMastDriver() : mast_(nullptr) {}

    CplMastDriver(SignalMast* mast) : mast_(mast) {}

    void setDiskPins(OutputBit red, OutputBit yellow, OutputBit green, OutputBit lunar = OutputBit()) {
        disk_ = {red, yellow, green, lunar};
    }

    void setMarkerPins(OutputBit top12, OutputBit upperR2 = OutputBit(),
                       OutputBit lowerR4 = OutputBit(), OutputBit bottom6 = OutputBit(),
                       OutputBit upperL10 = OutputBit(), OutputBit lowerL8 = OutputBit()) {
        markers_ = {top12, upperR2, lowerR4, bottom6, upperL10, lowerL8};
    }

    void drive(IOBus& io, uint32_t nowMs) {
        if (!mast_) return;

        bool flashPhase = (nowMs % 1000) < 500; // 1 Hz flash: 500ms ON / 500ms OFF

        // 1. Drive central disk lamp pairs
        bool r = false, y = false, g = false, l = false;
        Aspect aspect = mast_->head1();

        switch (aspect) {
            case Aspect::GREEN:
                g = true;
                break;
            case Aspect::FLASHING_GREEN:
                g = flashPhase;
                break;
            case Aspect::YELLOW:
                y = true;
                break;
            case Aspect::FLASHING_YELLOW:
                y = flashPhase;
                break;
            case Aspect::LUNAR:
                l = true;
                break;
            case Aspect::FLASHING_LUNAR:
                l = flashPhase;
                break;
            case Aspect::FLASHING_RED:
                r = flashPhase;
                break;
            case Aspect::RED:
            default:
                r = true;
                break;
            case Aspect::DARK:
                break;
        }

        if (disk_.redPin.isValid())    io.writeBit(disk_.redPin, r);
        if (disk_.yellowPin.isValid()) io.writeBit(disk_.yellowPin, y);
        if (disk_.greenPin.isValid())  io.writeBit(disk_.greenPin, g);
        if (disk_.lunarPin.isValid())  io.writeBit(disk_.lunarPin, l);

        // 2. Drive orbital markers
        uint8_t m = mast_->markers();

        bool m12 = (m & static_cast<uint8_t>(CplMarker::TOP_12)) != 0;
        bool m2  = (m & static_cast<uint8_t>(CplMarker::UPPER_R_2)) != 0;
        bool m4  = (m & static_cast<uint8_t>(CplMarker::LOWER_R_4)) != 0;
        bool m6  = (m & static_cast<uint8_t>(CplMarker::BOTTOM_6)) != 0;
        bool m10 = (m & static_cast<uint8_t>(CplMarker::UPPER_L_10)) != 0;
        bool m8  = (m & static_cast<uint8_t>(CplMarker::LOWER_L_8)) != 0;

        if (markers_.top12.isValid())    io.writeBit(markers_.top12, m12);
        if (markers_.upperR2.isValid())  io.writeBit(markers_.upperR2, m2);
        if (markers_.lowerR4.isValid())  io.writeBit(markers_.lowerR4, m4);
        if (markers_.bottom6.isValid())  io.writeBit(markers_.bottom6, m6);
        if (markers_.upperL10.isValid()) io.writeBit(markers_.upperL10, m10);
        if (markers_.lowerL8.isValid())  io.writeBit(markers_.lowerL8, m8);
    }

private:
    SignalMast* mast_;
    CplDiskPins disk_{};
    CplMarkerPins markers_{};
};

} // namespace FieldUnit

#endif // FIELDUNIT_CPL_MAST_DRIVER_H
