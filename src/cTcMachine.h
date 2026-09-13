#ifndef FIELDUNIT_CTC_MACHINE_H
#define FIELDUNIT_CTC_MACHINE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <initializer_list>
#include "types.h"
#include "ControlPoint.h"
#include "WireCodec.h"

namespace FieldUnit {

// =============================================================================
// 1. Panel Function Vocabulary (Domain Operations on a Column)
// =============================================================================

enum class PanelInput : uint8_t {
    SW_NORMAL,           // Switch lever at Normal
    SW_REVERSE,          // Switch lever at Reverse
    SIG_LEFT,            // Signal lever at Left (East)
    SIG_STOP,            // Signal lever at Center (Stop)
    SIG_RIGHT,           // Signal lever at Right (West)
    CODE_BUTTON,         // Station Code pushbutton
    MAINTAINER_CALL_SW,  // Maintainer call toggle switch
    LOCK_RELEASE_SW      // Electric lock release toggle/pushbutton
};

enum class PanelOutput : uint8_t {
    SW_NORMAL_LAMP,      // Normal correspondence indicator (NK)
    SW_REVERSE_LAMP,     // Reverse correspondence indicator (RK)
    SIG_LEFT_LAMP,       // Signal Left clear indicator (LK/EK)
    SIG_STOP_LAMP,       // Signal Stop indicator (SK)
    SIG_RIGHT_LAMP,      // Signal Right clear indicator (WK)
    MAINTAINER_LAMP,     // Maintainer call indicator (MCK)
    TRACK_LAMP_1,        // Track model diagram lamp 1
    TRACK_LAMP_2,        // Track model diagram lamp 2
    TRACK_LAMP_3,        // Track model diagram lamp 3
    TRACK_LAMP_4,        // Track model diagram lamp 4
    TRACK_LAMP_5,        // Track model diagram lamp 5
    TRACK_LAMP_6         // Track model diagram lamp 6
};

// =============================================================================
// 2. Abstract Hardware Interface [read / write][column, function]
// =============================================================================

class PanelHardware {
public:
    virtual ~PanelHardware() = default;
    virtual bool read(uint8_t column, PanelInput fn) = 0;
    virtual void write(uint8_t column, PanelOutput fn, bool state) = 0;
    virtual void begin() {}
    virtual void syncInputs() {}
    virtual void syncOutputs() {}
};

// =============================================================================
// 3. PanelColumn (Physical Faceplate Slice)
// =============================================================================

class CtcStation;

static constexpr uint8_t MAX_LAMPS_PER_COLUMN = 6;
static constexpr uint8_t MAX_NAME_LEN = 16;

class PanelColumn {
public:
    PanelColumn()
        : parentStation_(nullptr),
          columnNumber_(0),
          hasSwitch_(false),
          swIdx_(0),
          hasSignal_(false),
          sigIdx_(0),
          hasCodeButton_(false),
          hasMaintainerCall_(false),
          mcIdx_(0),
          tcStartIdx_(0),
          trackCount_(0) {
        swNum_[0] = '\0';
        sigNum_[0] = '\0';
        mcNum_[0] = '\0';
        for (uint8_t i = 0; i < MAX_LAMPS_PER_COLUMN; ++i) {
            trackNames_[i][0] = '\0';
        }
    }

    explicit PanelColumn(uint8_t columnNumber, CtcStation* parent = nullptr)
        : PanelColumn() {
        columnNumber_ = columnNumber;
        parentStation_ = parent;
    }

    uint8_t columnNumber() const { return columnNumber_; }
    void setParent(CtcStation* parent) { parentStation_ = parent; }

    // Fluent transition to another column within the same station
    PanelColumn& inColumn(uint8_t nextColumn);

    PanelColumn& withSwitch(const char* swNum);
    PanelColumn& withSignal(const char* sigNum);
    PanelColumn& withCodeButton();
    PanelColumn& withMaintainerCall(const char* mcNum = "1");
    PanelColumn& withTrackLamps(std::initializer_list<const char*> tracks);

    bool hasSwitch() const { return hasSwitch_; }
    uint8_t switchIndex() const { return swIdx_; }
    const char* switchNum() const { return swNum_; }

