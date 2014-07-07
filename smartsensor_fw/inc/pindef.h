
#ifndef INC_PINDEF_H_
#define INC_PINDEF_H_

#include "inc/smartsensor/pinutil.h"


#define F_CPU 8000000  // AVR clock frequency in Hz, used by util/delay.h, etc.

// Pin definitions
#define IN0        PIN_DEFINITION(C, 1)
#define IN1        PIN_DEFINITION(B, 3)
#define IN2        PIN_DEFINITION(B, 1)
#define IN3        PIN_DEFINITION(B, 2)

#define SS_UART_TX   PIN_DEFINITION(B, 0)
#define SS_UART_RX   PIN_DEFINITION(A, 7)
#define SS_UART_TXE  PIN_DEFINITION(A, 6)
#define SS_UART_nRXE PIN_DEFINITION(A, 5)  // Inverted RX enable pin

// Smart sensor bus UART registers
#define SS_UCSRnA UCSR0A
#define SS_U2Xn U2X0
#define SS_UBRRnH UBRR0H
#define SS_UBRRnL UBRR0L
#define SS_UCSRnB UCSR0B
#define SS_RXENn RXEN0
#define SS_TXENn TXEN0
#define SS_RXCIEn RXCIE0
#define SS_UCSRnC UCSR0C
#define SS_USBSn USBS0
#define SS_UCSZn0 UCSZ00
#define SS_UCSZn1 UCSZ01
#define SS_UCSRnA UCSR0A
#define SS_UDREn UDRE0
#define SS_UDRn UDR0
#define SS_USARTn__RX_vect USART0__RX_vect


#endif  // INC_PINDEF_H_
