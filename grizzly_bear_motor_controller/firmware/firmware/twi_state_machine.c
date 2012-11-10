#include "twi_state_machine.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/twi.h>

#include "i2c_register.h"

ISR(TWI_vect) {
  // Prevent gcc from generating inefficient 16-bit compare.
  unsigned char status = TW_STATUS;
  static unsigned char selected_register = 0;
  static unsigned char got_register = 0;

  switch (status) {
    case TW_BUS_ERROR:
      TWCR |= _BV(TWINT) | _BV(TWSTO);
      break;

    case TW_SR_SLA_ACK:
    case TW_SR_ARB_LOST_SLA_ACK:
    case TW_SR_GCALL_ACK:
    case TW_SR_ARB_LOST_GCALL_ACK:
      // Somebody is sending stuff to us.
      got_register = 0;
      TWCR = (TWCR | _BV(TWINT) | _BV(TWEA)) & (~(_BV(TWSTO)));
      break;

    case TW_SR_DATA_ACK:
    case TW_SR_GCALL_DATA_ACK:
      // These following two should never happen because we don't send nack.
    case TW_SR_DATA_NACK:
    case TW_SR_GCALL_DATA_NACK:
      // Data sent to us
      if (!got_register) {
        selected_register = TWDR;
        got_register = 1;
      }
      else
        set_i2c_reg(selected_register++, TWDR);

      // Always ack further bytes.
      TWCR = (TWCR | _BV(TWINT) | _BV(TWEA)) & (~(_BV(TWSTO)));
      break;

    case TW_SR_STOP:
      // Get ready for next command.
      TWCR = (TWCR | _BV(TWINT) | _BV(TWEA)) & (~(_BV(TWSTA) | _BV(TWSTO)));
      break;

    case TW_ST_SLA_ACK:
    case TW_ST_ARB_LOST_SLA_ACK:
    case TW_ST_DATA_ACK:
      // Somebody wants to read from us.
      // We always send as much data as the device requests.
      TWDR = provide_i2c_reg(selected_register++);
      TWCR = (TWCR | _BV(TWINT) | _BV(TWEA)) & (~(_BV(TWSTO)));
      break;

    case TW_ST_DATA_NACK:
    case TW_ST_LAST_DATA:
      // Master wants us to shut up now.
      TWCR = (TWCR | _BV(TWINT) | _BV(TWEA)) & (~(_BV(TWSTA) | _BV(TWSTO)));
      break;

    case TW_NO_INFO:
    default:
      // Should be unreachable.
      break;
  }
}

void init_i2c(unsigned char addr) {
  TWAR = addr;  // i2c address (general call not enabled)
  TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE);  // enable i2c
}