    bool hasSignal() const { return hasSignal_; }
    uint8_t signalIndex() const { return sigIdx_; }
    const char* signalNum() const { return sigNum_; }

    bool hasCodeButton() const { return hasCodeButton_; }
    bool hasMaintainerCall() const { return hasMaintainerCall_; }
    uint8_t maintainerIndex() const { return mcIdx_; }
    const char* maintainerNum() const { return mcNum_; }

    uint8_t trackCount() const { return trackCount_; }
    uint8_t trackStartIndex() const { return tcStartIdx_; }
    const char* trackName(uint8_t idx) const {
        return (idx < trackCount_) ? trackNames_[idx] : nullptr;
    }

    bool isCodePressed(PanelHardware& hw) const {
        return hasCodeButton_ && hw.read(columnNumber_, PanelInput::CODE_BUTTON);
    }

    // Read switch and signal levers into ControlTransaction demands
    void harvestDemands(PanelHardware& hw, ControlTransaction& ctl) const {
        if (hasSwitch_ && swIdx_ < MAX_APPLIANCES) {
            bool n = hw.read(columnNumber_, PanelInput::SW_NORMAL);
            bool r = hw.read(columnNumber_, PanelInput::SW_REVERSE);
            if (n && !r) {
                ctl.switchDemands[swIdx_] = SwitchDemand::NORMAL;
            } else if (r && !n) {
                ctl.switchDemands[swIdx_] = SwitchDemand::REVERSE;
            } else {
                ctl.switchDemands[swIdx_] = SwitchDemand::NO_CHANGE;
            }
        }

        if (hasSignal_ && sigIdx_ < MAX_APPLIANCES) {
            bool l = hw.read(columnNumber_, PanelInput::SIG_LEFT);
            bool s = hw.read(columnNumber_, PanelInput::SIG_STOP);
            bool r = hw.read(columnNumber_, PanelInput::SIG_RIGHT);
            if (r && !l) {
                ctl.signalDemands[sigIdx_] = SignalDemand::RIGHT;
            } else if (l && !r) {
                ctl.signalDemands[sigIdx_] = SignalDemand::LEFT;
            } else if (s || (!l && !r)) {
                ctl.signalDemands[sigIdx_] = SignalDemand::STOP;
            } else {
                ctl.signalDemands[sigIdx_] = SignalDemand::NO_CHANGE;
            }
        }

        if (hasMaintainerCall_ && mcIdx_ < MAX_APPLIANCES) {
            ctl.maintainerCall[mcIdx_] = hw.read(columnNumber_, PanelInput::MAINTAINER_CALL_SW);
        }
    }

