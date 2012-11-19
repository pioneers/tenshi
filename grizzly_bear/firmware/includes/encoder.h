#ifndef ENCODER_H_
#define ENCODER_H_
// This file contains code for the wheel encoders.

#include <stdint.h>

#include "util_macro.h"

// Number of counts the encoder has rotated.
DECLARE_I2C_REGISTER(int32_t, encoder_count);

// Called to configure encoder hardware on startup.
extern void init_encoder();

#endif  // ENCODER_H_
