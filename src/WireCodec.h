#ifndef FIELDUNIT_WIRE_CODEC_H
#define FIELDUNIT_WIRE_CODEC_H

#include "types.h"
#include "ControlPoint.h"

namespace FieldUnit {

// Mapping definitions for AAR CodeLine control bits
struct SwitchControlMap {
    uint8_t switchIndex;
    uint8_t normalByte;
    uint8_t normalBit;
    uint8_t reverseByte;
    uint8_t reverseBit;
};

struct SignalControlMap {
    uint8_t signalIndex;
    uint8_t byteIndex;
    uint8_t southBit; // e.g. bit 0 (2SG)
    uint8_t northBit; // e.g. bit 1 (2NG)
    uint8_t stopBit;  // e.g. bit 2 (2H)
};

struct MaintainerControlMap {
    uint8_t mcIndex;
    uint8_t byteIndex;
    uint8_t bitIndex;
};

// Mapping definitions for AAR CodeLine indication bits
struct SwitchIndicationMap {
    uint8_t switchIndex;
    uint8_t normalByte;
    uint8_t normalBit;  // NWK
    uint8_t reverseByte;
    uint8_t reverseBit; // RWK
};

struct TrackIndicationMap {
    uint8_t trackIndex;
    uint8_t byteIndex;
    uint8_t bitIndex;
    bool    activeHigh; // true if 1 = occupied (standard AAR)
};

struct SignalIndicationMap {
    uint8_t signalIndex;
    uint8_t byteIndex;
    uint8_t southBit;       // SGK
    uint8_t northBit;       // NGK
    uint8_t timeElementBit; // TEK
};

static constexpr uint8_t MAX_MAP_ENTRIES = 16;
static constexpr uint8_t MAX_CODE_BYTES  = 8;

class CodeLineCodec {
public:
    CodeLineCodec(uint8_t controlByteCount, uint8_t indicationByteCount)
        : controlByteCount_(controlByteCount),
          indicationByteCount_(indicationByteCount),
          switchControlMapCount_(0),
          signalControlMapCount_(0),
          mcControlMapCount_(0),
          switchIndMapCount_(0),
          trackIndMapCount_(0),
          signalIndMapCount_(0) {}

    uint8_t expectedControlBytes() const { return controlByteCount_; }
    uint8_t expectedIndicationBytes() const { return indicationByteCount_; }

    void mapSwitchControl(uint8_t swIdx, uint8_t nByte, uint8_t nBit, uint8_t rByte, uint8_t rBit) {
        if (switchControlMapCount_ < MAX_MAP_ENTRIES) {
            switchControlMaps_[switchControlMapCount_++] = {swIdx, nByte, nBit, rByte, rBit};
        }
    }

    void mapSignalControl(uint8_t sigIdx, uint8_t byteIdx, uint8_t sBit, uint8_t nBit, uint8_t hBit) {
        if (signalControlMapCount_ < MAX_MAP_ENTRIES) {
            signalControlMaps_[signalControlMapCount_++] = {sigIdx, byteIdx, sBit, nBit, hBit};
        }
    }

    void mapMaintainerControl(uint8_t mcIdx, uint8_t byteIdx, uint8_t bitIdx) {
        if (mcControlMapCount_ < MAX_MAP_ENTRIES) {
            mcControlMaps_[mcControlMapCount_++] = {mcIdx, byteIdx, bitIdx};
        }
    }

    void mapSwitchIndication(uint8_t swIdx, uint8_t nByte, uint8_t nBit, uint8_t rByte, uint8_t rBit) {
        if (switchIndMapCount_ < MAX_MAP_ENTRIES) {
            switchIndMaps_[switchIndMapCount_++] = {swIdx, nByte, nBit, rByte, rBit};
        }
    }

    void mapTrackIndication(uint8_t trackIdx, uint8_t byteIdx, uint8_t bitIdx, bool activeHigh = true) {
        if (trackIndMapCount_ < MAX_MAP_ENTRIES) {
            trackIndMaps_[trackIndMapCount_++] = {trackIdx, byteIdx, bitIdx, activeHigh};
        }
    }

    void mapSignalIndication(uint8_t sigIdx, uint8_t byteIdx, uint8_t sBit, uint8_t nBit, uint8_t teBit) {
        if (signalIndMapCount_ < MAX_MAP_ENTRIES) {
            signalIndMaps_[signalIndMapCount_++] = {sigIdx, byteIdx, sBit, nBit, teBit};
        }
    }

