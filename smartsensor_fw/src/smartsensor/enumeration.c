// This file handles enumeration packets

#include "inc/smartsensor/enumeration.h"

#define TYPE_ENUMERATION_ENTER 0xF0
#define TYPE_ENUMERATION_EXIT 0xF1
#define TYPE_ENUMERATION_SELECT 0xF2
#define TYPE_ENUMERATION_RESET 0xF3
#define TYPE_ENUMERATION_UNSELECT 0xF4

uint8_t enumerating = 0;

void enumerationPacket(uint8_t type, uint8_t *data, uint8_t len) {
  switch (type) {
    case TYPE_ENUMERATION_ENTER: enumerationEnter(data, len); break;
    case TYPE_ENUMERATION_EXIT: enumerationExit(data, len); break;
    case TYPE_ENUMERATION_SELECT: enumerationSelect(data, len); break;
    case TYPE_ENUMERATION_RESET: enumerationReset(data, len); break;
    case TYPE_ENUMERATION_UNSELECT: enumerationUnselect(data, len); break;
    default: break;
  }
}
void enumerationEnter(uint8_t *data, uint8_t len) {
  if (len == 21 && data[0] == 0xE6
                && data[1] == 0xAD
                && data[2] == 0xBB
                && data[3] == 0xE3
                && data[4] == 0x82
                && data[5] == 0x93
                && data[6] == 0xE3
                && data[7] == 0x81
                && data[8] == 0xA0
                && data[9] == 0xE4
                && data[10] == 0xB8
                && data[11] == 0x96
                && data[12] == 0xE7
                && data[13] == 0x95
                && data[14] == 0x8C
                && data[15] == 0xE6
                && data[16] == 0x88
                && data[17] == 0xA6
                && data[18] == 0xE7
                && data[19] == 0xB7
                && data[20] == 0x9A) {
    enumerating = 1;
    DIGITAL_SET_OUT(SS_UART_TXE);
    DIGITAL_SET_OUT(SS_UART_TX);
    DIGITAL_SET_HIGH(SS_UART_TXE);  // Turn PA6 on  // Enable bus driver
    SS_UCSRnB &= ~(1 << SS_TXENn);    // Disable UART transmiter
    DIGITAL_SET_LOW(SS_UART_TX);  // Drive TX pin low
  }
}
void enumerationExit(uint8_t *data, uint8_t len) {
  if (!enumerating) return;
  enumerating = 0;

  // Fix for floating bus when not driven
  DIGITAL_SET_HIGH(SS_UART_TX);
  for (uint8_t i = 0; i < 20; i++) {}

  DIGITAL_SET_LOW(SS_UART_TXE);  // Turn PA6 off  // Stop driving the bus
  SS_UCSRnB |= (1 << SS_TXENn);  // Enable UART transmiter
}
void enumerationReset(uint8_t *data, uint8_t len) {
  if (!enumerating) return;
  DIGITAL_SET_HIGH(SS_UART_TXE);  // Turn PA6 on  // Enable bus driver
  SS_UCSRnB &= ~(1 << SS_TXENn);    // Disable UART transmiter
  DIGITAL_SET_LOW(SS_UART_TX);  // Drive TX pin low
}
void enumerationSelect(uint8_t *data, uint8_t len) {
  if (!enumerating) return;
  if (len < 2*SMART_ID_LEN) return;
  for (uint8_t i = 0; i < SMART_ID_LEN; ++i) {
    if ((data[i] ^ smartID[i]) & data[i+SMART_ID_LEN]) return;  // Doesn't match
  }

  // Fix for floating bus when not driven
  DIGITAL_SET_HIGH(SS_UART_TX);
  for (uint8_t i = 0; i < 20; i++) {}

  DIGITAL_SET_LOW(SS_UART_TXE);  // Turn PA6 off  // Stop driving the bus
  SS_UCSRnB |= (1 << SS_TXENn);  // Enable UART transmiter
}
void enumerationUnselect(uint8_t *data, uint8_t len) {
  if (!enumerating) return;
  if (len < 2*SMART_ID_LEN) return;
  for (uint8_t i = 0; i < SMART_ID_LEN; ++i) {
    if ((data[i] ^ smartID[i]) & data[i+SMART_ID_LEN]) return;  // Doesn't match
  }
  enumerationReset(data, len);
}
