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

#define F_CPU 8000000  // AVR clock frequency in Hz, used by util/delay.h
#define SMART_BAUD 9600  // Smartsensor baud rate
#define SMART_ID_LEN 8   // Length of smartsensor personal ID

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdlib.h>
#include <stdint.h>

#include "inc/smartsensor/cobs.h"


// **** MACROS ****
#define TOGGLE_IN0 PORTC = PORTC ^ (1 << PC1);  // toggle IN0
#define TOGGLE_IN1 PORTB = PORTB ^ (1 << PB3);  // toggle IN1
#define TOGGLE_IN2 PORTB = PORTB ^ (1 << PB1);  // toggle IN2
#define TOGGLE_IN3 PORTB = PORTB ^ (1 << PB2);  // toggle IN3


// ****Forward declaration of functions****
void uart_init(uint32_t BUADRATE);
void USART_Transmit(uint8_t data);
void serialPrint(uint8_t *StringOfCharacters, size_t len);
void LF();
void CR();
void init_sensor_io();
uint8_t compare_ID();
uint8_t make_flags(int canIn, int pushPull, int openDr, int outAL);
void decode_active_packet(uint8_t data, size_t len);
void receive_and_echo();  // TODO(tobinsarah): implement working r&e

// ****Sensor Personal Data*** // to be a struct later.
uint8_t smartID[8] = {0, 1, 2, 3, 4, 5, 0x42, 7};
uint8_t my_frame = 0x11;  // TODO(tobinsarah): allow for multiple frames
uint32_t sample_rate = 0x0100;  // hardcoded for now;


// ****Global vars****
uint8_t volatile receivedByte;
uint8_t rxBuffer[255];
uint8_t decodedBuffer[255];
uint8_t encodedBuffer[255];  // TODO(tobinsarah): don't run out of SRAM!!

// rxBufferFlag = n: n = [0,255] means expecting to read nth byte of packet
// n = 0: main is done, 0xFF: main is processing, 0xFE: rx is done
int volatile rxBufferFlag;
uint8_t in_band_sigFlag;
int volatile rxBufferIndex;
int volatile packetType;
uint8_t volatile dataLen;

// Main
int main() {
  rxBufferFlag = 0;
  rxBufferIndex = 0;
  dataLen = 0;

  init_sensor_io();

  uart_init(SMART_BAUD);

  while (1) {
    if (rxBufferFlag == 0xFE) {  // if done receiving
      rxBufferFlag++;
      if (packetType == 0xFE) {  // if pingpong type
        cobs_decode(decodedBuffer, rxBuffer, dataLen);
        if (compare_ID()) {
          encodedBuffer[0] = 0x00;
          encodedBuffer[1] = 0xFE;
          encodedBuffer[2] = dataLen - 5;
          cobs_encode(encodedBuffer+3, decodedBuffer + 8, dataLen - 9);
          serialPrint(encodedBuffer, (dataLen - 5));
        }
      } else if (packetType < 0x80 && packetType == my_frame) {  // if active
        // read from controller (Digital Out)
        decode_active_packet(rxBuffer, dataLen);

        // reply to controller
        encodedBuffer[0] = 0x00;
        encodedBuffer[1] = 0x06;  // reply len: hardcoded for DIO
        decodedBuffer[0] = make_flags(1, 0, 0, 0);
        decodedBuffer[1] = (uint8_t) (sample_rate);
        decodedBuffer[2] = (uint8_t) (sample_rate >> 8);
        cobs_encode(encodedBuffer + 2, decodedBuffer, 3);
        serialPrint(encodedBuffer, 6);
      }
      rxBufferFlag = 0;  // should be below if statemt
    }
  }
  return 1;
}


// function to initialize UART
void uart_init(uint32_t Desired_Baudrate) {
  // If its not already set, calculate the baud rate dynamically,
  // based on current microcontroller speed and user specified
  // desired baudrate.
  #ifndef UBBR
  #define UBBR ((F_CPU)/(Desired_Baudrate*8UL)-1)
  #endif

  // Set to double speed mode.
  UCSR0A = UCSR0A | (1 << U2X0);

  // Set baud rate.
  UBRR0H = (uint8_t)(UBBR >> 8);
  UBRR0L = (uint8_t)UBBR;
  // Enable receiver, transmitter, and RxComplete interrupt
  UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
  // Set frame format: 8data, 1 stop bit
  UCSR0C = (1 << USBS0) | (1 << UCSZ00) | (1 << UCSZ01);
  // Enable the Global Interrupt Enbl flag to process interrupts
  sei();
}

void USART_Transmit(uint8_t data) {  // and toggle
  // Wait for empty transmit buffer
  while (!(UCSR0A & (1 << UDRE0))) {}  // TODO(tobinsarah): interruptTx

  // Put data into buffer to send the data.
  UDR0 = data;
}


