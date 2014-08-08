// Licensed to Pioneers in Engineering under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Pioneers in Engineering licenses
// this file to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
//  with the License.  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License

#include "inc/uart_serial_driver.h"

#include "inc/FreeRTOS.h"
#include "inc/pindef.h"
#include "inc/portable.h"
#include "inc/queue.h"
#include "inc/semphr.h"
#include "inc/task.h"

#define UART_RX_BUFFER_SIZE   256

typedef struct tag_uart_txn {
  uint8_t *data;
  size_t len;
  // Used only for TX
  int status;
} uart_txn;

typedef struct tag_uart_serial_module_private {
  uart_serial_module public;
  int uart_num;
  ssize_t (*length_finder_fn)(uart_serial_module *module, uint8_t byte);
  void (*txen_fn)(int enable);
  xQueueHandle txQueue;
  xQueueHandle rxQueue;
  xSemaphoreHandle txInUse;
  xSemaphoreHandle rxSignal;
  uart_txn *currentTxTxn;
  uart_txn *currentRxTxn;
} uart_serial_module_private;

typedef struct tag_uart_periph_info {
  USART_TypeDef *periph;
  DMA_TypeDef *dma;
  DMA_Stream_TypeDef *dma_stream_tx;
  int dma_stream_num_tx;
  int dma_channel_tx;
  DMA_Stream_TypeDef *dma_stream_rx;
  int dma_stream_num_rx;
  int dma_channel_rx;
  int pclk;
} uart_periph_info;

const uart_periph_info periph_info[6] = {
  {
    .periph = USART1,
    .dma = DMA2,
    .dma_stream_tx = DMA2_Stream7,
    .dma_stream_num_tx = 7,
    .dma_channel_tx = 4,
    .dma_stream_rx = DMA2_Stream5,
    .dma_stream_num_rx = 5,
    .dma_channel_rx = 4,
    .pclk = PCLK1_FREQ,
  }, {
    .periph = USART2,
    .dma = DMA1,
    .dma_stream_tx = DMA1_Stream6,
    .dma_stream_num_tx = 6,
    .dma_channel_tx = 4,
    .dma_stream_rx = DMA1_Stream5,
    .dma_stream_num_rx = 5,
    .dma_channel_rx = 4,
    .pclk = PCLK2_FREQ,
  }, {
    .periph = USART3,
    .dma = DMA1,
    .dma_stream_tx = DMA1_Stream3,
    .dma_stream_num_tx = 3,
    .dma_channel_tx = 4,
    .dma_stream_rx = DMA1_Stream1,
    .dma_stream_num_rx = 1,
    .dma_channel_rx = 4,
    .pclk = PCLK2_FREQ,
  }, {
    .periph = UART4,
    .dma = DMA1,
    .dma_stream_tx = DMA1_Stream4,
    .dma_stream_num_tx = 4,
    .dma_channel_tx = 4,
    .dma_stream_rx = DMA1_Stream2,
    .dma_stream_num_rx = 2,
    .dma_channel_rx = 4,
    .pclk = PCLK2_FREQ,
  }, {
    .periph = UART5,
    .dma = DMA1,
    .dma_stream_tx = DMA1_Stream7,
    .dma_stream_num_tx = 7,
    .dma_channel_tx = 4,
    .dma_stream_rx = DMA1_Stream0,
    .dma_stream_num_rx = 0,
    .dma_channel_rx = 4,
    .pclk = PCLK2_FREQ,
  }, {
    .periph = USART6,
    .dma = DMA2,
    .dma_stream_tx = DMA2_Stream6,
    .dma_stream_num_tx = 6,
    .dma_channel_tx = 5,
    .dma_stream_rx = DMA2_Stream1,
    .dma_stream_num_rx = 1,
    .dma_channel_rx = 5,
    .pclk = PCLK1_FREQ,
  },
};

const int dma_interrupt_flag_shift[] = {
  0,
  6,
  16,
  22,
  0,
  6,
  16,
  22,
};

void uart_serial_set_length_finder(uart_serial_module *_module,
  ssize_t (*length_finder_fn)(uart_serial_module *, uint8_t)) {
  uart_serial_module_private *module = (uart_serial_module_private *)_module;

  module->length_finder_fn = length_finder_fn;
}

