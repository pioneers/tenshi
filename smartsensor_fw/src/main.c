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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
// #include <util/delay.h>

#include "inc/smartsensor/cobs.h"
#include "inc/common.h"
#include "inc/digital.h"
#include "inc/enumeration.h"
#include "inc/maintenance.h"



#define RX_FLAG_READY 0
#define RX_FLAG_GET_TYPE 1
#define RX_FLAG_GET_LENGTH 2
#define RX_FLAG_GET_DATA 3
#define RX_FLAG_DATA_AVAILABLE 0xFE
#define RX_FLAG_DATA_IN_USE 0xFF


// Defined in common.c

// Should not use these functions outside of main.c
extern void uart_init(uint32_t BUADRATE);
extern void USART_Transmit_Start();
extern void USART_Transmit_Stop();
extern void USART_Transmit(uint8_t data);
extern void serialPrint(uint8_t *StringOfCharacters, size_t len);
extern void LF();
extern void CR();
extern int isMyChunk(uint8_t frameByte);


// Private global vars
uint8_t rxBuffer[255];
uint8_t encodedBuffer[255];  // TODO(tobinsarah): don't run out of SRAM!!
uint8_t decodedBuffer[255];
uint8_t volatile rxBufferFlag;
uint8_t in_band_sigFlag;
uint8_t volatile rxBufferIndex;
uint8_t volatile packetType;
uint8_t volatile dataLen;

uint8_t activeSendFlag = 0;



// Main
int main() {
  rxBufferFlag = RX_FLAG_READY;
  rxBufferIndex = 0;
  dataLen = 0;

  switch (SENSOR_TYPE) {
    case SENSOR_TYPE_DIGITAL:
      initDigital();
      break;
    default: break;
    // TODO(cduck): Add more smart sensors types
  }

  uart_init(SMART_BAUD);

  while (1) {
    if (activeSendFlag) {
      uint8_t pacLen = 0;
      switch (SENSOR_TYPE == SENSOR_TYPE_DIGITAL) {
        case SENSOR_TYPE_DIGITAL:
          activeDigitalSend(decodedBuffer, &pacLen);
          break;
        default: break;
        // TODO(cduck): Add more smart sensors types
      }
      // Send reply active packet
      if (pacLen > 0 && pacLen <= ACTIVE_PACKET_MAX_LEN) {
        encodedBuffer[0] = 0x00;
        encodedBuffer[1] = pacLen+3;
        cobs_encode(encodedBuffer+2, decodedBuffer, pacLen);
        serialPrint(encodedBuffer, pacLen+3);
      }
      activeSendFlag = 0;
    }
    if (rxBufferFlag == RX_FLAG_DATA_AVAILABLE) {  // if done receiving
      rxBufferFlag = RX_FLAG_DATA_IN_USE;
      if (dataLen >= 1) {
        cobs_decode(decodedBuffer, rxBuffer, dataLen);
      }
      if (packetType < 0x80) {
        switch (SENSOR_TYPE == SENSOR_TYPE_DIGITAL) {
          case SENSOR_TYPE_DIGITAL:
            activeDigitalRec(decodedBuffer, dataLen-1, in_band_sigFlag);
            break;
          default: break;
          // TODO(cduck): Add more smart sensors types
        }
      } else if (packetType <= 0xFD && packetType >= 0xF0) {
        enumerationPacket(packetType, decodedBuffer, dataLen-1);
      } else {
        uint8_t pacLen = 0;
        maintenancePacket(packetType, decodedBuffer, dataLen-1,
          decodedBuffer, &pacLen);
        // Send reply maintenance packet
        if (pacLen > 0 && pacLen <= 0xFF-4) {
          encodedBuffer[0] = 0x00;
          encodedBuffer[1] = packetType;  // Respond with same type as sent.
          encodedBuffer[2] = pacLen+4;
          cobs_encode(encodedBuffer+3, decodedBuffer, pacLen);
          serialPrint(encodedBuffer, pacLen+4);
        }
      }
      rxBufferFlag = RX_FLAG_READY;  // should be below if statement
    }
  }
  return 1;
}
// Not used now
/*
void transmitMaintenancePacket(uint8_t type, uint8_t *data, uint8_t len) {
  if (len > 0xFF-4) return -1;
  cobs_encode(encodedBuffer+3, data, len);
  encodedBuffer[0] = 0;
  encodedBuffer[1] = type;
  encodedBuffer[2] = len+4;
  serialPrint(encodedBuffer, encodedBuffer[2]);
  return 0;
}
int transmitActivePacket(uint8_t *data, uint8_t len, uint8_t inband) {
  if (len > ACTIVE_PACKET_MAX_LEN) return -1;
  cobs_encode(encodedBuffer+2, data, len);
  encodedBuffer[0] = 0;
  encodedBuffer[1] = ((len+3)&7) | (!!inband << 7);
  serialPrint(encodedBuffer, len+3);
  return 0;
}
*/


// Interrupt when recieving a byte from the smart sensor bus.
ISR(USART0__RX_vect) {
  // Fetch the received byte value into the variable "ByteReceived"
  uint8_t receivedByte = UDR0;
  if (rxBufferFlag == RX_FLAG_DATA_AVAILABLE ||
      rxBufferFlag == RX_FLAG_DATA_IN_USE) {
    return;
  } else if (receivedByte == 0x00) {  // if r 0x00 (whenever in Rmode)
    rxBufferFlag = RX_FLAG_GET_TYPE;
  } else if (rxBufferFlag == RX_FLAG_READY) {
    // Nothing
  } else if (rxBufferFlag == RX_FLAG_GET_TYPE) {  // looking for type
    packetType = receivedByte;
    rxBufferFlag = RX_FLAG_GET_LENGTH;
    if (packetType < 0x80) {  // if active packet
      if (isMyChunk(packetType)) {
        activeSendFlag = 1;
      } else {
        // If not allocated frame and subchunk #s
        rxBufferFlag = RX_FLAG_READY;
      }
    }
  } else if (rxBufferFlag == RX_FLAG_GET_LENGTH) {  // looking for len
    dataLen = receivedByte;
    if (receivedByte < 0x80) {  // if active packet
      dataLen &= 0x7F;  // remove in-band signalling from packet len
    }
    if (dataLen < 3) {
      // TODO(tobinsarah): deal with too short packets;
      rxBufferFlag = RX_FLAG_READY;
    } else if (dataLen == 3) {
      dataLen -= 3;
      rxBufferFlag = RX_FLAG_DATA_AVAILABLE;  // set to "done receiving" mode
    } else {
      dataLen -= 3;
      rxBufferIndex = 0;
      rxBufferFlag = RX_FLAG_GET_DATA;
    }
  } else if (rxBufferFlag == RX_FLAG_GET_DATA) {
    rxBuffer[rxBufferIndex] = receivedByte;
    rxBufferIndex++;
    if (rxBufferIndex >= dataLen) {
      // TODO(tobinsarah): deal with dataLen <= 3
      // TODO(tobinsarah): cobs encode in place so you can use one less buffer
      rxBufferFlag = RX_FLAG_DATA_AVAILABLE;
    }
  }
}
