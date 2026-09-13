/*
 * spcoast_ctc_test - Diagnostic Test Harness for SPCoast cTc Machine
 * 
 * Target: cpNode-Xiao (ESP32-C6) connected to 14x MAX7313 expanders (0x20..0x2D).
 * 
 * Stages executed:
 * 1. I2C Bus Scan: Reports all discovered I2C addresses to Serial.
 * 2. Lamp Test: Flashes all panel lamps ON for 2 seconds, then OFF.
 * 3. Lamp Chase: Sequentially walks through each column and lamp.
 * 4. Interactive Lever Mirror: Flipping any lever directly lights its lamp
 *    and prints the event to Serial (115200 baud).
 */

#include <Arduino.h>
#include <Wire.h>
#include <I2Cexpander.h>

#define USE_OLED
#ifdef USE_OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
bool oledAlive = false;

char oledLine1[24] = "SPCoast cTc Desk";
char oledLine2[24] = "Ready.";
char oledLine3[24] = "";
char oledLine4[24] = "";

void updateOled() {
    if (!oledAlive) return;
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);

    // Header
    oled.setCursor(0, 0);
    oled.print(oledLine1);
    oled.drawFastHLine(0, 10, 128, SSD1306_WHITE);

    // Line 2 (Event / Status)
    oled.setCursor(0, 16);
    oled.print(oledLine2);

    // Line 3 (Secondary Event)
    oled.setCursor(0, 32);
    oled.print(oledLine3);

    // Line 4 (Footer / Health)
    oled.setCursor(0, 50);
    oled.print(oledLine4);

    oled.display();
}
#endif

#define NUM_COLUMNS 14

// The 14 MAX7313 expanders at I2C chip addresses 0..13 (0x20..0x2D)
I2Cexpander m[NUM_COLUMNS];

// Bit masks from hardware design (active-LOW logic on MAX7313)
#define CTC_K_SIG_S      (uint16_t)0x8000 // Output bit 15: Stop jewel
#define CTC_K_SIG_W      (uint16_t)0x4000 // Output bit 14: West/Right jewel
#define CTC_K_SIG_E      (uint16_t)0x2000 // Output bit 13: East/Left jewel
#define CTC_S_CODE       (uint16_t)0x1000 // Input bit 12: Code button

#define CTC_S_SIG_W      (uint16_t)0x0800 // Input bit 11: Signal West/Right
#define CTC_S_SIG_S      (uint16_t)0x0400 // Input bit 10: Signal Center/Stop
#define CTC_S_SIG_E      (uint16_t)0x0200 // Input bit 9: Signal East/Left
#define CTC_K_MC         (uint16_t)0x0100 // Output bit 8: Maintainer Call lamp

#define CTC_S_SW_N       (uint16_t)0x0080 // Input bit 7: Switch Normal
#define CTC_S_SW_R       (uint16_t)0x0040 // Input bit 6: Switch Reverse
#define CTC_K_MB3        (uint16_t)0x0020 // Output bit 5: Model Board lamp 3
#define CTC_K_MB2        (uint16_t)0x0010 // Output bit 4: Model Board lamp 2

#define CTC_K_MB1        (uint16_t)0x0008 // Output bit 3: Model Board lamp 1
#define CTC_S_MC         (uint16_t)0x0004 // Input bit 2: Maintainer Call switch
#define CTC_K_SW_R       (uint16_t)0x0002 // Output bit 1: Switch Reverse lamp
#define CTC_K_SW_N       (uint16_t)0x0001 // Output bit 0: Switch Normal lamp

// Input pin direction mask: 1 = input, 0 = output (active-LOW lamps and inputs)
#define CTC_PANEL_MASK ((uint16_t)(CTC_S_CODE | CTC_S_SIG_W | CTC_S_SIG_S | CTC_S_SIG_E | CTC_S_SW_N | CTC_S_SW_R | CTC_S_MC))

void scanI2CBus() {
    Serial.println("\n--- Scanning I2C Bus ---");
    byte count = 0;
    byte maxCount = 0;
    for (byte addr = 1; addr < 127; ++addr) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("  Found I2C device at 0x%02X", addr);
            if (addr >= 0x20 && addr <= 0x2D) {
                Serial.printf(" (Column %d / MAX7313 #%d)", addr - 0x20 + 1, addr - 0x20);
                maxCount++;
            }
            Serial.println();
            count++;
        }
    }
    Serial.printf("--- Scan Complete: %d device(s) found ---\n\n", count);