static portTASK_FUNCTION_PROTO(uart_tx_task, pvParameters) {
  uart_serial_module_private *module =
    (uart_serial_module_private *)pvParameters;

  while (1) {
    uart_txn *txn;
    xQueueReceive(module->txQueue, &txn, portMAX_DELAY);
    xSemaphoreTake(module->txInUse, portMAX_DELAY);

    // We have a thing to send
    module->currentTxTxn = txn;
    txn->status = UART_SERIAL_SEND_SENDING;
    USART_TypeDef *periph_base = periph_info[module->uart_num - 1].periph;
    DMA_Stream_TypeDef *dma_stream_tx =
      periph_info[module->uart_num - 1].dma_stream_tx;
    dma_stream_tx->M0AR = (uint32_t)(txn->data);
    dma_stream_tx->NDTR = txn->len;
    // Turn on the TXEN pin
    module->txen_fn(1);
    // Clear TC and start DMA
    periph_base->SR &= ~USART_SR_TC;
    dma_stream_tx->CR |= DMA_SxCR_EN;
  }
}

static portTASK_FUNCTION_PROTO(uart_rx_task, pvParameters) {
  uart_serial_module_private *module =
    (uart_serial_module_private *)pvParameters;

  while (1) {
    xSemaphoreTake(module->rxSignal, portMAX_DELAY);

    // A packet just came in. We now need to allocate a new buffer.
    // We will block in this task if we can't.

    uart_txn *txn = pvPortMalloc(sizeof(uart_txn));
    txn->len = 0;
    txn->data = pvPortMalloc(UART_RX_BUFFER_SIZE);

    module->currentRxTxn = txn;
  }
}

uart_serial_module *uart_serial_init_module(int uart_num,
  ssize_t (*length_finder_fn)(uart_serial_module *, uint8_t),
  void (*txen_fn)(int), int baud) {
  uart_serial_module_private *module =
    pvPortMalloc(sizeof(uart_serial_module_private));

  USART_TypeDef *periph_base = periph_info[uart_num - 1].periph;
  DMA_TypeDef *dma_base = periph_info[uart_num - 1].dma;
  DMA_Stream_TypeDef *dma_stream_tx = periph_info[uart_num - 1].dma_stream_tx;
  DMA_Stream_TypeDef *dma_stream_rx = periph_info[uart_num - 1].dma_stream_rx;
  int pclk = periph_info[uart_num - 1].pclk;
  int dma_stream_num_tx = periph_info[uart_num - 1].dma_stream_num_tx;
  int dma_stream_num_rx = periph_info[uart_num - 1].dma_stream_num_rx;
  int dma_channel_tx = periph_info[uart_num - 1].dma_channel_tx;
  int dma_channel_rx = periph_info[uart_num - 1].dma_channel_rx;

  module->public.length_finder_state = 0;
  module->uart_num = uart_num;
  module->length_finder_fn = length_finder_fn;
  module->txen_fn = txen_fn;

  // Turn on UART peripheral
  periph_base->CR1 = USART_CR1_UE;
  // This + the previous get 8n1 asynchronous
  periph_base->CR2 = 0;
  // We have OVER8=0, so this calculates the baud rate taking into account
  // that BRR is fixed point.
  periph_base->BRR = (int)(((float)pclk) / ((float)baud) + 0.5f);
  // Turn on TX and RX
  periph_base->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_TCIE;
  // Enable TX DMA and error interrupt. We don't yet want RX DMA enabled.
  periph_base->CR3 = USART_CR3_DMAT | USART_CR3_EIE;
  // Initially, we want RX data interrupt enabled
  periph_base->CR1 |= USART_CR1_RXNEIE;

  // Configure the DMA streams
  // TX stream
  // Turn off stream
  dma_stream_tx->CR = 0;
  // Clear all flags
  if (dma_stream_num_tx < 4) {
    dma_base->LIFCR = 0b111101 << dma_interrupt_flag_shift[dma_stream_num_tx];
  } else {
    dma_base->HIFCR = 0b111101 << dma_interrupt_flag_shift[dma_stream_num_tx];
  }
  // Disable FIFO error interrupt
  dma_stream_tx->FCR = 0;
  // Read from UART DR
  dma_stream_tx->PAR = (uint32_t)(&(periph_base->DR));
  // TX selected channel, increment memory address, memory to peripheral,
  // enable interrupt on done and error
  dma_stream_tx->CR =
    (dma_channel_tx << 25) | DMA_SxCR_MINC | DMA_SxCR_DIR_0 | DMA_SxCR_TCIE |
    DMA_SxCR_TEIE;
  // RX stream
  // Turn off stream
  dma_stream_rx->CR = 0;
  // Clear all flags
  if (dma_stream_num_rx < 4) {
    dma_base->LIFCR = 0b111101 << dma_interrupt_flag_shift[dma_stream_num_rx];
  } else {
    dma_base->HIFCR = 0b111101 << dma_interrupt_flag_shift[dma_stream_num_rx];
  }
  // Disable FIFO error interrupt
  dma_stream_rx->FCR = 0;
  // Write to UART DR
  dma_stream_rx->PAR = (uint32_t)(&(periph_base->DR));
  // RX selected channel, increment memory address, peripheral to memory,
  // enable interrupt on done and error
  dma_stream_rx->CR =
    (dma_channel_rx << 25) | DMA_SxCR_MINC | DMA_SxCR_TCIE | DMA_SxCR_TEIE;

  // Allocate queues (max 8 entries each)
  module->txQueue = xQueueCreate(8, sizeof(module->txQueue));
  module->rxQueue = xQueueCreate(8, sizeof(module->rxQueue));

  // Allocate semaphores
  vSemaphoreCreateBinary(module->txInUse);
  xSemaphoreGive(module->txInUse);
  vSemaphoreCreateBinary(module->rxSignal);

  // Allocate a buffer for RX
  uart_txn *rxTxn = pvPortMalloc(sizeof(uart_txn));
  rxTxn->len = 0;
  rxTxn->data = pvPortMalloc(UART_RX_BUFFER_SIZE);
  module->currentRxTxn = rxTxn;
  module->currentTxTxn = NULL;

  // Start tasks
  xTaskCreate(uart_rx_task, "UART_RX", 256, module, tskIDLE_PRIORITY, NULL);
  xTaskCreate(uart_tx_task, "UART_TX", 256, module, tskIDLE_PRIORITY, NULL);

  return (uart_serial_module *)module;
}

