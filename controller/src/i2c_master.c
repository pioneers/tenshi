#include "inc/i2c_master.h"

#include "inc/FreeRTOS.h"
#include "inc/portable.h"
#include "inc/queue.h"
#include "inc/semphr.h"
#include "inc/stm32f4xx.h"
#include "inc/task.h"

#define STATE_IDLE  0
#define STATE_BUSY  1

typedef struct tag_i2c_transaction_obj {
  uint8_t *data_out;
  uint8_t *data_in;
  size_t len_out;
  size_t len_in;
  uint8_t addr;
  int status;
} i2c_transaction_obj;

typedef struct tag_i2c_master_module_private {
  i2c_master_module public;
  I2C_TypeDef *periph_base;
  int current_state;
  xQueueHandle txnQueue;
  xSemaphoreHandle inUse;
  i2c_transaction_obj *currentTxn;
} i2c_master_module_private;

static portTASK_FUNCTION_PROTO(i2c_master_task, pvParameters) {
  i2c_master_module_private *module = (i2c_master_module_private*)pvParameters;
  while (1) {
    i2c_transaction_obj *txn;
    xQueueReceive(module->txnQueue, &txn, portMAX_DELAY);
    xSemaphoreTake(module->inUse, portMAX_DELAY);
    module->currentTxn = txn;
    if (txn->len_out) {
      txn->status = TRANSACTION_STATUS_SENDING;
    } else {
      txn->status = TRANSACTION_STATUS_RECEIVING;
    }
    module->periph_base->CR1 |= I2C_CR1_START;
  }
}

i2c_master_module *i2c_master_init_module(void *periph_base) {
  i2c_master_module_private *module_obj =
    pvPortMalloc(sizeof(i2c_master_module_private));
  I2C_TypeDef *i2c = (I2C_TypeDef *)periph_base;

  // TODO(rqou): Figure out magic to make pin init and clock init work

  // Initialize the proper APB2 frequency (42 MHz) and the interrupt enable
  i2c->CR2 = I2C_CR2_ITBUFEN | I2C_CR2_ITERREN | I2C_CR2_ITEVTEN | 42;
  // Hardcoded fast mode, mostly equal duty, 400 kHz
  // Actual timings:
  //    tPclk1 = 23.80952381   ns
  //    tHigh  = 1500.00000003 ns
  //    tLow   = 2666.66666672 ns
  //    speed  = 375 kHz
  i2c->CCR = I2C_CCR_FS | I2C_CCR_DUTY | 7;
  // Rise time of max 300 ns according to I2C spec
  i2c->TRISE = 13;
  // Not sure about noise filters, so off for now
  i2c->FLTR = 0;
  // Enable the peripheral
  i2c->CR1 = I2C_CR1_PE;

  // Allocate queue
  module_obj->txnQueue = xQueueCreate(8, sizeof(module_obj->currentTxn));

  // Allocate semaphore
  vSemaphoreCreateBinary(module_obj->inUse);
  xSemaphoreGive(module_obj->inUse);

  // Fill in the struct stuff
  module_obj->periph_base = i2c;
  module_obj->current_state = STATE_IDLE;
  module_obj->currentTxn = NULL;

  // Start the task
  xTaskCreate(i2c_master_task, "I2C", 256, NULL, tskIDLE_PRIORITY, NULL);

  return (i2c_master_module *)module_obj;
}

void *i2c_issue_transaction(i2c_master_module *module, uint8_t addr,
  uint8_t *data_out, size_t len_out, uint8_t *data_in, size_t len_in) {
  i2c_transaction_obj *obj = pvPortMalloc(sizeof(i2c_transaction_obj));

  obj->data_out = data_out;
  obj->data_in = data_in;
  obj->len_out = len_out;
  obj->len_in = len_in;
  obj->addr = addr;
  obj->status = TRANSACTION_STATUS_QUEUED;

  xQueueSendToBack(((i2c_master_module_private*)module)->txnQueue,
    obj, portMAX_DELAY);

  return obj;
}

