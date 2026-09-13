#ifndef SPCOAST_IO_I2C_H
#define SPCOAST_IO_I2C_H

#include <Arduino.h>
#include <Wire.h>
#include <I2Cexpander.h>
#include <cTcMachine.h>

class PanelIO : public FieldUnit::PanelHardware {
public:
    PanelIO() {
        for (uint8_t i = 0; i < 14; ++i) {
            inputs_[i] = 0xFFFF;
            outputs_[i] = 0xFFFF;
            lastOutputs_[i] = 0x0000;
            codeArmed_[i] = false;
            codeTriggered_[i] = false;
        }
    }

    // Maps column 1..14 to its corresponding MAX7313 expander index (0..13)
    static uint8_t colToDev(uint8_t col) { return (col >= 1 && col <= 14) ? (col - 1) : 0; }

    void begin() override {
        Wire.begin();
        Wire.setClock(400000UL); // 400 kHz fast I2C
        for (uint8_t i = 0; i < 14; ++i) {
            m_[i].init(i, I2Cexpander::MAX7313, 0b0001111011000100);
            m_[i].put(0xFFFF); // All lamps OFF at startup (active-LOW)
            inputs_[i] = m_[i].get();
            lastOutputs_[i] = 0xFFFF;
            codeArmed_[i] = false;
            codeTriggered_[i] = false;
        }
    }

    // Single-pass bulk read: 14 fast 16-bit reads instead of 70+ individual I2C transactions
    void syncInputs() override {
        for (uint8_t i = 0; i < 14; ++i) {
            uint16_t prev = inputs_[i];
            inputs_[i] = m_[i].get();

            // Bit 12 (CODE button): active-LOW (0 = down/pressed, 1 = up/released)
            bool wasDown = (bitRead(prev, 12) == 0);
            bool isDown  = (bitRead(inputs_[i], 12) == 0);

            // Arm on press (high-to-low / 1 -> 0)
            if (isDown && !wasDown) {
                codeArmed_[i] = true;
            }
            // Trigger on release (low-to-high / 0 -> 1)
            else if (!isDown && wasDown && codeArmed_[i]) {
                codeArmed_[i] = false;
                codeTriggered_[i] = true; // Latched for this cycle
            }
        }
    }

    bool read(uint8_t col, FieldUnit::PanelInput fn) override {
        uint8_t dev = colToDev(col);
        uint16_t ival = inputs_[dev];
        switch (fn) {
            // Inputs are active-LOW: 0 = asserted, 1 = unasserted (pulled high)
            case FieldUnit::PanelInput::SW_NORMAL:          return bitRead(ival, 7) == 0;
            case FieldUnit::PanelInput::SW_REVERSE:         return bitRead(ival, 6) == 0;
            case FieldUnit::PanelInput::SIG_LEFT:           return bitRead(ival, 9) == 0;
            case FieldUnit::PanelInput::SIG_STOP:           return bitRead(ival, 10) == 0;
            case FieldUnit::PanelInput::SIG_RIGHT:          return bitRead(ival, 11) == 0;
            case FieldUnit::PanelInput::MAINTAINER_CALL_SW: return bitRead(ival, 2) == 0;
            case FieldUnit::PanelInput::CODE_BUTTON: {
                if (codeTriggered_[dev]) {
                    codeTriggered_[dev] = false; // Consume trigger
                    return true;
                }
                return false;
            }
            default: return false;
        }
    }

    void write(uint8_t col, FieldUnit::PanelOutput fn, bool state) override {
        uint8_t dev = colToDev(col);
        // Outputs are active-LOW on MAX7313: 0 = Lamp ON, 1 = Lamp OFF
        uint8_t bit = state ? 0 : 1;
        switch (fn) {
            case FieldUnit::PanelOutput::SW_NORMAL_LAMP:  bitWrite(outputs_[dev], 0, bit); break;
            case FieldUnit::PanelOutput::SW_REVERSE_LAMP: bitWrite(outputs_[dev], 1, bit); break;
            case FieldUnit::PanelOutput::TRACK_LAMP_1:    bitWrite(outputs_[dev], 3, bit); break;
            case FieldUnit::PanelOutput::TRACK_LAMP_2:    bitWrite(outputs_[dev], 4, bit); break;
            case FieldUnit::PanelOutput::TRACK_LAMP_3:    bitWrite(outputs_[dev], 5, bit); break;
            case FieldUnit::PanelOutput::MAINTAINER_LAMP: bitWrite(outputs_[dev], 8, bit); break;
            case FieldUnit::PanelOutput::SIG_LEFT_LAMP:   bitWrite(outputs_[dev], 13, bit); break;
            case FieldUnit::PanelOutput::SIG_RIGHT_LAMP:  bitWrite(outputs_[dev], 14, bit); break;
            case FieldUnit::PanelOutput::SIG_STOP_LAMP:   bitWrite(outputs_[dev], 15, bit); break;
            default: break;
        }
    }

    // Only writes to I2C if output bits have actually changed
    void syncOutputs() override {
        for (uint8_t i = 0; i < 14; ++i) {
            if (outputs_[i] != lastOutputs_[i]) {
                m_[i].put(outputs_[i]);
                lastOutputs_[i] = outputs_[i];
            }
        }
    }

private:
    I2Cexpander m_[14];
    uint16_t inputs_[14];
    uint16_t outputs_[14];
    uint16_t lastOutputs_[14];
    bool codeArmed_[14];
    bool codeTriggered_[14];
};

#endif // SPCOAST_IO_I2C_H
