#include "inc/debug_uart.h"

#include "inc/FreeRTOS.h"
#include "inc/queue.h"

#define BUFFER_SIZE 64

typedef struct tag_debug_uart_module_private {
  debug_uart_module public;
  SPI_TypeDef *periph;
  xQueueHandle txQueue;
  xQueueHandle rxQueue;
  // Used if escaping is needed
  int rx_next_is_escaped;
  char tx_next_send;
} debug_uart_module_private;

extern void debug_uart_dumb_write(debug_uart_module *_module,
  const char *buf, size_t len) {
  debug_uart_module_private *module = (debug_uart_module_private *)(_module);

  uint8_t dummy;
  (void) dummy;

  for (size_t i = 0; i < len; i++) {
    if (buf[i] == 0x00 || buf[i] == 0x01) {
      // Byte needs to be escaped

      // Wait for TX buffer empty
      while (!(module->periph->SR & SPI_SR_TXE)) {}
      // Escape byte
      module->periph->DR = 0x01;
      // Wait for incoming byte -- discard it
      while (!(module->periph->SR & SPI_SR_RXNE)) {}
      dummy = module->periph->DR;

      // Wait for TX buffer empty
      while (!(module->periph->SR & SPI_SR_TXE)) {}
      // Byte, escaped
      module->periph->DR = buf[i] + 1;
      // Wait for incoming byte -- discard it
      while (!(module->periph->SR & SPI_SR_RXNE)) {}
      dummy = module->periph->DR;
    } else {
      // No escaping needed here

      // Wait for TX buffer empty
      while (!(module->periph->SR & SPI_SR_TXE)) {}
      // Byte
      module->periph->DR = buf[i];
      // Wait for incoming byte -- discard it
      while (!(module->periph->SR & SPI_SR_RXNE)) {}
      dummy = module->periph->DR;
    }
  }
}

extern debug_uart_module *debug_uart_init(SPI_TypeDef *periph_base) {
  debug_uart_module_private *module =
    pvPortMalloc(sizeof(debug_uart_module_private));
  module->periph = periph_base;

  // Configure for:
  //   Master mode
  //   CPOL = 0 CPHA = 0 (clock idles low, samples on rising edges)
  //   fPCLK / 32 = 1.5 MHz
  //   MSB first
  //   NSS ignored
  //   8-bit data
  periph_base->CR1 = SPI_CR1_MSTR | SPI_CR1_BR_2 | SPI_CR1_SPE | SPI_CR1_SSI |
                     SPI_CR1_SSM;
  // Enable the TX and RX interrupts. Hope no errors occur (they should not,
  // because CRC and NSS are disabled and we should not ever overrun).
  periph_base->CR2 = SPI_CR2_TXEIE | SPI_CR2_RXNEIE;

  module->txQueue = xQueueCreate(BUFFER_SIZE, sizeof(char));
  module->rxQueue = xQueueCreate(BUFFER_SIZE, sizeof(char));

  module->rx_next_is_escaped = 0;
  module->tx_next_send = 0;

  return module;
}

extern void debug_uart_tx(debug_uart_module *_module,
  const char *buf, size_t len) {
  debug_uart_module_private *module = (debug_uart_module_private *)(_module);

  for (size_t i = 0; i < len; i++) {
    xQueueSendToBack(module->txQueue, &(buf[i]), portMAX_DELAY);
  }
}

extern size_t debug_uart_rx(debug_uart_module *_module,
  const char *buf, size_t buf_len) {
  debug_uart_module_private *module = (debug_uart_module_private *)(_module);

  size_t actual_len = 0;
  while (actual_len < buf_len) {
    if (!xQueueReceive(module->rxQueue, &(buf[actual_len++]), 0)) {
      break;
    }
  }

  return actual_len;
}

extern void debug_uart_irq(debug_uart_module *_module) {
  debug_uart_module_private *module = (debug_uart_module_private *)(_module);

  uint16_t sr = module->periph->SR;

  if (sr & SPI_SR_TXE) {
    // Need to send something
    if (module->tx_next_send) {
      // Send something that needed to be escaped from last time.
      module->periph->DR = module->tx_next_send;
      module->tx_next_send = 0;
    } else {
      uint8_t byte;
      if (xQueueReceiveFromISR(module->txQueue, &byte, NULL)) {
        // Some actual data to send
        if (byte == 0 || byte == 1) {
          // Needs escape
          module->tx_next_send = byte + 1;
          module->periph->DR = 0x01;
        } else {
          module->periph->DR = byte;
        }
      } else {
        // Nothing to send - send 0
        module->periph->DR = 0x00;
      }
    }
  }

  if (sr & SPI_SR_RXNE) {
    // Got some data
    uint8_t byte = module->periph->DR;
    if (byte == 0x00) {
      // Do nothing
    } else if (byte == 0x01) {
      // An escape byte -- don't store this one
      module->rx_next_is_escaped = 1;
    } else {
      // Normal byte
      if (module->rx_next_is_escaped) {
        byte--;
        module->rx_next_is_escaped = 0;
      }
      xQueueSendToBackFromISR(module->rxQueue, &byte, NULL);
    }
  }
}
