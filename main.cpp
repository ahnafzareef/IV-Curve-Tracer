#include <Arduino.h>
#include "driver/dac.h"

// ---- Pins ----
#define PIN_MDISC       32          // gate: disconnect switch (Q_Mdisc)
#define PIN_MSC         33          // gate: short-circuit switch (Q_Msc)
#define DAC_VEXT        DAC_CHANNEL_1  // IO25 -> variable load base drive

#define PIN_I_ADC       34          // IO34: I_Measured (ACS724 output, halved by R3/R4 divider)
#define PIN_V_ADC       36          // IO36 (SENSOR_VP): DUT+ voltage

// ---- Sweep parameters ----
#define N_POINTS        128
#define AVG_SAMPLES     16
#define SETTLE_MS       5

// ---- ACS724LLCTR-10AB-T calibration ----
// Vcc = 5V, sensitivity = 100 mV/A, zero-current output = 2.5V
// R3/R4 (1K/1K) divider halves output to ADC: V_adc = Viout / 2
// At 0 A: V_adc = 1.25 V -> ADC count = 1.25 / 3.3 * 4095 ~ 1552
// Per amp: 50 mV / (3.3 V / 4095) ~ 62 counts
#define I_ADC_ZERO      1552
#define I_ADC_PER_AMP   62.0f

// If DUT+ is scaled down through a voltage divider before reaching IO36,
// set V_SCALE to the inverse of the divider ratio (e.g. 2.0 for 1:1 divider).
#define V_SCALE         1.0f

struct IVPoint { float V, I; };
static IVPoint curve[N_POINTS];

static float adcAvg(int pin) {
    uint32_t s = 0;
    for (int i = 0; i < AVG_SAMPLES; i++) s += analogRead(pin);
    return (float)s / AVG_SAMPLES;
}

static float readI() {
    return (adcAvg(PIN_I_ADC) - I_ADC_ZERO) / I_ADC_PER_AMP;
}

static float readV() {
    return adcAvg(PIN_V_ADC) / 4095.0f * 3.3f * V_SCALE;
}

static void setLoad(uint8_t v) {
    dac_output_voltage(DAC_VEXT, v);
}

// Sweep Voc -> Isc -> IV curve, then stream CSV over Serial.
void sweep() {
    digitalWrite(PIN_MSC, LOW);
    digitalWrite(PIN_MDISC, LOW);
    setLoad(0);
    delay(100);

    // Connect DUT to measurement circuit
    digitalWrite(PIN_MDISC, HIGH);
    delay(50);

    // Open-circuit voltage
    float voc = readV();

    // Short-circuit current
    digitalWrite(PIN_MSC, HIGH);
    delay(20);
    float isc = readI();
    digitalWrite(PIN_MSC, LOW);
    delay(20);

    Serial.printf("VOC:%.4f\nISC:%.4f\n", voc, isc);

    // Sweep variable load from zero to full drive
    for (int i = 0; i < N_POINTS; i++) {
        uint8_t dac = (uint8_t)((float)i / (N_POINTS - 1) * 255);
        setLoad(dac);
        delay(SETTLE_MS);
        curve[i] = { readV(), readI() };
    }

    setLoad(0);
    digitalWrite(PIN_MDISC, LOW);

    Serial.println("DATA_START");
    for (int i = 0; i < N_POINTS; i++) {
        Serial.printf("%.4f,%.4f\n", curve[i].V, curve[i].I);
    }
    Serial.println("DATA_END");
}

void setup() {
    Serial.begin(115200);

    pinMode(PIN_MDISC, OUTPUT);
    pinMode(PIN_MSC, OUTPUT);
    digitalWrite(PIN_MDISC, LOW);
    digitalWrite(PIN_MSC, LOW);

    dac_output_enable(DAC_VEXT);
    setLoad(0);

    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);   // 0–3.9 V input range

    Serial.println("IV tracer ready. Send 's' to sweep.");
}

void loop() {
    if (Serial.available()) {
        char c = Serial.read();
        if (c == 's' || c == 'S') sweep();
    }
}
