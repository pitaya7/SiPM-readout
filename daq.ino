#include <_Teensy.h>
#include <ADC.h>


struct Event {                                                                                                                                                              
  uint32_t timestamp_us;
  uint8_t  channel;                                                                                                                                                  
};
  
  
// data input
static const uint8_t ADC_PINS[18] = {
  A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, 
  A10, A11, A12, A13, A14, A15, A16, A17
};

// peak reset signal
static const uint8_t RST_PINS[18] = {
  32, 33, 34, 35, 36, 37, 42, 43, 44, 
  45, 46, 47, 48, 49, 50, 51, 52, 53
};

// pins for trigger (fire if threshold reached)
// static const uint8_t TRG_PINS[18] = {
//  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 
//  10, 11, 12, 13, 28, 29, 30, 31
// };


void setup()
{



  ADC *adc = new ADC();

};


void loop()
{


}

