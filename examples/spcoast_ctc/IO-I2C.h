#ifndef SPCOAST_IO_I2C_H
#define SPCOAST_IO_I2C_H

#include <Arduino.h>
#include <Wire.h>
#include <I2Cexpander.h>
#include <cTcMachine.h>

class PanelIO : public FieldUnit::PanelHardware {
public:
    PanelIO() {}

    // Maps column 1..14 to its corresponding MAX7313 expander index (0..13)
    static uint8_t colToDev(uint8_t col) { return (col >= 1 && col <= 14) ? (col - 1) : 0; }

    bool read(uint8_t col, FieldUnit::PanelInput fn) override {
        uint8_t dev = colToDev(col);
        switch (fn) {
            case FieldUnit::PanelInput::SW_NORMAL:          return m_[dev].digitalRead(6) != 0;
            case FieldUnit::PanelInput::SW_REVERSE:         return m_[dev].digitalRead(7) != 0;
            case FieldUnit::PanelInput::SIG_LEFT:           return m_[dev].digitalRead(9) != 0;
            case FieldUnit::PanelInput::SIG_STOP:           return m_[dev].digitalRead(10) != 0;
            case FieldUnit::PanelInput::SIG_RIGHT:          return m_[dev].digitalRead(11) != 0;
            case FieldUnit::PanelInput::CODE_BUTTON:        return m_[dev].digitalRead(12) != 0;
            case FieldUnit::PanelInput::MAINTAINER_CALL_SW: return m_[dev].digitalRead(2) != 0;
            default: return false;
        }
    }

    void write(uint8_t col, FieldUnit::PanelOutput fn, bool state) override {
        uint8_t dev = colToDev(col);
        uint8_t val = state ? 1 : 0;
        switch (fn) {
            case FieldUnit::PanelOutput::SW_NORMAL_LAMP:  m_[dev].digitalWrite(0, val); break;
            case FieldUnit::PanelOutput::SW_REVERSE_LAMP: m_[dev].digitalWrite(1, val); break;
            case FieldUnit::PanelOutput::TRACK_LAMP_1:    m_[dev].digitalWrite(3, val); break;
            case FieldUnit::PanelOutput::TRACK_LAMP_2:    m_[dev].digitalWrite(4, val); break;
            case FieldUnit::PanelOutput::TRACK_LAMP_3:    m_[dev].digitalWrite(5, val); break;
            case FieldUnit::PanelOutput::MAINTAINER_LAMP: m_[dev].digitalWrite(8, val); break;
            case FieldUnit::PanelOutput::SIG_LEFT_LAMP:   m_[dev].digitalWrite(13, val); break;
            case FieldUnit::PanelOutput::SIG_RIGHT_LAMP:  m_[dev].digitalWrite(14, val); break;
            case FieldUnit::PanelOutput::SIG_STOP_LAMP:   m_[dev].digitalWrite(15, val); break;
            default: break;
        }
    }

    void begin() override {
        Wire.begin();
        // Initialize 14x MAX7313 expanders (I2C addresses 0..13)
        for (uint8_t i = 0; i < 14; ++i) {
            m_[i].init(i, I2Cexpander::MAX7313, 0b0001111011000100);
        }
    }

private:
    I2Cexpander m_[14];
};

#endif // SPCOAST_IO_I2C_H
