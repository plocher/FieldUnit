#ifndef FIELDUNIT_LINE_INPUT_H
#define FIELDUNIT_LINE_INPUT_H

#include "types.h"

namespace FieldUnit {

/**
 * Fact: a named vital boolean that a route can require.
 *
 * FieldUnit ADR 0007 D2. A route's home() terms are ANDed into its HR
 * condition; its proceed() term decides Clear versus Approach (the DR
 * condition). The operand of both is a Fact. The Fact's source is a
 * binding made at configuration: a LineInput today (a line circuit, a
 * topic, a neighbour's indication); a local mast later.
 *
 * Vital rule: asserted() is false whenever the source is released,
 * stale, or has never been set.
 */
class Fact {
public:
    virtual ~Fact() {}
    virtual const char* name() const = 0;
    virtual bool asserted() const = 0;
};

/**
 * LineInput: the consumer end of a line circuit.
 *
 * The H, D or F value of an HD line as the interlocking reads it
 * (FieldUnit-Subdivision ADR 0013 section 13: segment_clear, next_proceed,
 * traffic_free). Energized is asserted.
 *
 * Fail-safe, as for a remote TrackCircuit:
 * - released until the first update();
 * - released again when no update() arrives within the staleness
 *   timeout (default 5000 ms). A timeout of 0 disables staleness, for a
 *   fact that lives in the same processor's memory.
 */
class LineInput : public Fact {
public:
    LineInput() : LineInput("") {}

    explicit LineInput(const char* name, uint32_t stalenessTimeoutMs = 5000)
        : energized_(false),
          everUpdated_(false),
          stale_(false),
          index_(0),
          lastUpdateMs_(0),
          stalenessTimeoutMs_(stalenessTimeoutMs) {
        setName(name);
    }

    void setName(const char* name) {
        if (!name) { name_[0] = '\0'; return; }
        strncpy(name_, name, sizeof(name_) - 1);
        name_[sizeof(name_) - 1] = '\0';
    }

    const char* name() const override { return name_; }
    uint8_t index() const { return index_; }
    void setIndex(uint8_t idx) { index_ = idx; }

    // Vital query.
    bool asserted() const override {
        return energized_ && everUpdated_ && !stale_;
    }

    bool isStale() const { return everUpdated_ && stale_; }
    bool everUpdated() const { return everUpdated_; }

    // Update from a wire, a topic, or a decoded neighbour indication.
    void update(bool energized, uint32_t nowMs) {
        energized_ = energized;
        everUpdated_ = true;
        stale_ = false;
        lastUpdateMs_ = nowMs;
    }

    // Advance the clock: detect a lost source.
    void tick(uint32_t nowMs) {
        if (everUpdated_ && stalenessTimeoutMs_ > 0 &&
            (nowMs - lastUpdateMs_) > stalenessTimeoutMs_) {
            stale_ = true;
        }
    }

    void setStalenessTimeout(uint32_t timeoutMs) { stalenessTimeoutMs_ = timeoutMs; }
    uint32_t stalenessTimeout() const { return stalenessTimeoutMs_; }

private:
    char     name_[MAX_APPLIANCE_NAME_LEN];
    bool     energized_;
    bool     everUpdated_;
    bool     stale_;
    uint8_t  index_;
    uint32_t lastUpdateMs_;
    uint32_t stalenessTimeoutMs_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_LINE_INPUT_H