int i2c_transaction_status(i2c_master_module *module,
  void *transaction) {
  // TODO(rqou): Check very carefully the need for memory barriers or get fun
  // bugs here.

  return ((i2c_transaction_obj *)transaction)->status;
}

void i2c_handle_interrupt(i2c_master_module *_module) {
  uint32_t sr1_reg, sr2_reg;
  i2c_master_module_private *module = (i2c_master_module_private*)_module;
  i2c_transaction_obj *txn = module->currentTxn;

  sr1_reg = module->periph_base->SR1;
  sr2_reg = module->periph_base->SR2;

  // Transmitting data
  if (txn->status == TRANSACTION_STATUS_SENDING) {
    if (sr1_reg & I2C_SR1_SB) {
      // Send the address for a write
      module->periph_base->DR = txn->addr;
    } else if (sr1_reg & I2C_SR1_ADDR) {
      // Ignore this, we will get a TxE later
    } else if (sr1_reg & I2C_SR1_TXE) {
      // Write some data

      // If there's data, write it
      if (txn->len_out) {
        module->periph_base->DR = *(txn->data_out++);
        txn->len_out--;
      } else if (sr1_reg & I2C_SR1_BTF) {
        // No data left, and data is done sending. Either end or switch modes to
        // RX.
        if (txn->len_in) {
          module->periph_base->CR1 |= I2C_CR1_START;
          txn->status = TRANSACTION_STATUS_RECEIVING;
        } else {
          module->periph_base->CR1 |= I2C_CR1_STOP;
          txn->status = TRANSACTION_STATUS_DONE;
          module->currentTxn = NULL;
          xSemaphoreGiveFromISR(module->inUse, NULL);
        }
      }
    }
  } else if (txn->status == TRANSACTION_STATUS_RECEIVING) {
    if (sr1_reg & I2C_SR1_SB) {
      // Send the address for a read
      module->periph_base->DR = txn->addr | 1;
    } else if (sr1_reg & I2C_SR1_ADDR) {
      // If we are getting only one byte, we need to NACK and stop
      if (txn->len_in == 1) {
        module->periph_base->CR1 =
          (module->periph_base->CR1 & (~I2C_CR1_ACK)) | I2C_CR1_STOP;
      } else {
        // We need to ack
        module->periph_base->CR1 |= I2C_CR1_ACK;
      }
      // We will get a RxNE later
    } else if (sr1_reg & I2C_SR1_RXNE) {
      *(txn->data_in++) = module->periph_base->DR;
      txn->len_in--;
      // If we are on the second-last byte, we need to NACK and stop
      if (txn->len_in == 1) {
        module->periph_base->CR1 =
          (module->periph_base->CR1 & (~I2C_CR1_ACK)) | I2C_CR1_STOP;
      }
      // If there are no bytes left, we are done!
      if (txn->len_in == 0) {
        txn->status = TRANSACTION_STATUS_DONE;
        module->currentTxn = NULL;
        xSemaphoreGiveFromISR(module->inUse, NULL);
      }
    }
  }
}

void i2c_handle_interrupt_error(i2c_master_module *_module) {
  i2c_master_module_private *module = (i2c_master_module_private*)_module;
  i2c_transaction_obj *txn = module->currentTxn;

  if (txn) {
    txn->status = TRANSACTION_STATUS_ERROR;
    module->currentTxn = NULL;
    xSemaphoreGiveFromISR(module->inUse, NULL);
    module->periph_base->CR1 |= I2C_CR1_STOP;
  }
}

void i2c_transaction_finish(i2c_master_module *module,
  void *_transaction) {
  i2c_transaction_obj *transaction = (i2c_transaction_obj *)_transaction;
  // Ignore non-finished transactions
  if ((transaction->status != TRANSACTION_STATUS_DONE) &&
      (transaction->status != TRANSACTION_STATUS_ERROR)) {
    return;
  }

  vPortFree(transaction);
}
