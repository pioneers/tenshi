#ifndef ADC_H_
#define ADC_H_
// This file contains code for the ADC (analog to digital converter).
// The ADC is used for the current sensor.

#include <stdint.h>

#include "util_macro.h"

// Raw ADC value of pin connected to current sensor.
DECLARE_I2C_REGISTER(uint16_t, isense_adc);

// Called to configure ADC hardware on startup.
extern void init_adc();
// Called to update ADC values.
extern void read_adc();

#endif  // ADC_H_
