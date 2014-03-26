#ifndef INC_XBEE_FRAMING_H_
#define INC_XBEE_FRAMING_H_

#include <stdint.h>

#include "inc/uart_serial_driver.h"

#include "xbee_typpo.h"   // NOLINT(build/include)

extern ssize_t xbee_length_finder(uart_serial_module *module, uint8_t byte);
extern int xbee_verify_checksum(xbee_api_packet *packet);
extern void xbee_fill_checksum(xbee_api_packet *packet);

#endif  // INC_XBEE_FRAMING_H_
