#include "inc/driver_glue.h"

#include "inc/i2c_master.h"
#include "inc/pindef.h"
#include "inc/stm32f4xx.h"

i2c_master_module *i2c1_driver;

void i2c1_init(void) {
  // Enable appropriate GPIO banks
  RCC->AHB1ENR |= GPIO_BANK_AHB1ENR(PINDEF_I2C_SCL);
  RCC->AHB1ENR |= GPIO_BANK_AHB1ENR(PINDEF_I2C_SDA);

  // Configure the I/O
  CONFIGURE_IO(I2C_SCL);
  CONFIGURE_IO(I2C_SDA);

  // Enable I2C1 clock
  RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

  // Initialize the actual driver
  i2c1_driver = i2c_master_init_module(I2C1);

  // Enable the interrupt at priority 15 (lowest)
  // TODO(rqou): Better place to put things like 15 being lowest priority
  NVIC_SetPriority(I2C1_EV_IRQn, 15);
  NVIC_EnableIRQ(I2C1_EV_IRQn);
  NVIC_SetPriority(I2C1_ER_IRQn, 15);
  NVIC_EnableIRQ(I2C1_ER_IRQn);
}

void I2C1_EV_IRQHandler(void) {
  i2c_handle_interrupt(i2c1_driver);
}

void I2C1_ER_IRQHandler(void) {
  i2c_handle_interrupt_error(i2c1_driver);
}
