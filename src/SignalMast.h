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

    // Vital action: update the displayed aspect based on derived indication
    void setIndication(Indication ind) {
        currentIndication_ = ind;
        mapIndicationToAspects(ind);
    }

    void forceStop() {
        setIndication(Indication::STOP);
    }

private:
    // Standard North American 2-head / 1-head color-light rulebook mapping
    void mapIndicationToAspects(Indication ind) {
        switch (type_) {
            case MastType::ONE_HEAD:
            case MastType::DWARF:
                switch (ind) {
                    case Indication::CLEAR:
                        head1Aspect_ = Aspect::GREEN;
                        break;
                    case Indication::APPROACH:
                    case Indication::ADVANCE_APPROACH:
                        head1Aspect_ = Aspect::YELLOW;
                        break;
                    case Indication::RESTRICTING:
                    case Indication::DIVERGING_RESTRICTING:
                        head1Aspect_ = Aspect::LUNAR; // or FLASHING_RED
                        break;
                    case Indication::STOP:
                    default:
                        head1Aspect_ = Aspect::RED;
                        break;
                }
                head2Aspect_ = Aspect::DARK;
                head3Aspect_ = Aspect::DARK;
                break;

            case MastType::TWO_HEAD:
                switch (ind) {
                    case Indication::CLEAR:
                        head1Aspect_ = Aspect::GREEN;
                        head2Aspect_ = Aspect::RED;
                        break;
                    case Indication::APPROACH:
                        head1Aspect_ = Aspect::YELLOW;
                        head2Aspect_ = Aspect::RED;
                        break;
                    case Indication::ADVANCE_APPROACH:
                        head1Aspect_ = Aspect::FLASHING_YELLOW;
                        head2Aspect_ = Aspect::RED;
                        break;
                    case Indication::DIVERGING_CLEAR:
                        head1Aspect_ = Aspect::RED;
                        head2Aspect_ = Aspect::GREEN;
                        break;
                    case Indication::DIVERGING_APPROACH:
                        head1Aspect_ = Aspect::RED;
                        head2Aspect_ = Aspect::YELLOW;
                        break;
                    case Indication::RESTRICTING:
                    case Indication::DIVERGING_RESTRICTING:
                        head1Aspect_ = Aspect::RED;
                        head2Aspect_ = Aspect::LUNAR; // or RED_OVER_YELLOW on some lines
                        break;
                    case Indication::STOP:
                    default:
                        head1Aspect_ = Aspect::RED;
                        head2Aspect_ = Aspect::RED;
                        break;
                }
                head3Aspect_ = Aspect::DARK;
                break;

            case MastType::THREE_HEAD:
                // Extendable for 3-head systems (e.g. Medium vs Slow vs Restricting)
                head1Aspect_ = (ind == Indication::CLEAR) ? Aspect::GREEN : Aspect::RED;
                head2Aspect_ = (ind == Indication::DIVERGING_CLEAR) ? Aspect::GREEN : Aspect::RED;
                head3Aspect_ = Aspect::RED;
                break;
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
