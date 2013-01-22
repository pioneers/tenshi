#ifndef I2C_REGISTER_H_
#define I2C_REGISTER_H_
// This file contains code for the I2C registers.

#include <stdint.h>

// Gives the value for a particular I2C register.
// Register is passed as an argument and this function returns a byte.
// WARNING: This function is called from an ISR. It must complete reasonably
// quickly.
extern unsigned char provide_i2c_reg(unsigned char reg);
// Update a particular I2C register.
// Register number and new byte are passed as arguments.
// WARNING: This function is called from an ISR. It must complete reasonably
// quickly.
extern void set_i2c_reg(unsigned char reg, unsigned char val);

// special
#define REG_TYPE                      0x00
// This 'G' identifies to the master that this device is a Grizzly Bear
// controller. It will allow for sanity checking and/or autodetection of the
// type of motor controller. The previous Polar Bear did not define this
// register, so it would probably return 00.
#define REG_TYPE_GRIZZLY_ID             'G'
#define REG_PWM_MODE                  0x01
#define REG_PWM_MODE_TYPE             uint8_t
#define REG_TARGET_SPEED_NEW          0x04
#define REG_TARGET_SPEED_NEW_TYPE     uint32_t
// special
#define REG_APPLY_NEW_SPEED           0x08
#define REG_CURRENT_PWM_MODE          0x09
#define REG_CURRENT_PWM_MODE_TYPE     uint8_t
#define REG_CURRENT_TARGET_SPEED      0x0C
#define REG_CURRENT_TARGET_SPEED_TYPE uint32_t
#define REG_ISENSE_ADC                0x10
#define REG_ISENSE_ADC_TYPE           uint16_t
#define REG_ENCODER_COUNT             0x20
#define REG_ENCODER_COUNT_TYPE        int32_t
#define REG_REVISION                  0x60
#define REG_REVISION_LEN                21
#define REG_ILIMIT_ADC_THRESH         0x82
#define REG_ILIMIT_ADC_THRESH_TYPE    uint16_t
#define REG_ILIMIT_CLAMP_PWM          0x84
#define REG_ILIMIT_CLAMP_PWM_TYPE     int16_t
#define REG_ILIMIT_RETRY_TIME         0x86
#define REG_ILIMIT_RETRY_TIME_TYPE    uint16_t
#define REG_ILIMIT_RATIO_NUM          0x88
#define REG_ILIMIT_RATIO_NUM_TYPE     uint8_t
#define REG_ILIMIT_RATIO_DENOM        0x89
#define REG_ILIMIT_RATIO_DENOM_TYPE   uint8_t
#define REG_ILIMIT_RATIO_MAX_USE      0x8A
#define REG_ILIMIT_RATIO_MAX_USE_TYPE uint16_t
#define REG_ACCEL_LIMIT               0x90
#define REG_ACCEL_LIMIT_TYPE          int16_t
#define REG_UPTIME                    0x94
#define REG_UPTIME_TYPE               uint32_t

// This value is returned when reading a register that is not defined.
#define UNUSED_REG_VAL  0xFF

#endif  // I2C_REGISTER_H_
