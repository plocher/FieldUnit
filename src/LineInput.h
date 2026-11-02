#ifndef FIELDUNIT_LINE_INPUT_H
#define FIELDUNIT_LINE_INPUT_H

#include "types.h"
#include "Fact.h"

namespace FieldUnit {

/**
 * LineInput: the consumer end of a line circuit.
 *
 * The H, D or F value of an HD line as the interlocking reads it
 * (FieldUnit-Subdivision ADR 0013 section 13: segment_clear, next_proceed,
 * traffic_free). Energized is asserted.
 *
 * State and liveness are two things:
 * - update(value, nowMs): the state changed (and the source is alive);
 * - touch(nowMs): the source is alive and reports no change. Inert before
 *   the first update(), since a heartbeat from a source that has never
 *   reported a state still means "no clue".
 *
 * Fail-safe, as for a remote TrackCircuit:
 * - released until the first update();
 * - released again when neither update() nor touch() arrives within the
 *   staleness timeout (default 5000 ms). A timeout of 0 disables
 *   staleness, for a fact that lives in the same processor's memory.
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

    // State: update from a wire, a topic, or a decoded neighbour indication.
    void update(bool energized, uint32_t nowMs) {
        energized_ = energized;
        everUpdated_ = true;
        touch(nowMs);
    }

    // Liveness: the source is still watching and reports no change.
    void touch(uint32_t nowMs) {
        if (!everUpdated_) return;
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
