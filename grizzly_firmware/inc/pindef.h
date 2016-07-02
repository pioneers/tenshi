// Licensed to Pioneers in Engineering under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Pioneers in Engineering licenses
// this file to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
//  with the License.  You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License

#ifndef INC_PINDEF_H_
#define INC_PINDEF_H_

#define BOARD_REVISION          'D'

// The following macros are used for pin abstractions.
// The ones with two underscores should not be directly.
#define __IO(x, y)              P##x##y
#define __DDR(x, y)             DDR##x
#define __PIN(x, y)             PIN##x
#define __PORT(x, y)            PORT##x

// The following macros will expand to Pxn/DDRx/PINx/PORTx defines.
// They should be passed one of th pin definitions PINDEF_x.
// The extra indirection is necessary because macros will expand their
// arguments. This is needed to expand the PINDEF_x macros into two arguments
// for the __x macros
#define IO(x)                   __IO(x)
#define DDR(x)                  __DDR(x)
#define PIN(x)                  __PIN(x)
#define PORT(x)                 __PORT(x)

// dip switch address select
#define PINDEF_DIP1             F, 1
#define PINDEF_DIP2             F, 4
#define PINDEF_DIP3             F, 5
#if BOARD_REVISION < 'D'
#define PINDEF_DIP4             D, 3
#else
#define PINDEF_DIP4             B, 7
#endif
#define PINDEF_DIP5             F, 6
#define PINDEF_DIP6             F, 7

// rotary encoder
// These are REQUIRED to be on the same port bank
#define PINDEF_ENCA             B, 0
#define PINDEF_ENCB             B, 4

// high side driver endable
#define PINDEF_HIGHSIDEENABLE   E, 2
// high side driver fault (active low)
#define PINDEF_HIGHSIDEFAULT    D, 7

#define PINDEF_LEDGRN           D, 6
#define PINDEF_LEDRED           D, 4



// ////////////////////////////
// Smart sensor pin definitions
#include "inc/smartsensor/pinutil.h"

#define F_CPU 16000000  // AVR clock frequency in Hz, used by util/delay.h, etc.

// Different pin definition for ATMega32U4
// DDR is now DD
// Pull ups are controlled by the PORT register
#define PIN_DEFINITION(PORT_LETTER, PIN_NUMBER) \
          PORT##PORT_LETTER, P##PORT_LETTER##PIN_NUMBER, \
          PIN##PORT_LETTER, PIN##PORT_LETTER##PIN_NUMBER, \
          DDR##PORT_LETTER, DD##PORT_LETTER##PIN_NUMBER, \
          PORT##PORT_LETTER, P##PORT_LETTER##PIN_NUMBER

// Pin definitions
#define SS_UART_TX   PIN_DEFINITION(D, 3)
#define SS_UART_RX   PIN_DEFINITION(D, 2)
#define SS_UART_TXE  PIN_DEFINITION(E, 6)
#define SS_UART_nRXE PIN_DEFINITION(D, 5)  // Inverted RX enable pin

// Smart sensor bus UART registers
#define SS_UCSRnA UCSR1A
#define SS_U2Xn U2X1
#define SS_UBRRnH UBRR1H
#define SS_UBRRnL UBRR1L
#define SS_UCSRnB UCSR1B
#define SS_RXENn RXEN1
#define SS_TXENn TXEN1
#define SS_RXCIEn RXCIE1
#define SS_UCSRnC UCSR1C
#define SS_USBSn USBS1
#define SS_UCSZn0 UCSZ10
#define SS_UCSZn1 UCSZ11
#define SS_UCSRnA UCSR1A
#define SS_UDREn UDRE1
#define SS_UDRn UDR1
#define SS_USARTn__RX_vect USART1_RX_vect


#endif  // INC_PINDEF_H_
