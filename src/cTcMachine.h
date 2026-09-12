#ifndef FIELDUNIT_CTC_MACHINE_H
#define FIELDUNIT_CTC_MACHINE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <initializer_list>
#include "types.h"

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
          hasSignal_(false),
          hasCodeButton_(false),
          hasMaintainerCall_(false),
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

    PanelColumn& withSwitch(const char* swNum) {
        if (swNum) {
            strncpy(swNum_, swNum, sizeof(swNum_) - 1);
            swNum_[sizeof(swNum_) - 1] = '\0';
            hasSwitch_ = true;
        }
        return *this;
    }

    PanelColumn& withSignal(const char* sigNum) {
        if (sigNum) {
            strncpy(sigNum_, sigNum, sizeof(sigNum_) - 1);
            sigNum_[sizeof(sigNum_) - 1] = '\0';
            hasSignal_ = true;
        }
        return *this;
    }

    PanelColumn& withCodeButton() {
        hasCodeButton_ = true;
        return *this;
    }

    PanelColumn& withMaintainerCall(const char* mcNum = "1") {
        if (mcNum) {
            strncpy(mcNum_, mcNum, sizeof(mcNum_) - 1);
            mcNum_[sizeof(mcNum_) - 1] = '\0';
            hasMaintainerCall_ = true;
        }
        return *this;
    }

    PanelColumn& withTrackLamps(std::initializer_list<const char*> tracks) {
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

    bool hasSwitch() const { return hasSwitch_; }
    const char* switchNum() const { return swNum_; }

    bool hasSignal() const { return hasSignal_; }
    const char* signalNum() const { return sigNum_; }

    bool hasCodeButton() const { return hasCodeButton_; }
    bool hasMaintainerCall() const { return hasMaintainerCall_; }
    const char* maintainerNum() const { return mcNum_; }

    uint8_t trackCount() const { return trackCount_; }
    const char* trackName(uint8_t idx) const {
        return (idx < trackCount_) ? trackNames_[idx] : nullptr;
    }

    bool isCodePressed(PanelHardware& hw) const {
        return hasCodeButton_ && hw.read(columnNumber_, PanelInput::CODE_BUTTON);
    }

    // Read switch and signal levers into local demand tokens
    void appendDemands(PanelHardware& hw, char* outBuf, size_t maxLen) const {
        if (hasSwitch_) {
            bool n = hw.read(columnNumber_, PanelInput::SW_NORMAL);
            bool r = hw.read(columnNumber_, PanelInput::SW_REVERSE);
            char swTokens[32];
            if (n && !r) {
                snprintf(swTokens, sizeof(swTokens), "%sNWS, (%sRWS)", swNum_, swNum_);
            } else if (r && !n) {
                snprintf(swTokens, sizeof(swTokens), "(%sNWS), %sRWS", swNum_, swNum_);
            } else {
                snprintf(swTokens, sizeof(swTokens), "(%sNWS), (%sRWS)", swNum_, swNum_);
            }
            appendToken(outBuf, maxLen, swTokens);
        }

        if (hasSignal_) {
            bool l = hw.read(columnNumber_, PanelInput::SIG_LEFT);
            bool s = hw.read(columnNumber_, PanelInput::SIG_STOP);
            bool r = hw.read(columnNumber_, PanelInput::SIG_RIGHT);
            char sigTokens[48];
            if (r && !l) {
                snprintf(sigTokens, sizeof(sigTokens), "%sSGS, (%sNGS), (%sHS)", sigNum_, sigNum_, sigNum_);
            } else if (l && !r) {
                snprintf(sigTokens, sizeof(sigTokens), "(%sSGS), %sNGS, (%sHS)", sigNum_, sigNum_, sigNum_);
            } else if (s || (!l && !r)) {
                snprintf(sigTokens, sizeof(sigTokens), "(%sSGS), (%sNGS), %sHS", sigNum_, sigNum_, sigNum_);
            } else {
                snprintf(sigTokens, sizeof(sigTokens), "(%sSGS), (%sNGS), (%sHS)", sigNum_, sigNum_, sigNum_);
            }
            appendToken(outBuf, maxLen, sigTokens);
        }

        if (hasMaintainerCall_) {
            bool mc = hw.read(columnNumber_, PanelInput::MAINTAINER_CALL_SW);
            char mcToken[24];
            if (mc) {
                snprintf(mcToken, sizeof(mcToken), "MC%sS", mcNum_);
            } else {
                snprintf(mcToken, sizeof(mcToken), "(MC%sS)", mcNum_);
            }
            appendToken(outBuf, maxLen, mcToken);
        }
    }

    // Apply parsed field indications to this column's lamps
    void applyIndications(PanelHardware& hw, const char* indicationText) const {
        if (!indicationText) return;

        if (hasSwitch_) {
            char nwToken[24], rwToken[24];
            snprintf(nwToken, sizeof(nwToken), "%sNWK", swNum_);
            snprintf(rwToken, sizeof(rwToken), "%sRWK", swNum_);
            bool norm = isTokenAsserted(indicationText, nwToken);
            bool rev  = isTokenAsserted(indicationText, rwToken);
            hw.write(columnNumber_, PanelOutput::SW_NORMAL_LAMP, norm);
            hw.write(columnNumber_, PanelOutput::SW_REVERSE_LAMP, rev);
        }

        if (hasSignal_) {
            char ngToken[24], sgToken[24];
            snprintf(ngToken, sizeof(ngToken), "%sNGK", sigNum_);
            snprintf(sgToken, sizeof(sgToken), "%sSGK", sigNum_);
            bool left  = isTokenAsserted(indicationText, ngToken);
            bool right = isTokenAsserted(indicationText, sgToken);
            bool stop  = (!left && !right);
            hw.write(columnNumber_, PanelOutput::SIG_LEFT_LAMP, left);
            hw.write(columnNumber_, PanelOutput::SIG_RIGHT_LAMP, right);
            hw.write(columnNumber_, PanelOutput::SIG_STOP_LAMP, stop);
        }

        if (hasMaintainerCall_) {
            char mcToken[24];
            snprintf(mcToken, sizeof(mcToken), "MC%sK", mcNum_);
            bool mc = isTokenAsserted(indicationText, mcToken);
            hw.write(columnNumber_, PanelOutput::MAINTAINER_LAMP, mc);
        }

        for (uint8_t i = 0; i < trackCount_; ++i) {
            char tkToken[24];
            // Format: <trackName>K, e.g. 1T1K, 1EAK
            snprintf(tkToken, sizeof(tkToken), "%sK", trackNames_[i]);
            bool occupied = isTokenAsserted(indicationText, tkToken);
            PanelOutput lampFn = static_cast<PanelOutput>(static_cast<uint8_t>(PanelOutput::TRACK_LAMP_1) + i);
            hw.write(columnNumber_, lampFn, occupied);
        }
    }

private:
    CtcStation* parentStation_;
    uint8_t columnNumber_;
    bool hasSwitch_;
    char swNum_[MAX_NAME_LEN];
    bool hasSignal_;
    char sigNum_[MAX_NAME_LEN];
    bool hasCodeButton_;
    bool hasMaintainerCall_;
    char mcNum_[MAX_NAME_LEN];
    char trackNames_[MAX_LAMPS_PER_COLUMN][MAX_NAME_LEN];
    uint8_t trackCount_;

    static void appendToken(char* outBuf, size_t maxLen, const char* token) {
        if (!outBuf || !token || maxLen == 0) return;
        size_t curLen = strlen(outBuf);
        if (curLen > 0) {
            if (curLen + 2 < maxLen) {
                outBuf[curLen++] = ',';
                outBuf[curLen++] = ' ';
                outBuf[curLen] = '\0';
            }
        }
        strncat(outBuf, token, maxLen - strlen(outBuf) - 1);
    }

    // Helper: checks if token appears unparenthesized in comma-separated list
    static bool isTokenAsserted(const char* text, const char* targetToken) {
        if (!text || !targetToken) return false;
        const char* p = text;
        size_t targetLen = strlen(targetToken);

        while (*p) {
            // Skip whitespace and commas
            while (*p == ' ' || *p == '\t' || *p == ',' || *p == '\r' || *p == '\n') p++;
            if (!*p) break;

            bool inParen = false;
            if (*p == '(') {
                inParen = true;
                p++;
            }

            const char* tokenStart = p;
            while (*p && *p != ')' && *p != ',' && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n') {
                p++;
            }
            size_t tokenLen = p - tokenStart;

            if (tokenLen == targetLen && strncasecmp(tokenStart, targetToken, targetLen) == 0) {
                return !inParen; // True if unparenthesized
            }

            if (*p == ')') p++;
        }
        return false;
    }
};

