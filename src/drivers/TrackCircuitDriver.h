#ifndef FIELDUNIT_TRACK_CIRCUIT_DRIVER_H
#define FIELDUNIT_TRACK_CIRCUIT_DRIVER_H

#include "IOBus.h"
#include "../TrackCircuit.h"

namespace FieldUnit {

class TrackCircuitDriver {
public:
    TrackCircuitDriver() : tc_(nullptr), pin_{}, activeLow_(true) {}

    TrackCircuitDriver(TrackCircuit* tc, IOPin pin, bool activeLow = true)
        : tc_(tc), pin_(pin), activeLow_(activeLow) {}

    void sample(IOBus& io, uint32_t nowMs = 0) {
        if (!tc_ || !pin_.isValid()) return;

        bool rawBit = io.readBit(pin_);
        bool isOccupied = activeLow_ ? !rawBit : rawBit;

        tc_->update(isOccupied ? Occupancy::OCCUPIED : Occupancy::VACANT, Quality::GOOD, nowMs);
    }

private:
    TrackCircuit* tc_;
    IOPin pin_;
    bool activeLow_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_TRACK_CIRCUIT_DRIVER_H
