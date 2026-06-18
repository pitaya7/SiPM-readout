#include <Arduino.h>
#include <ADC.h>
#include <ADC_util.h>

  

static constexpr uint8_t NUM_CH = 18;

// [0xAA][0x55] will appear at the beginning of each dataset
static constexpr uint8_t SYNCA = 0xAA;
static constexpr uint8_t SYNCB = 0x55;


static constexpr uint16_t PKT_SIZE = 
/*
 * all available relavant pins listed below
 */ 

// data input pins
static const uint8_t ADC_PINS[18] = { 
    A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, 
    A10, A11, A12, A13, A14, A15, A16, A17
};

// peak reset signal pins
static const uint8_t RST_PINS[18] = { 
    32, 33, 34, 35, 36, 37, 42, 43, 44, 
    45, 46, 47, 48, 49, 50, 51, 52, 53
};

// pins for trigger (read if >1) -- unused
// static const uint8_t TRG_PINS[18] = {
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 
//  10, 11, 12, 13, 28, 29, 30, 31
// };

///////////////


//struct Event {                                                                        //                                                                                      
//  uint32_t timestamp_us;
//  uint8_t  channel;                                                                   //                                                                               
//};

// ring buffer
static constexpr uint16_t BUF_LEN = 2048;  // N-1 packets held
static constexpr uint16_t MASK = BUF_LEN - 1;

static constexpr uint32_t BUF_BYTES = BUF_LEN * PKT_SIZE;   

static volatile uint32_t ring_head = 0;
static volatile uint32_t ring_tail = 0;


/////////////////////////////


// briefly send reset signal (high) to channel
static void reset_ch(uint8_t ch) {
    digitalWrite( RST_PINS[ch], HIGH );
    delayMicroseconds(2);  // depends on time for cap to drain
    digitalWrite( RST_PINS[ch], LOW );
}


void setup()  // TODO
{
    ADC *adc = new ADC();

    Serial.begin(0); 
    while (!Serial); // Wait for host

    for (int i : ADC_PINS) {
        pinMode(ADC_PINS[i], INPUT_DISABLE);
    }
    // ADC0

    adc->adc0->setAveraging(1);  // higher val would average values
    adc->adc0->setResolution(  ) 
    adc->adc0->setConversionSpeed( ADC_CONVERSION_SPEED::VERY_HIGH_SPEED );
    adc->adc0->setSamplingSpeed( ADC_SAMPLING_SPEED::VERY_HIGH_SPEED );
    
    // ADC1
    adc->adc1->setAveraging(1);
    adc->adc1->setResolution(  )
    adc->adc1->setConversionSpeed( ADC_CONVERSION_SPEED::VERY_HIGH_SPEED );
    adc->adc1->setSamplingSpeed( ADC_SAMPLING_SPEED::VERY_HIGH_SPEED );




    for (uint8_t ch = 0; ch < 18; ch++) {
    //  pinMode(TRIG_PINS[ch], INPUT);
        pinMode(RST_PINS[ch],  OUTPUT);
        digitalWrite(RST_PINS[ch], LOW);
    }
}

//////////////////////////////

void loop()  // TODO
{

    // local copy of head and tail
    uint32_t head = ring_head; 
    uint32_t tail = ring_tail;

    if (head == tail) {  // buffer empty
        return;
    }
      

    // adc errors
    if (adc->adc0->fail_flag != ADC_ERROR::CLEAR) {
        Serial.print("ADC0: ");
        Serial.println(getStringADCError(adc->adc0->fail_flag));
    }
    if (adc->adc1->fail_flag != ADC_ERROR::CLEAR) {
        Serial.print("ADC1: ");
        Serial.println(getStringADCError(adc->adc1->fail_flag));
    }
  
    reset_ch(ch);
}

