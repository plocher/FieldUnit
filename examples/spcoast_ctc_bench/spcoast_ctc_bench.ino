/*
 * spcoast_ctc_bench - SPCoast cTc desk hardware bench
 *
 * Boot: brief lamp self-test.
 * Loop: opportunistic lever/code mirror (fast). OLED deferred/throttled.
 *
 * Station grouping (CODE / MC span the multi-column CP):
 *   cols 1-2, 3-4, 5-7, 8-10, 11-12, 13, 14
 * CODE down  -> MB1/2/3 on every column in that station
 * MC down    -> MCK on every column in that station
 *
 * Serial off by default — enable with BENCH_SERIAL.
 */

#include <Arduino.h>
#include <Wire.h>
#include <I2Cexpander.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// #define BENCH_SERIAL

static constexpr uint8_t NUM_COLUMNS = 14;
static constexpr uint8_t OLED_ADDR = 0x3C;

static constexpr uint16_t CTC_K_SIG_S = 0x8000;
static constexpr uint16_t CTC_K_SIG_W = 0x4000;
static constexpr uint16_t CTC_K_SIG_E = 0x2000;
static constexpr uint16_t CTC_S_CODE  = 0x1000;
static constexpr uint16_t CTC_S_SIG_W = 0x0800;
static constexpr uint16_t CTC_S_SIG_S = 0x0400;
static constexpr uint16_t CTC_S_SIG_E = 0x0200;
static constexpr uint16_t CTC_K_MC    = 0x0100;
static constexpr uint16_t CTC_S_SW_N  = 0x0080;
static constexpr uint16_t CTC_S_SW_R  = 0x0040;
static constexpr uint16_t CTC_K_MB3   = 0x0020;
static constexpr uint16_t CTC_K_MB2   = 0x0010;
static constexpr uint16_t CTC_K_MB1   = 0x0008;
static constexpr uint16_t CTC_S_MC    = 0x0004;
static constexpr uint16_t CTC_K_SW_R  = 0x0002;
static constexpr uint16_t CTC_K_SW_N  = 0x0001;

static constexpr uint16_t CTC_PANEL_MASK =
    (CTC_S_CODE | CTC_S_SIG_W | CTC_S_SIG_S | CTC_S_SIG_E |
     CTC_S_SW_N | CTC_S_SW_R | CTC_S_MC);

static constexpr uint16_t kInputSettleMs = 15;
static constexpr uint32_t kOledPeriodMs = 1000;

// Station spans matching spcoast_ctc configureDesk() (0-based inclusive)
// 1-2, 3-4, 5-7, 8-10, 11-12, 13, 14
static void stationRange(uint8_t col0, uint8_t& lo, uint8_t& hi) {
    if (col0 <= 1)       { lo = 0;  hi = 1; }
    else if (col0 <= 3)  { lo = 2;  hi = 3; }
    else if (col0 <= 6)  { lo = 4;  hi = 6; }
    else if (col0 <= 9)  { lo = 7;  hi = 9; }
    else if (col0 <= 11) { lo = 10; hi = 11; }
    else if (col0 == 12) { lo = 12; hi = 12; }
    else                 { lo = 13; hi = 13; }
}

struct DebouncedBit {
    bool stable = false;
    bool lastRaw = false;
    uint32_t lastChangeMs = 0;
    bool initialized = false;

    void update(bool raw, uint32_t nowMs) {
        if (!initialized) {
            lastRaw = raw;
            stable = raw;
            lastChangeMs = nowMs;
            initialized = true;
            return;
        }
        if (raw != lastRaw) {
            lastRaw = raw;
            lastChangeMs = nowMs;
            return;
        }
        if ((uint32_t)(nowMs - lastChangeMs) >= kInputSettleMs) {
            stable = lastRaw;
        }
    }
};

I2Cexpander m[NUM_COLUMNS];
uint16_t lastStable[NUM_COLUMNS];
uint16_t lastOutputs[NUM_COLUMNS];
bool primed[NUM_COLUMNS];

DebouncedBit dbCode[NUM_COLUMNS];
DebouncedBit dbSwN[NUM_COLUMNS];
DebouncedBit dbSwR[NUM_COLUMNS];
DebouncedBit dbSigE[NUM_COLUMNS];
DebouncedBit dbSigS[NUM_COLUMNS];
DebouncedBit dbSigW[NUM_COLUMNS];
DebouncedBit dbMc[NUM_COLUMNS];

// Per-station OR of CODE / MC (indexed by station lo column)
bool stationCodeDown[NUM_COLUMNS];
bool stationMcDown[NUM_COLUMNS];

uint32_t loopCount = 0;
uint32_t loopSpikeCount = 0;
uint32_t loopSpikeMaxMs = 0;
uint32_t lastLoopMs = 0;

Adafruit_SSD1306 oled(128, 64, &Wire);
bool oledAlive = false;
bool oledDirty = true;
char oledEvent[24] = "Waiting for levers";
uint32_t lastOledMs = 0;
uint8_t oledAnim = 0;

