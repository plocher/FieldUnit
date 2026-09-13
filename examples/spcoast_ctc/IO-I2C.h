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
        Wire.setClock(800000UL); // 800 kHz Fast-Mode Plus I2C

        // Probe for MAX7313 base address (check 0x20 first, fallback to 0x10)
        uint8_t baseAddr = 0x20;
        Wire.beginTransmission(0x20);
        if (Wire.endTransmission() != 0) {
            Wire.beginTransmission(0x10);
            if (Wire.endTransmission() == 0) {
                baseAddr = 0x10;
            }
        }
        Serial.printf("[I2C] Running at 800 kHz. Detected MAX7313 expanders at base address 0x%02X\n", baseAddr);

        for (uint8_t i = 0; i < 14; ++i) {
            uint8_t addr = baseAddr + i;
            m_[i].init(addr, I2Cexpander::MAX7313, 0b0001111011000100);
            m_[i].put(0xFFFF); // All lamps OFF at startup (active-LOW)
            inputs_[i] = m_[i].get();
            lastOutputs_[i] = 0xFFFF;
            codeArmed_[i] = false;
            codeTriggered_[i] = false;
        }
    }

    // Direct hardware lamp test: all ON for 2s, all OFF, then column chase
    void runLampTest() {
        Serial.println("--- Starting Hardware Lamp Test ---");
        // All ON (active-LOW: write 0x0000 to all 14 expanders)
        for (uint8_t i = 0; i < 14; ++i) m_[i].put(0x0000);
        delay(2000);

        // All OFF
        for (uint8_t i = 0; i < 14; ++i) m_[i].put(0xFFFF);
        delay(500);

        // Column chase across 14 columns
        uint16_t testBits[] = { 0x0001, 0x0002, 0x0008, 0x0010, 0x0020, 0x0100, 0x2000, 0x8000, 0x4000 };
        for (uint8_t col = 0; col < 14; ++col) {
            for (uint16_t bit : testBits) {
                m_[col].put((uint16_t)~bit); // Light single lamp
                delay(40);
            }
            m_[col].put(0xFFFF); // Off
        }
        Serial.println("--- Lamp Test Complete ---\n");
    }

    // Raw direct loopback mirror: connects inputs directly to outputs without 1-shot
    void directMirrorLoop() {
        for (uint8_t col = 0; col < 14; ++col) {
            uint16_t ival = m_[col].get();
            uint16_t oval = 0xFFFF; // All lamps OFF by default

            // Inputs (active-LOW: 0 = asserted)
            bool swN  = (ival & 0x0080) == 0; // bit 7
            bool swR  = (ival & 0x0040) == 0; // bit 6
            bool sigE = (ival & 0x0200) == 0; // bit 9
            bool sigS = (ival & 0x0400) == 0; // bit 10
            bool sigW = (ival & 0x0800) == 0; // bit 11
            bool mc   = (ival & 0x0004) == 0; // bit 2
            bool code = (ival & 0x1000) == 0; // bit 12

            // Mirror directly to outputs (active-LOW: 0 = ON)
            if (swN)  oval &= ~0x0001; // bit 0 (NK)
            if (swR)  oval &= ~0x0002; // bit 1 (RK)
            if (sigE) oval &= ~0x2000; // bit 13 (LK/EK)
            if (sigS) oval &= ~0x8000; // bit 15 (SK)
            if (sigW) oval &= ~0x4000; // bit 14 (WK)
            if (mc)   oval &= ~0x0100; // bit 8 (MCK)
            if (code) oval &= ~(0x0008 | 0x0010 | 0x0020); // bits 3,4,5 (M1,M2,M3)

            m_[col].put(oval);
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