void *uart_serial_send_data(uart_serial_module *module, uint8_t *data,
  size_t len) {
  uart_txn *txn = pvPortMalloc(sizeof(uart_txn));

  txn->data = data;
  txn->len = len;
  txn->status = UART_SERIAL_SEND_QUEUED;

  xQueueSendToBack(((uart_serial_module_private *)module)->txQueue, &txn,
    portMAX_DELAY);

  return txn;
}

int uart_serial_send_status(uart_serial_module *module, void *transaction) {
  (void) module;

  return ((uart_txn *)transaction)->status;
}

int uart_serial_send_finish(uart_serial_module *module, void *_transaction) {
  (void) module;

  uart_txn *transaction = (uart_txn *)_transaction;
  // Ignore non-finished transactions
  if ((transaction->status != UART_SERIAL_SEND_DONE) &&
      (transaction->status != UART_SERIAL_SEND_ERROR)) {
    return 0;
  }

  vPortFree(transaction);

  return 1;
}

// TODO(cduck): This function stays in an infinite loop.
// Copy the function code to use.
int uart_serial_send_and_finish_data(uart_serial_module *module, uint8_t *data,
  size_t len) {
  // TODO(rqou): Asynchronous?
  // TODO(rqou): Error handling
  void *txn = uart_serial_send_data(module, data, len);
  while ((uart_serial_send_status(module, txn) !=
      UART_SERIAL_SEND_DONE) &&
      (uart_serial_send_status(module, txn) !=
        UART_SERIAL_SEND_ERROR)) {}
  return uart_serial_send_finish(module, txn);
}

