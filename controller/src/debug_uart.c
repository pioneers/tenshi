#include "inc/debug_uart.h"

#include <stdint.h>
#include <string.h>
#include "inc/FreeRTOS.h"
#include "inc/queue.h"
#include "inc/stm32f4xx.h"
#include "inc/task.h"

#define QUEUE_BLOCK_SIZE    64
typedef struct {
  uint8_t len;
  char data[QUEUE_BLOCK_SIZE];
} queue_block;
#define QUEUE_TOTAL_BLOCKS  8

static xQueueHandle txQueue;

static portTASK_FUNCTION_PROTO(debugUartTxTask, pvParameters) {
  queue_block tx_buffer;
  while (1) {
    xQueueReceive(txQueue, &tx_buffer, portMAX_DELAY);

    // TODO(rqou): DMA?
    for (int i = 0; i < tx_buffer.len; i++) {
      while (!(USART1->SR & USART_SR_TXE)) {}
      USART1->DR = tx_buffer.data[i];
    }
  }
}

void debug_uart_setup() {
  // Enable alternate functions on PB6, PB7
  // TODO(rqou): Pin defs in separate header
  GPIOB->MODER |= (1 << (6 * 2 + 1)) | (1 << (7 * 2 + 1));
  // Set AF7 (USART1) on these pins
  GPIOB->AFR[0] |= (7 << (6*4)) | (7 << (7*4));

  // Turn on clock to USART1
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

  /// Turn on USART1
  USART1->CR1 |= USART_CR1_UE;
  // We don't have to set anything to get 8n1
  // This gives 1 mbaud with PCLK2 = 84 MHz
  USART1->BRR = 84;
  /// Turn on TX
  USART1->CR1 |= USART_CR1_TE;

  txQueue = xQueueCreate(8, sizeof(queue_block));
  xTaskCreate(debugUartTxTask, "dbgUartTx", 256, NULL, tskIDLE_PRIORITY, NULL);
}

void debug_uart_tx(const char *str, size_t len) {
  queue_block buffer;
  while (len / QUEUE_BLOCK_SIZE) {
    buffer.len = QUEUE_BLOCK_SIZE;
    memcpy(buffer.data, str, QUEUE_BLOCK_SIZE);
    xQueueSendToBack(txQueue, &buffer, portMAX_DELAY);
    str += QUEUE_BLOCK_SIZE;
    len -= QUEUE_BLOCK_SIZE;
  }

  if (len) {
    buffer.len = len;
    memcpy(buffer.data, str, len);

    xQueueSendToBack(txQueue, &buffer, portMAX_DELAY);
  }
}
