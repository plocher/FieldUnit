#ifndef SPCOAST_IO_CMRI_H
#define SPCOAST_IO_CMRI_H

#include <Arduino.h>
#include <cTcMachine.h>

class PanelIO : public FieldUnit::PanelHardware {
public:
    PanelIO() {
        memset(ib_, 0, sizeof(ib_));
        memset(ob_, 0, sizeof(ob_));
    }

    // In C/MRI, each 16-bit column corresponds to 2 bytes in IB[] and 2 bytes in OB[]
    static uint8_t colToByteOffset(uint8_t col) { return (col >= 1) ? (col - 1) * 2 : 0; }

    bool readBit(uint8_t col, uint8_t pin) {
        uint8_t base = colToByteOffset(col);
        uint8_t byteIdx = base + (pin / 8);
        uint8_t bitIdx  = pin % 8;
        if (byteIdx < sizeof(ib_)) {
            return (ib_[byteIdx] & (1 << bitIdx)) != 0;
        }
        return false;
    }

    void writeBit(uint8_t col, uint8_t pin, bool state) {
        uint8_t base = colToByteOffset(col);
        uint8_t byteIdx = base + (pin / 8);
        uint8_t bitIdx  = pin % 8;
        if (byteIdx < sizeof(ob_)) {
            if (state) ob_[byteIdx] |= (1 << bitIdx);
            else       ob_[byteIdx] &= ~(1 << bitIdx);
        }
    }

    bool read(uint8_t col, FieldUnit::PanelInput fn) override {
        switch (fn) {
            case FieldUnit::PanelInput::SW_NORMAL:          return readBit(col, 6);
            case FieldUnit::PanelInput::SW_REVERSE:         return readBit(col, 7);
            case FieldUnit::PanelInput::SIG_LEFT:           return readBit(col, 9);
            case FieldUnit::PanelInput::SIG_STOP:           return readBit(col, 10);
            case FieldUnit::PanelInput::SIG_RIGHT:          return readBit(col, 11);
            case FieldUnit::PanelInput::CODE_BUTTON:        return readBit(col, 12);
            case FieldUnit::PanelInput::MAINTAINER_CALL_SW: return readBit(col, 2);
            default: return false;
        }
    }

    void write(uint8_t col, FieldUnit::PanelOutput fn, bool state) override {
        switch (fn) {
            case FieldUnit::PanelOutput::SW_NORMAL_LAMP:  writeBit(col, 0, state); break;
            case FieldUnit::PanelOutput::SW_REVERSE_LAMP: writeBit(col, 1, state); break;
            case FieldUnit::PanelOutput::TRACK_LAMP_1:    writeBit(col, 3, state); break;
            case FieldUnit::PanelOutput::TRACK_LAMP_2:    writeBit(col, 4, state); break;
            case FieldUnit::PanelOutput::TRACK_LAMP_3:    writeBit(col, 5, state); break;
            case FieldUnit::PanelOutput::MAINTAINER_LAMP: writeBit(col, 8, state); break;
            case FieldUnit::PanelOutput::SIG_LEFT_LAMP:   writeBit(col, 13, state); break;
            case FieldUnit::PanelOutput::SIG_RIGHT_LAMP:  writeBit(col, 14, state); break;
            case FieldUnit::PanelOutput::SIG_STOP_LAMP:   writeBit(col, 15, state); break;
            default: break;
        }
    }

    void begin() override {}
    void syncInputs() override  {}
    void syncOutputs() override {}

    uint8_t* inputBuffer() { return ib_; }
    const uint8_t* outputBuffer() const { return ob_; }

private:
    uint8_t ib_[28]; // 14 columns * 2 bytes = 28 bytes
    uint8_t ob_[28];
};

#endif // SPCOAST_IO_CMRI_H