uint8_t *uart_serial_receive_packet(uart_serial_module *module,
  size_t *len_out, int shouldBlock) {
  uart_txn *txn;

  int ret = xQueueReceive(
    ((uart_serial_module_private *)module)->rxQueue, &txn,
    shouldBlock ? portMAX_DELAY : 0);

  if (!ret) {
    return NULL;
  }

  if (len_out) {
    *len_out = txn->len;
  }

  uint8_t *outBuf = txn->data;

  vPortFree(txn);

  return outBuf;
}

extern int uart_bus_logic_level(uart_serial_module *_module) {
  uart_serial_module_private *module = (uart_serial_module_private *)_module;
  switch (module->uart_num) {
    case 4:
      return !!(GPIO_BANK(PINDEF_SENSOR_CH1_RX)->IDR &
                (1 << GPIO_PIN(PINDEF_SENSOR_CH1_RX)) );
    case 2:
      return !!(GPIO_BANK(PINDEF_SENSOR_CH2_RX)->IDR &
                (1 << GPIO_PIN(PINDEF_SENSOR_CH2_RX)) );
    case 1:
      return !!(GPIO_BANK(PINDEF_SENSOR_CH3_RX)->IDR &
                (1 << GPIO_PIN(PINDEF_SENSOR_CH3_RX)) );
    case 6:
      return !!(GPIO_BANK(PINDEF_SENSOR_CH4_RX)->IDR &
                (1 << GPIO_PIN(PINDEF_SENSOR_CH4_RX)) );
    default:
      return -1;
  }
  return -1;
}

void uart_serial_handle_tx_dma_interrupt(uart_serial_module *_module) {
  uart_serial_module_private *module = (uart_serial_module_private *)_module;
  uint32_t isr;

  int uart_num = module->uart_num;
  DMA_TypeDef *dma_base = periph_info[uart_num - 1].dma;
  int dma_stream_num_tx = periph_info[uart_num - 1].dma_stream_num_tx;

  if (dma_stream_num_tx < 4) {
    isr = dma_base->LISR;
  } else {
    isr = dma_base->HISR;
  }

  int shift = dma_interrupt_flag_shift[dma_stream_num_tx];

  if (isr & (DMA_LISR_TEIF0 << shift)) {
    // If error
    if (module->currentTxTxn) {
      // TODO(rqou): Wat, can this be null?
      module->currentTxTxn->status = UART_SERIAL_SEND_ERROR;
    }

    if (dma_stream_num_tx < 4) {
      dma_base->LIFCR = DMA_LIFCR_CTEIF0 << shift;
    } else {
      dma_base->HIFCR = DMA_LIFCR_CTEIF0 << shift;
    }
  } else if (isr & (DMA_LISR_TCIF0 << shift)) {
    // If done
    module->currentTxTxn->status = UART_SERIAL_SEND_DONE;
    module->currentTxTxn = NULL;

    if (dma_stream_num_tx < 4) {
      dma_base->LIFCR = DMA_LIFCR_CTCIF0 << shift;
    } else {
      dma_base->HIFCR = DMA_LIFCR_CTCIF0 << shift;
    }

    xSemaphoreGiveFromISR(module->txInUse, NULL);
  }
}

void uart_serial_handle_rx_dma_interrupt(uart_serial_module *_module) {
  uart_serial_module_private *module = (uart_serial_module_private *)_module;
  uint32_t isr;

  int uart_num = module->uart_num;
  USART_TypeDef *periph_base = periph_info[uart_num - 1].periph;
  DMA_TypeDef *dma_base = periph_info[uart_num - 1].dma;
  int dma_stream_num_rx = periph_info[uart_num - 1].dma_stream_num_rx;

  if (dma_stream_num_rx < 4) {
    isr = dma_base->LISR;
  } else {
    isr = dma_base->HISR;
  }

  int shift = dma_interrupt_flag_shift[dma_stream_num_rx];

  if (isr & (DMA_LISR_TEIF0 << shift)) {
    // If error
    // TODO(rqou): Somehow report this error
    // Clear out the current transaction
    if (module->currentRxTxn) {
      module->currentRxTxn->len = 0;
    }

    if (dma_stream_num_rx < 4) {
      dma_base->LIFCR = DMA_LIFCR_CTEIF0 << shift;
    } else {
      dma_base->HIFCR = DMA_LIFCR_CTEIF0 << shift;
    }

    // Turn on the RXNE IRQ
    periph_base->CR1 |= USART_CR1_RXNEIE;
    // Turn off the RX DMA
    periph_base->CR3 &= ~USART_CR3_DMAR;

  } else if (isr & (DMA_LISR_TCIF0 << shift)) {
    // If done
    portBASE_TYPE ret =
      xQueueSendToBackFromISR(module->rxQueue, &(module->currentRxTxn), NULL);

    if (dma_stream_num_rx < 4) {
      dma_base->LIFCR = DMA_LIFCR_CTCIF0 << shift;
    } else {
      dma_base->HIFCR = DMA_LIFCR_CTCIF0 << shift;
    }

    // Turn on the RXNE IRQ
    periph_base->CR1 |= USART_CR1_RXNEIE;
    // Turn off the RX DMA
    periph_base->CR3 &= ~USART_CR3_DMAR;

    if (ret != pdPASS) {
      // Did not put into queue properly.
      // TODO(rqou): Error handling?!
      module->currentRxTxn->len = 0;
    } else {
      module->currentRxTxn = NULL;
      xSemaphoreGiveFromISR(module->rxSignal, NULL);
    }
  }
}

