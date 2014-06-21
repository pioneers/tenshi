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
#define SMART_BAUD 9600UL  // Smartsensor baud rate

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdlib.h>

// ****Forward declaration of functions****
void uart_init (long BUADRATE);
void USART_Transmit( unsigned char data );
void serialPrint(char *StringOfCharacters, size_t len);
void LF();
void CR();
void init_sensor_io();
void receive_and_echo(); // TODO(tobinsarah): implement working r&e code in this fn

// ****Global vars
int volatile a;
// receivedByte
char volatile receivedByte;
char rxBuffer[255];
// rxBufferFlag = n: n = [0,255] means expecting to read nth byte of packet
// n = 0: main is done, 0xFF: main is processing, 0xFE: rx is done
int volatile rxBufferFlag;
int volatile rxBufferIndex;
int volatile packetType;
int volatile dataLen;

// Main
int main() {
  a = 0;
  rxBufferFlag = 0;
  rxBufferIndex = 0;
  dataLen = 0;

  init_sensor_io();

  uart_init(SMART_BAUD);

  while (1) {
    if (rxBufferFlag == 0xFE) {  // if done receiving
      rxBufferFlag++;
      if (packetType == 0xFE) {  // if pingpong type
        serialPrint(rxBuffer, (size_t) (dataLen));
      }
      rxBufferFlag = 0;  // should be below if statemt
    }
  }
  return 1;
}


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

  uart_init(SMART_BAUD);
}


// interrupt for recieve and echo one byte.
ISR(USART0__RX_vect) {
  // Fetch the received byte value into the variable "ByteReceived"
  receivedByte = UDR0;
  if (rxBufferFlag == 0xFE || rxBufferFlag == 0xFF) {
    PORTC = PORTC ^ (1 << PC1); // toggle IN0
    return;
  }
  if (receivedByte == 0x00) { // if see zerobyte (any time when receinving)
      rxBufferFlag = 0x01;
  } else if (rxBufferFlag == 0x01 ) { // looking for type
    packetType = receivedByte;
    rxBufferFlag++;
  } else if (rxBufferFlag == 0x02) {  // looking for len
    PORTB = PORTB ^ (1 << PB2); // toggle IN3
    dataLen = receivedByte - 3;
    if (dataLen < 0); // TODO(tobinsarah): deal with too short packets;
    if (dataLen == 0) {
      rxBufferFlag = 0xFE;
    } else {
      rxBufferIndex = 0;
      rxBufferFlag++;
    }
  } else if (rxBufferIndex < dataLen - 1) { // TODO(tobinsarah): deal with dataLen <= 3
    PORTB = PORTB ^ (1 << PB1); // toggle IN2
    rxBuffer[rxBufferIndex] = receivedByte;
    rxBufferFlag++;
    rxBufferIndex++;
  } else {
    PORTB = PORTB ^ (1 << PB3); // toggle IN1
    rxBuffer[rxBufferIndex] = receivedByte;
    rxBufferFlag = 0xFE;
  }
}
