#include <_Teensy.h>
#include <ADC.h>

  
///////////////


/*
 * 18 pins for 18 channels
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

// pins for trigger (fire if threshold reached) -- not used atm
// static const uint8_t TRG_PINS[18] = {
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 
//  10, 11, 12, 13, 28, 29, 30, 31
// };

///////////////


struct Event {                                                                                                                                                              
  uint32_t timestamp_us;
  uint8_t  channel;                                                                                                                                                  
};

static constexpr uint32_t RING_LEN  = 256;

// TODO: ring buffer

/////////////////////////////

void setup()  // TODO
{
  Serial.begin(0); 


  ADC *adc = new ADC();

  for (uint8_t ch = 0; ch < 18; ch++) {
    pinMode(TRIG_PINS[ch], INPUT);
    pinMode(RST_PINS[ch],  OUTPUT);
    digitalWrite(RST_PINS[ch], LOW);
  }
}

//////////////////////////////

void loop()  // TODO
{



  /* briefly send reset signal (high) to channel ch
   *  > delay depends on minimum time for capacitor to drain */ 
  static void reset_ch(uint8_t ch) {
    digitalWrite(RST_PINS[ch], HIGH);
    delayMicroseconds(2); 
    digitalWrite(RST_PINS[ch], LOW);
  }
  
}

