
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
// #include <util/delay.h>

#include "inc/common.h"


// ****Sensor Personal Data*** // to be a struct later.
uint8_t smartID[SMART_ID_LEN] = {SMART_ID};
// Given a sample and a frame number, if the byte at index <frame> contains a
// one at the bit at position <sample>,
uint8_t my_frames[SS_NUM_FRAMES] = {0};
uint32_t sample_rate = 0x0100;  // hardcoded for now;



extern void uart_init(uint32_t BUADRATE);
extern void USART_Transmit_Start();
extern void USART_Transmit_Stop();
extern void USART_Transmit(uint8_t data);
extern void serialPrint(uint8_t *StringOfCharacters, size_t len);
static void LF();
static void CR();
extern int isMyChunk(uint8_t frameByte);




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
