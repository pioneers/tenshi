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

#define F_CPU 8000000UL  // AVR clock frequency in Hz, used by util/delay.h
#define SMART_BAUD 1000000UL  // Smartsensor baud rate

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdlib.h>
#include <util/delay.h>

// ****Forward declaration of functions****
void uart_init (long BUADRATE);
void USART_Transmit( unsigned char data );
void serialPrint(char *StringOfCharacters, size_t len);
void LF();
void CR();
void init_sensor_io();
void toggle_sensor_io();


// function to initialize UART
void uart_init (long Desired_Baudrate)
{
  // If its not already set, calculate the baud rate dynamically,
  // based on current microcontroller speed and user specified
  // desired baudrate.
  #ifndef UBBR
  #define UBBR ((F_CPU)/(Desired_Baudrate*8UL)-1)
  #endif

  // Set to double speed mode.
  UCSR0A = UCSR0A | (1 << U2X0);

  //Set baud rate.
  UBRR0H = (unsigned char)(UBBR>>8);
  UBRR0L = (unsigned char)UBBR;
  //Enable receiver and transmitter
  UCSR0B = (1<<RXEN0)|(1<<TXEN0);
  //Set frame format: 8data, 1 stop bit
  UCSR0C = (1<<USBS0)|(1<<UCSZ00)|(1<<UCSZ01);   // 8bit data format
}

void USART_Transmit( unsigned char data ) // and toggle
{
  // Wait for empty transmit buffer
  while (!(UCSR0A & (1 << UDRE0))) {}  // TODO(tobinsarah): interruptTx

  // Put data into buffer to send the data.
  UDR0 = data;
}


// Print an array of chars instead of just one char.
void serialPrint(char *StringOfCharacters, size_t len)
{
  for (size_t i = 0; i < len; i++){
    USART_Transmit(*StringOfCharacters++);
  }
}

// TODO(tobinsarah): delete LF() and CR() if not using them
// functions that transmit Line Feed and Carriage Return
void LF() {USART_Transmit(0x0A);}
void CR() {USART_Transmit(0x0D);}

void init_sensor_io() {
  // portA not used by board, so this should do nothing.
  PORTA = (0 << PA0) | (0 << PA1) | (0 << PA2);
  DDRA = (1 << DDRA0) | (1 << DDRA1) | (1 << DDRA2);

  // B3 should be IN1
  PORTB = (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3);
  DDRB = (1 << DDRB0) | (1 << DDRB1) | (1 << DDRB2) | (1 << DDRB3);

  // C1 sould be IN0
  PORTC = (0 << PC0) | (0 << PC1) | (0 << PC2);
  DDRC = (1 << DDRC0) | (1 << DDRC1) | (1 << DDRC2);
}

//toggles one (soon to be more?) sensor
void toggle_sensor_io() {
  int x = 1;
  while (x) {
    PORTB = PORTB ^ (1 << PB3);
  }
}


// Main
int main(){
  // portA not used right now so this should do nothing
  PORTA = (0 << PA0) | (0 << PA1) | (0 << PA2);
  DDRA = (1 << DDRA0) | (1 << DDRA1) | (1 << DDRA2);

  // B3 should be IN1
  PORTB = (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3);
  DDRB = (1 << DDRB0) | (1 << DDRB1) | (1 << DDRB2) | (1 << DDRB3);

  // C1 sould be IN0
  PORTC = (0 << PC0) | (0 << PC1) | (0 << PC2);
  DDRC = (1 << DDRC0) | (1 << DDRC1) | (1 << DDRC2);

  uart_init(SMART_BAUD);

  char data[] = {0, 4, 1, 2};

  while(1) {
    serialPrint(data, 4);
  }
  return 1;
}
