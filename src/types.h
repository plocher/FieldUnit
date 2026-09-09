#ifndef FIELDUNIT_TYPES_H
#define FIELDUNIT_TYPES_H

#include <stdint.h>
#include <stdbool.h>

namespace FieldUnit {

// Quality and health for all physical inputs
enum class Quality : uint8_t {
    GOOD = 0,
    LOST_COMMS = 1,
    DEFECT = 2
};

// Qualified value wrapper (ISO 26262 / vital signal pattern)
template <typename T>
struct Qualified {
    T value;
    Quality quality;
    uint32_t ageMs;

    bool isValid() const {
        return quality == Quality::GOOD;
    }
};

// Track occupancy states
enum class Occupancy : uint8_t {
    VACANT = 0,
    OCCUPIED = 1
};

// Turnout physical positions and correspondence
enum class TurnoutPosition : uint8_t {
    NORMAL = 1,
    REVERSE = 2,
    MOVING = 3,
    OUT_OF_CORRESPONDENCE = 4,
    UNKNOWN = 5
};

// Turnout lock reasons (bitmask)
enum class TurnoutLock : uint8_t {
    UNLOCKED        = 0x00,
    DETECTOR_LOCKED = 0x01,  // Track circuit over points is occupied
    ROUTE_LOCKED    = 0x02,  // Active cleared route reserves this switch
    TIME_LOCKED     = 0x04,  // Approach timer running down after signal knockdown
    HAND_UNLOCKED   = 0x08   // Local electric switch lock released
};

inline TurnoutLock operator|(TurnoutLock a, TurnoutLock b) {
    return static_cast<TurnoutLock>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline TurnoutLock operator&(TurnoutLock a, TurnoutLock b) {
    return static_cast<TurnoutLock>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

// Dispatcher traffic authority request
enum class DirectionAuthority : uint8_t {
    STOP = 0,
    LEFT = 1,
    RIGHT = 2
};

// Operational Indications (Rulebook meanings to train crew)
enum class Indication : uint8_t {
    STOP = 0,
    RESTRICTING = 1,
    APPROACH = 2,
    ADVANCE_APPROACH = 3,
    APPROACH_DIVERGING = 4,
    DIVERGING_RESTRICTING = 5,
    DIVERGING_APPROACH = 6,
    DIVERGING_CLEAR = 7,
    CLEAR = 8
};

// Visual Aspects (Lamps / Appearance)
enum class Aspect : uint8_t {
    DARK = 0,
    RED = 1,
    YELLOW = 2,
    GREEN = 3,
    LUNAR = 4,
    FLASHING_RED = 5,
    FLASHING_YELLOW = 6,
    // Multi-head combinations (simplified for color-light)
    RED_OVER_RED = 10,
    RED_OVER_YELLOW = 11,
    RED_OVER_GREEN = 12,
    RED_OVER_LUNAR = 13,
    YELLOW_OVER_RED = 14,
    YELLOW_OVER_GREEN = 15,
    GREEN_OVER_RED = 16
};

} // namespace FieldUnit

#endif // FIELDUNIT_TYPES_H
