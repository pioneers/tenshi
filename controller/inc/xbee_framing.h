#ifndef INC_XBEE_FRAMING_H_
#define INC_XBEE_FRAMING_H_

#include <stdint.h>

#include "inc/uart_serial_driver.h"

#define XBEE_MAGIC  0x7E

#define XBEE_API_TYPE_TX64  0x00
#define XBEE_API_TYPE_RX64  0x80

typedef struct __attribute__((packed)) tag_xbee_tx64_header {
  uint8_t xbee_api_type;
  uint8_t frameId;
  uint64_t xbee_dest_addr;
  uint8_t options;
  uint8_t data[];
} xbee_tx64_header;

typedef struct __attribute__((packed)) tag_xbee_rx64_header {
  uint8_t xbee_api_type;
  uint64_t xbee_src_addr;
  uint8_t rssi;
  uint8_t options;
  uint8_t data[];
} xbee_rx64_header;

typedef struct __attribute__((packed)) tag_xbee_api_packet {
  uint8_t xbee_api_magic;
  // Note: big endian
  uint16_t length;
  union {
    xbee_tx64_header tx64;
    xbee_rx64_header rx64;
    uint8_t xbee_api_type;
    uint8_t payload[1];
  };
} xbee_api_packet;

extern ssize_t xbee_length_finder(uart_serial_module *module, uint8_t byte);
extern int xbee_verify_checksum(xbee_api_packet *packet);
extern void xbee_fill_checksum(xbee_api_packet *packet);

#endif  // INC_XBEE_FRAMING_H_
