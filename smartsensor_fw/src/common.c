
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
// #include <util/delay.h>

#include "inc/common.h"


// ****Sensor Personal Data*** // to be a struct later.
uint8_t smartID[SMART_ID_LEN] = {SMART_ID};
// Only uses last three bits
uint8_t my_frame = 0x11 & 0x7;  // TODO(tobinsarah): allow for multiple frames
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

  DIGITAL_PULLUP_ON(UART0_RX);

  sei();  // Enable interrupts
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

// TODO(tobinsarah): proper logic for checking if its my frame
int isMyChunk(uint8_t frameByte) {
  return !((frameByte & 0x7) ^ my_frame);
}

