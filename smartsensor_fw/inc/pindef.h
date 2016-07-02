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

#include "inc/smartsensor/pinutil.h"


#define F_CPU 8000000  // AVR clock frequency in Hz, used by util/delay.h, etc.

// Pin definitions
#define IN0        PIN_DEFINITION(C, 1)
#define IN1        PIN_DEFINITION(B, 1)
#define IN2        PIN_DEFINITION(B, 2)
#define IN3        PIN_DEFINITION(B, 3)
#define PWM0       PIN_DEFINITION(A, 6)
#define PWM1       PIN_DEFINITION(A, 5)
#define PWM2       PIN_DEFINITION(B, 3)
#define PWM3       PIN_DEFINITION(C, 0)
#define A_IN0      ((1 << MUX3) | (1 << MUX1))  // ADC 10
#define A_IN1      ((1 << MUX2) | (1 << MUX1))  // ADC 6
#define A_IN2      ((1 << MUX2) | (1 << MUX1) | (1 << MUX0))  // ADC 7
#define A_IN3      ((1 << MUX3))  // ADC 8


#define SS_UART_TX   PIN_DEFINITION(B, 0)
#define SS_UART_RX   PIN_DEFINITION(A, 7)
#define SS_UART_TXE  PIN_DEFINITION(A, 4)
#define SS_UART_nRXE PIN_DEFINITION(A, 3)  // Inverted RX enable pin

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
