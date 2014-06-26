#include "inc/driver_glue.h"

#include "inc/i2c_master.h"
#include "inc/pindef.h"
#include "inc/stm32f4xx.h"
#include "inc/xbee_framing.h"

////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////

#define LENGTH_FINDER_DEFAULT ss_length_finder

// This length finder expects packets with format {0, entire length, ...}
static ssize_t dummy_length_finder(uart_serial_module *module, uint8_t byte) {
  if (byte == 0x00) {
    module->length_finder_state = 1;
    return -2;  // -2 means that this is the first byte of the packet.
  }
  if (module->length_finder_state == 1) {
    module->length_finder_state = 0;
    if (byte <= 2)return -1;  // -1 means nothing is known about the length.
    return byte-2;  // The positive number of bytes REMAINING.
  }

  return 0;
}

// This length finder expects packets with format {0, slllllll, ...}
// Uses the low 7 bits of the second byte for the length.
static ssize_t ss_length_finder(uart_serial_module *module,
  uint8_t byte) {
  if (byte == 0x00) {
    module->length_finder_state = 1;
    return -2;  // -2 means that this is the first byte of the packet.
  }
  if (module->length_finder_state == 1) {
    if (byte & 0x80) {  // The type byte of a maintenance packet
      module->length_finder_state = 2;
      return -1;  // -1 means nothing is known about the length.
    } else {  // The length byte of an active packet
      module->length_finder_state = 0;
      if ((byte & 0x7F) <= 2) return 0;
      return (byte & 0x7F)-2;  // The positive number of bytes REMAINING.
    }
  }
  if (module->length_finder_state == 2) {
    // The length byte of a maintenance packet
    module->length_finder_state = 3;
    if (byte <= 3) return 0;
    return byte-3;  // The positive number of bytes REMAINING.
  }

  return 0;
}

