#ifndef ADDR_JUMPER_H_
#define ADDR_JUMPER_H_
// This file contains code for the I2C address jumpers.

#define ADDR_COMBO_1  (0x09 << 1)
#define ADDR_COMBO_2  (0x0A << 1)
#define ADDR_COMBO_3  (0x0B << 1)
#define ADDR_COMBO_4  (0x0C << 1)
#define ADDR_COMBO_5  (0x0D << 1)
#define ADDR_COMBO_6  (0x0E << 1)
#define ADDR_COMBO_7  (0x0F << 1)

// Called to read jumpers. Returns an I2C address suitable for TWAR.
unsigned char determine_addr(void);

#endif  // ADDR_JUMPER_H_
