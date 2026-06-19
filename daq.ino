//#include <Arduino.h>
#include <ADC.h>
//#include <ADC_Module.h>

  

/* Note on pins: 42-47 are the SD slot and 48-54 are actually 
 * pads, for QSPI memory. Maybe still usable but kind of a pain
 */ 

 // data input pins
static const uint8_t ADC_PINS[18] = { 
    A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, 
    A10, A11, A12, A13, A14, A15, A16, A17
};

// !!TODO!! Shift register or something for reset
static const uint8_t RST_PINS[18] = {                                        32, 33, 34, 35, 36, 37, 42, 43, 44,
    45, 46, 47, 48, 49, 50, 51, 52, 53
};     

 // pins for trigger (read if >1) 
static const uint8_t TRG_PINS[18] = {
     0,   1,  2,  3,  4,  5,  6,  7,  8,
     9,  10, 11, 12, 28, 29, 30, 31, 32
};


///////////////


static constexpr uint8_t NUM_CH = 18;




/* Packet format:
 *  [0xAA][0x55]: delimiter for each packet         (2)
 *  sequence number, uint16                         (2)
 *  timestamp, uint32. 4294967296 us ~= 1.2 hrs     (4)
 *  channel number, uint8                           (1)
 *  analog reading, uint16                          (2)
 *  [0x00]: to make a round number (can replace)    (1)
 */ 

static constexpr uint8_t PKT_SIZE = 12; 


struct Event 
{ 
  uint32_t timestamp_us;
  uint8_t  channel;
};

// ring buffer
//      512 KB total ram
static constexpr uint16_t BUF_LEN = 2048;  // holds N-1 packets
static constexpr uint16_t MASK = BUF_LEN - 1;
 // 2048 pkt * 44 B/pkt = 90112 B = 88 KiB
static constexpr uint32_t BUF_BYTES = BUF_LEN * PKT_SIZE; 

static volatile Event buffer[BUF_LEN];
static volatile uint32_t ring_head = 0;
static volatile uint32_t ring_tail = 0;

/* Note: USB buffer is 4 KiB. This might be editable somehow, but not 
 * with Serial.addMemoryForWrite() unlike other serial ports on the teensy */


///////////////////


// TODO: replace 
//
// briefly send reset signal (high) to channel
static void reset_ch(uint8_t ch) 
{
    digitalWrite( RST_PINS[ch], HIGH );
    delayMicroseconds(2);  // depends on time for cap to drain
    digitalWrite( RST_PINS[ch], LOW );
}


//////////////////////

/* 
 * Explanation:
 *
 * attachInterrupt(pin, isr, mode) accepts an ISR (interrupt function) but this can't have 
 * any parameters (e.g. channel/pin number). Thus, a seperate ISR must be created for each 
 * channel (using template below), so that channel can be recorded. 
 *
 * However, a for loop such as that in setup() containing attachInterrupt
 * can't pass ch to trigger_isr<ch> because the functions are supposed to be created at compile-
 * time, but the loop is run-time. Instead, the loop accesses an array of pointers to already-
 * instantiated functions. 
 *
 * There were probably easier ways to do this :P
 *
 * https://docs.arduino.cc/language-reference/en/functions/external-interrupts/attachInterrupt/
 *
 */ 

template<uint8_t ch>
FASTRUN static void trigger_isr() {  
    if ((ring_head - ring_tail) >= BUF_LEN) { return; }

    uint32_t index = ring_head & MASK;
    buffer[index].timestamp_us = micros();
    buffer[index].channel = ch;
    ring_head++;
}

// Use template for each 
static void (* const isr_table[18])() = {
    trigger_isr<0>,  trigger_isr<1>,  trigger_isr<2>,  trigger_isr<3>,
    trigger_isr<4>,  trigger_isr<5>,  trigger_isr<6>,  trigger_isr<7>,
    trigger_isr<8>,  trigger_isr<9>,  trigger_isr<10>, trigger_isr<11>,
    trigger_isr<12>, trigger_isr<13>, trigger_isr<14>, trigger_isr<15>,
    trigger_isr<16>, trigger_isr<17>
};


static ADC *adc = new ADC();

void setup()  // TODO
{


    Serial.begin(0); 
    while (!Serial); // Wait for host

   
     // ADC0
    adc->adc0->setAveraging(1);  // no averaging
    adc->adc0->setResolution(12); 
    adc->adc0->setConversionSpeed( ADC_CONVERSION_SPEED::VERY_HIGH_SPEED );
    adc->adc0->setSamplingSpeed( ADC_SAMPLING_SPEED::VERY_HIGH_SPEED );
    adc->enableInterrupts(ADC_0);
    // probably 2.5V, external
    //adc->setReference( ADC_REFERENCE::REF_EXT, ADC_0 );  

     // ADC1
    adc->adc1->setAveraging(1);
    adc->adc1->setResolution(12);
    adc->adc1->setConversionSpeed( ADC_CONVERSION_SPEED::VERY_HIGH_SPEED );
    adc->adc1->setSamplingSpeed( ADC_SAMPLING_SPEED::VERY_HIGH_SPEED );
    adc->enableInterrupts(ADC_1);
    //adc->setReference( ADC_REFERENCE::REF_EXT, ADC_1 );



    for (uint8_t ch = 0; ch < NUM_CH; ch++) 
    {
        pinMode(TRIG_PINS[ch], INPUT);
        pinMode(ADC_PINS[ch], INPUT_DISABLE);
        attachInterrupt(
            digitalPinToInterrupt(TRIG_PINS[ch]),
            isr_table[ch],  // see trigger_isr() declaration
            RISING 
        );
    }
}

//////////////////////////////
static uint16_t seq = 0;

void loop()  // TODO
{

    // local copy of head and tail
    uint32_t head = ring_head; 
    uint32_t tail = ring_tail;
    if (head == tail) { return };  // buffer empty 
   /* If head > buffer size, start index from 0 again. Explaination: 
    *    buffer size should be power of 2, so that if
    *    len = 1000 then mask = 0111, &c. Thus, if 
    *    head = 1011, then (head & mask) = 0011 and so on
    */ 
    uint32_t index = tail & MASK;
    uint8_t ch = buffer[index].channel;
    uint32_t ts  = buffer[index].timestamp_us;
    ring_tail++;

    uint16_t adcval = (uint16_t)adc->analogRead(ADC_PINS[ch]);

      
    reset_ch(ch); // TODO

    // adc errors
    if (adc->adc0->fail_flag != ADC_ERROR::CLEAR) {
        Serial.print("ADC0: ");
        Serial.println(getStringADCError(adc->adc0->fail_flag));
        adc->resetError();
    }
    if (adc->adc1->fail_flag != ADC_ERROR::CLEAR) {
        Serial.print("ADC1: ");
        Serial.println(getStringADCError(adc->adc1->fail_flag));
        adc->resetError();
    } 
    
    uint8_t pkt[PKT_SIZE];
    pkt[0]  = 0xAA;
    pkt[1]  = 0x55;
    pkt[2]  = seq >> 8;
    pkt[3]  = seq & 0xFF;
    pkt[4]  = ts >> 24;
    pkt[5]  = (ts >> 16) & 0xFF;
    pkt[6]  = (ts >>  8) & 0xFF;
    pkt[7]  = ts & 0xFF;
    pkt[8]  = ch;
    pkt[9]  = adcval >> 8;
    pkt[10] = adcval & 0xFF;
    pkt[11] = 0x00;

    Serial.write(pkt, PKT_SIZE);
    seq++;
}

