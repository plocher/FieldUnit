#ifndef FIELDUNIT_CONSOLE_H
#define FIELDUNIT_CONSOLE_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include "types.h"
#include "ControlPoint.h"
#include "PlantSerializer.h"
#include "WireCodec.h"

namespace FieldUnit {

/**
 * FieldUnitConsole - CDC Serial Multiplexer Router
 *
 * Multiplexes a single bidirectional stream (e.g. USB CDC Serial) between:
 * 1. Command & Control (C&C) Verbs:
 *    - `load json <json>` : Dynamically configures plant in RAM via PlantSerializer
 *    - `dump json`        : Serializes current plant topology to JSON
 *    - `status`           : Emits JSON summary of active appliances & health
 *    - `shunt <tc>`       : Shunts a track circuit to OCCUPIED
 *    - `clear <tc>`       : Clears a track circuit to VACANT
 *    - `throw <sw> <N|R>` : Throws a switch and updates feedback
 *    - `reset`            : Resets plant to neutral state
 * 2. AAR Supervisory CodeLine Snapshots:
 *    - Any line ending in 'S' tokens (`1NWS, 2NGS`) feeds the vital interlocking
 *      and immediately returns an AAR indication truth line (`1NWK, 2NGK...`).
 */
class FieldUnitConsole {
public:
    typedef void (*OutputCallback)(const char* line);

    FieldUnitConsole(ControlPoint& cp, OutputCallback outCb = nullptr, IOBus* ioBus = nullptr)
        : cp_(&cp), outCb_(outCb), ioBus_(ioBus), linePos_(0) {
        lineBuf_[0] = '\0';
    }

    void setOutputCallback(OutputCallback cb) {
        outCb_ = cb;
    }

    void setIOBus(IOBus* ioBus) {
        ioBus_ = ioBus;
    }

    // Process a single byte received from stream
    void processByte(char c, uint32_t nowMs = 0) {
        if (c == '\r') return; // Ignore CR
        if (c == '\n') {
            if (linePos_ > 0) {
                lineBuf_[linePos_] = '\0';
                processLine(lineBuf_, nowMs);
                linePos_ = 0;
                lineBuf_[0] = '\0';
            }
            return;
        }

        if (linePos_ + 1 < sizeof(lineBuf_)) {
            lineBuf_[linePos_++] = c;
        }
    }

    // Process a complete line directly
    void processLine(const char* rawLine, uint32_t nowMs = 0) {
        if (!rawLine) return;

        // Skip leading whitespace
        const char* p = rawLine;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '\0') return;

        // 1. Check for C&C Verbs
        if (strncasecmp(p, "load json ", 10) == 0 || strncasecmp(p, "load ", 5) == 0) {
            const char* jsonStart = (strncasecmp(p, "load json ", 10) == 0) ? (p + 10) : (p + 5);
            while (*jsonStart == ' ' || *jsonStart == '\t') jsonStart++;
            handleLoadJson(jsonStart);
            return;
        }

        if (strcasecmp(p, "dump json") == 0 || strcasecmp(p, "dump") == 0) {
            handleDumpJson();
            return;
        }

        if (strcasecmp(p, "status") == 0) {
            handleStatus(nowMs);
            return;
        }

        if (strncasecmp(p, "shunt ", 6) == 0) {
            handleShunt(p + 6, nowMs);
            return;
        }

        if (strncasecmp(p, "clear ", 6) == 0) {
            handleClear(p + 6, nowMs);
            return;
        }

        if (strncasecmp(p, "loopback", 8) == 0) {
            const char* arg = p + 8;
            while (*arg == ' ' || *arg == '\t') arg++;
            handleLoopback(arg);
            return;
        }

        if (strncasecmp(p, "throw ", 6) == 0) {
            handleThrow(p + 6, nowMs);
            return;
        }

        if (strcasecmp(p, "reset") == 0) {
            handleReset();
            return;
        }

        if (strcasecmp(p, "help") == 0) {
            handleHelp();
            return;
        }

        // 2. Not a C&C verb -> Treat as AAR CodeLine Transaction snapshot
        handleCodeLine(p, nowMs);
    }

private:
    void sendResponse(const char* msg) {
        if (outCb_) {
            outCb_(msg);
        }
    }

