#include "adc.h"

#include <avr/io.h>
#include <util/atomic.h>

DECLARE_I2C_REGISTER_C(uint16_t, isense_adc);

void init_adc() {
  // Disable digital input buffer.
  DIDR0 = _BV(ADC0D);
  // There is no point setting ADCSRA/ADMUX here.
}

// This function configures the ADC and reads the analog value from ADC0.
// This pin is connected to the current sensor chip. The value is placed into
// a global that can be accessed using get_isense_adc. There is only one ADC
// channel in use on this board.
void read_adc() {
  // Select AVcc and ADC channel 0.
  ADMUX = _BV(REFS0);
  ADCSRB = 0; // Free running mode.
  ADCSRA = _BV(ADEN) | _BV(ADIF) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0) |
      _BV(ADSC); // Enable adc, clear any interrupt, div = 128, start ADC

  while (ADCSRA & _BV(ADSC)) {} // wait for finish

  uint16_t tmp = ADCL;
  tmp |= ((uint16_t)(ADCH) << 8);

  set_isense_adc(tmp);
}