#ifdef BENCH_SERIAL
#define BENCH_PRINT(...) do { Serial.printf(__VA_ARGS__); } while (0)
#define BENCH_PRINTLN(s) do { Serial.println(s); } while (0)
#else
#define BENCH_PRINT(...) do {} while (0)
#define BENCH_PRINTLN(s) do {} while (0)
#endif

void oledPaint() {
    if (!oledAlive) return;
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(0, 0);
    oled.printf("Panel Bench [%c]", "|/-\\"[oledAnim & 3]);
    oled.drawFastHLine(0, 10, 128, SSD1306_WHITE);
    oled.setCursor(0, 16);
    oled.print("Mirror mode");
    oled.setCursor(0, 32);
    oled.print(oledEvent);
    oled.setCursor(0, 48);
    oled.printf("lp=%lu maxdt=%lu",
                (unsigned long)loopCount,
                (unsigned long)loopSpikeMaxMs);
    oled.display();
    oledDirty = false;
}

void noteEvent(const char* ev) {
    strncpy(oledEvent, ev, sizeof(oledEvent) - 1);
    oledEvent[sizeof(oledEvent) - 1] = '\0';
    oledDirty = true;
}

void allLamps(uint16_t word) {
    for (uint8_t i = 0; i < NUM_COLUMNS; ++i) {
        m[i].put(word);
        lastOutputs[i] = word;
    }
}

void runLampTest() {
    BENCH_PRINTLN("--- Lamp test ---");
    noteEvent("ALL ON");
    oledPaint();
    allLamps(0x0000);
    delay(300);
    noteEvent("ALL OFF");
    oledPaint();
    allLamps(0xFFFF);
    delay(150);

    const uint16_t bits[] = {
        CTC_K_SW_N, CTC_K_SW_R, CTC_K_MB1, CTC_K_MB2, CTC_K_MB3,
        CTC_K_MC, CTC_K_SIG_E, CTC_K_SIG_S, CTC_K_SIG_W
    };
    for (uint8_t col = 0; col < NUM_COLUMNS; ++col) {
        char ev[24];
        snprintf(ev, sizeof(ev), "Chase %02u", col + 1);
        noteEvent(ev);
        if ((col & 1) == 0) oledPaint();
        for (uint16_t b : bits) {
            m[col].put((uint16_t)~b);
            delay(12);
        }
        m[col].put(0xFFFF);
        lastOutputs[col] = 0xFFFF;
    }
    noteEvent("Flip levers / CODE");
    oledPaint();
    BENCH_PRINTLN("--- Mirror ---");
}

void setup() {
#ifdef BENCH_SERIAL
    Serial.begin(115200);
    uint32_t t0 = millis();
    while (!Serial && (millis() - t0) < 1500) {
    }
    Serial.println("SPCoast cTc PANEL BENCH");
#endif

    Wire.begin();
    Wire.setClock(800000UL);
#if defined(ESP32)
    Wire.setTimeOut(20);
#endif

    Wire.beginTransmission(OLED_ADDR);
    if (Wire.endTransmission() == 0) {
        oledAlive = oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR, false, false);
    }

    for (uint8_t i = 0; i < NUM_COLUMNS; ++i) {
        m[i].init(i, I2Cexpander::MAX7313, CTC_PANEL_MASK, /*debounce=*/false);
        m[i].put(0xFFFF);
        lastOutputs[i] = 0xFFFF;
        lastStable[i] = 0;
        primed[i] = false;
        stationCodeDown[i] = false;
        stationMcDown[i] = false;
    }

    runLampTest();
    lastLoopMs = millis();
    lastOledMs = lastLoopMs;
}