    // Unpack raw wire bytes into a plant-complete ControlTransaction
    // Returns false if packet is truncated or contains orthogonal bit violations (corrupted)
    bool unpackControls(const uint8_t* bytes, size_t length, ControlTransaction& ctl) const {
        if (length < controlByteCount_) {
            return false; // Truncated packet
        }

        ctl = ControlTransaction(); // Reset to clean defaults

        // 1. Unpack switch controls (AAR NW and RW bits)
        for (uint8_t i = 0; i < switchControlMapCount_; ++i) {
            const SwitchControlMap& m = switchControlMaps_[i];
            bool nBit = (bytes[m.normalByte] & (1 << m.normalBit)) != 0;
            bool rBit = (bytes[m.reverseByte] & (1 << m.reverseBit)) != 0;

            if (nBit && rBit) {
                return false; // Corrupted: switch commanded both Normal and Reverse
            }
            if (nBit) {
                ctl.switchDemands[m.switchIndex] = SwitchDemand::NORMAL;
            } else if (rBit) {
                ctl.switchDemands[m.switchIndex] = SwitchDemand::REVERSE;
            } else {
                ctl.switchDemands[m.switchIndex] = SwitchDemand::NO_CHANGE;
            }
        }

        // 2. Unpack signal controls (AAR SG, NG, H bits)
        for (uint8_t i = 0; i < signalControlMapCount_; ++i) {
            const SignalControlMap& m = signalControlMaps_[i];
            bool sBit = (bytes[m.byteIndex] & (1 << m.southBit)) != 0;
            bool nBit = (bytes[m.byteIndex] & (1 << m.northBit)) != 0;
            bool hBit = (bytes[m.byteIndex] & (1 << m.stopBit)) != 0;

            uint8_t activeCount = (sBit ? 1 : 0) + (nBit ? 1 : 0) + (hBit ? 1 : 0);
            if (activeCount > 1) {
                return false; // Corrupted: conflicting signal direction bits
            }

            if (hBit) {
                ctl.signalDemands[m.signalIndex] = SignalDemand::STOP;
            } else if (nBit) {
                ctl.signalDemands[m.signalIndex] = SignalDemand::LEFT;
            } else if (sBit) {
                ctl.signalDemands[m.signalIndex] = SignalDemand::RIGHT;
            } else {
                ctl.signalDemands[m.signalIndex] = SignalDemand::NO_CHANGE;
            }
        }

        // 3. Unpack maintainer calls
        for (uint8_t i = 0; i < mcControlMapCount_; ++i) {
            const MaintainerControlMap& m = mcControlMaps_[i];
            bool mcBit = (bytes[m.byteIndex] & (1 << m.bitIndex)) != 0;
            ctl.maintainerCall[m.mcIndex] = mcBit;
        }

        return true;
    }

    // Pack plant-complete IndicationVector into raw wire bytes
    bool packIndications(const IndicationVector& ind, uint8_t* outBytes, size_t maxLength) const {
        if (maxLength < indicationByteCount_) {
            return false;
        }

        for (uint8_t b = 0; b < indicationByteCount_; ++b) {
            outBytes[b] = 0;
        }

        // 1. Pack switch correspondence (NWK / RWK)
        for (uint8_t i = 0; i < switchIndMapCount_; ++i) {
            const SwitchIndicationMap& m = switchIndMaps_[i];
            if (m.switchIndex < ind.switchCount) {
                const SwitchIndication& s = ind.switches[m.switchIndex];
                if (s.inCorrespondence && s.position == SwitchPosition::NORMAL) {
                    outBytes[m.normalByte] |= (1 << m.normalBit);
                } else if (s.inCorrespondence && s.position == SwitchPosition::REVERSE) {
                    outBytes[m.reverseByte] |= (1 << m.reverseBit);
                }
                // Out-of-correspondence or moving leaves both bits 0
            }
        }

        // 2. Pack track occupancies (TR)
        for (uint8_t i = 0; i < trackIndMapCount_; ++i) {
            const TrackIndicationMap& m = trackIndMaps_[i];
            if (m.trackIndex < ind.trackCircuitCount) {
                const TrackCircuitIndication& t = ind.trackCircuits[m.trackIndex];
                bool occ = (t.occupancy == Occupancy::OCCUPIED);
                if (!m.activeHigh) occ = !occ;
                if (occ) {
                    outBytes[m.byteIndex] |= (1 << m.bitIndex);
                }
            }
        }

        // 3. Pack signal indications (SGK, NGK, TEK)
        for (uint8_t i = 0; i < signalIndMapCount_; ++i) {
            const SignalIndicationMap& m = signalIndMaps_[i];
            if (m.signalIndex < ind.signalCount) {
                const SignalIndication& s = ind.signals[m.signalIndex];
                if (s.activeAuthority == DirectionAuthority::RIGHT) {
                    outBytes[m.byteIndex] |= (1 << m.southBit);
                } else if (s.activeAuthority == DirectionAuthority::LEFT) {
                    outBytes[m.byteIndex] |= (1 << m.northBit);
                }
                if (s.timeLocked) {
                    outBytes[m.byteIndex] |= (1 << m.timeElementBit);
                }
            }
        }

        return true;
    }

private:
    uint8_t controlByteCount_;
    uint8_t indicationByteCount_;

    SwitchControlMap switchControlMaps_[MAX_MAP_ENTRIES];
    uint8_t switchControlMapCount_;

    SignalControlMap signalControlMaps_[MAX_MAP_ENTRIES];
    uint8_t signalControlMapCount_;

    MaintainerControlMap mcControlMaps_[MAX_MAP_ENTRIES];
    uint8_t mcControlMapCount_;

    SwitchIndicationMap switchIndMaps_[MAX_MAP_ENTRIES];
    uint8_t switchIndMapCount_;

    TrackIndicationMap trackIndMaps_[MAX_MAP_ENTRIES];
    uint8_t trackIndMapCount_;

    SignalIndicationMap signalIndMaps_[MAX_MAP_ENTRIES];
    uint8_t signalIndMapCount_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_WIRE_CODEC_H
