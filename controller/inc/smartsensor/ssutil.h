#ifndef INC_SMARTSENSOR_SSUTIL_H_
#define INC_SMARTSENSOR_SSUTIL_H_

#include "inc/FreeRTOS.h"
#include "inc/semphr.h"
#include "inc/event_groups.h"

#include "inc/uart_serial_driver.h"


#define SMART_ID_LEN 8   // Length of smartsensor personal ID
#define SS_MAX_ACTIVE_LEN (16-4)  // Most bytes that can be sent to a sensor
                                  // during active bus mode.

// Timeout
#define SENSOR_WAIT_TIME (1000/portTICK_PERIOD_MS)  // 1 seconds

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
  uint8_t *outgoingBytes;
  size_t incomingLen;
  uint8_t *incomingBytes;
} SSState;

/*
typedef struct {
  size_t len;
  uint8_t *bytes;
} ???;
*/

extern EventGroupHandle_t ssBusEventGroup;

extern size_t numSensors;
extern SSState *sensorArr;
extern xSemaphoreHandle sensorArrLock;


int ss_uart_serial_send_and_finish_data(uart_serial_module *module,
  const uint8_t *data, size_t len);
int ss_all_uart_serial_send_and_finish_data(const uint8_t *data, size_t len);
int ss_send_maintenance(uart_serial_module *module, uint8_t type,
  const uint8_t *data, uint8_t len);
int ss_all_send_maintenance(uint8_t type, const uint8_t *data, uint8_t len);
int ss_send_ping_pong(SSState sensor, const uint8_t *data, uint8_t len);

int ss_send_enum_enter(uart_serial_module *module);
int ss_send_enum_exit(uart_serial_module *module);
int ss_send_enum_reset(uart_serial_module *module);
int ss_send_enum_select(uart_serial_module *module, uint8_t id[8],
  uint8_t mask[8]);
// Returns 1 if any sensors are still driving a logic level 0 onto the bus.
int ss_recieve_enum_any_unselected(uart_serial_module *module);

int ss_setup_active_sample(uart_serial_module *module, uint8_t inband,
  uint8_t sample, uint8_t frame, uint8_t *data, uint8_t len);


void allocIncomingBytes(SSState sensor, uint8_t requiredLen);
int checkOutgoingBytes(SSState sensor, uint8_t requiredLen);

#endif  // INC_SMARTSENSOR_SSUTIL_H_
