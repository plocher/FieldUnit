#ifndef FIELDUNIT_TRACK_CIRCUIT_DRIVER_H
#define FIELDUNIT_TRACK_CIRCUIT_DRIVER_H

#include "IOBus.h"
#include "../TrackCircuit.h"

namespace FieldUnit {

class TrackCircuitDriver {
public:
    TrackCircuitDriver() : tc_(nullptr), sensor_{} {}

    TrackCircuitDriver(TrackCircuit* tc, InputBit sensor)
        : tc_(tc), sensor_(sensor) {}

    void sample(IOBus& io, uint32_t nowMs = 0) {
        if (!tc_ || !sensor_.isValid()) return;

        // readBit handles polarity inversion automatically
        bool isOccupied = io.readBit(sensor_);

        tc_->update(isOccupied ? Occupancy::OCCUPIED : Occupancy::VACANT, Quality::GOOD, nowMs);
    }

private:
    TrackCircuit* tc_;
    InputBit      sensor_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_TRACK_CIRCUIT_DRIVER_H
