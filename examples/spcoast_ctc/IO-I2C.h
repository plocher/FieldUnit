#ifndef SPCOAST_IO_I2C_H
#define SPCOAST_IO_I2C_H

#include <Arduino.h>
#include <Wire.h>
#include <I2Cexpander.h>
#include <cTcMachine.h>

// Time-based debounce for one active-LOW input bit (stable after settleMs of same raw value).
struct DebouncedInput {
    bool stable = false;      // debounced logical pressed/asserted (active-high sense)
    bool lastRaw = false;
    uint32_t lastChangeMs = 0;
    bool initialized = false;

    void update(bool rawAsserted, uint32_t nowMs, uint16_t settleMs) {
        if (!initialized) {
            lastRaw = rawAsserted;
            stable = rawAsserted;
            lastChangeMs = nowMs;
            initialized = true;
            return;
        }
        if (rawAsserted != lastRaw) {
            lastRaw = rawAsserted;
            lastChangeMs = nowMs;
            return;
        }
        if ((uint32_t)(nowMs - lastChangeMs) >= settleMs) {
            stable = lastRaw;
        }
    }
};

class PanelIO : public FieldUnit::PanelHardware {
public:
    static constexpr uint16_t kInputSettleMs = 15;
    // Input direction mask: 1 = input (CODE, SIG W/S/E, SW N/R, MC)
    static constexpr uint16_t kPanelInputMask = 0x1EC4;

    PanelIO() {
        for (uint8_t i = 0; i < 14; ++i) {
            inputs_[i] = 0xFFFF;
            rawInputs_[i] = 0xFFFF;
            outputs_[i] = 0xFFFF;
            lastOutputs_[i] = 0x0000;
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
            // Single-shot I2C read; time debounce handled in syncInputs()
            m_[i].init(addr, I2Cexpander::MAX7313, kPanelInputMask, /*debounce=*/false);
            m_[i].put(0xFFFF); // All lamps OFF at startup (active-LOW)
            rawInputs_[i] = (uint16_t)m_[i].get();
            inputs_[i] = rawInputs_[i];
            lastOutputs_[i] = 0xFFFF;
            // Seed debouncers from initial raw levels (active-LOW pin → asserted when bit==0)
            uint32_t now = millis();
            updateDebouncers_(i, rawInputs_[i], now);
            applyDebouncedToInputs_(i);
            codeOneShot_[i].update(debouncedAsserted_(i, 12));
        }

        // Restore 800 kHz Fast-Mode Plus clock overridden by I2Cexpander::init (which defaults to 400 kHz)
        Wire.setClock(800000UL);
    }

    // Single-pass bulk read + time debounce; CODE OneShot sees debounced press/release only.
    void syncInputs() override {
        uint32_t now = millis();
        for (uint8_t i = 0; i < 14; ++i) {
            rawInputs_[i] = (uint16_t)m_[i].get();
            updateDebouncers_(i, rawInputs_[i], now);
            applyDebouncedToInputs_(i);
            // OneShot: debounced PUSHED / RELEASED on bit 12
            codeOneShot_[i].update(debouncedAsserted_(i, 12));
        }
    }

    FieldUnit::OneShot& codeOneShot(uint8_t col) override {
        return codeOneShot_[colToDev(col)];
    }

    // Debounced physical pin reads: true = asserted/closed
    bool read(uint8_t col, FieldUnit::PanelInput fn) override {
        uint8_t dev = colToDev(col);
        switch (fn) {
            case FieldUnit::PanelInput::SW_NORMAL:          return debouncedAsserted_(dev, 7);
            case FieldUnit::PanelInput::SW_REVERSE:         return debouncedAsserted_(dev, 6);
            case FieldUnit::PanelInput::SIG_LEFT:           return debouncedAsserted_(dev, 9);
            case FieldUnit::PanelInput::SIG_STOP:           return debouncedAsserted_(dev, 10);
            case FieldUnit::PanelInput::SIG_RIGHT:          return debouncedAsserted_(dev, 11);
            case FieldUnit::PanelInput::MAINTAINER_CALL_SW: return debouncedAsserted_(dev, 2);
            case FieldUnit::PanelInput::CODE_BUTTON:        return debouncedAsserted_(dev, 12);
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
    // Input bit indices that are debounced (matches kPanelInputMask)
    static constexpr uint8_t kDebounceBits[] = {2, 6, 7, 9, 10, 11, 12};
    static constexpr uint8_t kDebounceBitCount = sizeof(kDebounceBits) / sizeof(kDebounceBits[0]);

    static uint8_t debounceSlot_(uint8_t bit) {
        for (uint8_t s = 0; s < kDebounceBitCount; ++s) {
            if (kDebounceBits[s] == bit) return s;
        }
        return 0;
    }

    bool debouncedAsserted_(uint8_t dev, uint8_t bit) const {
        return debounce_[dev][debounceSlot_(bit)].stable;
    }

    void updateDebouncers_(uint8_t dev, uint16_t raw, uint32_t nowMs) {
        for (uint8_t s = 0; s < kDebounceBitCount; ++s) {
            uint8_t bit = kDebounceBits[s];
            bool asserted = (bitRead(raw, bit) == 0); // active-LOW
            debounce_[dev][s].update(asserted, nowMs, kInputSettleMs);
        }
    }

    // Rebuild inputs_ word from debounced levels (1 = released/high, 0 = pressed/low)
    void applyDebouncedToInputs_(uint8_t dev) {
        uint16_t v = rawInputs_[dev];
        for (uint8_t s = 0; s < kDebounceBitCount; ++s) {
            uint8_t bit = kDebounceBits[s];
            bitWrite(v, bit, debounce_[dev][s].stable ? 0 : 1);
        }
        inputs_[dev] = v;
    }

    I2Cexpander m_[14];
    uint16_t rawInputs_[14];
    uint16_t inputs_[14];
    uint16_t outputs_[14];
    uint16_t lastOutputs_[14];
    DebouncedInput debounce_[14][kDebounceBitCount];
    FieldUnit::OneShot codeOneShot_[14];
};

#endif // SPCOAST_IO_I2C_H