    // Apply verified plant indications to this column's physical lamps
    void applyIndications(PanelHardware& hw, const IndicationVector& ind) const {
        if (hasSwitch_ && swIdx_ < ind.switchCount) {
            const SwitchIndication& s = ind.switches[swIdx_];
            bool norm = s.inCorrespondence && (s.position == SwitchPosition::NORMAL);
            bool rev  = s.inCorrespondence && (s.position == SwitchPosition::REVERSE);
            hw.write(columnNumber_, PanelOutput::SW_NORMAL_LAMP, norm);
            hw.write(columnNumber_, PanelOutput::SW_REVERSE_LAMP, rev);
        }

        if (hasSignal_ && sigIdx_ < ind.signalCount) {
            const SignalIndication& s = ind.signals[sigIdx_];
            bool left  = (s.activeAuthority == DirectionAuthority::LEFT);
            bool right = (s.activeAuthority == DirectionAuthority::RIGHT);
            bool stop  = (!left && !right);
            hw.write(columnNumber_, PanelOutput::SIG_LEFT_LAMP, left);
            hw.write(columnNumber_, PanelOutput::SIG_RIGHT_LAMP, right);
            hw.write(columnNumber_, PanelOutput::SIG_STOP_LAMP, stop);
        }

        if (hasMaintainerCall_ && mcIdx_ < MAX_APPLIANCES) {
            hw.write(columnNumber_, PanelOutput::MAINTAINER_LAMP, ind.maintainerCall[mcIdx_]);
        }

        for (uint8_t i = 0; i < trackCount_; ++i) {
            uint8_t tcIdx = tcStartIdx_ + i;
            bool occupied = false;
            if (tcIdx < ind.trackCircuitCount) {
                occupied = (ind.trackCircuits[tcIdx].occupancy == Occupancy::OCCUPIED);
            }
            PanelOutput lampFn = static_cast<PanelOutput>(static_cast<uint8_t>(PanelOutput::TRACK_LAMP_1) + i);
            hw.write(columnNumber_, lampFn, occupied);
        }
    }

private:
    friend class CtcStation;
    CtcStation* parentStation_;
    uint8_t columnNumber_;
    bool hasSwitch_;
    uint8_t swIdx_;
    char swNum_[MAX_NAME_LEN];
    bool hasSignal_;
    uint8_t sigIdx_;
    char sigNum_[MAX_NAME_LEN];
    bool hasCodeButton_;
    bool hasMaintainerCall_;
    uint8_t mcIdx_;
    char mcNum_[MAX_NAME_LEN];
    uint8_t tcStartIdx_;
    char trackNames_[MAX_LAMPS_PER_COLUMN][MAX_NAME_LEN];
    uint8_t trackCount_;
};

// =============================================================================
// 4. CtcStation (Control Point Spanning 1 to N Columns)
// =============================================================================

static constexpr uint8_t MAX_COLUMNS_PER_STATION = 4;

class CtcStation {
public:
    CtcStation()
        : name_(""),
          columnCount_(0),
          switchCount_(0),
          signalCount_(0),
          trackCount_(0),
          maintainerCount_(0),
          codecBuilt_(false) {}

    explicit CtcStation(const char* name)
        : name_(name ? name : ""),
          columnCount_(0),
          switchCount_(0),
          signalCount_(0),
          trackCount_(0),
          maintainerCount_(0),
          codecBuilt_(false) {}

    const char* name() const { return name_; }

    AarTextCodec& codec() { return codec_; }
    const AarTextCodec& codec() const { return codec_; }

    // Builds the canonical AAR wire schema (Switches -> Tracks -> Signals -> Maintainers)
    // and preallocates Strategy B buffers once during setup()
    void preallocateBuffers() {
        buildCodec();
        codec_.preallocateBuffers();
    }

    void buildCodec() {
        if (codecBuilt_) return;
        codec_.clearEntries();
        switchCount_ = 0;
        signalCount_ = 0;
        trackCount_ = 0;
        maintainerCount_ = 0;

        // Phase 1: All switches in column order
        for (uint8_t c = 0; c < columnCount_; ++c) {
            if (columns_[c].hasSwitch_) {
                columns_[c].swIdx_ = switchCount_++;
                codec_.addDecodeEntry(decodeSwitch(columns_[c].swIdx_, columns_[c].swNum_));
                codec_.addEncodeEntry(encodeSwitch(columns_[c].swIdx_, columns_[c].swNum_));
            }
        }

        // Phase 2: All track circuits in column order
        for (uint8_t c = 0; c < columnCount_; ++c) {
            columns_[c].tcStartIdx_ = trackCount_;
            for (uint8_t t = 0; t < columns_[c].trackCount_; ++t) {
                uint8_t idx = trackCount_++;
                codec_.addEncodeEntry(encodeTrack(idx, columns_[c].trackNames_[t]));
            }
        }

        // Phase 3: All signals in column order
        for (uint8_t c = 0; c < columnCount_; ++c) {
            if (columns_[c].hasSignal_) {
                columns_[c].sigIdx_ = signalCount_++;
                codec_.addDecodeEntry(decodeSignal(columns_[c].sigIdx_, columns_[c].sigNum_));
                codec_.addEncodeEntry(encodeSignal(columns_[c].sigIdx_, columns_[c].sigNum_));
            }
        }

        // Phase 4: Maintainer calls in column order
        for (uint8_t c = 0; c < columnCount_; ++c) {
            if (columns_[c].hasMaintainerCall_) {
                columns_[c].mcIdx_ = maintainerCount_++;
                codec_.addDecodeEntry(decodeMaintainer(columns_[c].mcIdx_, columns_[c].mcNum_));
                codec_.addEncodeEntry(encodeMaintainer(columns_[c].mcIdx_, columns_[c].mcNum_));
            }
        }

        codecBuilt_ = true;
    }