// This length finder expects packets with format {0, type, length, ...}
// Only used when testing loopback.
// Doen't work yet.
static ssize_t ss_maintenance_length_finder(uart_serial_module *module,
  uint8_t byte) {
  if (byte == 0x00) {
    module->length_finder_state = 1;
    return -2;  // -2 means that this is the first byte of the packet.
  }
  if (module->length_finder_state == 1) {
    return -1;
  }
  if (module->length_finder_state == 2) {
    module->length_finder_state = 0;
    if (byte & 0x7F <= 2)
      return -1;  // -1 means nothing is known about the length.
    return (byte & 0x7F)-3;  // The positive number of bytes REMAINING.
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

uart_serial_module *ssBusses[4];

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
  ssBusses[0] =
  smartsensor_1 = uart_serial_init_module(4, LENGTH_FINDER_DEFAULT,
    smartsensor1_txen, SMART_SENSOR_BAUD);

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

////////////////////////////////////////////////////////////////////////////////

uart_serial_module *smartsensor_2;

static void smartsensor2_txen(int enable) {
  if (enable) {
    GPIO_BANK(PINDEF_SENSOR_CH2_TXE)->BSRRL =
      (1 << GPIO_PIN(PINDEF_SENSOR_CH2_TXE));
  } else {
    GPIO_BANK(PINDEF_SENSOR_CH2_TXE)->BSRRH =
      (1 << GPIO_PIN(PINDEF_SENSOR_CH2_TXE));
  }
}

void smartsensor2_init(void) {
  // Enable appropriate GPIO banks
  RCC->AHB1ENR |= GPIO_BANK_AHB1ENR(PINDEF_SENSOR_CH2_TX);
  RCC->AHB1ENR |= GPIO_BANK_AHB1ENR(PINDEF_SENSOR_CH2_RX);
  RCC->AHB1ENR |= GPIO_BANK_AHB1ENR(PINDEF_SENSOR_CH2_TXE);

  // Configure the I/O
  CONFIGURE_IO(SENSOR_CH2_TX);
  CONFIGURE_IO(SENSOR_CH2_RX);
  CONFIGURE_IO(SENSOR_CH2_TXE);

  // Enable the UART2 clock
  RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

  // Enable the DMA
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

  // Initialize the actual driver
  ssBusses[1] =
  smartsensor_2 = uart_serial_init_module(2, LENGTH_FINDER_DEFAULT,
    smartsensor2_txen, SMART_SENSOR_BAUD);

  // Enable the interrupt at priority 15 (lowest)
  // TODO(rqou): Better place to put things like 15 being lowest priority
  NVIC_SetPriority(USART2_IRQn, 15);
  NVIC_EnableIRQ(USART2_IRQn);
  // TODO(rqou): This DMA stuff shouldn't be split here and in the driver part.
  NVIC_SetPriority(DMA1_Stream6_IRQn, 15);
  NVIC_EnableIRQ(DMA1_Stream6_IRQn);
  NVIC_SetPriority(DMA1_Stream5_IRQn, 15);
  NVIC_EnableIRQ(DMA1_Stream5_IRQn);
}

void USART2_IRQHandler(void) {
  uart_serial_handle_uart_interrupt(smartsensor_2);
}

void DMA1_Stream6_IRQHandler(void) {
  uart_serial_handle_tx_dma_interrupt(smartsensor_2);
}

void DMA1_Stream5_IRQHandler(void) {
  uart_serial_handle_rx_dma_interrupt(smartsensor_2);
}

////////////////////////////////////////////////////////////////////////////////

uart_serial_module *smartsensor_3;

static void smartsensor3_txen(int enable) {
  if (enable) {
    GPIO_BANK(PINDEF_SENSOR_CH3_TXE)->BSRRL =
      (1 << GPIO_PIN(PINDEF_SENSOR_CH3_TXE));
  } else {
    GPIO_BANK(PINDEF_SENSOR_CH3_TXE)->BSRRH =
      (1 << GPIO_PIN(PINDEF_SENSOR_CH3_TXE));
  }
}

void smartsensor3_init(void) {
  // Enable appropriate GPIO banks
  RCC->AHB1ENR |= GPIO_BANK_AHB1ENR(PINDEF_SENSOR_CH3_TX);
  RCC->AHB1ENR |= GPIO_BANK_AHB1ENR(PINDEF_SENSOR_CH3_RX);
  RCC->AHB1ENR |= GPIO_BANK_AHB1ENR(PINDEF_SENSOR_CH3_TXE);

  // Configure the I/O
  CONFIGURE_IO(SENSOR_CH3_TX);
  CONFIGURE_IO(SENSOR_CH3_RX);
  CONFIGURE_IO(SENSOR_CH3_TXE);

  // Enable the UART1 clock
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

  // Enable the DMA
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;

  // Initialize the actual driver
  ssBusses[2] =
  smartsensor_3 = uart_serial_init_module(1, LENGTH_FINDER_DEFAULT,
    smartsensor3_txen, SMART_SENSOR_BAUD);

  // Enable the interrupt at priority 15 (lowest)
  // TODO(rqou): Better place to put things like 15 being lowest priority
  NVIC_SetPriority(USART1_IRQn, 15);
  NVIC_EnableIRQ(USART1_IRQn);
  // TODO(rqou): This DMA stuff shouldn't be split here and in the driver part.
  NVIC_SetPriority(DMA2_Stream7_IRQn, 15);
  NVIC_EnableIRQ(DMA2_Stream7_IRQn);
  NVIC_SetPriority(DMA2_Stream5_IRQn, 15);
  NVIC_EnableIRQ(DMA2_Stream5_IRQn);
}

void USART1_IRQHandler(void) {
  uart_serial_handle_uart_interrupt(smartsensor_3);
}

void DMA2_Stream7_IRQHandler(void) {
  uart_serial_handle_tx_dma_interrupt(smartsensor_3);
}

void DMA2_Stream5_IRQHandler(void) {
  uart_serial_handle_rx_dma_interrupt(smartsensor_3);
}

////////////////////////////////////////////////////////////////////////////////

uart_serial_module *smartsensor_4;

static void smartsensor4_txen(int enable) {
  if (enable) {
    GPIO_BANK(PINDEF_SENSOR_CH4_TXE)->BSRRL =
      (1 << GPIO_PIN(PINDEF_SENSOR_CH4_TXE));
  } else {
    GPIO_BANK(PINDEF_SENSOR_CH4_TXE)->BSRRH =
      (1 << GPIO_PIN(PINDEF_SENSOR_CH4_TXE));
  }
}

void smartsensor4_init(void) {
  // Enable appropriate GPIO banks
  RCC->AHB1ENR |= GPIO_BANK_AHB1ENR(PINDEF_SENSOR_CH4_TX);
  RCC->AHB1ENR |= GPIO_BANK_AHB1ENR(PINDEF_SENSOR_CH4_RX);
  RCC->AHB1ENR |= GPIO_BANK_AHB1ENR(PINDEF_SENSOR_CH4_TXE);

  // Configure the I/O
  CONFIGURE_IO(SENSOR_CH4_TX);
  CONFIGURE_IO(SENSOR_CH4_RX);
  CONFIGURE_IO(SENSOR_CH4_TXE);

  // Enable the USART6 clock
  RCC->APB2ENR |= RCC_APB2ENR_USART6EN;

  // Enable the DMA
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;

  // Initialize the actual driver
  ssBusses[3] =
  smartsensor_4 = uart_serial_init_module(6, LENGTH_FINDER_DEFAULT,
    smartsensor4_txen, SMART_SENSOR_BAUD);

  // Enable the interrupt at priority 15 (lowest)
  // TODO(rqou): Better place to put things like 15 being lowest priority
  NVIC_SetPriority(USART6_IRQn, 15);
  NVIC_EnableIRQ(USART6_IRQn);
  // TODO(rqou): This DMA stuff shouldn't be split here and in the driver part.
  NVIC_SetPriority(DMA2_Stream6_IRQn, 15);
  NVIC_EnableIRQ(DMA2_Stream6_IRQn);
  NVIC_SetPriority(DMA2_Stream1_IRQn, 15);
  NVIC_EnableIRQ(DMA2_Stream1_IRQn);
}

void USART6_IRQHandler(void) {
  uart_serial_handle_uart_interrupt(smartsensor_4);
}

void DMA2_Stream6_IRQHandler(void) {
  uart_serial_handle_tx_dma_interrupt(smartsensor_4);
}

void DMA2_Stream1_IRQHandler(void) {
  uart_serial_handle_rx_dma_interrupt(smartsensor_4);
}

////////////////////////////////////////////////////////////////////////////////

uart_serial_module *radio_driver;

static void radio_txen(int enable) {
  // We aren't currently using SPI or anything, so we don't need to do anything
  // here.
  (void) enable;
}

void radio_driver_init(void) {
  // Enable appropriate GPIO banks
  RCC->AHB1ENR |= GPIO_BANK_AHB1ENR(PINDEF_RADIO_TX);
  RCC->AHB1ENR |= GPIO_BANK_AHB1ENR(PINDEF_RADIO_RX);

  // Configure the I/O
  CONFIGURE_IO(RADIO_TX);
  CONFIGURE_IO(RADIO_RX);

  // Enable the USART3 clock
  RCC->APB1ENR |= RCC_APB1ENR_USART3EN;

  // Enable the DMA
  RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

  // Initialize the actual driver
  radio_driver = uart_serial_init_module(3, xbee_length_finder,
    radio_txen, 57600);

  // Enable the interrupt at priority 15 (lowest)
  // TODO(rqou): Better place to put things like 15 being lowest priority
  NVIC_SetPriority(USART3_IRQn, 15);
  NVIC_EnableIRQ(USART3_IRQn);
  // TODO(rqou): This DMA stuff shouldn't be split here and in the driver part.
  NVIC_SetPriority(DMA1_Stream3_IRQn, 15);
  NVIC_EnableIRQ(DMA1_Stream3_IRQn);
  NVIC_SetPriority(DMA1_Stream1_IRQn, 15);
  NVIC_EnableIRQ(DMA1_Stream1_IRQn);
}

void USART3_IRQHandler(void) {
  uart_serial_handle_uart_interrupt(radio_driver);
}

void DMA1_Stream3_IRQHandler(void) {
  uart_serial_handle_tx_dma_interrupt(radio_driver);
}

void DMA1_Stream1_IRQHandler(void) {
  uart_serial_handle_rx_dma_interrupt(radio_driver);
}
