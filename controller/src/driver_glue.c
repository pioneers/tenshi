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

uart_serial_module *smartsensor_1;

static void smartsensor1_txen(int enable) {
  if (enable) {
    GPIO_BANK(PINDEF_SENSOR_CH1_TXE)->BSRRL =
      (1 << GPIO_PIN(PINDEF_SENSOR_CH1_TXE));
  } else {
    GPIO_BANK(PINDEF_SENSOR_CH1_TXE)->BSRRH =
      (1 << GPIO_PIN(PINDEF_SENSOR_CH1_TXE));
  }
}

static ssize_t dummy_length_finder(uart_serial_module *module, uint8_t byte) {
  if (module->length_finder_state == 0) {
    if (byte == 0x00) {
      module->length_finder_state = 1;
      return -1;
    }
  } else if (module->length_finder_state == 1) {
    module->length_finder_state = 0;
    return byte;
  }

  return -1;
}

void smartsensor1_init(void) {
  // Enable appropriate GPIO banks
  RCC->AHB1ENR |= GPIO_BANK_AHB1ENR(PINDEF_SENSOR_CH1_TX);
  RCC->AHB1ENR |= GPIO_BANK_AHB1ENR(PINDEF_SENSOR_CH1_RX);
  RCC->AHB1ENR |= GPIO_BANK_AHB1ENR(PINDEF_SENSOR_CH1_TXE);

  // Configure the I/O
  CONFIGURE_IO(SENSOR_CH1_TX);
  CONFIGURE_IO(SENSOR_CH1_RX);
  CONFIGURE_IO(SENSOR_CH1_TXE);

  // Enable the UART4 clock
  RCC->APB1ENR |= RCC_APB1ENR_UART4EN;

  // Enable the DMA
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

  // Initialize the actual driver
  smartsensor_1 = uart_serial_init_module(4, dummy_length_finder,
    smartsensor1_txen, 1000000);

  // Enable the interrupt at priority 15 (lowest)
  // TODO(rqou): Better place to put things like 15 being lowest priority
  NVIC_SetPriority(UART4_IRQn, 15);
  NVIC_EnableIRQ(UART4_IRQn);
  // TODO(rqou): This DMA stuff shouldn't be split here and in the driver part.
  NVIC_SetPriority(DMA1_Stream4_IRQn, 15);
  NVIC_EnableIRQ(DMA1_Stream4_IRQn);
  NVIC_SetPriority(DMA1_Stream2_IRQn, 15);
  NVIC_EnableIRQ(DMA1_Stream2_IRQn);
}

void UART4_IRQHandler(void) {
  uart_serial_handle_uart_interrupt(smartsensor_1);
}

void DMA1_Stream4_IRQHandler(void) {
  uart_serial_handle_tx_dma_interrupt(smartsensor_1);
}

void DMA1_Stream2_IRQHandler(void) {
  uart_serial_handle_rx_dma_interrupt(smartsensor_1);
}