// =============================================================================
// 4. CtcStation (Control Point Spanning 1 to N Columns)
// =============================================================================

static constexpr uint8_t MAX_COLUMNS_PER_STATION = 4;

class CtcStation {
public:
    CtcStation() : name_(""), columnCount_(0) {}

    explicit CtcStation(const char* name)
        : name_(name ? name : ""), columnCount_(0) {}

    const char* name() const { return name_; }

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

    // Check if the station's CODE button was pushed; if so, gather demands across all columns
    bool pollCode(PanelHardware& hw, char* outTokens, size_t maxLen) {
        bool triggered = false;
        for (uint8_t i = 0; i < columnCount_; ++i) {
            if (columns_[i].isCodePressed(hw)) {
                triggered = true;
                break;
            }
        }

        if (triggered && outTokens && maxLen > 0) {
            outTokens[0] = '\0';
            for (uint8_t i = 0; i < columnCount_; ++i) {
                columns_[i].appendDemands(hw, outTokens, maxLen);
            }
            return true;
        }
        return false;
    }

    // Apply incoming field indications across all constituent columns
    void applyIndications(PanelHardware& hw, const char* indicationText) {
        for (uint8_t i = 0; i < columnCount_; ++i) {
            columns_[i].applyIndications(hw, indicationText);
        }
    }

private:
    const char* name_;
    PanelColumn columns_[MAX_COLUMNS_PER_STATION];
    uint8_t columnCount_;
    PanelColumn dummyColumn_;
};

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

    // Route inbound indication packet to the matching station
    bool applyIndications(const char* cpName, const char* indicationText) {
        CtcStation* st = findStation(cpName);
        if (st) {
            st->applyIndications(hardware_, indicationText);
            return true;
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
