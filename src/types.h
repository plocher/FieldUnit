#ifndef FIELDUNIT_TYPES_H
#define FIELDUNIT_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

namespace FieldUnit {

// Standard string buffer bounds
static constexpr uint8_t MAX_APPLIANCE_NAME_LEN = 16;
static constexpr uint8_t MAX_ROUTE_NAME_LEN     = 32;

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

// Switch physical positions and correspondence (AAR Switch, not Turnout)
enum class SwitchPosition : uint8_t {
    NORMAL = 1,
    REVERSE = 2,
    MOVING = 3,
    OUT_OF_CORRESPONDENCE = 4,
    UNKNOWN = 5
};

// Switch lock reasons (bitmask)
enum class SwitchLock : uint8_t {
    UNLOCKED        = 0x00,
    DETECTOR_LOCKED = 0x01,  // Track circuit over points is occupied
    ROUTE_LOCKED    = 0x02,  // Active cleared route reserves this switch
    TIME_LOCKED     = 0x04,  // Approach timer running down after signal knockdown
    HAND_LOCKED     = 0x08,  // Local electric switch lock engaged (points secured)
    HAND_UNLOCKED   = 0x08   // Deprecated alias for backwards compatibility
};

inline SwitchLock operator|(SwitchLock a, SwitchLock b) {
    return static_cast<SwitchLock>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline SwitchLock operator&(SwitchLock a, SwitchLock b) {
    return static_cast<SwitchLock>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

// Dispatcher switch demand in a control transaction
enum class SwitchDemand : uint8_t {
    NO_CHANGE = 0, // 00 on wire: leave switch in existing state
    NORMAL    = 1, // 10 on wire: command switch to Normal
    REVERSE   = 2  // 01 on wire: command switch to Reverse
};

// Dispatcher electric switch lock demand in a control transaction
enum class ElectricLockDemand : uint8_t {
    NO_CHANGE = 0, // Leave electric lock state unchanged
    LOCK      = 1, // Command electric lock to secure switch (normal locked)
    UNLOCK    = 2  // Command electric lock to release switch (crew may throw)
};

// Dispatcher signal demand in a control transaction
enum class SignalDemand : uint8_t {
    NO_CHANGE = 0, // 000 on wire: leave signal authority unchanged
    STOP      = 1, // 001 on wire (H bit): drop signal to STOP
    LEFT      = 2, // 010 on wire (NG/L bit): grant Leftward/Northward authority
    RIGHT     = 3  // 100 on wire (SG/R bit): grant Rightward/Southward authority
};

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
    CLEAR = 8,
    APPROACH_RESTRICTING = 9,
    APPROACH_MEDIUM = 10,
    MEDIUM_CLEAR = 11,
    MEDIUM_APPROACH = 12,
    SLOW_CLEAR = 13,
    SLOW_APPROACH = 14,
    CAB_SPEED = 15,
    APPROACH_SLOW = 16
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
    FLASHING_GREEN = 7,
    FLASHING_LUNAR = 8,
    // Multi-head combinations (simplified for color-light)
    RED_OVER_RED = 10,
    RED_OVER_YELLOW = 11,
    RED_OVER_GREEN = 12,
    RED_OVER_LUNAR = 13,
    RED_OVER_FLASHING_RED = 14,
    RED_OVER_FLASHING_YELLOW = 15,
    YELLOW_OVER_RED = 16,
    YELLOW_OVER_YELLOW = 17,
    YELLOW_OVER_GREEN = 18,
    YELLOW_OVER_LUNAR = 19,
    GREEN_OVER_RED = 20,
    GREEN_OVER_GREEN = 21,
    FLASHING_RED_OVER_RED = 22,
    LUNAR_OVER_RED = 23
};

// B&O Color-Position-Light Orbital Marker Positions (Bitmask)
enum class CplMarker : uint8_t {
    NONE       = 0x00,
    TOP_12     = 0x01,  // 12 o'clock: Normal Speed route (High Speed)
    UPPER_R_2  = 0x02,  // 2 o'clock: Medium Speed route
    LOWER_R_4  = 0x04,  // 4 o'clock: Limited Speed route (or Approach Restriction)
    BOTTOM_6   = 0x08,  // 6 o'clock: Slow Speed route / Stop & Proceed
    LOWER_L_8  = 0x10,  // 8 o'clock: Auxiliary / Restricting (low marker)
    UPPER_L_10 = 0x20   // 10 o'clock: Cab Speed / Advance
};

inline CplMarker operator|(CplMarker a, CplMarker b) {
    return static_cast<CplMarker>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline CplMarker operator&(CplMarker a, CplMarker b) {
    return static_cast<CplMarker>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

} // namespace FieldUnit

#endif // FIELDUNIT_TYPES_H
