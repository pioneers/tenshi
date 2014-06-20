// Licensed to Pioneers in Engineering under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  Pioneers in Engineering licenses
// this file to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
//  with the License.  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License

#define F_CPU 8000000  // AVR clock frequency in Hz, used by util/delay.h
#define SMART_BAUD 19200  // Smartsensor baud rate
#define SMART_ID_LEN 8   // Length of smartsensor personal ID

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdlib.h>
#include <stdint.h>
// #include <util/delay.h>

#include "inc/smartsensor/cobs.h"


// **** MACROS ****
#define TOGGLE_IN0 PORTC = PORTC ^ (1 << PC1);  // toggle IN0
#define TOGGLE_IN1 PORTB = PORTB ^ (1 << PB3);  // toggle IN1
#define TOGGLE_IN2 PORTB = PORTB ^ (1 << PB1);  // toggle IN2
#define TOGGLE_IN3 PORTB = PORTB ^ (1 << PB2);  // toggle IN3


// ****Forward declaration of functions****
void enumerationPacket(uint8_t type, uint8_t *data, uint8_t len);
void enumerationEnter(uint8_t *data, uint8_t len);
void enumerationExit(uint8_t *data, uint8_t len);
void enumerationReset(uint8_t *data, uint8_t len);
void enumerationSelect(uint8_t *data, uint8_t len);
uint8_t enumerating = 0;

void uart_init(uint32_t BUADRATE);
void USART_Transmit_Start();
void USART_Transmit_Stop();
void USART_Transmit(uint8_t data);
void serialPrint(uint8_t *StringOfCharacters, size_t len);
void LF();
void CR();
void init_sensor_io();
uint8_t compare_ID(uint8_t *buffer);
uint8_t make_flags(uint8_t canIn, uint8_t pushPull, uint8_t openDr,
  uint8_t outAL);
void decode_active_packet(uint8_t *data, size_t len);
void receive_and_echo();  // TODO(tobinsarah): implement working r&e

