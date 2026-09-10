#ifndef FIELDUNIT_SIGNAL_ASPECT_POLICY_H
#define FIELDUNIT_SIGNAL_ASPECT_POLICY_H

#include "types.h"

namespace FieldUnit {

// Multi-head resolved physical lamp aspects
struct MastAspects {
    Aspect head1;
    Aspect head2;
    Aspect head3;

    constexpr MastAspects()
        : head1(Aspect::RED), head2(Aspect::DARK), head3(Aspect::DARK) {}

    constexpr MastAspects(Aspect h1, Aspect h2 = Aspect::DARK, Aspect h3 = Aspect::DARK)
        : head1(h1), head2(h2), head3(h3) {}
};

// Signature for pluggable rulebook policies:
// Given (Indication, headCount, isDwarf), returns individual head aspects.
typedef MastAspects (*AspectResolver)(Indication ind, uint8_t headCount, bool isDwarf);

namespace AspectPolicies {

/**
 * Standard Default Route Signaling Policy (Traditional / Pre-1980s standard)
 * - Clear: Green (Two-Head: Green over Red)
 * - Approach: Yellow (Two-Head: Yellow over Red)
 * - Advance Approach: Flashing Yellow (Two-Head: Flashing Yellow over Red)
 * - Diverging Clear: Red over Green
 * - Diverging Approach: Red over Yellow
 * - Restricting / Diverging Restricting: Lunar (Two-Head: Red over Lunar)
 * - Stop: Red (Two-Head: Red over Red)
 */
inline MastAspects defaultRoute(Indication ind, uint8_t headCount, bool isDwarf) {
    if (isDwarf || headCount == 1) {
        switch (ind) {
            case Indication::CLEAR:
            case Indication::DIVERGING_CLEAR:
                return MastAspects(Aspect::GREEN);
            case Indication::APPROACH:
            case Indication::DIVERGING_APPROACH:
                return MastAspects(Aspect::YELLOW);
            case Indication::ADVANCE_APPROACH:
                return MastAspects(Aspect::FLASHING_YELLOW);
            case Indication::RESTRICTING:
            case Indication::DIVERGING_RESTRICTING:
                return MastAspects(Aspect::LUNAR);
            case Indication::STOP:
            default:
                return MastAspects(Aspect::RED);
        }
    }

    // Two-Head / Three-Head Masts
    Aspect h3 = (headCount >= 3) ? Aspect::RED : Aspect::DARK;
    switch (ind) {
        case Indication::CLEAR:
            return MastAspects(Aspect::GREEN, Aspect::RED, h3);
        case Indication::APPROACH:
            return MastAspects(Aspect::YELLOW, Aspect::RED, h3);
        case Indication::ADVANCE_APPROACH:
            return MastAspects(Aspect::FLASHING_YELLOW, Aspect::RED, h3);
        case Indication::DIVERGING_CLEAR:
            return MastAspects(Aspect::RED, Aspect::GREEN, h3);
        case Indication::DIVERGING_APPROACH:
            return MastAspects(Aspect::RED, Aspect::YELLOW, h3);
        case Indication::DIVERGING_RESTRICTING:
        case Indication::RESTRICTING:
            return MastAspects(Aspect::RED, Aspect::LUNAR, h3);
        case Indication::APPROACH_DIVERGING:
            return MastAspects(Aspect::YELLOW, Aspect::YELLOW, h3);
        case Indication::APPROACH_MEDIUM:
            return MastAspects(Aspect::YELLOW, Aspect::GREEN, h3);
        case Indication::MEDIUM_CLEAR:
            return MastAspects(Aspect::RED, Aspect::GREEN, h3);
        case Indication::STOP:
        default:
            return MastAspects(Aspect::RED, Aspect::RED, h3);
    }
}

/**
 * Southern Pacific 1969 Rulebook Policy (Lunar Era)
 * - Rule 281 Clear: Green over Red (Dwarf: Green)
 * - Rule 285 Approach: Yellow over Red (Dwarf: Yellow)
 * - Rule 283 Diverging Clear: Red over Green
 * - Rule 286 Diverging Approach: Red over Yellow
 * - Rule 290 Restricting: Red over Lunar (Dwarf: Lunar)
 * - Rule 292 Stop: Red over Red (Dwarf: Red)
 */
inline MastAspects sp1969(Indication ind, uint8_t headCount, bool isDwarf) {
    return defaultRoute(ind, headCount, isDwarf);
}

/**
 * Southern Pacific 1985 / Later Rulebook Policy (Flashing Red Era)
 * - Rule 281 Clear: Green over Red (Dwarf: Green)
 * - Rule 285 Approach: Yellow over Red (Dwarf: Yellow)
 * - Rule 283 Diverging Clear: Red over Green
 * - Rule 286 Diverging Approach: Red over Yellow
 * - Rule 290 Restricting: Red over FLASHING RED (Dwarf: FLASHING RED)
 * - Rule 292 Stop: Red over Red (Dwarf: Red)
 */
inline MastAspects sp1985(Indication ind, uint8_t headCount, bool isDwarf) {
    if (isDwarf || headCount == 1) {
        if (ind == Indication::RESTRICTING || ind == Indication::DIVERGING_RESTRICTING) {
            return MastAspects(Aspect::FLASHING_RED);
        }
        return defaultRoute(ind, headCount, isDwarf);
    }

    Aspect h3 = (headCount >= 3) ? Aspect::RED : Aspect::DARK;
    if (ind == Indication::RESTRICTING || ind == Indication::DIVERGING_RESTRICTING) {
        return MastAspects(Aspect::RED, Aspect::FLASHING_RED, h3);
    }
    return defaultRoute(ind, headCount, isDwarf);
}

/**
 * GCOR Modern Speed Signaling Policy
 * - Clear: Green over Red
 * - Approach: Yellow over Red
 * - Advance Approach: Flashing Yellow over Red
 * - Approach Diverging: Yellow over Yellow
 * - Diverging Clear: Red over Green
 * - Diverging Approach: Red over Yellow
 * - Restricting: Red over Flashing Red (or Red over Lunar)
 * - Stop: Red over Red
 */
inline MastAspects gcorSpeed(Indication ind, uint8_t headCount, bool isDwarf) {
    return sp1985(ind, headCount, isDwarf);
}

/**
 * New York Central (NYC) / Eastern 3-Head Speed Signaling Policy
 * - Rule 281 Clear: Green over Red over Red (Two-Head: Green over Red, Dwarf: Green)
 * - Rule 282 Approach Medium: Yellow over Green over Red (Two-Head: Yellow over Green)
 * - Rule 282A Advance Approach: Yellow over Yellow over Red (Two-Head: Yellow over Yellow)
 * - Rule 283 Medium Clear: Red over Green over Red (Two-Head: Red over Green)
 * - Rule 284 Approach Slow: Yellow over Red over Green (Two-Head: Yellow over Red)
 * - Rule 285 Approach: Yellow over Red over Red (Two-Head: Yellow over Red)
 * - Rule 286 Medium Approach: Red over Yellow over Red (Two-Head: Red over Yellow)
 * - Rule 287 Slow Clear: Red over Red over Green
 * - Rule 288 Slow Approach: Red over Red over Yellow
 * - Rule 290 Restricting: Red over Red over Yellow (Two-Head: Red over Yellow, Dwarf: Yellow)
 * - Rule 292 Stop: Red over Red over Red (Two-Head: Red over Red, Dwarf: Red)
 */
inline MastAspects nycSpeed(Indication ind, uint8_t headCount, bool isDwarf) {
    if (isDwarf || headCount == 1) {
        switch (ind) {
            case Indication::CLEAR:
            case Indication::MEDIUM_CLEAR:
            case Indication::SLOW_CLEAR:
                return MastAspects(Aspect::GREEN);
            case Indication::APPROACH:
            case Indication::MEDIUM_APPROACH:
            case Indication::SLOW_APPROACH:
            case Indication::RESTRICTING:
            case Indication::DIVERGING_RESTRICTING:
                return MastAspects(Aspect::YELLOW);
            case Indication::ADVANCE_APPROACH:
                return MastAspects(Aspect::FLASHING_YELLOW);
            case Indication::STOP:
            default:
                return MastAspects(Aspect::RED);
        }
    }

    if (headCount == 2) {
        switch (ind) {
            case Indication::CLEAR:
                return MastAspects(Aspect::GREEN, Aspect::RED);
            case Indication::APPROACH_MEDIUM:
                return MastAspects(Aspect::YELLOW, Aspect::GREEN);
            case Indication::ADVANCE_APPROACH:
            case Indication::APPROACH_DIVERGING:
                return MastAspects(Aspect::YELLOW, Aspect::YELLOW);
            case Indication::MEDIUM_CLEAR:
            case Indication::DIVERGING_CLEAR:
                return MastAspects(Aspect::RED, Aspect::GREEN);
            case Indication::APPROACH:
                return MastAspects(Aspect::YELLOW, Aspect::RED);
            case Indication::MEDIUM_APPROACH:
            case Indication::DIVERGING_APPROACH:
            case Indication::RESTRICTING:
            case Indication::DIVERGING_RESTRICTING:
                return MastAspects(Aspect::RED, Aspect::YELLOW);
            case Indication::STOP:
            default:
                return MastAspects(Aspect::RED, Aspect::RED);
        }
    }

    // Three-Head Mast (Standard Eastern Interlocking Home Signal)
    switch (ind) {
        case Indication::CLEAR:
            return MastAspects(Aspect::GREEN, Aspect::RED, Aspect::RED);
        case Indication::APPROACH_MEDIUM:
            return MastAspects(Aspect::YELLOW, Aspect::GREEN, Aspect::RED);
        case Indication::ADVANCE_APPROACH:
        case Indication::APPROACH_DIVERGING:
            return MastAspects(Aspect::YELLOW, Aspect::YELLOW, Aspect::RED);
        case Indication::MEDIUM_CLEAR:
        case Indication::DIVERGING_CLEAR:
            return MastAspects(Aspect::RED, Aspect::GREEN, Aspect::RED);
        case Indication::APPROACH:
            return MastAspects(Aspect::YELLOW, Aspect::RED, Aspect::RED);
        case Indication::MEDIUM_APPROACH:
        case Indication::DIVERGING_APPROACH:
            return MastAspects(Aspect::RED, Aspect::YELLOW, Aspect::RED);
        case Indication::SLOW_CLEAR:
            return MastAspects(Aspect::RED, Aspect::RED, Aspect::GREEN);
        case Indication::SLOW_APPROACH:
            return MastAspects(Aspect::RED, Aspect::RED, Aspect::YELLOW);
        case Indication::RESTRICTING:
        case Indication::DIVERGING_RESTRICTING:
            return MastAspects(Aspect::RED, Aspect::RED, Aspect::YELLOW);
        case Indication::STOP:
        default:
            return MastAspects(Aspect::RED, Aspect::RED, Aspect::RED);
    }
}

/**
 * Pennsylvania Railroad (PRR) Position-Light Policy
 * Amber light rows wired to Head pins (Horizontal=Red, Diagonal=Yellow, Vertical=Green)
 * - Clear: Vertical (Head 1 Green, Head 2 Dark)
 * - Approach: Diagonal (Head 1 Yellow, Head 2 Dark)
 * - Medium Clear: Horizontal over Vertical (Head 1 Red, Head 2 Green)
 * - Medium Approach: Horizontal over Diagonal (Head 1 Red, Head 2 Yellow)
 * - Restricting: Horizontal over Lower Diagonal (Head 1 Red, Head 2 Yellow)
 * - Stop: Horizontal (Head 1 Red, Head 2 Dark)
 */
inline MastAspects prrPositionLight(Indication ind, uint8_t headCount, bool isDwarf) {
    if (isDwarf || headCount == 1) {
        switch (ind) {
            case Indication::CLEAR:
            case Indication::MEDIUM_CLEAR:
                return MastAspects(Aspect::GREEN);
            case Indication::APPROACH:
            case Indication::RESTRICTING:
            case Indication::DIVERGING_RESTRICTING:
                return MastAspects(Aspect::YELLOW);
            case Indication::STOP:
            default:
                return MastAspects(Aspect::RED);
        }
    }

    Aspect h2Dark = Aspect::DARK;
    Aspect h3 = Aspect::DARK;
    switch (ind) {
        case Indication::CLEAR:
            return MastAspects(Aspect::GREEN, h2Dark, h3);
        case Indication::APPROACH:
            return MastAspects(Aspect::YELLOW, h2Dark, h3);
        case Indication::MEDIUM_CLEAR:
        case Indication::DIVERGING_CLEAR:
            return MastAspects(Aspect::RED, Aspect::GREEN, h3);
        case Indication::MEDIUM_APPROACH:
        case Indication::DIVERGING_APPROACH:
        case Indication::RESTRICTING:
        case Indication::DIVERGING_RESTRICTING:
            return MastAspects(Aspect::RED, Aspect::YELLOW, h3);
        case Indication::STOP:
        default:
            return MastAspects(Aspect::RED, h2Dark, h3);
    }
}

/**
 * Standard Upper-Quadrant Semaphore Policy (1, 2, or 3 Blades)
 * - Clear (90 deg vertical): Top blade Green
 * - Diverging Clear: Top blade Red (0 deg), Lower blade Green (90 deg)
 * - Approach (45 deg diagonal): Top blade Yellow (45 deg)
 * - Diverging Approach: Top blade Red (0 deg), Lower blade Yellow (45 deg)
 * - Stop (0 deg horizontal): All blades Red (0 deg)
 */
inline MastAspects upperQuadrantSemaphore(Indication ind, uint8_t headCount, bool isDwarf) {
    return defaultRoute(ind, headCount, isDwarf);
}

} // namespace AspectPolicies

} // namespace FieldUnit

#endif // FIELDUNIT_SIGNAL_ASPECT_POLICY_H
