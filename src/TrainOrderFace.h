#ifndef FIELDUNIT_TRAIN_ORDER_FACE_H
#define FIELDUNIT_TRAIN_ORDER_FACE_H

#include "types.h"
#include "SignalMast.h"

namespace FieldUnit {

// One face of a train order board (FieldUnit ADR 0010): a ranked list of
// inputs and one indication out. In each scan the face shows the indication
// of the highest-ranked input that is asserted, and its rest indication when
// none is. The rank is the order of the input() calls, not the order in time
// in which the inputs became asserted.
//
// The face has no debounce, no timer and no memory of an earlier result. The
// values given with set() are the present state of the lever contacts, as the
// I/O system reports them.
//
// A face is not part of the interlocking (D4). Its mast is not added to an
// InterlockingPlant, so it has no index, no place on the code line and no
// place in the plant JSON, and tick() does not see it. The sketch calls
// evaluate() beside its cycle.
//
// The words are data (D2): the generated sketch gives the rest indication and
// each input's indication at setup. Nothing is fixed here.
class TrainOrderFace {
public:
    // A face has three logical inputs: STOP, SLOW and CLEAR. Because the
    // board is not vital, a lever gives two of them and none asserted
    // implies CLEAR (the rest indication). A face with an explicit CLEAR
    // input is allowed, so the limit is three (the owner, 2026-11-12).
    // Declared in the rank STOP, SLOW, CLEAR with rest CLEAR, such a face
    // gives: no input, Clear; SLOW with CLEAR, Slow; STOP with anything, Stop.
    static constexpr uint8_t MAX_INPUTS = 3;

    // `mast` is the one-head mast this face sets (D3); nothing else sets it.
    // `rest` is what the face shows with no input asserted (D6).
    TrainOrderFace(SignalMast* mast, Indication rest)
        : mast_(mast), rest_(rest), inputCount_(0) {
        for (uint8_t i = 0; i < MAX_INPUTS; ++i) {
            indication_[i] = rest;
            asserted_[i] = false;
        }
    }

    // Add the next input in rank order: the first call is rank 0 and wins
    // over every later one. A call beyond MAX_INPUTS adds nothing;
    // inputCount() shows what was taken.
    TrainOrderFace& input(Indication whenAsserted) {
        if (inputCount_ < MAX_INPUTS) {
            indication_[inputCount_++] = whenAsserted;
        }
        return *this;
    }

    // Give the present value of the input at `rank`. An input that is never
    // set reads not asserted. A rank that was not declared is ignored.
    void set(uint8_t rank, bool asserted) {
        if (rank < inputCount_) asserted_[rank] = asserted;
    }

    // What the present inputs give. Changes nothing.
    Indication resolve() const {
        for (uint8_t i = 0; i < inputCount_; ++i) {
            if (asserted_[i]) return indication_[i];
        }
        return rest_;
    }

    // Set the mast from the present inputs and return what was set.
    Indication evaluate() {
        Indication ind = resolve();
        if (mast_) mast_->setIndication(ind);
        return ind;
    }

    SignalMast* mast() const { return mast_; }
    Indication rest() const { return rest_; }
    uint8_t inputCount() const { return inputCount_; }
    bool asserted(uint8_t rank) const { return rank < inputCount_ && asserted_[rank]; }
    Indication inputIndication(uint8_t rank) const {
        return rank < inputCount_ ? indication_[rank] : rest_;
    }

private:
    SignalMast* mast_;
    Indication  rest_;
    Indication  indication_[MAX_INPUTS];
    bool        asserted_[MAX_INPUTS];
    uint8_t     inputCount_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_TRAIN_ORDER_FACE_H
