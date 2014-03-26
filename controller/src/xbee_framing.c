#include "inc/xbee_framing.h"

#include "inc/stm32f4xx.h"
#include "inc/core_cm4.h"
#include "inc/core_cmInstr.h"

#define LENGTH_FINDER_STATE_IDLE  0
#define LENGTH_FINDER_STATE_LEN0  1
#define LENGTH_FINDER_STATE_LEN1  2

ssize_t xbee_length_finder(uart_serial_module *module, uint8_t byte) {
  // We use a jank hack to put the first byte of the length in bits 15 to 8 of
  // the state and use bits 7 to 0 for the actual state part.
  switch (module->length_finder_state & 0xFF) {
  case LENGTH_FINDER_STATE_IDLE:
    if (byte == XBEE_MAGIC) {
      module->length_finder_state = LENGTH_FINDER_STATE_LEN0;
    }
    return -1;
  case LENGTH_FINDER_STATE_LEN0:
    module->length_finder_state = LENGTH_FINDER_STATE_LEN1 | (byte << 8);
    return -1;
  case LENGTH_FINDER_STATE_LEN1:
    {
      uint16_t len = module->length_finder_state & 0xFF00;
      len |= byte;
      module->length_finder_state = LENGTH_FINDER_STATE_IDLE;
      // Length does not include checksum -- add 1 byte
      return len + 1;
    }
  default:
    // Should not happen
    module->length_finder_state = LENGTH_FINDER_STATE_IDLE;
    return -1;
  }
}

int xbee_verify_checksum(xbee_api_packet *packet) {
  if (packet->xbee_api_magic != XBEE_MAGIC) {
    return 0;
  }

  uint8_t sum = 0;
  for (int i = 0; i < __REV16(packet->length) + 1; i++) {
    sum += packet->payload.bytes[i];
  }

  return sum == 0xFF;
}

void xbee_fill_checksum(xbee_api_packet *packet) {
  uint8_t sum = 0;
  for (int i = 0; i < __REV16(packet->length); i++) {
    sum += packet->payload.bytes[i];
  }

  packet->payload.bytes[__REV16(packet->length)] = 0xFF - sum;
}