void uart_serial_handle_uart_interrupt(uart_serial_module *_module) {
  uart_serial_module_private *module = (uart_serial_module_private *)_module;

  int uart_num = module->uart_num;
  USART_TypeDef *periph_base = periph_info[uart_num - 1].periph;
  DMA_Stream_TypeDef *dma_stream_rx = periph_info[uart_num - 1].dma_stream_rx;

  uint32_t sr = periph_base->SR;

  if ((sr & USART_SR_FE) || (sr & USART_SR_ORE) || (sr & USART_SR_NE)) {
    // An error
    // TODO(rqou): Somehow report this error
    // Turn off the RX DMA
    dma_stream_rx->CR &= ~DMA_SxCR_EN;
    if (module->currentRxTxn) {
      // Clear out the current transaction
      module->currentRxTxn->len = 0;
    }

    // Clear the error by reading DR
    uint32_t dummy_dr = periph_base->DR;
    (void) dummy_dr;
  } else if (sr & USART_SR_TC) {
    // Transfer complete, turn off TXE
    module->txen_fn(0);
    // Clear TC so we don't keep triggering IRQ
    periph_base->SR &= ~USART_SR_TC;
  } else if (sr & USART_SR_RXNE) {
    // Got a byte
    uint32_t dr = periph_base->DR;
    // NOTE: Watch concurrency issues here. There may be issues unless the
    // DMA and this interrupt are at the same priority.
    if (module->currentRxTxn) {
      ssize_t possible_len = module->length_finder_fn(_module, dr);
      switch (possible_len) {
        case  0:
        case -1:
          // Don't know the length
          if (module->currentRxTxn->len+1
               >= UART_RX_BUFFER_SIZE) {
            // TODO(rqou): Report this error!
            // TODO(rqou): Can this break length finders?
            module->currentRxTxn->len = 0;
            return;
          }
          module->currentRxTxn->data[module->currentRxTxn->len++] = dr;
          break;
        case -2:
          // Just recieved the start byte of the packet
          module->currentRxTxn->len = 1;
          module->currentRxTxn->data[0] = dr;
          break;
        default:
          module->currentRxTxn->data[module->currentRxTxn->len++] = dr;
          if (possible_len < 0)break;  // Shouldn't happen.

          // We know how much more we want to get
          if (possible_len + module->currentRxTxn->len
               > UART_RX_BUFFER_SIZE) {
            // TODO(rqou): Report this error!
            // TODO(rqou): Can this break length finders?
            module->currentRxTxn->len = 0;
            return;
          }

          // Turn off the RXNE IRQ
          periph_base->CR1 &= ~USART_CR1_RXNEIE;
          // Turn on the RX DMA
          periph_base->CR3 |= USART_CR3_DMAR;
          // Do the DMA
          dma_stream_rx->M0AR =
            (uint32_t)(module->currentRxTxn->data + module->currentRxTxn->len);
          module->currentRxTxn->len += possible_len;
          dma_stream_rx->NDTR = possible_len;
          dma_stream_rx->CR |= DMA_SxCR_EN;
          break;
      }
    }
  }
}
