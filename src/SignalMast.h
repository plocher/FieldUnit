#ifndef FIELDUNIT_SIGNAL_MAST_H
#define FIELDUNIT_SIGNAL_MAST_H

#include "types.h"

namespace FieldUnit {

enum class MastType : uint8_t {
    ONE_HEAD = 1,
    TWO_HEAD = 2,
    THREE_HEAD = 3,
    DWARF = 4
};

class SignalMast {
public:
    SignalMast() : SignalMast("", MastType::ONE_HEAD) {}

    SignalMast(const char* name, MastType type)
        : name_(name),
          type_(type),
          currentIndication_(Indication::STOP),
          head1Aspect_(Aspect::RED),
          head2Aspect_(Aspect::RED),
          head3Aspect_(Aspect::DARK) {}

    const char* name() const { return name_; }
    MastType type() const { return type_; }
    Indication currentIndication() const { return currentIndication_; }

    Aspect head1() const { return head1Aspect_; }
    Aspect head2() const { return head2Aspect_; }
    Aspect head3() const { return head3Aspect_; }

    // Vital action: update the displayed aspect on a specific head
    // Non-targeted heads display their STOP/RED marker
    void setHeadIndication(uint8_t headIndex, Indication ind) {
        forceStop(); // Reset all heads to red marker first
        currentIndication_ = ind;

        Aspect asp = mapIndicationToSingleHead(ind);
        if (headIndex == 0) {
            head1Aspect_ = asp;
        } else if (headIndex == 1) {
            head2Aspect_ = asp;
        } else if (headIndex == 2) {
            head3Aspect_ = asp;
        }
    }

    // Single-head or whole-mast aspect update
    void setIndication(Indication ind) {
        setHeadIndication(0, ind);
    }

    void forceStop() {
        currentIndication_ = Indication::STOP;
        head1Aspect_ = Aspect::RED;
        head2Aspect_ = (type_ >= MastType::TWO_HEAD) ? Aspect::RED : Aspect::DARK;
        head3Aspect_ = (type_ >= MastType::THREE_HEAD) ? Aspect::RED : Aspect::DARK;
    }

private:
    static Aspect mapIndicationToSingleHead(Indication ind) {
        switch (ind) {
            case Indication::CLEAR:
            case Indication::DIVERGING_CLEAR:
                return Aspect::GREEN;
            case Indication::APPROACH:
            case Indication::DIVERGING_APPROACH:
                return Aspect::YELLOW;
            case Indication::ADVANCE_APPROACH:
                return Aspect::FLASHING_YELLOW;
            case Indication::RESTRICTING:
            case Indication::DIVERGING_RESTRICTING:
                return Aspect::LUNAR;
            case Indication::STOP:
            default:
                return Aspect::RED;
        }
    }

    const char* name_;
    MastType type_;
    Indication currentIndication_;
    Aspect head1Aspect_;
    Aspect head2Aspect_;
    Aspect head3Aspect_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_SIGNAL_MAST_H