    void handleLoadJson(const char* json) {
        bool ok = cp_->deserialize(json);
        char resp[256];
        if (ok) {
            snprintf(resp, sizeof(resp),
                     "{\"status\":\"OK\",\"msg\":\"Plant loaded\",\"name\":\"%s\",\"tracks\":%u,\"switches\":%u,\"masts\":%u,\"routes\":%u}",
                     cp_->name(), cp_->trackCircuitCount(), cp_->switchCount(), cp_->mastCount(),
                     cp_->engine().routeCount());
        } else {
            snprintf(resp, sizeof(resp), "{\"status\":\"ERROR\",\"msg\":\"Failed to parse JSON\"}");
        }
        sendResponse(resp);
    }

    void handleDumpJson() {
        static char s_jsonBuf[4096];
        if (cp_->serialize(s_jsonBuf, sizeof(s_jsonBuf), false /*compact*/)) {
            sendResponse(s_jsonBuf);
        } else {
            sendResponse("{\"status\":\"ERROR\",\"msg\":\"Serialization buffer overflow\"}");
        }
    }

    void handleStatus(uint32_t nowMs) {
        char resp[256];
        snprintf(resp, sizeof(resp),
                 "{\"plant\":\"%s\",\"tracks\":%u,\"switches\":%u,\"masts\":%u,\"routes\":%u,\"timeMs\":%lu}",
                 cp_->name(), cp_->trackCircuitCount(), cp_->switchCount(), cp_->mastCount(),
                 cp_->engine().routeCount(), static_cast<unsigned long>(nowMs));
        sendResponse(resp);
    }

    void handleShunt(const char* tcName, uint32_t nowMs) {
        char nameBuf[32];
        sscanf(tcName, "%31s", nameBuf);
        TrackCircuit* tc = cp_->findTrackCircuit(nameBuf);
        char resp[128];
        if (tc) {
            tc->update(Occupancy::OCCUPIED, Quality::GOOD, nowMs);
            snprintf(resp, sizeof(resp), "{\"status\":\"OK\",\"action\":\"shunted\",\"track\":\"%s\"}", nameBuf);
        } else {
            snprintf(resp, sizeof(resp), "{\"status\":\"ERROR\",\"msg\":\"Track circuit not found\",\"track\":\"%s\"}", nameBuf);
        }
        sendResponse(resp);
    }

    void handleClear(const char* tcName, uint32_t nowMs) {
        char nameBuf[32];
        sscanf(tcName, "%31s", nameBuf);
        TrackCircuit* tc = cp_->findTrackCircuit(nameBuf);
        char resp[128];
        if (tc) {
            tc->update(Occupancy::VACANT, Quality::GOOD, nowMs);
            snprintf(resp, sizeof(resp), "{\"status\":\"OK\",\"action\":\"cleared\",\"track\":\"%s\"}", nameBuf);
        } else {
            snprintf(resp, sizeof(resp), "{\"status\":\"ERROR\",\"msg\":\"Track circuit not found\",\"track\":\"%s\"}", nameBuf);
        }
        sendResponse(resp);
    }

    void handleLoopback(const char* arg) {
        if (!ioBus_) {
            sendResponse("{\"status\":\"ERROR\",\"msg\":\"No IOBus attached to console\"}");
            return;
        }

        uint8_t writeVal = 0xAA; // Default alternating pattern 10101010
        if (arg && *arg) {
            char* endp = nullptr;
            writeVal = static_cast<uint8_t>(strtol(arg, &endp, 0));
        }

        // Port B (offset 1, bits 0..7) are OUTPUTS
        for (uint8_t b = 0; b < 8; ++b) {
            bool bitVal = (writeVal & (1 << b)) != 0;
            ioBus_->writeBit(OutputBit(0, 1, b), bitVal);
        }
        ioBus_->flush();

        // Port A (offset 0, bits 0..7) are INPUTS (jumpered back from Port B)
        uint8_t readVal = 0;
        for (uint8_t b = 0; b < 8; ++b) {
            if (ioBus_->readBit(InputBit(0, 0, b))) {
                readVal |= (1 << b);
            }
        }

        char resp[128];
        snprintf(resp, sizeof(resp),
                 "{\"status\":\"OK\",\"action\":\"loopback\",\"written\":%u,\"read\":%u,\"match\":%s}",
                 static_cast<unsigned>(writeVal), static_cast<unsigned>(readVal),
                 (writeVal == readVal) ? "true" : "false");
        sendResponse(resp);
    }

