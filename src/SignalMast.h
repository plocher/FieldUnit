#ifndef FIELDUNIT_SIGNAL_MAST_H
#define FIELDUNIT_SIGNAL_MAST_H

#include "types.h"
#include "SignalAspectPolicy.h"

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

    SignalMast(const char* name, MastType type, AspectResolver policy = AspectPolicies::defaultRoute)
        : name_(name),
          type_(type),
          policy_(policy ? policy : AspectPolicies::defaultRoute),
          index_(0),
          currentIndication_(Indication::STOP),
          head1Aspect_(Aspect::RED),
          head2Aspect_(Aspect::RED),
          head3Aspect_(Aspect::DARK) {}

    const char* name() const { return name_; }
    uint8_t index() const { return index_; }
    void setIndex(uint8_t idx) { index_ = idx; }
    MastType type() const { return type_; }
    Indication currentIndication() const { return currentIndication_; }

    uint8_t headCount() const {
        switch (type_) {
            case MastType::ONE_HEAD: return 1;
            case MastType::TWO_HEAD: return 2;
            case MastType::THREE_HEAD: return 3;
            case MastType::DWARF: return 1;
            default: return 1;
        }
    }

    bool isDwarf() const {
        return type_ == MastType::DWARF;
    }

    SignalMast* setAspectPolicy(AspectResolver policy) {
        policy_ = policy ? policy : AspectPolicies::defaultRoute;
        return this;
    }

    SignalMast* withAspectPolicy(AspectResolver policy) {
        return setAspectPolicy(policy);
    }

    AspectResolver aspectPolicy() const {
        return policy_;
    }

    Aspect head1() const { return head1Aspect_; }
    Aspect head2() const { return head2Aspect_; }
    Aspect head3() const { return head3Aspect_; }

    // Composite aspect representing visual appearance of the mast
    Aspect compositeAspect() const {
        if (type_ == MastType::ONE_HEAD || type_ == MastType::DWARF) {
            return head1Aspect_;
        }
        if (type_ == MastType::TWO_HEAD) {
            if (head1Aspect_ == Aspect::GREEN && head2Aspect_ == Aspect::RED) return Aspect::GREEN_OVER_RED;
            if (head1Aspect_ == Aspect::RED && head2Aspect_ == Aspect::GREEN) return Aspect::RED_OVER_GREEN;
            if (head1Aspect_ == Aspect::YELLOW && head2Aspect_ == Aspect::RED) return Aspect::YELLOW_OVER_RED;
            if (head1Aspect_ == Aspect::RED && head2Aspect_ == Aspect::YELLOW) return Aspect::RED_OVER_YELLOW;
            if (head1Aspect_ == Aspect::RED && head2Aspect_ == Aspect::LUNAR)  return Aspect::RED_OVER_LUNAR;
            if (head1Aspect_ == Aspect::RED && head2Aspect_ == Aspect::FLASHING_RED) return Aspect::RED_OVER_FLASHING_RED;
            if (head1Aspect_ == Aspect::RED && head2Aspect_ == Aspect::FLASHING_YELLOW) return Aspect::RED_OVER_FLASHING_YELLOW;
            if (head1Aspect_ == Aspect::YELLOW && head2Aspect_ == Aspect::YELLOW) return Aspect::YELLOW_OVER_YELLOW;
            if (head1Aspect_ == Aspect::YELLOW && head2Aspect_ == Aspect::GREEN) return Aspect::YELLOW_OVER_GREEN;
            if (head1Aspect_ == Aspect::RED && head2Aspect_ == Aspect::RED)   return Aspect::RED_OVER_RED;
            return head1Aspect_;
        }
        return head1Aspect_;
    }

    // Set indication across the entire mast using the configured AspectPolicy
    void setIndication(Indication ind) {
        currentIndication_ = ind;
        MastAspects a = policy_(ind, headCount(), isDwarf());
        head1Aspect_ = a.head1;
        head2Aspect_ = (headCount() >= 2) ? a.head2 : Aspect::DARK;
        head3Aspect_ = (headCount() >= 3) ? a.head3 : Aspect::DARK;
    }

    // Vital action: update the displayed aspect on a specific head
    // Integrates with AspectPolicy for diverging vs main route indications
    void setHeadIndication(uint8_t headIndex, Indication ind) {
        currentIndication_ = ind;
        if (headIndex == 0) {
            setIndication(ind);
            return;
        }

        Indication effInd = ind;
        if (headIndex == 1) {
            if (ind == Indication::CLEAR) effInd = Indication::DIVERGING_CLEAR;
            else if (ind == Indication::APPROACH) effInd = Indication::DIVERGING_APPROACH;
            else if (ind == Indication::RESTRICTING) effInd = Indication::DIVERGING_RESTRICTING;
        }
        setIndication(effInd);
    }

    void forceStop() {
        setIndication(Indication::STOP);
    }

private:
    const char* name_;
    MastType type_;
    AspectResolver policy_;
    uint8_t index_;
    Indication currentIndication_;
    Aspect head1Aspect_;
    Aspect head2Aspect_;
    Aspect head3Aspect_;
};

} // namespace FieldUnit

#endif // FIELDUNIT_SIGNAL_MAST_H
