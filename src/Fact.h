#ifndef FIELDUNIT_FACT_H
#define FIELDUNIT_FACT_H

#include "types.h"

namespace FieldUnit {

/**
 * Fact: a named vital boolean that a route or a signal location can require.
 *
 * FieldUnit ADR 0007 D2 and ADR 0008 D2. A route's home() terms are ANDed
 * into its HR condition; its proceed() term decides Clear versus Approach
 * (the DR condition). The operand of both is a Fact. Sources today:
 * - LineInput: the H, D or F value of an HD line as read from a wire, a
 *   topic or a neighbour's code line indications;
 * - TrackCircuit: the TR, asserted when the circuit is clear;
 * - AutomaticSignal: the H and D it produces for the location in rear.
 *
 * Vital rule: asserted() is false whenever the source is released, stale,
 * or has never been set.
 */
class Fact {
public:
    virtual ~Fact() {}
    virtual const char* name() const = 0;
    virtual bool asserted() const = 0;
};

/**
 * ReleasedFact: a fact with no source and no slot to hold one (FieldUnit
 * ADR 0009 D1). A binding whose name resolves to nothing and for which the
 * plant has no line-input slot left binds here, so the term reads released
 * and the route is at Stop. One shared instance; never asserted.
 */
class ReleasedFact : public Fact {
public:
    const char* name() const override { return "(unsourced)"; }
    bool asserted() const override { return false; }
};

inline const Fact* releasedFact() {
    static ReleasedFact instance;
    return &instance;
}

} // namespace FieldUnit

#endif // FIELDUNIT_FACT_H