void loop() {
    uint32_t now = millis();
    uint32_t dt = now - lastLoopMs;
    lastLoopMs = now;
    loopCount++;
    if (dt > 10) {
        loopSpikeCount++;
        if (dt > loopSpikeMaxMs) loopSpikeMaxMs = dt;
    }

    // Pass 1: read + debounce all columns
    for (uint8_t col = 0; col < NUM_COLUMNS; ++col) {
        uint16_t raw = (uint16_t)m[col].get();
        auto rawAssert = [&](uint16_t mask) -> bool { return (raw & mask) == 0; };

        dbSwN[col].update(rawAssert(CTC_S_SW_N), now);
        dbSwR[col].update(rawAssert(CTC_S_SW_R), now);
        dbSigE[col].update(rawAssert(CTC_S_SIG_E), now);
        dbSigS[col].update(rawAssert(CTC_S_SIG_S), now);
        dbSigW[col].update(rawAssert(CTC_S_SIG_W), now);
        dbMc[col].update(rawAssert(CTC_S_MC), now);
        dbCode[col].update(rawAssert(CTC_S_CODE), now);
    }

    // Pass 2: OR CODE/MC across each station group
    for (uint8_t i = 0; i < NUM_COLUMNS; ++i) {
        stationCodeDown[i] = false;
        stationMcDown[i] = false;
    }
    static const uint8_t kGroupLos[] = {0, 2, 4, 7, 10, 12, 13};
    for (uint8_t g = 0; g < sizeof(kGroupLos); ++g) {
        uint8_t lo = 0, hi = 0;
        stationRange(kGroupLos[g], lo, hi);
        bool code = false;
        bool mc = false;
        for (uint8_t c = lo; c <= hi; ++c) {
            if (dbCode[c].stable) code = true;
            if (dbMc[c].stable) mc = true;
        }
        for (uint8_t c = lo; c <= hi; ++c) {
            stationCodeDown[c] = code;
            stationMcDown[c] = mc;
        }
    }

    // Pass 3: lamps + edge log
    for (uint8_t col = 0; col < NUM_COLUMNS; ++col) {
        uint16_t stable = 0xFFFF;
        bitWrite(stable, 7, dbSwN[col].stable ? 0 : 1);
        bitWrite(stable, 6, dbSwR[col].stable ? 0 : 1);
        bitWrite(stable, 9, dbSigE[col].stable ? 0 : 1);
        bitWrite(stable, 10, dbSigS[col].stable ? 0 : 1);
        bitWrite(stable, 11, dbSigW[col].stable ? 0 : 1);
        bitWrite(stable, 2, dbMc[col].stable ? 0 : 1);
        bitWrite(stable, 12, dbCode[col].stable ? 0 : 1);

        uint16_t oval = 0xFFFF;
        // Local lever mirrors
        if (dbSwN[col].stable)  oval &= ~CTC_K_SW_N;
        if (dbSwR[col].stable)  oval &= ~CTC_K_SW_R;
        if (dbSigE[col].stable) oval &= ~CTC_K_SIG_E;
        if (dbSigS[col].stable) oval &= ~CTC_K_SIG_S;
        if (dbSigW[col].stable) oval &= ~CTC_K_SIG_W;

        // Station-wide CODE -> model board lamps on all columns in the CP
        if (stationCodeDown[col]) {
            oval &= ~(CTC_K_MB1 | CTC_K_MB2 | CTC_K_MB3);
        }
        // Station-wide MC -> maintainer call lamps on all columns in the CP
        // (e.g. Gilroy pair shares one MC lever, two request lamps)
        if (stationMcDown[col]) {
            oval &= ~CTC_K_MC;
        }

        if (oval != lastOutputs[col]) {
            lastOutputs[col] = oval;
            m[col].put(oval);
        }

        if (!primed[col]) {
            lastStable[col] = stable;
            primed[col] = true;
            continue;
        }
        if (stable == lastStable[col]) {
            continue;
        }

        uint16_t diff = stable ^ lastStable[col];
        char ev[24] = "";

        if (diff & CTC_S_SW_N) {
            BENCH_PRINT("[COL %02u] SW %s\n", col + 1, dbSwN[col].stable ? "N" : "-");
            snprintf(ev, sizeof(ev), "C%02u SW %s", col + 1, dbSwN[col].stable ? "N" : "-");
        }
        if (diff & CTC_S_SW_R) {
            BENCH_PRINT("[COL %02u] SW %s\n", col + 1, dbSwR[col].stable ? "R" : "-");
            snprintf(ev, sizeof(ev), "C%02u SW %s", col + 1, dbSwR[col].stable ? "R" : "-");
        }
        if (diff & CTC_S_SIG_E) {
            BENCH_PRINT("[COL %02u] SIG %s\n", col + 1, dbSigE[col].stable ? "L" : "-");
            snprintf(ev, sizeof(ev), "C%02u SIG %s", col + 1, dbSigE[col].stable ? "L" : "-");
        }
        if (diff & CTC_S_SIG_S) {
            BENCH_PRINT("[COL %02u] SIG %s\n", col + 1, dbSigS[col].stable ? "C" : "-");
            snprintf(ev, sizeof(ev), "C%02u SIG %s", col + 1, dbSigS[col].stable ? "C" : "-");
        }
        if (diff & CTC_S_SIG_W) {
            BENCH_PRINT("[COL %02u] SIG %s\n", col + 1, dbSigW[col].stable ? "R" : "-");
            snprintf(ev, sizeof(ev), "C%02u SIG %s", col + 1, dbSigW[col].stable ? "R" : "-");
        }
        if (diff & CTC_S_MC) {
            BENCH_PRINT("[COL %02u] MC %s (station)\n", col + 1, dbMc[col].stable ? "ON" : "OFF");
            snprintf(ev, sizeof(ev), "C%02u MC %s", col + 1, dbMc[col].stable ? "ON" : "OFF");
        }
        if (diff & CTC_S_CODE) {
            if (dbCode[col].stable) {
                BENCH_PRINT("[COL %02u] CODE down (station MB)\n", col + 1);
                snprintf(ev, sizeof(ev), "C%02u CODE down", col + 1);
            } else {
                BENCH_PRINT("[COL %02u] CODE up\n", col + 1);
                snprintf(ev, sizeof(ev), "C%02u CODE up", col + 1);
            }
        }

        if (ev[0] != '\0') {
            noteEvent(ev);
        }
        lastStable[col] = stable;
    }

    if (oledAlive && (oledDirty || (now - lastOledMs) >= kOledPeriodMs)) {
        lastOledMs = now;
        oledAnim++;
        oledPaint();
    }
}
