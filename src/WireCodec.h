#ifndef FIELDUNIT_WIRE_CODEC_H
#define FIELDUNIT_WIRE_CODEC_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <initializer_list>

#include "types.h"
#include "ControlPoint.h"
#include "Switch.h"
#include "SignalControl.h"
#include "TrackCircuit.h"

namespace FieldUnit {

static constexpr uint8_t MAX_MAP_ENTRIES = 32;
static constexpr uint8_t MAX_CODE_BYTES  = 16;

// Strips common modeler prefixes like "SW" or "SIG" defensively to obtain authentic railroad ID
inline const char* cleanRailroadId(const char* rawName, char* outBuf, size_t maxLen) {
    if (!rawName || maxLen == 0) {
        if (maxLen > 0) outBuf[0] = '\0';
        return outBuf;
    }
    const char* p = rawName;
    if ((p[0] == 'S' || p[0] == 's') && (p[1] == 'W' || p[1] == 'w') && (p[2] != '\0')) {
        p += 2;
        if (*p == '_' || *p == '-') p++;
    } else if ((p[0] == 'S' || p[0] == 's') && (p[1] == 'I' || p[1] == 'i') && (p[2] == 'G' || p[2] == 'g') && (p[3] != '\0')) {
        p += 3;
        if (*p == '_' || *p == '-') p++;
    }
    size_t i = 0;
    while (*p && i + 1 < maxLen) {
        outBuf[i++] = *p++;
    }
    outBuf[i] = '\0';
    return outBuf;
}

// Case-insensitive ASCII token equality check
inline bool tokenEqualsIgnoreCase(const char* a, const char* b) {
    if (!a || !b) return false;
    while (*a && *b) {
        char ca = (*a >= 'a' && *a <= 'z') ? static_cast<char>(*a - 32) : *a;
        char cb = (*b >= 'a' && *b <= 'z') ? static_cast<char>(*b - 32) : *b;
        if (ca != cb) return false;
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

// Fluent Entry Declarations for Codec Configuration
struct DecodeEntry {
    enum class Type : uint8_t {
        SWITCH,
        SIGNAL,
        MAINTAINER,
        SKIP_BITS,
        PAD_TO_BYTE
    };
    Type type;
    uint8_t applianceIndex;
    char name[16];
    uint8_t skipCount;
};

struct EncodeEntry {
    enum class Type : uint8_t {
        SWITCH,
        TRACK,
        SIGNAL,
        MAINTAINER,
        SKIP_BITS,
        PAD_TO_BYTE
    };
    Type type;
    uint8_t applianceIndex;
    char name[16];
    uint8_t skipCount;
    bool activeHigh;
};

struct CodecPadding {
    bool padToByte;
    uint8_t count;
    operator DecodeEntry() const {
        DecodeEntry e{};
        e.type = padToByte ? DecodeEntry::Type::PAD_TO_BYTE : DecodeEntry::Type::SKIP_BITS;
        e.applianceIndex = 0;
        e.name[0] = '\0';
        e.skipCount = count;
        return e;
    }
    operator EncodeEntry() const {
        EncodeEntry e{};
        e.type = padToByte ? EncodeEntry::Type::PAD_TO_BYTE : EncodeEntry::Type::SKIP_BITS;
        e.applianceIndex = 0;
        e.name[0] = '\0';
        e.skipCount = count;
        e.activeHigh = true;
        return e;
    }
};

inline CodecPadding padToByte() {
    return CodecPadding{true, 0};
}

inline CodecPadding skipBits(uint8_t n) {
    return CodecPadding{false, n};
}

// Decode helper functions (Inbound Controls)
inline DecodeEntry decodeSwitch(const Switch* sw, const char* customName = nullptr) {
    DecodeEntry e{};
    e.type = DecodeEntry::Type::SWITCH;
    e.applianceIndex = sw ? sw->index() : 0;
    const char* src = customName ? customName : (sw ? sw->name() : "");
    cleanRailroadId(src, e.name, sizeof(e.name));
    e.skipCount = 0;
    return e;
}

inline DecodeEntry decodeSwitch(const Switch& sw, const char* customName = nullptr) {
    return decodeSwitch(&sw, customName);
}

inline DecodeEntry decodeSwitch(uint8_t swIdx, const char* customName = nullptr) {
    DecodeEntry e{};
    e.type = DecodeEntry::Type::SWITCH;
    e.applianceIndex = swIdx;
    cleanRailroadId(customName ? customName : "", e.name, sizeof(e.name));
    e.skipCount = 0;
    return e;
}

inline DecodeEntry decodeSignal(const SignalControl* sig, const char* customName = nullptr) {
    DecodeEntry e{};
    e.type = DecodeEntry::Type::SIGNAL;
    e.applianceIndex = sig ? sig->index() : 0;
    const char* src = customName ? customName : (sig ? sig->name() : "");
    cleanRailroadId(src, e.name, sizeof(e.name));
    e.skipCount = 0;
    return e;
}

inline DecodeEntry decodeSignal(const SignalControl& sig, const char* customName = nullptr) {
    return decodeSignal(&sig, customName);
}

inline DecodeEntry decodeSignal(uint8_t sigIdx, const char* customName = nullptr) {
    DecodeEntry e{};
    e.type = DecodeEntry::Type::SIGNAL;
    e.applianceIndex = sigIdx;
    cleanRailroadId(customName ? customName : "", e.name, sizeof(e.name));
    e.skipCount = 0;
    return e;
}

inline DecodeEntry decodeMaintainer(uint8_t mcIdx, const char* customName = nullptr) {
    DecodeEntry e{};
    e.type = DecodeEntry::Type::MAINTAINER;
    e.applianceIndex = mcIdx;
    if (customName && customName[0] != '\0') {
        cleanRailroadId(customName, e.name, sizeof(e.name));
    } else {
        snprintf(e.name, sizeof(e.name), "MC%u", static_cast<unsigned>(mcIdx + 1));
    }
    e.skipCount = 0;
    return e;
}

inline DecodeEntry decodeMaintainer(int mcIdx) {
    return decodeMaintainer(static_cast<uint8_t>(mcIdx), nullptr);
}

inline DecodeEntry decodeMaintainer(const char* customName) {
    return decodeMaintainer(static_cast<uint8_t>(0), customName);
}

inline DecodeEntry decodeMaintainer() {
    return decodeMaintainer(static_cast<uint8_t>(0), nullptr);
}

// Encode helper functions (Outbound Indications)
inline EncodeEntry encodeSwitch(const Switch* sw, const char* customName = nullptr) {
    EncodeEntry e{};
    e.type = EncodeEntry::Type::SWITCH;
    e.applianceIndex = sw ? sw->index() : 0;
    const char* src = customName ? customName : (sw ? sw->name() : "");
    cleanRailroadId(src, e.name, sizeof(e.name));
    e.skipCount = 0;
    e.activeHigh = true;
    return e;
}

inline EncodeEntry encodeSwitch(const Switch& sw, const char* customName = nullptr) {
    return encodeSwitch(&sw, customName);
}

inline EncodeEntry encodeSwitch(uint8_t swIdx, const char* customName = nullptr) {
    EncodeEntry e{};
    e.type = EncodeEntry::Type::SWITCH;
    e.applianceIndex = swIdx;
    cleanRailroadId(customName ? customName : "", e.name, sizeof(e.name));
    e.skipCount = 0;
    e.activeHigh = true;
    return e;
}

inline EncodeEntry encodeTrack(const TrackCircuit* tc, const char* customName = nullptr, bool activeHigh = true) {
    EncodeEntry e{};
    e.type = EncodeEntry::Type::TRACK;
    e.applianceIndex = tc ? tc->index() : 0;
    const char* src = customName ? customName : (tc ? tc->name() : "");
    cleanRailroadId(src, e.name, sizeof(e.name));
    e.skipCount = 0;
    e.activeHigh = activeHigh;
    return e;
}

inline EncodeEntry encodeTrack(const TrackCircuit* tc, bool activeHigh) {
    return encodeTrack(tc, nullptr, activeHigh);
}

inline EncodeEntry encodeTrack(const TrackCircuit& tc, bool activeHigh = true) {
    return encodeTrack(&tc, nullptr, activeHigh);
}

inline EncodeEntry encodeTrack(uint8_t trackIdx, const char* customName = nullptr, bool activeHigh = true) {
    EncodeEntry e{};
    e.type = EncodeEntry::Type::TRACK;
    e.applianceIndex = trackIdx;
    cleanRailroadId(customName ? customName : "", e.name, sizeof(e.name));
    e.skipCount = 0;
    e.activeHigh = activeHigh;
    return e;
}

inline EncodeEntry encodeTrack(uint8_t trackIdx, bool activeHigh) {
    return encodeTrack(trackIdx, nullptr, activeHigh);
}

inline EncodeEntry encodeSignal(const SignalControl* sig, const char* customName = nullptr) {
    EncodeEntry e{};
    e.type = EncodeEntry::Type::SIGNAL;
    e.applianceIndex = sig ? sig->index() : 0;
    const char* src = customName ? customName : (sig ? sig->name() : "");
    cleanRailroadId(src, e.name, sizeof(e.name));
    e.skipCount = 0;
    e.activeHigh = true;
    return e;
}

inline EncodeEntry encodeSignal(const SignalControl& sig, const char* customName = nullptr) {
    return encodeSignal(&sig, customName);
}

inline EncodeEntry encodeSignal(uint8_t sigIdx, const char* customName = nullptr) {
    EncodeEntry e{};
    e.type = EncodeEntry::Type::SIGNAL;
    e.applianceIndex = sigIdx;
    cleanRailroadId(customName ? customName : "", e.name, sizeof(e.name));
    e.skipCount = 0;
    e.activeHigh = true;
    return e;
}

inline EncodeEntry encodeMaintainer(uint8_t mcIdx, const char* customName = nullptr) {
    EncodeEntry e{};
    e.type = EncodeEntry::Type::MAINTAINER;
    e.applianceIndex = mcIdx;
    if (customName && customName[0] != '\0') {
        cleanRailroadId(customName, e.name, sizeof(e.name));
    } else {
        snprintf(e.name, sizeof(e.name), "MC%u", static_cast<unsigned>(mcIdx + 1));
    }
    e.skipCount = 0;
    e.activeHigh = true;
    return e;
}

inline EncodeEntry encodeMaintainer(int mcIdx) {
    return encodeMaintainer(static_cast<uint8_t>(mcIdx), nullptr);
}

inline EncodeEntry encodeMaintainer(const char* customName) {
    return encodeMaintainer(static_cast<uint8_t>(0), customName);
}

inline EncodeEntry encodeMaintainer() {
    return encodeMaintainer(static_cast<uint8_t>(0), nullptr);
}

// =============================================================================
// AarTextCodec - Human & Machine Readable Symbolic AAR Token Stream over CodeLine
// =============================================================================
// Conservative in what is produced (exact declaration order),
// liberal in what is consumed (any order, whitespace/case insensitive).
// Unambiguous:
// - Control tokens MUST end with 'S' (1NWS, 1RWS, 2SGS, 2NGS, 2HS, MC1S)
// - Indication tokens MUST end with 'K' (1NWK, 1RWK, 1T1K, 2SGK, 2NGK, 2TEK, MC1K)
// - Asserted = TOKEN, Negated / Dropped = (TOKEN)
class AarTextCodec {
public:
    AarTextCodec()
        : decodeEntryCount_(0),
          encodeEntryCount_(0),
          unknownSymbolCount_(0),
          vitalConflictCount_(0) {
        lastUnknownSymbol_[0] = '\0';
    }

    void decodeControls(std::initializer_list<DecodeEntry> entries) {
        decodeEntryCount_ = 0;
        for (const auto& entry : entries) {
            if (decodeEntryCount_ < MAX_MAP_ENTRIES) {
                decodeEntries_[decodeEntryCount_++] = entry;
            }
        }
    }

    void encodeIndications(std::initializer_list<EncodeEntry> entries) {
        encodeEntryCount_ = 0;
        for (const auto& entry : entries) {
            if (encodeEntryCount_ < MAX_MAP_ENTRIES) {
                encodeEntries_[encodeEntryCount_++] = entry;
            }
        }
    }

    // Decode inbound comma-separated control tokens into a ControlTransaction
    // Order-independent: tokens may appear in any sequence.
    // If a vital conflict occurs, ctl.vitalValid is marked false (vital isolation).
    bool decodeControls(const char* text, ControlTransaction& ctl) {
        if (!text) return false;

        ctl = ControlTransaction(); // Defaults: all NO_CHANGE, vitalValid = true

        const char* p = text;
        while (*p) {
            // 1. Skip commas and leading whitespace
            while (*p && (*p == ',' || *p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) {
                p++;
            }
            if (!*p) break;

            // 2. Extract next token up to comma or end-of-string
            const char* tokStart = p;
            while (*p && *p != ',') {
                p++;
            }
            const char* tokEnd = p;

            // Trim trailing whitespace from raw token
            while (tokEnd > tokStart && (tokEnd[-1] == ' ' || tokEnd[-1] == '\t' || tokEnd[-1] == '\r' || tokEnd[-1] == '\n')) {
                tokEnd--;
            }

            size_t rawLen = tokEnd - tokStart;
            if (rawLen == 0) continue;

            // Copy raw token for fault inspection
            char rawToken[32];
            size_t copyLen = (rawLen < sizeof(rawToken) - 1) ? rawLen : sizeof(rawToken) - 1;
            memcpy(rawToken, tokStart, copyLen);
            rawToken[copyLen] = '\0';

            // 3. Determine polarity: parenthesized (TOKEN) is negated; bare TOKEN is asserted
            bool asserted = true;
            const char* coreStart = tokStart;
            const char* coreEnd = tokEnd;

            if (*coreStart == '(') {
                coreStart++;
                while (coreStart < coreEnd && (*coreStart == ' ' || *coreStart == '\t')) {
                    coreStart++;
                }
                if (coreEnd > coreStart && coreEnd[-1] == ')') {
                    coreEnd--;
                    while (coreEnd > coreStart && (coreEnd[-1] == ' ' || coreEnd[-1] == '\t')) {
                        coreEnd--;
                    }
                    asserted = false;
                }
            }

            size_t coreLen = coreEnd - coreStart;
            if (coreLen == 0) continue;

            char token[32];
            copyLen = (coreLen < sizeof(token) - 1) ? coreLen : sizeof(token) - 1;
            memcpy(token, coreStart, copyLen);
            token[copyLen] = '\0';

            // 4. Strict railroad convention: Control tokens MUST end in 'S' or 's'
            size_t tlen = strlen(token);
            if (tlen < 2 || (token[tlen - 1] != 'S' && token[tlen - 1] != 's')) {
                // Not a valid control token (e.g. missing S suffix, or indication token K sent on control line)
                unknownSymbolCount_++;
                strncpy(lastUnknownSymbol_, rawToken, sizeof(lastUnknownSymbol_) - 1);
                lastUnknownSymbol_[sizeof(lastUnknownSymbol_) - 1] = '\0';
                continue;
            }

            // Strip the trailing 'S' to compare with base mnemonic (e.g. "1NW", "2SG", "MC1")
            token[tlen - 1] = '\0';

            // 5. Match against configured decode entries
            bool matched = false;
            for (uint8_t i = 0; i < decodeEntryCount_; ++i) {
                const DecodeEntry& entry = decodeEntries_[i];

                if (entry.type == DecodeEntry::Type::SWITCH) {
                    char nwExpected[32], rwExpected[32];
                    snprintf(nwExpected, sizeof(nwExpected), "%sNW", entry.name);
                    snprintf(rwExpected, sizeof(rwExpected), "%sRW", entry.name);

                    if (tokenEqualsIgnoreCase(token, nwExpected)) {
                        matched = true;
                        if (asserted) {
                            if (ctl.switchDemands[entry.applianceIndex] == SwitchDemand::REVERSE) {
                                // Conflicting demands for the same switch in one transmission!
                                ctl.vitalValid = false;
                                vitalConflictCount_++;
                            }
                            ctl.switchDemands[entry.applianceIndex] = SwitchDemand::NORMAL;
                        }
                        break;
                    } else if (tokenEqualsIgnoreCase(token, rwExpected)) {
                        matched = true;
                        if (asserted) {
                            if (ctl.switchDemands[entry.applianceIndex] == SwitchDemand::NORMAL) {
                                ctl.vitalValid = false;
                                vitalConflictCount_++;
                            }
                            ctl.switchDemands[entry.applianceIndex] = SwitchDemand::REVERSE;
                        }
                        break;
                    }
                } else if (entry.type == DecodeEntry::Type::SIGNAL) {
                    char sgExpected[32], ngExpected[32], hExpected[32];
                    snprintf(sgExpected, sizeof(sgExpected), "%sSG", entry.name);
                    snprintf(ngExpected, sizeof(ngExpected), "%sNG", entry.name);
                    snprintf(hExpected, sizeof(hExpected), "%sH", entry.name);

                    if (tokenEqualsIgnoreCase(token, sgExpected)) {
                        matched = true;
                        if (asserted) {
                            if (ctl.signalDemands[entry.applianceIndex] != SignalDemand::NO_CHANGE &&
                                ctl.signalDemands[entry.applianceIndex] != SignalDemand::RIGHT) {
                                ctl.vitalValid = false;
                                vitalConflictCount_++;
                            }
                            ctl.signalDemands[entry.applianceIndex] = SignalDemand::RIGHT;
                        }
                        break;
                    } else if (tokenEqualsIgnoreCase(token, ngExpected)) {
                        matched = true;
                        if (asserted) {
                            if (ctl.signalDemands[entry.applianceIndex] != SignalDemand::NO_CHANGE &&
                                ctl.signalDemands[entry.applianceIndex] != SignalDemand::LEFT) {
                                ctl.vitalValid = false;
                                vitalConflictCount_++;
                            }
                            ctl.signalDemands[entry.applianceIndex] = SignalDemand::LEFT;
                        }
                        break;
                    } else if (tokenEqualsIgnoreCase(token, hExpected)) {
                        matched = true;
                        if (asserted) {
                            if (ctl.signalDemands[entry.applianceIndex] != SignalDemand::NO_CHANGE &&
                                ctl.signalDemands[entry.applianceIndex] != SignalDemand::STOP) {
                                ctl.vitalValid = false;
                                vitalConflictCount_++;
                            }
                            ctl.signalDemands[entry.applianceIndex] = SignalDemand::STOP;
                        }
                        break;
                    }
                } else if (entry.type == DecodeEntry::Type::MAINTAINER) {
                    if (tokenEqualsIgnoreCase(token, entry.name)) {
                        matched = true;
                        ctl.maintainerCall[entry.applianceIndex] = asserted;
                        break;
                    }
                }
            }

            if (!matched) {
                unknownSymbolCount_++;
                strncpy(lastUnknownSymbol_, rawToken, sizeof(lastUnknownSymbol_) - 1);
                lastUnknownSymbol_[sizeof(lastUnknownSymbol_) - 1] = '\0';
            }
        }

        return true;
    }

    // Encode plant indication vector into human-readable comma-delimited AAR tokens
    // Strictly formatted in the exact order declared in encodeIndications()
    bool encodeIndications(const IndicationVector& ind, char* buffer, size_t maxLength, size_t& lengthOut) const {
        if (!buffer || maxLength == 0) {
            lengthOut = 0;
            return false;
        }

        size_t offset = 0;
        buffer[0] = '\0';

        auto appendToken = [&](const char* token, bool asserted) -> bool {
            if (offset > 0) {
                if (offset + 2 >= maxLength) return false;
                buffer[offset++] = ',';
                buffer[offset++] = ' ';
            }
            size_t tokLen = strlen(token);
            if (!asserted) {
                if (offset + tokLen + 2 >= maxLength) return false;
                buffer[offset++] = '(';
                memcpy(buffer + offset, token, tokLen);
                offset += tokLen;
                buffer[offset++] = ')';
            } else {
                if (offset + tokLen >= maxLength) return false;
                memcpy(buffer + offset, token, tokLen);
                offset += tokLen;
            }
            buffer[offset] = '\0';
            return true;
        };

        for (uint8_t i = 0; i < encodeEntryCount_; ++i) {
            const EncodeEntry& entry = encodeEntries_[i];
            switch (entry.type) {
                case EncodeEntry::Type::SWITCH: {
                    char nwkToken[32], rwkToken[32];
                    snprintf(nwkToken, sizeof(nwkToken), "%sNWK", entry.name);
                    snprintf(rwkToken, sizeof(rwkToken), "%sRWK", entry.name);

                    bool nwk = false;
                    bool rwk = false;
                    if (entry.applianceIndex < ind.switchCount) {
                        const SwitchIndication& s = ind.switches[entry.applianceIndex];
                        if (s.inCorrespondence) {
                            if (s.position == SwitchPosition::NORMAL) nwk = true;
                            else if (s.position == SwitchPosition::REVERSE) rwk = true;
                        }
                    }
                    if (!appendToken(nwkToken, nwk)) return false;
                    if (!appendToken(rwkToken, rwk)) return false;
                    break;
                }
                case EncodeEntry::Type::TRACK: {
                    char trackToken[32];
                    size_t nlen = strlen(entry.name);
                    if (nlen > 0 && (entry.name[nlen - 1] == 'K' || entry.name[nlen - 1] == 'k')) {
                        snprintf(trackToken, sizeof(trackToken), "%s", entry.name);
                    } else {
                        snprintf(trackToken, sizeof(trackToken), "%sK", entry.name);
                    }
                    bool occ = false;
                    if (entry.applianceIndex < ind.trackCircuitCount) {
                        occ = (ind.trackCircuits[entry.applianceIndex].occupancy == Occupancy::OCCUPIED);
                        if (!entry.activeHigh) occ = !occ;
                    }
                    if (!appendToken(trackToken, occ)) return false;
                    break;
                }
                case EncodeEntry::Type::SIGNAL: {
                    char sgkToken[32], ngkToken[32], tekToken[32];
                    snprintf(sgkToken, sizeof(sgkToken), "%sSGK", entry.name);
                    snprintf(ngkToken, sizeof(ngkToken), "%sNGK", entry.name);
                    snprintf(tekToken, sizeof(tekToken), "%sTEK", entry.name);

                    bool sgk = false;
                    bool ngk = false;
                    bool tek = false;
                    if (entry.applianceIndex < ind.signalCount) {
                        const SignalIndication& s = ind.signals[entry.applianceIndex];
                        if (s.activeAuthority == DirectionAuthority::RIGHT) sgk = true;
                        else if (s.activeAuthority == DirectionAuthority::LEFT) ngk = true;
                        if (s.timeLocked) tek = true;
                    }
                    if (!appendToken(sgkToken, sgk)) return false;
                    if (!appendToken(ngkToken, ngk)) return false;
                    if (!appendToken(tekToken, tek)) return false;
                    break;
                }
                case EncodeEntry::Type::MAINTAINER: {
                    char mcToken[32];
                    size_t nlen = strlen(entry.name);
                    if (nlen > 0 && (entry.name[nlen - 1] == 'K' || entry.name[nlen - 1] == 'k')) {
                        snprintf(mcToken, sizeof(mcToken), "%s", entry.name);
                    } else {
                        snprintf(mcToken, sizeof(mcToken), "%sK", entry.name);
                    }
                    bool mck = (entry.applianceIndex < MAX_APPLIANCES) ? ind.maintainerCall[entry.applianceIndex] : false;
                    if (!appendToken(mcToken, mck)) return false;
                    break;
                }
                case EncodeEntry::Type::PAD_TO_BYTE:
                case EncodeEntry::Type::SKIP_BITS:
                    break;
            }
        }

        lengthOut = offset;
        return true;
    }

    bool encodeIndications(const IndicationVector& ind, char* buffer, size_t maxLength) const {
        size_t ignored = 0;
        return encodeIndications(ind, buffer, maxLength, ignored);
    }

    // Fault Monitoring Subsystem
    uint16_t unknownSymbolCount() const { return unknownSymbolCount_; }
    const char* lastUnknownSymbol() const { return lastUnknownSymbol_; }
    uint16_t vitalConflictCount() const { return vitalConflictCount_; }

    void resetFaults() {
        unknownSymbolCount_ = 0;
        vitalConflictCount_ = 0;
        lastUnknownSymbol_[0] = '\0';
    }

private:
    DecodeEntry decodeEntries_[MAX_MAP_ENTRIES];
    uint8_t     decodeEntryCount_;

    EncodeEntry encodeEntries_[MAX_MAP_ENTRIES];
    uint8_t     encodeEntryCount_;

    uint16_t unknownSymbolCount_;
    uint16_t vitalConflictCount_;
    char     lastUnknownSymbol_[32];
};

// =============================================================================
// BitPackedCodec - Classic Dense C/MRI IB[] / OB[] Bitstream Codec
// =============================================================================
struct SwitchControlMap {
    uint8_t switchIndex;
    uint8_t normalByte;
    uint8_t normalBit;
    uint8_t reverseByte;
    uint8_t reverseBit;
};

struct SignalControlMap {
    uint8_t signalIndex;
    uint8_t southByte;
    uint8_t southBit;
    uint8_t northByte;
    uint8_t northBit;
    uint8_t stopByte;
    uint8_t stopBit;
};

struct MaintainerControlMap {
    uint8_t mcIndex;
    uint8_t byteIndex;
    uint8_t bitIndex;
};

struct SwitchIndicationMap {
    uint8_t switchIndex;
    uint8_t normalByte;
    uint8_t normalBit;
    uint8_t reverseByte;
    uint8_t reverseBit;
};

struct TrackIndicationMap {
    uint8_t trackIndex;
    uint8_t byteIndex;
    uint8_t bitIndex;
    bool    activeHigh;
};

struct SignalIndicationMap {
    uint8_t signalIndex;
    uint8_t southByte;
    uint8_t southBit;
    uint8_t northByte;
    uint8_t northBit;
    uint8_t timeElementByte;
    uint8_t timeElementBit;
};

struct MaintainerIndicationMap {
    uint8_t mcIndex;
    uint8_t byteIndex;
    uint8_t bitIndex;
};

class BitPackedCodec {
public:
    BitPackedCodec(uint8_t controlByteCount = 0, uint8_t indicationByteCount = 0)
        : controlByteCount_(controlByteCount),
          indicationByteCount_(indicationByteCount),
          switchControlMapCount_(0),
          signalControlMapCount_(0),
          mcControlMapCount_(0),
          switchIndMapCount_(0),
          trackIndMapCount_(0),
          signalIndMapCount_(0),
          mcIndMapCount_(0) {}

    uint8_t expectedControlBytes() const { return controlByteCount_; }
    uint8_t expectedIndicationBytes() const { return indicationByteCount_; }

    void mapSwitchControl(uint8_t swIdx, uint8_t nByte, uint8_t nBit, uint8_t rByte, uint8_t rBit) {
        if (switchControlMapCount_ < MAX_MAP_ENTRIES) {
            switchControlMaps_[switchControlMapCount_++] = {swIdx, nByte, nBit, rByte, rBit};
        }
    }

    void mapSignalControl(uint8_t sigIdx, uint8_t byteIdx, uint8_t sBit, uint8_t nBit, uint8_t hBit) {
        mapSignalControl(sigIdx, byteIdx, sBit, byteIdx, nBit, byteIdx, hBit);
    }

    void mapSignalControl(uint8_t sigIdx, uint8_t sByte, uint8_t sBit, uint8_t nByte, uint8_t nBit, uint8_t hByte, uint8_t hBit) {
        if (signalControlMapCount_ < MAX_MAP_ENTRIES) {
            signalControlMaps_[signalControlMapCount_++] = {sigIdx, sByte, sBit, nByte, nBit, hByte, hBit};
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
        mapSignalIndication(sigIdx, byteIdx, sBit, byteIdx, nBit, byteIdx, teBit);
    }

    void mapSignalIndication(uint8_t sigIdx, uint8_t sByte, uint8_t sBit, uint8_t nByte, uint8_t nBit, uint8_t teByte, uint8_t teBit) {
        if (signalIndMapCount_ < MAX_MAP_ENTRIES) {
            signalIndMaps_[signalIndMapCount_++] = {sigIdx, sByte, sBit, nByte, nBit, teByte, teBit};
        }
    }

    void mapMaintainerIndication(uint8_t mcIdx, uint8_t byteIdx, uint8_t bitIdx) {
        if (mcIndMapCount_ < MAX_MAP_ENTRIES) {
            mcIndMaps_[mcIndMapCount_++] = {mcIdx, byteIdx, bitIdx};
        }
    }

    void decodeControls(std::initializer_list<DecodeEntry> entries) {
        switchControlMapCount_ = 0;
        signalControlMapCount_ = 0;
        mcControlMapCount_ = 0;

        uint16_t currentBitIndex = 0;
        for (const auto& entry : entries) {
            switch (entry.type) {
                case DecodeEntry::Type::PAD_TO_BYTE:
                    currentBitIndex = (currentBitIndex + 7) & ~7;
                    break;
                case DecodeEntry::Type::SKIP_BITS:
                    currentBitIndex += entry.skipCount;
                    break;
                case DecodeEntry::Type::SWITCH: {
                    uint8_t nByte = currentBitIndex / 8;
                    uint8_t nBit  = currentBitIndex % 8;
                    uint8_t rByte = (currentBitIndex + 1) / 8;
                    uint8_t rBit  = (currentBitIndex + 1) % 8;
                    mapSwitchControl(entry.applianceIndex, nByte, nBit, rByte, rBit);
                    currentBitIndex += 2;
                    break;
                }
                case DecodeEntry::Type::SIGNAL: {
                    uint8_t sByte = currentBitIndex / 8;
                    uint8_t sBit  = currentBitIndex % 8;
                    uint8_t nByte = (currentBitIndex + 1) / 8;
                    uint8_t nBit  = (currentBitIndex + 1) % 8;
                    uint8_t hByte = (currentBitIndex + 2) / 8;
                    uint8_t hBit  = (currentBitIndex + 2) % 8;
                    mapSignalControl(entry.applianceIndex, sByte, sBit, nByte, nBit, hByte, hBit);
                    currentBitIndex += 3;
                    break;
                }
                case DecodeEntry::Type::MAINTAINER: {
                    uint8_t mcByte = currentBitIndex / 8;
                    uint8_t mcBit  = currentBitIndex % 8;
                    mapMaintainerControl(entry.applianceIndex, mcByte, mcBit);
                    currentBitIndex += 1;
                    break;
                }
            }
        }
        uint8_t neededBytes = (currentBitIndex + 7) / 8;
        if (neededBytes > controlByteCount_) {
            controlByteCount_ = neededBytes;
        }
    }

    void encodeIndications(std::initializer_list<EncodeEntry> entries) {
        switchIndMapCount_ = 0;
        trackIndMapCount_ = 0;
        signalIndMapCount_ = 0;
        mcIndMapCount_ = 0;

        uint16_t currentBitIndex = 0;
        for (const auto& entry : entries) {
            switch (entry.type) {
                case EncodeEntry::Type::PAD_TO_BYTE:
                    currentBitIndex = (currentBitIndex + 7) & ~7;
                    break;
                case EncodeEntry::Type::SKIP_BITS:
                    currentBitIndex += entry.skipCount;
                    break;
                case EncodeEntry::Type::SWITCH: {
                    uint8_t nByte = currentBitIndex / 8;
                    uint8_t nBit  = currentBitIndex % 8;
                    uint8_t rByte = (currentBitIndex + 1) / 8;
                    uint8_t rBit  = (currentBitIndex + 1) % 8;
                    mapSwitchIndication(entry.applianceIndex, nByte, nBit, rByte, rBit);
                    currentBitIndex += 2;
                    break;
                }
                case EncodeEntry::Type::TRACK: {
                    uint8_t tByte = currentBitIndex / 8;
                    uint8_t tBit  = currentBitIndex % 8;
                    mapTrackIndication(entry.applianceIndex, tByte, tBit, entry.activeHigh);
                    currentBitIndex += 1;
                    break;
                }
                case EncodeEntry::Type::SIGNAL: {
                    uint8_t sByte = currentBitIndex / 8;
                    uint8_t sBit  = currentBitIndex % 8;
                    uint8_t nByte = (currentBitIndex + 1) / 8;
                    uint8_t nBit  = (currentBitIndex + 1) % 8;
                    uint8_t teByte = (currentBitIndex + 2) / 8;
                    uint8_t teBit  = (currentBitIndex + 2) % 8;
                    mapSignalIndication(entry.applianceIndex, sByte, sBit, nByte, nBit, teByte, teBit);
                    currentBitIndex += 3;
                    break;
                }
                case EncodeEntry::Type::MAINTAINER: {
                    uint8_t mcByte = currentBitIndex / 8;
                    uint8_t mcBit  = currentBitIndex % 8;
                    mapMaintainerIndication(entry.applianceIndex, mcByte, mcBit);
                    currentBitIndex += 1;
                    break;
                }
            }
        }
        uint8_t neededBytes = (currentBitIndex + 7) / 8;
        if (neededBytes > indicationByteCount_) {
            indicationByteCount_ = neededBytes;
        }
    }

    bool unpackControls(const uint8_t* bytes, size_t length, ControlTransaction& ctl) const {
        if (length < controlByteCount_) {
            return false;
        }

        ctl = ControlTransaction();

        // 1. Unpack switch controls
        for (uint8_t i = 0; i < switchControlMapCount_; ++i) {
            const SwitchControlMap& m = switchControlMaps_[i];
            bool nBit = (bytes[m.normalByte] & (1 << m.normalBit)) != 0;
            bool rBit = (bytes[m.reverseByte] & (1 << m.reverseBit)) != 0;

            if (nBit && rBit) {
                ctl.vitalValid = false; // Conflicting vital bits
                return false;
            }
            if (nBit) {
                ctl.switchDemands[m.switchIndex] = SwitchDemand::NORMAL;
            } else if (rBit) {
                ctl.switchDemands[m.switchIndex] = SwitchDemand::REVERSE;
            } else {
                ctl.switchDemands[m.switchIndex] = SwitchDemand::NO_CHANGE;
            }
        }

        // 2. Unpack signal controls
        for (uint8_t i = 0; i < signalControlMapCount_; ++i) {
            const SignalControlMap& m = signalControlMaps_[i];
            bool sBit = (bytes[m.southByte] & (1 << m.southBit)) != 0;
            bool nBit = (bytes[m.northByte] & (1 << m.northBit)) != 0;
            bool hBit = (bytes[m.stopByte] & (1 << m.stopBit)) != 0;

            uint8_t activeCount = (sBit ? 1 : 0) + (nBit ? 1 : 0) + (hBit ? 1 : 0);
            if (activeCount > 1) {
                ctl.vitalValid = false; // Conflicting signal direction
                return false;
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

        // 3. Unpack maintainer calls (non-vital)
        for (uint8_t i = 0; i < mcControlMapCount_; ++i) {
            const MaintainerControlMap& m = mcControlMaps_[i];
            bool mcBit = (bytes[m.byteIndex] & (1 << m.bitIndex)) != 0;
            ctl.maintainerCall[m.mcIndex] = mcBit;
        }

        return true;
    }

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
                    outBytes[m.southByte] |= (1 << m.southBit);
                } else if (s.activeAuthority == DirectionAuthority::LEFT) {
                    outBytes[m.northByte] |= (1 << m.northBit);
                }
                if (s.timeLocked) {
                    outBytes[m.timeElementByte] |= (1 << m.timeElementBit);
                }
            }
        }

        // 4. Pack maintainer calls (MCK)
        for (uint8_t i = 0; i < mcIndMapCount_; ++i) {
            const MaintainerIndicationMap& m = mcIndMaps_[i];
            if (m.mcIndex < MAX_APPLIANCES && ind.maintainerCall[m.mcIndex]) {
                outBytes[m.byteIndex] |= (1 << m.bitIndex);
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

    MaintainerIndicationMap mcIndMaps_[MAX_MAP_ENTRIES];
    uint8_t mcIndMapCount_;
};

// Backwards-compatible alias for existing C/MRI code and tests
using CodeLineCodec = BitPackedCodec;

} // namespace FieldUnit

#endif // FIELDUNIT_WIRE_CODEC_H
