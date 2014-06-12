#ifndef INC_SMARTSENSOR_SSUTIL_H_
#define INC_SMARTSENSOR_SSUTIL_H_

#include "inc/FreeRTOS.h"
#include "inc/semphr.h"
#include "inc/event_groups.h"

#include "inc/uart_serial_driver.h"


#define MAGIC_SEQUENCE_LEN 21
#define MAGIC_SEQUENCE 0xE6, 0xAD, 0xBB, 0xE3, 0x82, 0x93, 0xE3, 0x81, 0xA0, \
                       0xE4, 0xB8, 0x96, 0xE7, 0x95, 0x8C, 0xE6, 0x88, 0xA6, \
                       0xE7, 0xB7, 0x9A /* NOLINT(*) */
#define MAGIC_SEQUENCE_COBS \
                       0x16, 0xe6, 0xad, 0xbb, 0xe3, 0x82, 0x93, 0xe3, 0x81, \
                       0xa0, 0xe4, 0xb8, 0x96, 0xe7, 0x95, 0x8c, 0xe6, 0x88, \
                       0xa6, 0xe7, 0xb7, 0x9a /* NOLINT(*) */


typedef struct {
  uint8_t id[8];
  uint8_t busNum;
  xSemaphoreHandle lock;
  size_t outgoingLen;
  uint8_t outgoingBytes;
  size_t incomingLen;
  uint8_t incomingBytes;
} SSState;

/*
typedef struct {
  size_t len;
  uint8_t *bytes;
} ???;
*/

EventGroupHandle_t ssBusEventGroup;

size_t numSensors;
SSState *sensorArr;
xSemaphoreHandle sensorArrLock;


int ss_uart_serial_send_and_finish_data(uart_serial_module *module,
  uint8_t *data, size_t len);
int ss_all_uart_serial_send_and_finish_data(uint8_t *data, size_t len);
int ss_send_maintenance(uart_serial_module *module, uint8_t type,
  uint8_t *data, uint8_t len);
int ss_all_send_maintenance(uint8_t type, uint8_t *data, uint8_t len);
int ss_send_ping_pong(SSState sensor, uint8_t *data, uint8_t len);


#endif  // INC_SMARTSENSOR_SSUTIL_H_
