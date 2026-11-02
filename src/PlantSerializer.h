#ifndef FIELDUNIT_PLANT_SERIALIZER_H
#define FIELDUNIT_PLANT_SERIALIZER_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#if !defined(__AVR__)
#include <strings.h>
#endif
#include "types.h"
#include "SignalAspectPolicy.h"
#include "InterlockingPlant.h"

namespace FieldUnit {

namespace PlantSerializer {

// -------------------------------------------------------------
// Enum / String Conversion Helpers
// -------------------------------------------------------------

inline const char* policyToName(AspectResolver policy) {
    if (policy == AspectPolicies::sp1969) return "sp1969";
    if (policy == AspectPolicies::sp1985) return "sp1985";
    if (policy == AspectPolicies::gcorSpeed) return "gcorSpeed";
    if (policy == AspectPolicies::nycSpeed) return "nycSpeed";
    if (policy == AspectPolicies::prrPositionLight) return "prrPositionLight";
    if (policy == AspectPolicies::upperQuadrantSemaphore) return "upperQuadrantSemaphore";
    if (policy == AspectPolicies::boCpl) return "boCpl";
    return "defaultRoute";
}

inline AspectResolver nameToPolicy(const char* name) {
    if (!name) return AspectPolicies::defaultRoute;
    if (strcasecmp(name, "sp1969") == 0) return AspectPolicies::sp1969;
    if (strcasecmp(name, "sp1985") == 0) return AspectPolicies::sp1985;
    if (strcasecmp(name, "gcorSpeed") == 0) return AspectPolicies::gcorSpeed;
    if (strcasecmp(name, "nycSpeed") == 0) return AspectPolicies::nycSpeed;
    if (strcasecmp(name, "prrPositionLight") == 0) return AspectPolicies::prrPositionLight;
    if (strcasecmp(name, "upperQuadrantSemaphore") == 0) return AspectPolicies::upperQuadrantSemaphore;
    if (strcasecmp(name, "boCpl") == 0) return AspectPolicies::boCpl;
    return AspectPolicies::defaultRoute;
}

inline const char* mastTypeToName(MastType t) {
    switch (t) {
        case MastType::ONE_HEAD: return "ONE_HEAD";
        case MastType::TWO_HEAD: return "TWO_HEAD";
        case MastType::THREE_HEAD: return "THREE_HEAD";
        case MastType::DWARF: return "DWARF";
        default: return "ONE_HEAD";
    }
}

inline MastType nameToMastType(const char* str) {
    if (!str) return MastType::ONE_HEAD;
    if (strcasecmp(str, "TWO_HEAD") == 0) return MastType::TWO_HEAD;
    if (strcasecmp(str, "THREE_HEAD") == 0) return MastType::THREE_HEAD;
    if (strcasecmp(str, "DWARF") == 0) return MastType::DWARF;
    return MastType::ONE_HEAD;
}

inline const char* directionToName(DirectionAuthority dir) {
    switch (dir) {
        case DirectionAuthority::LEFT: return "LEFT";
        case DirectionAuthority::RIGHT: return "RIGHT";
        case DirectionAuthority::STOP:
        default: return "STOP";
    }
}

inline DirectionAuthority nameToDirection(const char* str) {
    if (!str) return DirectionAuthority::STOP;
    if (strcasecmp(str, "LEFT") == 0) return DirectionAuthority::LEFT;
    if (strcasecmp(str, "RIGHT") == 0) return DirectionAuthority::RIGHT;
    return DirectionAuthority::STOP;
}

inline const char* positionToName(SwitchPosition pos) {
    switch (pos) {
        case SwitchPosition::NORMAL: return "NORMAL";
        case SwitchPosition::REVERSE: return "REVERSE";
        default: return "NORMAL";
    }
}

inline SwitchPosition nameToPosition(const char* str) {
    if (!str) return SwitchPosition::NORMAL;
    if (strcasecmp(str, "REVERSE") == 0 || strcasecmp(str, "R") == 0 || strcasecmp(str, "THROWN") == 0) {
        return SwitchPosition::REVERSE;
    }
    return SwitchPosition::NORMAL;
}

inline const char* indicationToName(Indication ind) {
    switch (ind) {
        case Indication::CLEAR: return "CLEAR";
        case Indication::APPROACH: return "APPROACH";
        case Indication::ADVANCE_APPROACH: return "ADVANCE_APPROACH";
        case Indication::MEDIUM_CLEAR: return "MEDIUM_CLEAR";
        case Indication::DIVERGING_CLEAR: return "DIVERGING_CLEAR";
        case Indication::MEDIUM_APPROACH: return "MEDIUM_APPROACH";
        case Indication::DIVERGING_APPROACH: return "DIVERGING_APPROACH";
        case Indication::APPROACH_MEDIUM: return "APPROACH_MEDIUM";
        case Indication::APPROACH_SLOW: return "APPROACH_SLOW";
        case Indication::APPROACH_DIVERGING: return "APPROACH_DIVERGING";
        case Indication::SLOW_CLEAR: return "SLOW_CLEAR";
        case Indication::SLOW_APPROACH: return "SLOW_APPROACH";
        case Indication::RESTRICTING: return "RESTRICTING";
        case Indication::DIVERGING_RESTRICTING: return "DIVERGING_RESTRICTING";
        case Indication::APPROACH_RESTRICTING: return "APPROACH_RESTRICTING";
        case Indication::CAB_SPEED: return "CAB_SPEED";
        case Indication::STOP:
        default: return "STOP";
    }
}

inline Indication nameToIndication(const char* str) {
    if (!str) return Indication::STOP;
    if (strcasecmp(str, "CLEAR") == 0) return Indication::CLEAR;
    if (strcasecmp(str, "APPROACH") == 0) return Indication::APPROACH;
    if (strcasecmp(str, "ADVANCE_APPROACH") == 0) return Indication::ADVANCE_APPROACH;
    if (strcasecmp(str, "MEDIUM_CLEAR") == 0) return Indication::MEDIUM_CLEAR;
    if (strcasecmp(str, "DIVERGING_CLEAR") == 0) return Indication::DIVERGING_CLEAR;
    if (strcasecmp(str, "MEDIUM_APPROACH") == 0) return Indication::MEDIUM_APPROACH;
    if (strcasecmp(str, "DIVERGING_APPROACH") == 0) return Indication::DIVERGING_APPROACH;
    if (strcasecmp(str, "APPROACH_MEDIUM") == 0) return Indication::APPROACH_MEDIUM;
    if (strcasecmp(str, "APPROACH_SLOW") == 0) return Indication::APPROACH_SLOW;
    if (strcasecmp(str, "APPROACH_DIVERGING") == 0) return Indication::APPROACH_DIVERGING;
    if (strcasecmp(str, "SLOW_CLEAR") == 0) return Indication::SLOW_CLEAR;
    if (strcasecmp(str, "SLOW_APPROACH") == 0) return Indication::SLOW_APPROACH;
    if (strcasecmp(str, "RESTRICTING") == 0) return Indication::RESTRICTING;
    if (strcasecmp(str, "DIVERGING_RESTRICTING") == 0) return Indication::DIVERGING_RESTRICTING;
    if (strcasecmp(str, "APPROACH_RESTRICTING") == 0) return Indication::APPROACH_RESTRICTING;
    if (strcasecmp(str, "CAB_SPEED") == 0) return Indication::CAB_SPEED;
    return Indication::STOP;
}

// -------------------------------------------------------------
// Lightweight In-Place JSON Helper Scanner
// Zero dynamic heap allocation.
// -------------------------------------------------------------

inline void skipWhitespace(const char*& p) {
    while (*p && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) {
        p++;
    }
}

inline bool skipValue(const char*& p) {
    skipWhitespace(p);
    if (!*p) return false;

    if (*p == '"') {
        p++;
        while (*p && *p != '"') {
            if (*p == '\\' && *(p + 1)) p++;
            p++;
        }
        if (*p == '"') p++;
        return true;
    } else if (*p == '{') {
        p++;
        int depth = 1;
        while (*p && depth > 0) {
            if (*p == '"') {
                p++;
                while (*p && *p != '"') {
                    if (*p == '\\' && *(p + 1)) p++;
                    p++;
                }
            } else if (*p == '{') {
                depth++;
            } else if (*p == '}') {
                depth--;
            }
            if (*p) p++;
        }
        return depth == 0;
    } else if (*p == '[') {
        p++;
        int depth = 1;
        while (*p && depth > 0) {
            if (*p == '"') {
                p++;
                while (*p && *p != '"') {
                    if (*p == '\\' && *(p + 1)) p++;
                    p++;
                }
            } else if (*p == '[') {
                depth++;
            } else if (*p == ']') {
                depth--;
            }
            if (*p) p++;
        }
        return depth == 0;
    } else {
        // Primitive: number or boolean
        while (*p && *p != ',' && *p != '}' && *p != ']' && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n') {
            p++;
        }
        return true;
    }
}

inline bool parseString(const char*& p, char* out, size_t maxLen) {
    skipWhitespace(p);
    if (*p != '"') return false;
    p++;
    size_t idx = 0;
    while (*p && *p != '"') {
        char c = *p++;
        if (c == '\\' && *p) {
            c = *p++;
        }
        if (idx + 1 < maxLen) {
            out[idx++] = c;
        }
    }
    out[idx] = '\0';
    if (*p == '"') p++;
    return true;
}

inline bool parseInt(const char*& p, int32_t& out) {
    skipWhitespace(p);
    char* endPtr = nullptr;
    out = static_cast<int32_t>(strtol(p, &endPtr, 10));
    if (endPtr == p) return false;
    p = endPtr;
    return true;
}

inline bool findKey(const char* objStart, const char* key, const char*& valStart) {
    const char* p = objStart;
    skipWhitespace(p);
    if (*p != '{') return false;
    p++;

    while (*p && *p != '}') {
        skipWhitespace(p);
        if (*p == '"') {
            char k[64];
            if (!parseString(p, k, sizeof(k))) return false;
            skipWhitespace(p);
            if (*p != ':') return false;
            p++;
            skipWhitespace(p);

            if (strcmp(k, key) == 0) {
                valStart = p;
                return true;
            } else {
                skipValue(p);
            }
        } else {
            p++;
        }
        skipWhitespace(p);
        if (*p == ',') p++;
    }
    return false;
}

// -------------------------------------------------------------
// Serialization: InterlockingPlant -> JSON
// -------------------------------------------------------------

inline bool serialize(const InterlockingPlant& cp, char* buffer, size_t maxLen, bool pretty = true) {
    if (!buffer || maxLen == 0) return false;

    size_t offset = 0;
    const char* nl = pretty ? "\n" : "";
    const char* sp = pretty ? "  " : "";
    const char* sp2 = pretty ? "    " : "";
    const char* sp3 = pretty ? "      " : "";

    auto append = [&](const char* str) -> bool {
        size_t len = strlen(str);
        if (offset + len >= maxLen) return false;
        memcpy(buffer + offset, str, len);
        offset += len;
        buffer[offset] = '\0';
        return true;
    };

    char line[256];

    // Root opening
    if (!append("{")) return false;
    if (!append(nl)) return false;

    // Plant Name & Default Aspect Policy
    snprintf(line, sizeof(line), "%s\"name\": \"%s\",%s", sp, cp.name(), nl);
    if (!append(line)) return false;

    snprintf(line, sizeof(line), "%s\"defaultAspectPolicy\": \"%s\",%s",
             sp, policyToName(cp.defaultAspectPolicy()), nl);
    if (!append(line)) return false;

    // 1. Track Circuits
    snprintf(line, sizeof(line), "%s\"trackCircuits\": [%s", sp, nl);
    if (!append(line)) return false;
    for (uint8_t i = 0; i < cp.trackCircuitCount(); ++i) {
        const TrackCircuit* tc = cp.trackCircuit(i);
        snprintf(line, sizeof(line), "%s{\"name\": \"%s\", \"dropoutDelayMs\": %u}%s%s",
                 sp2, tc->name(), static_cast<unsigned>(tc->dropoutDelay()),
                 (i + 1 < cp.trackCircuitCount()) ? "," : "", nl);
        if (!append(line)) return false;
    }
    snprintf(line, sizeof(line), "%s],%s", sp, nl);
    if (!append(line)) return false;

    // 1b. Line inputs (FieldUnit ADR 0007 D2)
    if (cp.lineInputCount() > 0) {
        snprintf(line, sizeof(line), "%s\"lineInputs\": [%s", sp, nl);
        if (!append(line)) return false;
        for (uint8_t i = 0; i < cp.lineInputCount(); ++i) {
            const LineInput* li = cp.lineInput(i);
            snprintf(line, sizeof(line), "%s{\"name\": \"%s\", \"stalenessTimeoutMs\": %u}%s%s",
                     sp2, li->name(), static_cast<unsigned>(li->stalenessTimeout()),
                     (i + 1 < cp.lineInputCount()) ? "," : "", nl);
            if (!append(line)) return false;
        }
        snprintf(line, sizeof(line), "%s],%s", sp, nl);
        if (!append(line)) return false;
    }

    // 2. Switches (non-derail appliances only)
    uint8_t plainSwitchCount = 0;
    uint8_t derailCount = 0;
    for (uint8_t i = 0; i < cp.switchCount(); ++i) {
        if (cp.getSwitch(i)->isDerail()) derailCount++;
        else plainSwitchCount++;
    }

    snprintf(line, sizeof(line), "%s\"switches\": [%s", sp, nl);
    if (!append(line)) return false;
    uint8_t plainEmitted = 0;
    for (uint8_t i = 0; i < cp.switchCount(); ++i) {
        const Switch* sw = cp.getSwitch(i);
        if (sw->isDerail()) continue;
        plainEmitted++;
        const TrackCircuit* os = cp.findDetectorCircuitForSwitch(sw);
        if (os) {
            snprintf(line, sizeof(line), "%s{\"name\": \"%s\", \"os\": \"%s\"}%s%s",
                     sp2, sw->name(), os->name(),
                     (plainEmitted < plainSwitchCount) ? "," : "", nl);
        } else {
            snprintf(line, sizeof(line), "%s{\"name\": \"%s\"}%s%s",
                     sp2, sw->name(),
                     (plainEmitted < plainSwitchCount) ? "," : "", nl);
        }
        if (!append(line)) return false;
    }
    snprintf(line, sizeof(line), "%s],%s", sp, nl);
    if (!append(line)) return false;

    // 2b. Derails (independent or dependent *D)
    snprintf(line, sizeof(line), "%s\"derails\": [%s", sp, nl);
    if (!append(line)) return false;
    uint8_t derailEmitted = 0;
    for (uint8_t i = 0; i < cp.switchCount(); ++i) {
        const Switch* sw = cp.getSwitch(i);
        if (!sw->isDerail()) continue;
        derailEmitted++;
        const TrackCircuit* os = cp.findDetectorCircuitForSwitch(sw);
        if (os) {
            snprintf(line, sizeof(line), "%s{\"name\": \"%s\", \"os\": \"%s\"}%s%s",
                     sp2, sw->name(), os->name(),
                     (derailEmitted < derailCount) ? "," : "", nl);
        } else {
            snprintf(line, sizeof(line), "%s{\"name\": \"%s\"}%s%s",
                     sp2, sw->name(),
                     (derailEmitted < derailCount) ? "," : "", nl);
        }
        if (!append(line)) return false;
    }
    snprintf(line, sizeof(line), "%s],%s", sp, nl);
    if (!append(line)) return false;

    // 3. Crossovers
    snprintf(line, sizeof(line), "%s\"crossovers\": [%s", sp, nl);
    if (!append(line)) return false;
    for (uint8_t i = 0; i < cp.crossoverCount(); ++i) {
        const Crossover* xo = cp.crossover(i);
        snprintf(line, sizeof(line), "%s{\"name\": \"%s\", \"switchA\": \"%s\", \"switchB\": \"%s\"}%s%s",
                 sp2, xo->name(), xo->switchA() ? xo->switchA()->name() : "",
                 xo->switchB() ? xo->switchB()->name() : "",
                 (i + 1 < cp.crossoverCount()) ? "," : "", nl);
        if (!append(line)) return false;
    }
    snprintf(line, sizeof(line), "%s],%s", sp, nl);
    if (!append(line)) return false;

    // 4. Signal Controls
    snprintf(line, sizeof(line), "%s\"signalControls\": [%s", sp, nl);
    if (!append(line)) return false;
    for (uint8_t i = 0; i < cp.authorityCount(); ++i) {
        const SignalControl* sc = cp.authority(i);
        snprintf(line, sizeof(line), "%s{\"name\": \"%s\"}%s%s",
                 sp2, sc->name(), (i + 1 < cp.authorityCount()) ? "," : "", nl);
        if (!append(line)) return false;
    }
    snprintf(line, sizeof(line), "%s],%s", sp, nl);
    if (!append(line)) return false;

    // 5. Signal Masts (an automatic signal's mast is written with it, 5b)
    snprintf(line, sizeof(line), "%s\"signalMasts\": [%s", sp, nl);
    if (!append(line)) return false;
    uint8_t plainMastCount = 0;
    for (uint8_t i = 0; i < cp.mastCount(); ++i) {
        if (!cp.isAutomaticMast(cp.mast(i))) plainMastCount++;
    }
    uint8_t plainMastEmitted = 0;
    for (uint8_t i = 0; i < cp.mastCount(); ++i) {
        const SignalMast* sm = cp.mast(i);
        if (cp.isAutomaticMast(sm)) continue;
        plainMastEmitted++;
        snprintf(line, sizeof(line), "%s{\"name\": \"%s\", \"type\": \"%s\", \"aspectPolicy\": \"%s\"}%s%s",
                 sp2, sm->name(), mastTypeToName(sm->type()), policyToName(sm->aspectPolicy()),
                 (plainMastEmitted < plainMastCount) ? "," : "", nl);
        if (!append(line)) return false;
    }
    snprintf(line, sizeof(line), "%s],%s", sp, nl);
    if (!append(line)) return false;

    // 5b. Automatic signals (FieldUnit ADR 0008 D6)
    if (cp.automaticSignalCount() > 0) {
        snprintf(line, sizeof(line), "%s\"automaticSignals\": [%s", sp, nl);
        if (!append(line)) return false;
        for (uint8_t i = 0; i < cp.automaticSignalCount(); ++i) {
            const AutomaticSignal* a = cp.automaticSignal(i);
            const SignalMast* sm = a->mast();
            snprintf(line, sizeof(line), "%s{\"name\": \"%s\", \"type\": \"%s\", \"aspectPolicy\": \"%s\", ",
                     sp2, a->name(),
                     sm ? mastTypeToName(sm->type()) : "ONE_HEAD",
                     sm ? policyToName(sm->aspectPolicy()) : "defaultRoute");
            if (!append(line)) return false;
            if (!append("\"governs\": [")) return false;
            for (uint8_t g = 0; g < a->governedCount(); ++g) {
                snprintf(line, sizeof(line), "\"%s\"%s", a->governed(g)->name(), (g + 1 < a->governedCount()) ? ", " : "");
                if (!append(line)) return false;
            }
            if (!append("], \"approach\": [")) return false;
            for (uint8_t g = 0; g < a->approachCount(); ++g) {
                snprintf(line, sizeof(line), "\"%s\"%s", a->approachFact(g)->name(), (g + 1 < a->approachCount()) ? ", " : "");
                if (!append(line)) return false;
            }
            if (!append("]")) return false;
            if (a->hIn() || a->dIn()) {
                snprintf(line, sizeof(line), ", \"reads\": {\"h\": \"%s\", \"d\": \"%s\"}",
                         a->hIn() ? a->hIn()->name() : "", a->dIn() ? a->dIn()->name() : "");
                if (!append(line)) return false;
            }
            if (a->opposingSignal()) {
                snprintf(line, sizeof(line), ", \"opposing\": \"%s\"", a->opposingSignal()->name());
                if (!append(line)) return false;
            }
            snprintf(line, sizeof(line), "}%s%s", (i + 1 < cp.automaticSignalCount()) ? "," : "", nl);
            if (!append(line)) return false;
        }
        snprintf(line, sizeof(line), "%s],%s", sp, nl);
        if (!append(line)) return false;
    }

    // 6. Detector Locks
    snprintf(line, sizeof(line), "%s\"detectorLocks\": [%s", sp, nl);
    if (!append(line)) return false;
    for (uint8_t i = 0; i < cp.detectorLockCount(); ++i) {
        const Switch* sw = cp.detectorLockSwitch(i);
        const TrackCircuit* tc = cp.detectorLockTrackCircuit(i);
        snprintf(line, sizeof(line), "%s{\"switch\": \"%s\", \"trackCircuit\": \"%s\"}%s%s",
                 sp2, sw ? sw->name() : "", tc ? tc->name() : "",
                 (i + 1 < cp.detectorLockCount()) ? "," : "", nl);
        if (!append(line)) return false;
    }
    snprintf(line, sizeof(line), "%s],%s", sp, nl);
    if (!append(line)) return false;

    // 7. Routes
    snprintf(line, sizeof(line), "%s\"routes\": [%s", sp, nl);
    if (!append(line)) return false;
    const InterlockingEngine& eng = cp.engine();
    uint8_t plainRouteCount = 0;
    for (uint8_t i = 0; i < eng.routeCount(); ++i) {
        if (!eng.route(i).isAutomatic()) plainRouteCount++;
    }
    uint8_t plainRouteEmitted = 0;
    for (uint8_t i = 0; i < eng.routeCount(); ++i) {
        const Route& r = eng.route(i);
        if (r.isAutomatic()) continue;   // written under automaticSignals
        plainRouteEmitted++;
        snprintf(line, sizeof(line), "%s{%s", sp2, nl);
        if (!append(line)) return false;

        snprintf(line, sizeof(line), "%s\"name\": \"%s\",%s", sp3, r.name(), nl);
        if (!append(line)) return false;

        if (r.authority()) {
            snprintf(line, sizeof(line), "%s\"governedBy\": {\"signal\": \"%s\", \"direction\": \"%s\"},%s",
                     sp3, r.authority()->name(), directionToName(r.direction()), nl);
            if (!append(line)) return false;
        }

        if (r.mast()) {
            snprintf(line, sizeof(line), "%s\"displays\": {\"mast\": \"%s\", \"maxIndication\": \"%s\"},%s",
                     sp3, r.mast()->name(), indicationToName(r.aspectCeiling()), nl);
            if (!append(line)) return false;
        }

        // aligns switches
        snprintf(line, sizeof(line), "%s\"aligns\": [%s", sp3, nl);
        if (!append(line)) return false;
        for (uint8_t s = 0; s < r.switchCount(); ++s) {
            const SwitchRequirement& req = r.switchReq(s);
            if (req.releasingBlock) {
                snprintf(line, sizeof(line), "%s  {\"switch\": \"%s\", \"position\": \"%s\", \"releasingBlock\": \"%s\"}%s%s",
                         sp3, req.switchRef->name(), positionToName(req.requiredPosition),
                         req.releasingBlock->name(), (s + 1 < r.switchCount()) ? "," : "", nl);
            } else {
                snprintf(line, sizeof(line), "%s  {\"switch\": \"%s\", \"position\": \"%s\"}%s%s",
                         sp3, req.switchRef->name(), positionToName(req.requiredPosition),
                         (s + 1 < r.switchCount()) ? "," : "", nl);
            }
            if (!append(line)) return false;
        }
        snprintf(line, sizeof(line), "%s],%s", sp3, nl);
        if (!append(line)) return false;

        // clears blocks
        snprintf(line, sizeof(line), "%s\"clears\": [", sp3);
        if (!append(line)) return false;
        for (uint8_t b = 0; b < r.blockCount(); ++b) {
            snprintf(line, sizeof(line), "\"%s\"%s", r.block(b)->name(), (b + 1 < r.blockCount()) ? ", " : "");
            if (!append(line)) return false;
        }
        snprintf(line, sizeof(line), "],%s", nl);
        if (!append(line)) return false;

        // home facts (HR terms)
        if (r.homeFactCount() > 0) {
            snprintf(line, sizeof(line), "%s\"home\": [", sp3);
            if (!append(line)) return false;
            for (uint8_t f = 0; f < r.homeFactCount(); ++f) {
                snprintf(line, sizeof(line), "\"%s\"%s", r.homeFact(f)->name(), (f + 1 < r.homeFactCount()) ? ", " : "");
                if (!append(line)) return false;
            }
            snprintf(line, sizeof(line), "],%s", nl);
            if (!append(line)) return false;
        }

        // proceed fact (DR term)
        if (r.proceedFact()) {
            snprintf(line, sizeof(line), "%s\"proceed\": \"%s\",%s", sp3, r.proceedFact()->name(), nl);
            if (!append(line)) return false;
        }

        // entrance
        if (r.entranceBlock()) {
            snprintf(line, sizeof(line), "%s\"entrance\": \"%s\"%s%s",
                     sp3, r.entranceBlock()->name(), r.approachBlock() ? "," : "", nl);
            if (!append(line)) return false;
        }

        // approaching
        if (r.approachBlock()) {
            snprintf(line, sizeof(line), "%s\"approaching\": \"%s\"%s",
                     sp3, r.approachBlock()->name(), nl);
            if (!append(line)) return false;
        }

        snprintf(line, sizeof(line), "%s}%s%s", sp2, (plainRouteEmitted < plainRouteCount) ? "," : "", nl);
        if (!append(line)) return false;
    }
    snprintf(line, sizeof(line), "%s]%s", sp, nl);
    if (!append(line)) return false;

    // Root closing
    if (!append("}")) return false;
    if (!append(nl)) return false;

    return true;
}

// -------------------------------------------------------------
// Deserialization: JSON -> InterlockingPlant
// -------------------------------------------------------------

inline bool deserialize(InterlockingPlant& cp, const char* json) {
    if (!json) return false;

    // Reset plant to neutral empty state before loading new definition
    cp.reset();

    const char* val = nullptr;

    // 0. Plant Name
    if (findKey(json, "name", val)) {
        char plantName[32];
        if (parseString(val, plantName, sizeof(plantName))) {
            cp.setName(plantName);
        }
    }

    // 1. Default Aspect Policy
    if (findKey(json, "defaultAspectPolicy", val)) {
        char policyStr[32];
        if (parseString(val, policyStr, sizeof(policyStr))) {
            cp.setDefaultAspectPolicy(nameToPolicy(policyStr));
        }
    }

    // 2. Track Circuits
    if (findKey(json, "trackCircuits", val)) {
        skipWhitespace(val);
        if (*val == '[') {
            val++;
            while (*val && *val != ']') {
                skipWhitespace(val);
                if (*val == '{') {
                    const char* itemStart = val;
                    const char* tcVal = nullptr;
                    char name[32] = "";
                    int32_t delay = 0;

                    if (findKey(itemStart, "name", tcVal)) {
                        parseString(tcVal, name, sizeof(name));
                    }
                    if (findKey(itemStart, "dropoutDelayMs", tcVal)) {
                        parseInt(tcVal, delay);
                    }
                    if (name[0] != '\0') {
                        cp.addTrackCircuit(name, static_cast<uint32_t>(delay));
                    }
                    skipValue(val);
                } else {
                    val++;
                }
                skipWhitespace(val);
                if (*val == ',') val++;
            }
        }
    }

    // 2b. Line inputs
    if (findKey(json, "lineInputs", val)) {
        skipWhitespace(val);
        if (*val == '[') {
            val++;
            while (*val && *val != ']') {
                skipWhitespace(val);
                if (*val == '{') {
                    const char* itemStart = val;
                    const char* liVal = nullptr;
                    char name[32] = "";
                    int32_t timeout = 5000;

                    if (findKey(itemStart, "name", liVal)) {
                        parseString(liVal, name, sizeof(name));
                    }
                    if (findKey(itemStart, "stalenessTimeoutMs", liVal)) {
                        parseInt(liVal, timeout);
                    }
                    if (name[0] != '\0') {
                        cp.addLineInput(name, static_cast<uint32_t>(timeout));
                    }
                    skipValue(val);
                } else {
                    val++;
                }
                skipWhitespace(val);
                if (*val == ',') val++;
            }
        }
    }

    // 3. Switches (optional "os" binds detector lock)
    if (findKey(json, "switches", val)) {
        skipWhitespace(val);
        if (*val == '[') {
            val++;
            while (*val && *val != ']') {
                skipWhitespace(val);
                if (*val == '{') {
                    const char* itemStart = val;
                    const char* swVal = nullptr;
                    char name[32] = "";
                    char osName[32] = "";

                    if (findKey(itemStart, "name", swVal)) {
                        parseString(swVal, name, sizeof(name));
                    }
                    if (findKey(itemStart, "os", swVal)) {
                        parseString(swVal, osName, sizeof(osName));
                    }
                    if (name[0] != '\0') {
                        if (osName[0] != '\0') {
                            cp.addSwitch(name, osName);
                        } else {
                            cp.addSwitch(name);
                        }
                    }
                    skipValue(val);
                } else {
                    val++;
                }
                skipWhitespace(val);
                if (*val == ',') val++;
            }
        }
    }

    // 3b. Derails (after switches so dependent *D base exists)
    if (findKey(json, "derails", val)) {
        skipWhitespace(val);
        if (*val == '[') {
            val++;
            while (*val && *val != ']') {
                skipWhitespace(val);
                if (*val == '{') {
                    const char* itemStart = val;
                    const char* dVal = nullptr;
                    char name[32] = "";
                    char osName[32] = "";

                    if (findKey(itemStart, "name", dVal)) {
                        parseString(dVal, name, sizeof(name));
                    }
                    if (findKey(itemStart, "os", dVal)) {
                        parseString(dVal, osName, sizeof(osName));
                    }
                    if (name[0] != '\0') {
                        if (osName[0] != '\0') {
                            cp.addDerail(name, osName);
                        } else {
                            cp.addDerail(name);
                        }
                    }
                    skipValue(val);
                } else {
                    val++;
                }
                skipWhitespace(val);
                if (*val == ',') val++;
            }
        }
    }

    // 4. Crossovers
    if (findKey(json, "crossovers", val)) {
        skipWhitespace(val);
        if (*val == '[') {
            val++;
            while (*val && *val != ']') {
                skipWhitespace(val);
                if (*val == '{') {
                    const char* itemStart = val;
                    const char* xoVal = nullptr;
                    char name[32] = "";
                    char swA[32] = "";
                    char swB[32] = "";

                    if (findKey(itemStart, "name", xoVal)) {
                        parseString(xoVal, name, sizeof(name));
                    }
                    if (findKey(itemStart, "switchA", xoVal)) {
                        parseString(xoVal, swA, sizeof(swA));
                    }
                    if (findKey(itemStart, "switchB", xoVal)) {
                        parseString(xoVal, swB, sizeof(swB));
                    }
                    if (name[0] != '\0' && swA[0] != '\0' && swB[0] != '\0') {
                        cp.addCrossover(name, swA, swB);
                    }
                    skipValue(val);
                } else {
                    val++;
                }
                skipWhitespace(val);
                if (*val == ',') val++;
            }
        }
    }

    // 5. Signal Controls
    if (findKey(json, "signalControls", val)) {
        skipWhitespace(val);
        if (*val == '[') {
            val++;
            while (*val && *val != ']') {
                skipWhitespace(val);
                if (*val == '{') {
                    const char* itemStart = val;
                    const char* scVal = nullptr;
                    char name[32] = "";

                    if (findKey(itemStart, "name", scVal)) {
                        parseString(scVal, name, sizeof(name));
                    }
                    if (name[0] != '\0') {
                        cp.addSignalControl(name);
                    }
                    skipValue(val);
                } else {
                    val++;
                }
                skipWhitespace(val);
                if (*val == ',') val++;
            }
        }
    }

    // 6. Signal Masts
    if (findKey(json, "signalMasts", val)) {
        skipWhitespace(val);
        if (*val == '[') {
            val++;
            while (*val && *val != ']') {
                skipWhitespace(val);
                if (*val == '{') {
                    const char* itemStart = val;
                    const char* smVal = nullptr;
                    char name[32] = "";
                    char typeStr[32] = "ONE_HEAD";
                    char policyStr[32] = "";

                    if (findKey(itemStart, "name", smVal)) {
                        parseString(smVal, name, sizeof(name));
                    }
                    if (findKey(itemStart, "type", smVal)) {
                        parseString(smVal, typeStr, sizeof(typeStr));
                    }
                    if (findKey(itemStart, "aspectPolicy", smVal)) {
                        parseString(smVal, policyStr, sizeof(policyStr));
                    }
                    if (name[0] != '\0') {
                        MastType mt = nameToMastType(typeStr);
                        AspectResolver policy = (policyStr[0] != '\0') ? nameToPolicy(policyStr) : nullptr;
                        cp.addSignalMast(name, mt, policy);
                    }
                    skipValue(val);
                } else {
                    val++;
                }
                skipWhitespace(val);
                if (*val == ',') val++;
            }
        }
    }

    // 7. Detector Locks
    if (findKey(json, "detectorLocks", val)) {
        skipWhitespace(val);
        if (*val == '[') {
            val++;
            while (*val && *val != ']') {
                skipWhitespace(val);
                if (*val == '{') {
                    const char* itemStart = val;
                    const char* dlVal = nullptr;
                    char swName[32] = "";
                    char tcName[32] = "";

                    if (findKey(itemStart, "switch", dlVal)) {
                        parseString(dlVal, swName, sizeof(swName));
                    }
                    if (findKey(itemStart, "trackCircuit", dlVal)) {
                        parseString(dlVal, tcName, sizeof(tcName));
                    }
                    if (swName[0] != '\0' && tcName[0] != '\0') {
                        cp.bindDetectorLock(swName, tcName);
                    }
                    skipValue(val);
                } else {
                    val++;
                }
                skipWhitespace(val);
                if (*val == ',') val++;
            }
        }
    }

    // 6b. Automatic signals (FieldUnit ADR 0008 D6). Two passes: every
    // signal exists before any binding, so a local location may read
    // another's outputs whatever the order in the file.
    for (int pass = 0; pass < 2; ++pass) {
        if (!findKey(json, "automaticSignals", val)) break;
        skipWhitespace(val);
        if (*val != '[') break;
        val++;
        while (*val && *val != ']') {
            skipWhitespace(val);
            if (*val == '{') {
                const char* itemStart = val;
                const char* aVal = nullptr;
                char name[32] = "";
                if (findKey(itemStart, "name", aVal)) {
                    parseString(aVal, name, sizeof(name));
                }
                if (name[0] != '\0') {
                    if (pass == 0) {
                        char typeStr[32] = "ONE_HEAD";
                        char policyStr[32] = "";
                        if (findKey(itemStart, "type", aVal)) {
                            parseString(aVal, typeStr, sizeof(typeStr));
                        }
                        if (findKey(itemStart, "aspectPolicy", aVal)) {
                            parseString(aVal, policyStr, sizeof(policyStr));
                        }
                        AspectResolver policy = (policyStr[0] != '\0') ? nameToPolicy(policyStr) : nullptr;
                        cp.addAutomaticSignal(name, nameToMastType(typeStr), policy);
                    } else {
                        AutomaticSignal* a = cp.findAutomaticSignal(name);
                        if (a) {
                            const char* listKeys[2] = { "governs", "approach" };
                            for (int k = 0; k < 2; ++k) {
                                if (!findKey(itemStart, listKeys[k], aVal)) continue;
                                skipWhitespace(aVal);
                                if (*aVal != '[') continue;
                                aVal++;
                                while (*aVal && *aVal != ']') {
                                    skipWhitespace(aVal);
                                    if (*aVal == '"') {
                                        char fname[32] = "";
                                        if (parseString(aVal, fname, sizeof(fname))) {
                                            if (k == 0) a->governs({ fname }); else a->approach({ fname });
                                        }
                                    } else {
                                        aVal++;
                                    }
                                    skipWhitespace(aVal);
                                    if (*aVal == ',') aVal++;
                                }
                            }
                            if (findKey(itemStart, "reads", aVal)) {
                                const char* readsObj = aVal;
                                const char* rv = nullptr;
                                char hName[32] = "";
                                char dName[32] = "";
                                if (findKey(readsObj, "h", rv)) parseString(rv, hName, sizeof(hName));
                                if (findKey(readsObj, "d", rv)) parseString(rv, dName, sizeof(dName));
                                a->reads(hName[0] ? hName : nullptr, dName[0] ? dName : nullptr);
                            }
                            if (findKey(itemStart, "opposing", aVal)) {
                                char oName[32] = "";
                                if (parseString(aVal, oName, sizeof(oName))) {
                                    a->opposing(oName);
                                }
                            }
                        }
                    }
                }
                skipValue(val);
            } else {
                val++;
            }
            skipWhitespace(val);
            if (*val == ',') val++;
        }
    }

    // 8. Routes
    if (findKey(json, "routes", val)) {
        skipWhitespace(val);
        if (*val == '[') {
            val++;
            while (*val && *val != ']') {
                skipWhitespace(val);
                if (*val == '{') {
                    const char* routeObj = val;
                    const char* rVal = nullptr;
                    char rName[32] = "";

                    if (findKey(routeObj, "name", rVal)) {
                        parseString(rVal, rName, sizeof(rName));
                    }

                    if (rName[0] != '\0') {
                        Route& r = cp.route(rName);

                        // governedBy
                        if (findKey(routeObj, "governedBy", rVal)) {
                            const char* gbObj = rVal;
                            const char* gbVal = nullptr;
                            char sigName[32] = "";
                            char dirStr[32] = "STOP";

                            if (findKey(gbObj, "signal", gbVal)) {
                                parseString(gbVal, sigName, sizeof(sigName));
                            }
                            if (findKey(gbObj, "direction", gbVal)) {
                                parseString(gbVal, dirStr, sizeof(dirStr));
                            }
                            if (sigName[0] != '\0') {
                                r.governedBy(sigName, nameToDirection(dirStr));
                            }
                        }

                        // displays
                        if (findKey(routeObj, "displays", rVal)) {
                            const char* dObj = rVal;
                            const char* dVal = nullptr;
                            char mastName[32] = "";
                            char indStr[32] = "STOP";

                            if (findKey(dObj, "mast", dVal)) {
                                parseString(dVal, mastName, sizeof(mastName));
                            }
                            if (findKey(dObj, "maxIndication", dVal)) {
                                parseString(dVal, indStr, sizeof(indStr));
                            }
                            if (mastName[0] != '\0') {
                                r.displays(mastName, nameToIndication(indStr));
                            }
                        }

                        // aligns switches
                        if (findKey(routeObj, "aligns", rVal)) {
                            skipWhitespace(rVal);
                            if (*rVal == '[') {
                                rVal++;
                                while (*rVal && *rVal != ']') {
                                    skipWhitespace(rVal);
                                    if (*rVal == '{') {
                                        const char* swItem = rVal;
                                        const char* sVal = nullptr;
                                        char sName[32] = "";
                                        char posStr[32] = "NORMAL";
                                        char relName[32] = "";

                                        if (findKey(swItem, "switch", sVal)) {
                                            parseString(sVal, sName, sizeof(sName));
                                        }
                                        if (findKey(swItem, "position", sVal)) {
                                            parseString(sVal, posStr, sizeof(posStr));
                                        }
                                        if (findKey(swItem, "releasingBlock", sVal)) {
                                            parseString(sVal, relName, sizeof(relName));
                                        }

                                        if (sName[0] != '\0') {
                                            r.align(sName, nameToPosition(posStr),
                                                    (relName[0] != '\0') ? relName : nullptr);
                                        }
                                        skipValue(rVal);
                                    } else {
                                        rVal++;
                                    }
                                    skipWhitespace(rVal);
                                    if (*rVal == ',') rVal++;
                                }
                            }
                        }

                        // clears blocks
                        if (findKey(routeObj, "clears", rVal)) {
                            skipWhitespace(rVal);
                            if (*rVal == '[') {
                                rVal++;
                                while (*rVal && *rVal != ']') {
                                    skipWhitespace(rVal);
                                    if (*rVal == '"') {
                                        char tcName[32] = "";
                                        if (parseString(rVal, tcName, sizeof(tcName))) {
                                            r.clearBlock(tcName);
                                        }
                                    } else {
                                        rVal++;
                                    }
                                    skipWhitespace(rVal);
                                    if (*rVal == ',') rVal++;
                                }
                            }
                        }

                        // home facts (HR terms)
                        if (findKey(routeObj, "home", rVal)) {
                            skipWhitespace(rVal);
                            if (*rVal == '[') {
                                rVal++;
                                while (*rVal && *rVal != ']') {
                                    skipWhitespace(rVal);
                                    if (*rVal == '"') {
                                        char factName[32] = "";
                                        if (parseString(rVal, factName, sizeof(factName))) {
                                            r.homeFact(factName);
                                        }
                                    } else {
                                        rVal++;
                                    }
                                    skipWhitespace(rVal);
                                    if (*rVal == ',') rVal++;
                                }
                            }
                        }

                        // proceed fact (DR term)
                        if (findKey(routeObj, "proceed", rVal)) {
                            char factName[32];
                            if (parseString(rVal, factName, sizeof(factName))) {
                                r.proceed(factName);
                            }
                        }

                        // entrance
                        if (findKey(routeObj, "entrance", rVal)) {
                            char entName[32];
                            if (parseString(rVal, entName, sizeof(entName))) {
                                r.entrance(entName);
                            }
                        }

                        // approaching
                        if (findKey(routeObj, "approaching", rVal)) {
                            char appName[32];
                            if (parseString(rVal, appName, sizeof(appName))) {
                                r.approaching(appName);
                            }
                        }
                    }
                    skipValue(val);
                } else {
                    val++;
                }
                skipWhitespace(val);
                if (*val == ',') val++;
            }
        }
    }

    return true;
}

} // namespace PlantSerializer

// InterlockingPlant serialization convenience methods
inline bool InterlockingPlant::serialize(char* buffer, size_t maxLen, bool pretty) const {
    return PlantSerializer::serialize(*this, buffer, maxLen, pretty);
}

inline bool InterlockingPlant::deserialize(const char* json) {
    return PlantSerializer::deserialize(*this, json);
}

} // namespace FieldUnit

#endif // FIELDUNIT_PLANT_SERIALIZER_H
