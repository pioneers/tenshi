#include <avr/io.h>
#include <stdint.h>

int main() {
  // Init IO so that UART0 TX and SPI MISO are set to outputs and everything
  // else is set to input
  DDRA = 0b00000000;
  DDRB = 0b00000101;
  DDRC = 0b00000000;

  // Init UART0 as 1Mbaud 8n1
  UCSR0A = _BV(U2X0);
  UCSR0B = _BV(TXEN0) | _BV(RXEN0);
  UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
  UCSR0D = 0;

  // Init USI as SPI slave
  USICR = _BV(USIWM0) | _BV(USICS1);

  while (1) {
    // Shuffle from SPI to UART
    uint8_t spi_to_send = 0x00;
    uint8_t spi_rx = 0x00;
    uint8_t spi_needs_escape = 0;
    uint8_t uart_escape_cache = 0;
    if (USISR & _BV(USIOIF)) {
      spi_rx = USIDR;
      USIDR = spi_to_send;
      if (spi_needs_escape) {
        // Byte after escape
        // Need to write to UART
        while (!(UCSR0A & _BV(UDRE0))) {}
        UDR0 = spi_rx - 1;
      } else {
        // Not handling byte after escape
        if (spi_rx != 0x00) {
          if (spi_rx == 0x01) {
            spi_needs_escape = 1;
          } else {
            // Need to write to UART
            while (!(UCSR0A & _BV(UDRE0))) {}
            UDR0 = spi_rx;
          }
        }
      }
    }

    if (uart_escape_cache) {
      spi_to_send = uart_escape_cache + 1;
      uart_escape_cache = 0;
    } else {
      // Shuffle from UART to SPI
      if (UCSR0A & _BV(RXC0)) {
        uint8_t byte = UDR0;
        if (byte == 0x00 || byte == 0x01) {
          // Need escape
          uart_escape_cache = byte;
          spi_to_send = 0x01;
        } else {
          // No need to escape
          spi_to_send = byte;
        }
      }
    }
  }
}
