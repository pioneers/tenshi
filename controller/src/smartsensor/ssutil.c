
#include <stdio.h>
#include <string.h>

#include "inc/smartsensor/ssutil.h"
#include "inc/smartsensor/cobs.h"

#include "inc/driver_glue.h"
#include "inc/uart_serial_driver.h"


EventGroupHandle_t ssBusEventGroup;

size_t numSensors = 0;
SSState *sensorArr = NULL;
xSemaphoreHandle sensorArrLock;



// TODO(cduck): Move this to uart_serial_driver.c and make it work
int ss_uart_serial_send_and_finish_data(uart_serial_module *module,
  const uint8_t *data, size_t len) {
  // TODO(rqou): Asynchronous?
  // TODO(rqou): Error handling
  void *txn = uart_serial_send_data(module, data, len);
  while ((uart_serial_send_status(module, txn) !=
      UART_SERIAL_SEND_DONE) &&
      (uart_serial_send_status(module, txn) !=
        UART_SERIAL_SEND_ERROR)) {}
  return uart_serial_send_finish(module, txn);
}
// TODO(cduck): Enable COMM3 after finished using it for debugging
int ss_all_uart_serial_send_and_finish_data(const uint8_t *data, size_t len) {
  // TODO(rqou): Asynchronous?
  // TODO(rqou): Error handling
  void *txn1 = uart_serial_send_data(smartsensor_1, data, len);
  void *txn2 = uart_serial_send_data(smartsensor_2, data, len);
  // void *txn3 = uart_serial_send_data(smartsensor_3, data, len);
  // void *txn4 = uart_serial_send_data(smartsensor_4, data, len);
  while (((uart_serial_send_status(smartsensor_1, txn1)
              != UART_SERIAL_SEND_DONE) &&
          (uart_serial_send_status(smartsensor_1, txn1)
              != UART_SERIAL_SEND_ERROR) ) ||
         ((uart_serial_send_status(smartsensor_2, txn2)
              != UART_SERIAL_SEND_DONE) &&
          (uart_serial_send_status(smartsensor_2, txn2)
              != UART_SERIAL_SEND_ERROR) )
         //      ||
         // ((uart_serial_send_status(smartsensor_3, txn3)
         //      != UART_SERIAL_SEND_DONE) &&
         //  (uart_serial_send_status(smartsensor_3, txn3)
         //      != UART_SERIAL_SEND_ERROR) ) ||
         // ((uart_serial_send_status(smartsensor_4, txn4)
         //     != UART_SERIAL_SEND_DONE) &&
         // (uart_serial_send_status(smartsensor_4, txn4)
         //     != UART_SERIAL_SEND_ERROR) )
         ) {}
  return uart_serial_send_finish(smartsensor_1, txn1) &
         uart_serial_send_finish(smartsensor_2, txn2);  // &
         // uart_serial_send_finish(smartsensor_3, txn3) &
         // uart_serial_send_finish(smartsensor_4, txn4);
}
int ss_send_maintenance(uart_serial_module *module, uint8_t type,
  const uint8_t *data, uint8_t len) {
  if (len > 255-4)return 0;

  uint8_t *data_cobs = (uint8_t*)pvPortMalloc(4+len);
  // Four extra for 0x00, type, len, and extra COBS byte.

  data_cobs[0] = 0x00;
  data_cobs[1] = type;
  data_cobs[2] = len+4;
  cobs_encode(data_cobs+3, data, len);

  int r = ss_uart_serial_send_and_finish_data(module, data_cobs, len+4);
  vPortFree(data_cobs);
  return r;
}
int ss_all_send_maintenance(uint8_t type, const uint8_t *data, uint8_t len) {
  if (len > 255-4)return 0;

  uint8_t *data_cobs = (uint8_t*)pvPortMalloc(4+len);
  // Four extra for 0x00, type, len, and extra COBS byte.

  data_cobs[0] = 0x00;
  data_cobs[1] = type;
  data_cobs[2] = len+4;
  cobs_encode(data_cobs+3, data, len);

  int r = ss_all_uart_serial_send_and_finish_data(data_cobs, len+4);
  vPortFree(data_cobs);
  return r;
}
int ss_send_ping_pong(SSState sensor, const uint8_t *data, uint8_t len) {
  if (len > 255-4-8)return 0;

  uint8_t *temp = (uint8_t*)pvPortMalloc(len+8);
  for (uint8_t i = 0; i < 8; i++) {
    temp[i] = sensor.id[i];
  }
  for (uint8_t i = 0; i < len; i++) {
    temp[8+i] = data[i];
  }

  int r = ss_send_maintenance(ssBusses[sensor.busNum], 0xFE, temp, len+8);
  vPortFree(temp);
  return r;
}