// ****Sensor Personal Data*** // to be a struct later.
uint8_t smartID[8] = {0, 1, 2, 3, 4, 5, 0x42, 7};
// Only uses last three bits
uint8_t my_frame = 0x14 & 0x7;  // TODO(tobinsarah): allow for multiple frames
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
  // char data[] = {0, 4, 1, 2};
  rxBufferFlag = 0;
  rxBufferIndex = 0;
  dataLen = 0;

  init_sensor_io();

  uart_init(SMART_BAUD);

  while (1) {
    if (rxBufferFlag == 0xFE) {  // if done receiving
      rxBufferFlag++;
      if (dataLen >= 1) {
        cobs_decode(decodedBuffer, rxBuffer, dataLen);
      }
      if (packetType == 0xFE) {  // if pingpong type
        if (compare_ID(decodedBuffer)) {
        encodedBuffer[0] = 0x00;
        encodedBuffer[1] = 0xFE;
        encodedBuffer[2] = dataLen - 5;
        cobs_encode(encodedBuffer+3, decodedBuffer + 8, dataLen - 9);
        serialPrint(encodedBuffer, (dataLen - 5));
        }
      } else if (packetType < 0x80 && (packetType & 0x7) == my_frame) {
        // if active
        // read from controller (Digital Out)
        if (dataLen >= 2) decode_active_packet(decodedBuffer, dataLen-1);

        // reply to controller
        encodedBuffer[0] = 0x00;
        encodedBuffer[1] = 4;  // reply len: hardcoded for DIO
        decodedBuffer[0] = ((!!(PINC & (1 << PINC1))) << 0);
        // make_flags(1, 0, 0, 0);
        // decodedBuffer[1] = (uint8_t) (sample_rate);
        // decodedBuffer[2] = (uint8_t) (sample_rate >> 8);
        cobs_encode(encodedBuffer + 2, decodedBuffer, 1);
        // serialPrint(encodedBuffer, encodedBuffer[1]);
      } else if (packetType <= 0xFD && packetType >= 0xF0) {
        // if (dataLen >= 2) {
          // cobs_decode(decodedBuffer, rxBuffer, dataLen);
          enumerationPacket(packetType, decodedBuffer, dataLen-1);
        // } else {
        //   enumerationPacket(packetType, 0, NULL);
        // }
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

  // Set nRX enable and TX enable to outputs
  PORTA &= ~((1 << PA5) | (1 << PA6));  // Set PA5 and PA6 to 0
  DDRA |= (1 << DDRA5) | (1 << DDRA6);

  sei();
}

void USART_Transmit_Start() {
  PORTA |= (1 << PA6);  // Turn PA6 on
}
void USART_Transmit_Stop() {
  PORTA &= ~(1 << PA6);  // Turn PA6 off
}

void USART_Transmit(uint8_t data) {  // and toggle
  // Wait for empty transmit buffer
  while (!(UCSR0A & (1 << UDRE0))) {  // TODO(tobinsarah): interruptTx
    // TOGGLE_IN1;
  }
  // Put data into buffer to send the data.
  UDR0 = data;
}


// Print an array of chars instead of just one char.
void serialPrint(uint8_t *StringOfCharacters, size_t len) {
  USART_Transmit_Start();
  USART_Transmit(0);
  for (size_t i = 0; i < len; i++) {
    USART_Transmit(*StringOfCharacters++);
  }
  while (!(UCSR0A & (1 << UDRE0))) {}
  USART_Transmit(0);
  while (!(UCSR0A & (1 << UDRE0))) {}
  USART_Transmit_Stop();
}

// TODO(tobinsarah): delete LF() and CR() if not using them
// functions that transmit Line Feed and Carriage Return
void LF() {
  USART_Transmit_Start();
  USART_Transmit(0x0A);
  USART_Transmit_Stop();
}
void CR() {
  USART_Transmit_Start();
  USART_Transmit(0x0D);
  USART_Transmit_Stop();
}

// TODO(tobinsarah): allow configuration as input/output for each relevant pin
// DDRxn: 1 --> output, 0 --> input
void init_sensor_io() {
  // portA not used by board, so this should do nothing.
  PORTA |= (0 << PA0) | (0 << PA1) | (0 << PA2);
  DDRA |= (1 << DDRA0) | (1 << DDRA1) | (1 << DDRA2);

  // B3 should be IN1
  PORTB = (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3);
  DDRB = (1 << DDRB0) | (1 << DDRB1) | (1 << DDRB2) | (1 << DDRB3);

  // C1 sould be IN0
  PORTC = (0 << PC0) | (0 << PC1) | (0 << PC2);
  DDRC = (0 << DDRC0) | (0 << DDRC1) | (0 << DDRC2);

  // uart_init(SMART_BAUD);
}

// For ping-pong packet, check if its my ID.
uint8_t compare_ID(uint8_t *buffer) {
  uint8_t val = 1;
  for (uint8_t i = 0; i < SMART_ID_LEN; i++) {
    val &= (smartID[i] == buffer[i]);
  }
  return val;
}

// assemble flags byte for DIO
//   bit0 of flags -- can input
//   bit1 of flags -- input is active low
//   bit2 of flags -- can output push-pull
//   bit3 of flags -- can output open-drain
//   bit4 of flags -- output is active low
uint8_t make_flags(uint8_t canIn, uint8_t pushPull, uint8_t openDr,
  uint8_t outAL) {
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
void decode_active_packet(uint8_t *data, size_t len) {
  if (len < 1) return;
  // currently assuming digital
  PORTC = (PORTC & ~(1 << PC1)) | ((!!(data[0] & 1)) << PC1);  // UPDATE IN0
  PORTB = (PORTB & ~(1 << PB3)) | ((!!(data[0] & 2)) << PB3);  // UPDATE IN1
  PORTB = (PORTB & ~(1 << PB1)) | ((!!(data[0] & 4)) << PB1);  // UPDATE IN2
  PORTB = (PORTB & ~(1 << PB2)) | ((!!(data[0] & 8)) << PB2);  // UPDATE IN3
}

// interrupt for recieve and echo one byte.
ISR(USART0__RX_vect) {
  // Fetch the received byte value into the variable "ByteReceived"
  receivedByte = UDR0;
  if (rxBufferFlag == 0xFE || rxBufferFlag == 0xFF) {
    return;
  } else if (receivedByte == 0x00) {  // if r 0x00 (whenever in Rmode)
    rxBufferFlag = 0x01;
  } else if (rxBufferFlag == 0x00) {
    // Nothing
  } else if (rxBufferFlag == 0x01) {  // looking for type
    packetType = receivedByte;
    rxBufferFlag++;
    // TODO(tobinsarah): proper logic for checking if its my frame
    if (packetType < 0x80) {  // if active packet
      // If not allowed frame and subchunk #s ???????
      if (((packetType & 0x7) ^ my_frame)) {
        rxBufferFlag = 0;
      }
    }
  } else if (rxBufferFlag == 0x02) {  // looking for len
    dataLen = receivedByte;
    if (receivedByte < 0x80) {  // if active packet
      dataLen &= 0x7F;  // remove in-band signalling from packet len
    }
    if (dataLen < 3) {
      // TODO(tobinsarah): deal with too short packets;
      rxBufferFlag = 0;
    } else if (dataLen == 3) {
      dataLen -= 3;
      rxBufferFlag = 0xFE;  // set to "done receiving" mode
    } else {
      dataLen -= 3;
      rxBufferIndex = 0;
      rxBufferFlag++;
    }
  } else if (rxBufferFlag == 0x03) {
    rxBuffer[rxBufferIndex] = receivedByte;
    rxBufferIndex++;
    // TOGGLE_IN2
    if (rxBufferIndex >= dataLen) {
      // TODO(tobinsarah): deal with dataLen <= 3
      // TODO(tobinsarah): cobs encode in place so you can use one less buffer
      // TOGGLE_IN1;
      rxBufferFlag = 0xFE;
    }
  }
}




void enumerationPacket(uint8_t type, uint8_t *data, uint8_t len) {
  switch (type) {
    case 0xF0: enumerationEnter(data, len); break;
    case 0xF1: enumerationExit(data, len); break;
    case 0xF2: enumerationSelect(data, len); break;
    case 0xF3: enumerationReset(data, len); break;
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
    USART_Transmit_Start();
    UCSR0B &= ~(1 << TXEN0);  // Disable UART transmiter
    PORTB &= ~(1 << PB0);     // Drive TX pin low
  }
}
void enumerationExit(uint8_t *data, uint8_t len) {
  if (!enumerating) return;
  enumerating = 0;
  USART_Transmit_Stop();    // Stop driving the bus
  UCSR0B &= ~(1 << TXEN0);  // Enable UART transmiter
}
void enumerationReset(uint8_t *data, uint8_t len) {
  if (!enumerating) return;
  USART_Transmit_Start();
  UCSR0B &= ~(1 << TXEN0);  // Disable UART transmiter
  PORTB &= ~(1 << PB0);     // Drive TX pin low
}
void enumerationSelect(uint8_t *data, uint8_t len) {
  if (!enumerating) return;
  if (len < 2*SMART_ID_LEN) return;
  for (uint8_t i = 0; i < SMART_ID_LEN; ++i) {
    if ((data[i] ^ smartID[i]) & data[i+SMART_ID_LEN]) return;  // Doesn't match
  }
  USART_Transmit_Stop();    // Stop driving the bus
  UCSR0B &= ~(1 << TXEN0);  // Enable UART transmiter
}