    void handleThrow(const char* args, uint32_t nowMs) {
        char swName[32] = "";
        char posStr[32] = "";
        sscanf(args, "%31s %31s", swName, posStr);
        Switch* sw = cp_->findSwitch(swName);
        char resp[128];
        if (sw) {
            SwitchPosition target = SwitchPosition::NORMAL;
            if (strcasecmp(posStr, "R") == 0 || strcasecmp(posStr, "REVERSE") == 0 || strcasecmp(posStr, "THROWN") == 0) {
                target = SwitchPosition::REVERSE;
            }
            bool thrown = sw->throwSwitch(target, nowMs);
            if (thrown) {
                sw->updateFeedback(target); // Force correspondence for manual test
                snprintf(resp, sizeof(resp), "{\"status\":\"OK\",\"switch\":\"%s\",\"position\":\"%s\"}",
                         swName, (target == SwitchPosition::REVERSE) ? "REVERSE" : "NORMAL");
            } else {
                snprintf(resp, sizeof(resp), "{\"status\":\"REJECTED\",\"msg\":\"Switch locked\",\"switch\":\"%s\"}", swName);
            }
        } else {
            snprintf(resp, sizeof(resp), "{\"status\":\"ERROR\",\"msg\":\"Switch not found\",\"switch\":\"%s\"}", swName);
        }
        sendResponse(resp);
    }

    void handleReset() {
        cp_->reset();
        sendResponse("{\"status\":\"OK\",\"action\":\"reset\",\"msg\":\"Plant reset to empty\"}");
    }

    void handleHelp() {
        sendResponse("{\"commands\":[\"load json <JSON>\",\"dump json\",\"status\",\"shunt <TC>\",\"clear <TC>\",\"throw <SW> <N|R>\",\"reset\",\"<AAR Controls: 1NWS, 2NGS>\"]}");
    }

    void handleCodeLine(const char* line, uint32_t nowMs) {
        // Auto-configure codec mappings from plant if needed
        AarTextCodec codec;
        configureCodecFromPlant(codec);

        ControlTransaction ctl;
        if (codec.decodeControls(line, ctl)) {
            cp_->applyControlTransaction(ctl, nowMs);
        }

        // Run vital scan cycle
        cp_->tick(nowMs);

        // Export and transmit verified indication vector
        IndicationVector ind;
        cp_->exportIndicationVector(ind);
        char outBuf[512];
        size_t lenOut = 0;
        if (codec.encodeIndications(ind, outBuf, sizeof(outBuf), lenOut)) {
            sendResponse(outBuf);
        }
    }

    void configureCodecFromPlant(AarTextCodec& codec) {
        codec.clearEntries();
        // 1. Decode Controls
        for (uint8_t i = 0; i < cp_->switchCount(); ++i) {
            codec.addDecodeEntry(decodeSwitch(cp_->getSwitch(i)));
        }
        for (uint8_t i = 0; i < cp_->authorityCount(); ++i) {
            codec.addDecodeEntry(decodeSignal(cp_->authority(i)));
        }

        // 2. Encode Indications
        for (uint8_t i = 0; i < cp_->switchCount(); ++i) {
            codec.addEncodeEntry(encodeSwitch(cp_->getSwitch(i)));
        }
        for (uint8_t i = 0; i < cp_->trackCircuitCount(); ++i) {
            codec.addEncodeEntry(encodeTrack(cp_->trackCircuit(i)));
        }
        for (uint8_t i = 0; i < cp_->authorityCount(); ++i) {
            codec.addEncodeEntry(encodeSignal(cp_->authority(i)));
        }
    }

    ControlPoint* cp_;
    OutputCallback outCb_;
    IOBus* ioBus_;
    char lineBuf_[4096];
    size_t linePos_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_CONSOLE_H