// Print an array of chars instead of just one char.
void serialPrint(uint8_t *StringOfCharacters, size_t len) {
  for (size_t i = 0; i < len; i++) {
    USART_Transmit(*StringOfCharacters++);
  }
}

// TODO(tobinsarah): delete LF() and CR() if not using them
// functions that transmit Line Feed and Carriage Return
void LF() {USART_Transmit(0x0A);}
void CR() {USART_Transmit(0x0D);}

// TODO(tobinsarah): allow configuration as input/output for each relevant pin
void init_sensor_io() {
  // portA not used by board, so this should do nothing.
  PORTA = (0 << PA0) | (0 << PA1) | (0 << PA2);
  DDRA = (1 << DDRA0) | (1 << DDRA1) | (1 << DDRA2);

  // B3 should be IN1
  PORTB = (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3);
  DDRB = (1 << DDRB0) | (1 << DDRB1) | (1 << DDRB2) | (1 << DDRB3);

  // C1 sould be IN0
  PORTC = (0 << PC0) | (0 << PC1) | (0 << PC2);
  DDRC = (1 << DDRC0) | (1 << DDRC2);

  uart_init(SMART_BAUD);
}

// For ping-pong packet, check if its my ID.
uint8_t compare_ID() {
  uint8_t val = 1;
  for (uint8_t i; i < SMART_ID_LEN; i ++) {
    val &= (smartID[i] == decodedBuffer[i]);
  }
  return val;
}

// assemble flags byte for DIO
//   bit0 of flags -- can input
//   bit1 of flags -- input is active low
//   bit2 of flags -- can output push-pull
//   bit3 of flags -- can output open-drain
//   bit4 of flags -- output is active low
uint8_t make_flags(int canIn, int pushPull, int openDr, int outAL) {
  uint8_t flagbyte = (uint8_t) (canIn & 1);
  flagbyte |= ((!!(PINC & (1 << PINC1))) << 1);
  flagbyte |= ((pushPull & 1) << 2);
  flagbyte |= ((openDr & 1) << 3);
  flagbyte |= ((outAL & 1) << 4);
  return flagbyte;
}

// assemble flags byte for DIO
//   bit0 of flags -- can input
//   bit1 of flags -- input is active low
//   bit2 of flags -- can output push-pull
//   bit3 of flags -- can output open-drain
//   bit4 of flags -- output is active low
void decode_active_packet(uint8_t data, size_t len) {
  // currently assuming digital
    PORTC |= (!!(data & 0x01)) << PC1; // UPDATE IN0
    PORTB |= (!!(data & 0x01)) << PB3; // UPDATE IN1
    PORTB |= (!!(data & 0x01)) << PB1; // UPDATE IN2
    PORTB |= (!!(data & 0x01)) << PB2; // UPDATE IN3
}

// interrupt for recieve and echo one byte.
ISR(USART0__RX_vect) {
  // Fetch the received byte value into the variable "ByteReceived"
  receivedByte = UDR0;
  if (rxBufferFlag == 0xFE || rxBufferFlag == 0xFF) {
    TOGGLE_IN0;
    return;
  } else if (receivedByte == 0x00) {  // if r 0x00 (whenever in Rmode)
    rxBufferFlag = 0x01;
  } else if (rxBufferFlag == 0x01) {  // looking for type
    packetType = receivedByte;
    rxBufferFlag++;
    // TODO(tobinsarah): proper logic for checking if its my frame
    if (packetType < 0x80) {  // if active packet
      // If not allowed frame and subchunk #s ???????
      if (!((packetType & 0x3F) ^ my_frame)) {
        rxBufferFlag = 0;
      }
    }
  } else if (rxBufferFlag == 0x02) {  // looking for len
    dataLen = receivedByte;
    if (rxBufferFlag < 0x80) {  // if active packet
      dataLen |= 0x7F;  // remove in-band signalling from packet len
    } else if (dataLen < 3) {
      // TODO(tobinsarah): deal with too short packets;
    } else if (dataLen == 3) {
      rxBufferFlag = 0xFE;  // set to "done receiving" mode
    } else {
      dataLen -= 3;
      rxBufferIndex = 0;
      rxBufferFlag++;
    }
  } else if (rxBufferIndex < dataLen - 1) {
    // TODO(tobinsarah): deal with dataLen <= 3
    // TODO(tobinsarah): cobs encode in place so you can use one less buffer
    rxBuffer[rxBufferIndex] = receivedByte;
    rxBufferFlag++;
    rxBufferIndex++;
  } else {
    TOGGLE_IN1;
    rxBuffer[rxBufferIndex] = receivedByte;
    rxBufferFlag = 0xFE;
  }
}