#ifdef USE_OLED
    if (oledAlive) {
        snprintf(oledLine2, sizeof(oledLine2), "I2C: %d MAX7313s", maxCount);
        snprintf(oledLine4, sizeof(oledLine4), "Total I2C Devs: %d", count);
        updateOled();
    }
#endif
}

// Active-LOW helper: 0 = Lamp ON, 1 = Lamp OFF
void allLampsOn() {
    for (int col = 0; col < NUM_COLUMNS; ++col) {
        m[col].put((uint16_t)0x0000); // Drive all outputs LOW (ON)
    }
}

void allLampsOff() {
    for (int col = 0; col < NUM_COLUMNS; ++col) {
        m[col].put((uint16_t)0xFFFF); // Drive all outputs HIGH (OFF)
    }
}

void runLampChase() {
    Serial.println("--- Starting Column Lamp Chase ---");
#ifdef USE_OLED
    snprintf(oledLine2, sizeof(oledLine2), "Lamp Chase 1..14");
    updateOled();
#endif
    allLampsOff();
    delay(500);

    uint16_t testBits[] = {
        CTC_K_SW_N, CTC_K_SW_R,
        CTC_K_MB1, CTC_K_MB2, CTC_K_MB3,
        CTC_K_MC,
        CTC_K_SIG_E, CTC_K_SIG_S, CTC_K_SIG_W
    };

    for (int col = 0; col < NUM_COLUMNS; ++col) {
        Serial.printf("  Chase Column %d...\n", col + 1);
#ifdef USE_OLED
        snprintf(oledLine3, sizeof(oledLine3), "Chasing Col %02d", col + 1);
        updateOled();
#endif
        for (uint16_t bit : testBits) {
            m[col].put((uint16_t)~bit); // Light only this lamp (active-LOW)
            delay(80);
        }
        m[col].put((uint16_t)0xFFFF); // Off
    }
    Serial.println("--- Chase Complete ---\n");
}

uint16_t lastInputs[NUM_COLUMNS];

void setup() {
    Serial.begin(115200);
    // On USB CDC (Xiao), wait up to 3s for Serial Monitor to connect
    uint32_t t0 = millis();
    while (!Serial && millis() - t0 < 3000);

    Serial.println("====================================================");
    Serial.println("   SPCOAST CTC PHYSICAL DESK BENCH TEST             ");
    Serial.println("====================================================");

    Wire.begin();

#ifdef USE_OLED
    oledAlive = oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    if (oledAlive) {
        oled.clearDisplay();
        oled.dim(true);
        snprintf(oledLine2, sizeof(oledLine2), "Booting I2C...");
        updateOled();
    }
#endif

    // 1. Scan bus
    scanI2CBus();

    // 2. Initialize 14x MAX7313 expanders
    for (int i = 0; i < NUM_COLUMNS; ++i) {
        m[i].init(i, I2Cexpander::MAX7313, CTC_PANEL_MASK);
        lastInputs[i] = 0;
    }

    // 3. Global Lamp Flash Test
    Serial.println("Testing ALL lamps: ON for 2 seconds...");
#ifdef USE_OLED
    snprintf(oledLine2, sizeof(oledLine2), "ALL LAMPS: ON");
    updateOled();
#endif
    allLampsOn();
    delay(2000);
    allLampsOff();
    Serial.println("All lamps OFF.\n");
#ifdef USE_OLED
    snprintf(oledLine2, sizeof(oledLine2), "ALL LAMPS: OFF");
    updateOled();
#endif
    delay(500);

    // 4. Lamp Chase across columns
    runLampChase();

    Serial.println("====================================================");
    Serial.println("   ENTERED INTERACTIVE BENCH MIRROR MODE            ");
    Serial.println("   Flip levers to see lamps respond & events log    ");
    Serial.println("====================================================\n");

#ifdef USE_OLED
    snprintf(oledLine2, sizeof(oledLine2), "Interactive Mirror");
    snprintf(oledLine3, sizeof(oledLine3), "Ready for levers");
    snprintf(oledLine4, sizeof(oledLine4), "I2C: 14 Expanders OK");
    updateOled();
#endif
}