    PanelColumn& inColumn(uint8_t columnNumber) {
        for (uint8_t i = 0; i < columnCount_; ++i) {
            if (columns_[i].columnNumber() == columnNumber) {
                return columns_[i];
            }
        }
        if (columnCount_ < MAX_COLUMNS_PER_STATION) {
            columns_[columnCount_] = PanelColumn(columnNumber, this);
            return columns_[columnCount_++];
        }
        return dummyColumn_;
    }

    uint8_t columnCount() const { return columnCount_; }
    const PanelColumn& column(uint8_t idx) const { return columns_[idx]; }

    uint8_t currentTrackCount() const { return trackCount_; }

    uint8_t registerSwitch(const char* swNum) {
        uint8_t idx = switchCount_++;
        codec_.addDecodeEntry(decodeSwitch(idx, swNum));
        codec_.addEncodeEntry(encodeSwitch(idx, swNum));
        return idx;
    }

    uint8_t registerSignal(const char* sigNum) {
        uint8_t idx = signalCount_++;
        codec_.addDecodeEntry(decodeSignal(idx, sigNum));
        codec_.addEncodeEntry(encodeSignal(idx, sigNum));
        return idx;
    }

    uint8_t registerTrack(const char* trackName) {
        uint8_t idx = trackCount_++;
        codec_.addEncodeEntry(encodeTrack(idx, trackName));
        return idx;
    }

    uint8_t registerMaintainer(const char* mcNum) {
        uint8_t idx = maintainerCount_++;
        codec_.addDecodeEntry(decodeMaintainer(idx, mcNum));
        codec_.addEncodeEntry(encodeMaintainer(idx, mcNum));
        return idx;
    }

    // Typed demand polling
    bool pollCode(PanelHardware& hw, ControlTransaction& ctl) {
        bool triggered = false;
        for (uint8_t i = 0; i < columnCount_; ++i) {
            if (columns_[i].isCodePressed(hw)) {
                triggered = true;
                break;
            }
        }
        if (triggered) {
            ctl = ControlTransaction();
            for (uint8_t i = 0; i < columnCount_; ++i) {
                columns_[i].harvestDemands(hw, ctl);
            }
            return true;
        }
        return false;
    }

    // AAR string token polling using configured codec
    bool pollCode(PanelHardware& hw, char* outTokens, size_t maxLen) {
        ControlTransaction ctl;
        if (pollCode(hw, ctl)) {
            size_t written = 0;
            return codec_.encodeControls(ctl, outTokens, maxLen, written);
        }
        return false;
    }

    // Direct preallocated encoding (Strategy B)
    const char* pollCode(PanelHardware& hw) {
        ControlTransaction ctl;
        if (pollCode(hw, ctl)) {
            return codec_.encodeControls(ctl);
        }
        return nullptr;
    }

    // Typed indication apply
    void applyIndications(PanelHardware& hw, const IndicationVector& ind) {
        for (uint8_t i = 0; i < columnCount_; ++i) {
            columns_[i].applyIndications(hw, ind);
        }
    }