int ss_send_enum_enter(uart_serial_module *module) {
  #define ENUM_ENTER_LEN (MAGIC_SEQUENCE_LEN+4)
  static const uint8_t data[] = {0, 0xF0, ENUM_ENTER_LEN, MAGIC_SEQUENCE_COBS};
  return ss_uart_serial_send_and_finish_data(module, data, ENUM_ENTER_LEN);
}
int ss_send_enum_exit(uart_serial_module *module) {
  #define ENUM_EXIT_LEN 3
  static const uint8_t data[] = {0, 0xF1, ENUM_EXIT_LEN};
  return ss_uart_serial_send_and_finish_data(module, data, ENUM_EXIT_LEN);
}
int ss_send_enum_reset(uart_serial_module *module) {
  #define ENUM_RESET_LEN 3
  static const uint8_t data[] = {0, 0xF3, ENUM_RESET_LEN};
  return ss_uart_serial_send_and_finish_data(module, data, ENUM_RESET_LEN);
}
int ss_send_enum_select(uart_serial_module *module, uint8_t id[8],
  uint8_t mask[8]) {
  static const uint8_t len = 8+8;  // Not COBS
  uint8_t data[16];  // Not COBS
  for (int i = 0; i < 8; ++i) {
    data[i] = id[i];
    data[i+8] = mask[i];
  }
  return ss_send_maintenance(module, 0xF2, data, len);
}
int ss_recieve_enum_any_unselected(uart_serial_module *module) {
  int r;
  switch ((r = uart_bus_logic_level(module))) {
    case 0: return 1;
    case 1: return 0;
    default: return r;
  }
  return r;
}


/*
int ss_all_reset_bus() {
  static const 
}
*/
int ss_setup_active_sample(uart_serial_module *module, uint8_t inband,
  uint8_t sample, uint8_t frame, uint8_t *data, uint8_t len) {
  // TODO(cduck): Support in-band signalling

  // TODO(cduck): Split up packet if too long.
  if (len > 12)return 0;  // Too long for active packet
  if ((frame & 7) == 0)return 0;  // Frame can never be zero

  uint8_t *data_cobs = (uint8_t*)pvPortMalloc(4+len);
  // Four extra for 0x00, sample/frame, len, and extra COBS byte.

  data_cobs[0] = 0x00;
  data_cobs[1] = ((!!inband) << 6) | ((sample & 7) << 3) | (frame & 7);
  data_cobs[2] = len+4;
  cobs_encode(data_cobs+3, data, len);

  int r = ss_all_uart_serial_send_and_finish_data(data_cobs, len+4);
  vPortFree(data_cobs);
  return r;
}





// Assuming the sensor is already locked
void allocIncomingBytes(SSState sensor, uint8_t requiredLen) {
  if (sensor.incomingBytes == NULL) {
    sensor.incomingLen = requiredLen;
    sensor.incomingBytes = pvPortMalloc(requiredLen);
  }
  if (sensor.incomingLen != requiredLen) {
    vPortFree(sensor.incomingBytes);
    sensor.incomingLen = requiredLen;
    sensor.incomingBytes = pvPortMalloc(requiredLen);
  }
}

// Assuming the sensor is already locked
int checkOutgoingBytes(SSState sensor, uint8_t requiredLen) {
  return (sensor.outgoingBytes != NULL) && (sensor.outgoingLen >= requiredLen);
}

