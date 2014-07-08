
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
// #include <util/delay.h>

#include "inc/smartsensor/cobs.h"
#include "inc/smartsensor/common.h"
#include "inc/smartsensor/ss_type.h"
#include "inc/smartsensor/enumeration.h"
#include "inc/smartsensor/maintenance.h"


// ****Sensor Personal Data*** // to be a struct later.
uint8_t smartID[SMART_ID_LEN] = {SMART_ID};
// Given a sample and a frame number, if the byte at index <frame> contains a
// one at the bit at position <sample>,
uint8_t my_frames[SS_NUM_FRAMES] = {0};
uint32_t sample_rate = 0x0100;  // hardcoded for now;






#define RX_FLAG_READY 0
#define RX_FLAG_GET_TYPE 1
#define RX_FLAG_GET_LENGTH 2
#define RX_FLAG_GET_DATA 3
#define RX_FLAG_DATA_AVAILABLE 0xFE
#define RX_FLAG_DATA_IN_USE 0xFF



void uart_init(uint32_t BUADRATE);
void USART_Transmit_Start();
void USART_Transmit_Stop();
void USART_Transmit(uint8_t data);
void serialPrint(uint8_t *StringOfCharacters, size_t len);
static void LF();
static void CR();
int isMyChunk(uint8_t frameByte);



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




void ssInit() {
  rxBufferFlag = RX_FLAG_READY;
  rxBufferIndex = 0;
  dataLen = 0;

  ssInitType();

  uart_init(SMART_BAUD);
}

void ssMainUpdate() {
  if (activeSendFlag) {
    uint8_t pacLen = 0;
    uint8_t inband = 0;
    ssActiveSend(decodedBuffer, &pacLen, &inband);
    // Send reply active packet
    if (pacLen > 0 && pacLen <= ACTIVE_PACKET_MAX_LEN) {
      encodedBuffer[0] = 0x00;
      encodedBuffer[1] = packetType;
      encodedBuffer[2] = ((!!inband) << 7) | (pacLen+4);
      cobs_encode(encodedBuffer+3, decodedBuffer, pacLen);
      serialPrint(encodedBuffer, pacLen+4);
    }
    activeSendFlag = 0;
  }
  if (rxBufferFlag == RX_FLAG_DATA_AVAILABLE) {  // if done receiving
    rxBufferFlag = RX_FLAG_DATA_IN_USE;
    if (dataLen >= 1) {
      cobs_decode(decodedBuffer, rxBuffer, dataLen);
    }
    if (packetType < 0x80) {
      ssActiveInRec(decodedBuffer, dataLen-1, in_band_sigFlag);
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




// Interrupt when recieving a byte from the smart sensor bus.
ISR(SS_USARTn__RX_vect) {
  // Fetch the received byte value into the variable "ByteReceived"
  uint8_t receivedByte = SS_UDRn;
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






// function to initialize UART
void uart_init(uint32_t Desired_Baudrate) {
  // If its not already set, calculate the baud rate dynamically,
  // based on current microcontroller speed and user specified
  // desired baudrate.
  #ifndef UBBR
  #define UBBR ((F_CPU)/(Desired_Baudrate*8UL)-1)
  #endif

  // Set to double speed mode.
  SS_UCSRnA |= (1 << SS_U2Xn);

  // Set baud rate.
  SS_UBRRnH = (uint8_t)(UBBR >> 8);
  SS_UBRRnL = (uint8_t)UBBR;
  // Enable receiver, transmitter, and RxComplete interrupt
  SS_UCSRnB = (1 << SS_RXENn) | (1 << SS_TXENn) | (1 << SS_RXCIEn);
  // Set frame format: 8data, 1 stop bit
  SS_UCSRnC = (1 << SS_USBSn) | (1 << SS_UCSZn0) | (1 << SS_UCSZn1);
  // Enable the Global Interrupt Enbl flag to process interrupts

  DIGITAL_SET_OUT(SS_UART_TXE);  // Set nRX enable and TX enable as outputs
  DIGITAL_SET_OUT(SS_UART_nRXE);
  DIGITAL_SET_LOW(SS_UART_TXE);  // Set nRX enable and TX enable to 0
  DIGITAL_SET_LOW(SS_UART_nRXE);  // Enable RX and disable TX

  DIGITAL_PULLUP_ON(SS_UART_RX);

  sei();  // Enable interrupts
}

void USART_Transmit_Start() {
  DIGITAL_SET_HIGH(SS_UART_TXE);  // Set TX enable to 1
}
void USART_Transmit_Stop() {
  DIGITAL_SET_LOW(SS_UART_TXE);  // Set TX enable to 1
}

void USART_Transmit(uint8_t data) {  // and toggle
  // Wait for empty transmit buffer
  while (!(SS_UCSRnA & (1 << SS_UDREn))) {}  // TODO(tobinsarah): interruptTx
  // Put data into buffer to send the data.
  SS_UDRn = data;
}


// Print an array of chars instead of just one char.
void serialPrint(uint8_t *StringOfCharacters, size_t len) {
  USART_Transmit_Start();
  for (size_t i = 0; i < len; i++) {
    USART_Transmit(*StringOfCharacters++);
  }
  // TODO(cduck): Properly wait for end of transmission
  USART_Transmit(0xFF);
  while (!(SS_UCSRnA & (1 << SS_UDREn))) {}
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

int isMyChunk(uint8_t frameByte) {
  uint8_t sample = (frameByte >> 3) & 0x7;
  // frame > SS_NUM_FRAMES if subtraction overflows.
  uint8_t frame = (frameByte & 0x07) - SS_FIRST_FRAME;
  return (frame < SS_NUM_FRAMES) && ((my_frames[frame] >> sample) & 1);
}