    // AAR string token indication apply using configured codec
    bool applyIndications(PanelHardware& hw, const char* indicationText) {
        IndicationVector ind;
        if (codec_.decodeIndications(indicationText, ind)) {
            applyIndications(hw, ind);
            return true;
        }
        return false;
    }

private:
    const char* name_;
    PanelColumn columns_[MAX_COLUMNS_PER_STATION];
    uint8_t columnCount_;
    uint8_t switchCount_;
    uint8_t signalCount_;
    uint8_t trackCount_;
    uint8_t maintainerCount_;
    bool codecBuilt_;
    AarTextCodec codec_;
    PanelColumn dummyColumn_;
};

inline PanelColumn& PanelColumn::withSwitch(const char* swNum) {
    if (swNum) {
        strncpy(swNum_, swNum, sizeof(swNum_) - 1);
        swNum_[sizeof(swNum_) - 1] = '\0';
        hasSwitch_ = true;
    }
    return *this;
}

inline PanelColumn& PanelColumn::withSignal(const char* sigNum) {
    if (sigNum) {
        strncpy(sigNum_, sigNum, sizeof(sigNum_) - 1);
        sigNum_[sizeof(sigNum_) - 1] = '\0';
        hasSignal_ = true;
    }
    return *this;
}

inline PanelColumn& PanelColumn::withCodeButton() {
    hasCodeButton_ = true;
    return *this;
}

inline PanelColumn& PanelColumn::withMaintainerCall(const char* mcNum) {
    if (mcNum) {
        strncpy(mcNum_, mcNum, sizeof(mcNum_) - 1);
        mcNum_[sizeof(mcNum_) - 1] = '\0';
        hasMaintainerCall_ = true;
    }
    return *this;
}

inline PanelColumn& PanelColumn::withTrackLamps(std::initializer_list<const char*> tracks) {
    trackCount_ = 0;
    for (const char* t : tracks) {
        if (trackCount_ < MAX_LAMPS_PER_COLUMN && t) {
            strncpy(trackNames_[trackCount_], t, sizeof(trackNames_[trackCount_]) - 1);
            trackNames_[trackCount_][sizeof(trackNames_[trackCount_]) - 1] = '\0';
            trackCount_++;
        }
    }
    return *this;
}

// Fluent jump from one column to another on the same station
inline PanelColumn& PanelColumn::inColumn(uint8_t nextColumn) {
    if (parentStation_) {
        return parentStation_->inColumn(nextColumn);
    }
    return *this;
}

// =============================================================================
// 5. cTcMachine / OfficeUnit (The Entire Dispatcher Console)
// =============================================================================

static constexpr uint8_t MAX_STATIONS_PER_DESK = 16;

class cTcMachine {
public:
    explicit cTcMachine(PanelHardware& hardware)
        : hardware_(hardware), stationCount_(0) {}

    CtcStation& addStation(const char* name) {
        for (uint8_t i = 0; i < stationCount_; ++i) {
            if (strcmp(stations_[i].name(), name) == 0) {
                return stations_[i];
            }
        }
        if (stationCount_ < MAX_STATIONS_PER_DESK) {
            stations_[stationCount_] = CtcStation(name);
            return stations_[stationCount_++];
        }
        return dummyStation_;
    }

    size_t stationCount() const { return stationCount_; }
    CtcStation& station(size_t idx) { return stations_[idx]; }
    const CtcStation& station(size_t idx) const { return stations_[idx]; }

    CtcStation* findStation(const char* name) {
        if (!name) return nullptr;
        for (uint8_t i = 0; i < stationCount_; ++i) {
            if (strcmp(stations_[i].name(), name) == 0) {
                return &stations_[i];
            }
        }
        return nullptr;
    }

    // Preallocate worst-case wire buffers across all stations (Strategy B)
    void preallocateBuffers() {
        for (uint8_t i = 0; i < stationCount_; ++i) {
            stations_[i].preallocateBuffers();
        }
    }

    void begin() {
        preallocateBuffers();
    }

    // Route inbound indication packet to the matching station and verify version sync
    bool applyIndications(const char* cpName, const char* indicationText) {
        CtcStation* st = findStation(cpName);
        if (st) {
            return st->applyIndications(hardware_, indicationText);
        }
        return false;
    }

    // Check all stations for a CODE button push; compiles tokens for that station
    bool pollCode(size_t& outStationIdx, char* outTokens, size_t maxLen) {
        for (size_t i = 0; i < stationCount_; ++i) {
            if (stations_[i].pollCode(hardware_, outTokens, maxLen)) {
                outStationIdx = i;
                return true;
            }
        }
        return false;
    }

    PanelHardware& hardware() { return hardware_; }

private:
    PanelHardware& hardware_;
    CtcStation stations_[MAX_STATIONS_PER_DESK];
    uint8_t stationCount_;
    CtcStation dummyStation_;
};

// Synonyms matching railroad domain terminology
using CtcMachine = cTcMachine;
using OfficeUnit = cTcMachine;

} // namespace FieldUnit

#endif // FIELDUNIT_CTC_MACHINE_H
