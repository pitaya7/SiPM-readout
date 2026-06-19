#include <Arduino.h>
#include <ADC.h>

/* 
 *  ***  No-Interrupt version  ***
 *
 * Continuously reads channels one-by-one, about 27 us per cycle.
 *  Timestamps are collected when the reading is, and thus could potentially 
 *  be off from the true event time by up to 27 us.
 *
 * It is simpler, however.
 *
 *
 * Packet format:
 *  [0xAA][0x55]: delimiter for each packet         (2)
 *  sequence number, uint16                         (2)
 *  timestamp, uint32. 4294967296 us ~= 1.2 hrs     (4)
 *  channel number, uint8                           (1)
 *  analog reading, uint16                          (2)
 *  [0x00]: to make a round number (div by 4)       (1)
 */ 


static constexpr uint8_t PKT_SIZE = 12; 
static constexpr uint8_t NUM_CH = 18;
static constexpr uint16_t THRESHOLD = 10;  // min reading to capture

// input
static const uint8_t ADC_PINS[18] = {
    A0,  A1,  A2,  A3,  A4,  A5,  A6,  A7,  A8,
    A9, A10, A11, A12, A13, A14, A15, A16, A17
};

// for resetting capacitor 
static const uint8_t RST_PINS[18] = {
     0,  1,  2,  3,  4,  5,  6,  7, 8,
     9, 10, 11, 12, 28, 29, 30, 31, 32
};


static void reset_ch(uint8_t ch) 
{
    digitalWriteFast(RST_PINS[ch], HIGH);
    delayMicroseconds(2);
    digitalWriteFast(RST_PINS[ch], LOW);
}



static ADC *adc = new ADC();
static uint16_t pkt_seq = 0;

void setup() 
{
    Serial.begin(0);
    while (!Serial);

    
    for (ADC_Module* m : {adc->adc0, adc->adc1}) {
        m->setResolution(12);
        m->setAveraging(1);
        m->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED);
        m->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED)
        m->setReference( ADC_REFERENCE::REF_3V3);  // change to external
    }

    for (uint8_t ch = 0; ch < 18; ch++) {
        pinMode(RST_PINS[ch], OUTPUT);
        pinMode(ADC_PINS[ch], INPUT_DISABLE);  // makes faster
        digitalWriteFast(RST_PINS[ch], LOW);
    }
}
void loop() 
{
    for (uint8_t ch = 0; ch < 18; ch++) 
    {
        uint16_t adcval = (uint16_t)adc->analogRead(ADC_PINS[ch]);
        if (adcval < THRESHOLD) continue;

        uint32_t ts = micros();
        reset_channel(ch);

        uint8_t pkt[12];
        pkt[0]  = 0xAA;
        pkt[1]  = 0x55;
        pkt[2]  = pkt_seq >> 8;
        pkt[3]  = pkt_seq & 0xFF;
        pkt[4]  = ts >> 24;
        pkt[5]  = (ts >> 16) & 0xFF;
        pkt[6]  = (ts >>  8) & 0xFF;
        pkt[7]  = ts & 0xFF;
        pkt[8]  = ch;
        pkt[9]  = adcval >> 8;
        pkt[10] = adcval & 0xFF;
        pkt[11] = 0x00;

        Serial.write(pkt, PKT_SIZE);
        pkt_seq++;
    }
}