void loop() {
    for (int col = 0; col < NUM_COLUMNS; ++col) {
        uint16_t ival = m[col].get(); // Read active-LOW inputs
        uint16_t oval = 0xFFFF;        // Start with all lamps OFF (0xFFFF)

        // Active-LOW inputs: (ival & CTC_S_...) == 0 means asserted
        bool swN  = (ival & CTC_S_SW_N) == 0;
        bool swR  = (ival & CTC_S_SW_R) == 0;
        bool sigE = (ival & CTC_S_SIG_E) == 0;
        bool sigS = (ival & CTC_S_SIG_S) == 0;
        bool sigW = (ival & CTC_S_SIG_W) == 0;
        bool mc   = (ival & CTC_S_MC) == 0;
        bool code = (ival & CTC_S_CODE) == 0;

        // Mirror Levers directly to Lamps (active-LOW: clear bit to light lamp)
        if (swN)  oval &= ~CTC_K_SW_N;
        if (swR)  oval &= ~CTC_K_SW_R;
        if (sigE) oval &= ~CTC_K_SIG_E;
        if (sigS) oval &= ~CTC_K_SIG_S;
        if (sigW) oval &= ~CTC_K_SIG_W;
        if (mc)   oval &= ~CTC_K_MC;

        // Pressing Code button momentarily illuminates model board lamps 1, 2, 3
        if (code) {
            oval &= ~(CTC_K_MB1 | CTC_K_MB2 | CTC_K_MB3);
        }

        m[col].put(oval);

        // Log state changes to Serial and OLED
        if (ival != lastInputs[col]) {
            uint16_t diff = ival ^ lastInputs[col];
            char eventBuf[32] = "";

            if (diff & CTC_S_SW_N) {
                Serial.printf("[COL %02d] Switch -> %s\n", col + 1, swN ? "NORMAL" : "CENTER/OFF");
                snprintf(eventBuf, sizeof(eventBuf), "Col %02d: SW %s", col + 1, swN ? "NORM" : "OFF");
            }
            if (diff & CTC_S_SW_R) {
                Serial.printf("[COL %02d] Switch -> %s\n", col + 1, swR ? "REVERSE" : "CENTER/OFF");
                snprintf(eventBuf, sizeof(eventBuf), "Col %02d: SW %s", col + 1, swR ? "REV" : "OFF");
            }
            if (diff & CTC_S_SIG_E) {
                Serial.printf("[COL %02d] Signal -> %s\n", col + 1, sigE ? "LEFT (EAST)" : "OFF");
                snprintf(eventBuf, sizeof(eventBuf), "Col %02d: SIG %s", col + 1, sigE ? "LEFT" : "OFF");
            }
            if (diff & CTC_S_SIG_S) {
                Serial.printf("[COL %02d] Signal -> %s\n", col + 1, sigS ? "STOP" : "OFF");
                snprintf(eventBuf, sizeof(eventBuf), "Col %02d: SIG %s", col + 1, sigS ? "STOP" : "OFF");
            }
            if (diff & CTC_S_SIG_W) {
                Serial.printf("[COL %02d] Signal -> %s\n", col + 1, sigW ? "RIGHT (WEST)" : "OFF");
                snprintf(eventBuf, sizeof(eventBuf), "Col %02d: SIG %s", col + 1, sigW ? "RIGHT" : "OFF");
            }
            if (diff & CTC_S_MC) {
                Serial.printf("[COL %02d] Maintainer Call -> %s\n", col + 1, mc ? "ACTIVE" : "OFF");
                snprintf(eventBuf, sizeof(eventBuf), "Col %02d: MC %s", col + 1, mc ? "ON" : "OFF");
            }
            if (diff & CTC_S_CODE && code) {
                Serial.printf("[COL %02d] *** CODE BUTTON PRESSED ***\n", col + 1);
                snprintf(eventBuf, sizeof(eventBuf), "Col %02d: *** CODE ***", col + 1);
            }

#ifdef USE_OLED
            if (eventBuf[0] != '\0') {
                strncpy(oledLine3, oledLine2, sizeof(oledLine3) - 1);
                strncpy(oledLine2, eventBuf, sizeof(oledLine2) - 1);
                updateOled();
            }
#endif
            lastInputs[col] = ival;
        }
    }
}
