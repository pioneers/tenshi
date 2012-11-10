#ifndef I2C_REGISTER_H_
#define I2C_REGISTER_H_
// This file contains code for the I2C registers.

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
#define REG_PWM_MODE_LEN                1
#define REG_TARGET_SPEED_NEW          0x04
#define REG_TARGET_SPEED_NEW_LEN        4
// special
#define REG_APPLY_NEW_SPEED           0x08
#define REG_CURRENT_TARGET_SPEED      0x0C
#define REG_CURRENT_TARGET_SPEED_LEN    4
#define REG_ISENSE_ADC                0x10
#define REG_ISENSE_ADC_LEN              2
#define REG_ENCODER_COUNT             0x20
#define REG_ENCODER_COUNT_LEN           4
#define REG_REVISION                  0x60
#define REG_REVISION_LEN                21

// This value is returned when reading a register that is not defined.
#define UNUSED_REG_VAL  0xFF

#endif  // I2C_REGISTER_H_
