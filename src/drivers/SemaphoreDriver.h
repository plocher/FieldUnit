#ifndef FIELDUNIT_SEMAPHORE_DRIVER_H
#define FIELDUNIT_SEMAPHORE_DRIVER_H

#include "IOBus.h"
#include "../SignalMast.h"

namespace FieldUnit {

// Physical configuration for an individual mechanical semaphore blade / arm
struct SemaphoreArm {
    uint8_t  device;        // Servo controller or expander board index
    uint8_t  channel;       // Servo channel / PWM pin index
    uint16_t stopAngle;     // Angle in degrees for STOP (horizontal)
    uint16_t approachAngle; // Angle in degrees for APPROACH / CAUTION (45 deg)
    uint16_t clearAngle;    // Angle in degrees for CLEAR / PROCEED (90 deg vertical)
};

// Driver for servo-actuated semaphore signal masts (Upper or Lower Quadrant)
class SemaphoreDriver {
public:
    SemaphoreDriver() : mast_(nullptr), armCount_(0) {}

    SemaphoreDriver(SignalMast* mast) : mast_(mast), armCount_(0) {}

    void addArm(uint8_t device, uint8_t channel,
                uint16_t stopAngle = 0,
                uint16_t approachAngle = 45,
                uint16_t clearAngle = 90) {
        if (armCount_ < 3) {
            arms_[armCount_++] = {device, channel, stopAngle, approachAngle, clearAngle};
        }
    }

    void drive(IOBus& io) {
        if (!mast_) return;

        if (armCount_ >= 1) driveArm(io, arms_[0], mast_->head1());
        if (armCount_ >= 2) driveArm(io, arms_[1], mast_->head2());
        if (armCount_ >= 3) driveArm(io, arms_[2], mast_->head3());
    }

private:
    void driveArm(IOBus& io, const SemaphoreArm& arm, Aspect aspect) {
        uint16_t targetAngle = arm.stopAngle;

        switch (aspect) {
            case Aspect::GREEN:
            case Aspect::FLASHING_GREEN:
                targetAngle = arm.clearAngle;
                break;
            case Aspect::YELLOW:
            case Aspect::FLASHING_YELLOW:
                targetAngle = arm.approachAngle;
                break;
            case Aspect::RED:
            case Aspect::FLASHING_RED:
            case Aspect::DARK:
            default:
                targetAngle = arm.stopAngle;
                break;
        }

        io.writeAngle(arm.device, arm.channel, targetAngle);
    }

    SignalMast*  mast_;
    SemaphoreArm arms_[3];
    uint8_t      armCount_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_SEMAPHORE_DRIVER_H
