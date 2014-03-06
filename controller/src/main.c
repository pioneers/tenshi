#include <stdio.h>

#include "inc/FreeRTOS.h"
#include "inc/button_driver.h"
#include "inc/driver_glue.h"
#include "inc/i2c_master.h"
#include "inc/led_driver.h"
#include "inc/pindef.h"
#include "inc/stm32f4xx.h"
#include "inc/core_cm4.h"
#include "inc/core_cmInstr.h"
#include "inc/task.h"
#include "inc/xbee_framing.h"

#define PIER_INCOMINGDATA_IDENT   254
#define PIER_OUTGOINGDATA_IDENT   253

typedef struct __attribute__((packed)) tag_pier_incomingdata {
  uint8_t ident;
  uint8_t fieldtime;
  uint8_t flags;
  uint8_t analog[7];
  uint8_t digital;
} pier_incomingdata;

typedef struct __attribute__((packed)) tag_pier_outgoingdata {
  uint8_t ident;
  uint8_t rssi;
  uint8_t batteryStatus;
  uint8_t analog[7];
  uint8_t digital;
} pier_outgoingdata;

static portTASK_FUNCTION_PROTO(blinkTask, pvParameters) {
  uint8_t buf[64];

  uint8_t counter = 0;

  while (1) {
    size_t len;
    uint8_t *buf2;
    void *txn;
    // Blink green LED
    // GPIO_BANK(PINDEF_GREEN_LED)->ODR ^=
    //   (1 << GPIO_PIN(PINDEF_GREEN_LED));

    // debug_uart_tx("Hello World!\r\n", 14);

    // vTaskDelay(1000);

    /*if (button_driver_get_button_state(0) ||
        button_driver_get_button_state(1)) {
      led_driver_set_mode(PATTERN_BACK_AND_FORTH);
    } else {
      led_driver_set_mode(PATTERN_DEFAULT_CHASER);
    }*/

    /*void *txn = i2c_issue_transaction(i2c1_driver, 0x1c, "\x60", 1, buf, 32);
    while ((i2c_transaction_status(i2c1_driver, txn) !=
        I2C_TRANSACTION_STATUS_DONE) &&
        (i2c_transaction_status(i2c1_driver, txn) !=
          I2C_TRANSACTION_STATUS_ERROR)) {}
    i2c_transaction_finish(i2c1_driver, txn);*/

    /*
    txn = uart_serial_send_data(smartsensor_1, "\x00\x0A""CDEFGHIJKL",
      12);
    while ((uart_serial_send_status(smartsensor_1, txn) !=
        UART_SERIAL_SEND_DONE) &&
        (uart_serial_send_status(smartsensor_1, txn) !=
          UART_SERIAL_SEND_ERROR)) {}
    uart_serial_send_finish(smartsensor_1, txn);

    buf2 = uart_serial_receive_packet(smartsensor_1, &len, 1);
    vPortFree(buf2);

    txn = uart_serial_send_data(smartsensor_2, "\x00\x0A""CDEFGHIJKL",
      12);
    while ((uart_serial_send_status(smartsensor_2, txn) !=
        UART_SERIAL_SEND_DONE) &&
        (uart_serial_send_status(smartsensor_2, txn) !=
          UART_SERIAL_SEND_ERROR)) {}
    uart_serial_send_finish(smartsensor_2, txn);

    buf2 = uart_serial_receive_packet(smartsensor_2, &len, 1);
    vPortFree(buf2);

    txn = uart_serial_send_data(smartsensor_3, "\x00\x0A""CDEFGHIJKL",
      12);
    while ((uart_serial_send_status(smartsensor_3, txn) !=
        UART_SERIAL_SEND_DONE) &&
        (uart_serial_send_status(smartsensor_3, txn) !=
          UART_SERIAL_SEND_ERROR)) {}
    uart_serial_send_finish(smartsensor_3, txn);

    buf2 = uart_serial_receive_packet(smartsensor_3, &len, 1);
    vPortFree(buf2);

    txn = uart_serial_send_data(smartsensor_4, "\x00\x0A""CDEFGHIJKL",
      12);
    while ((uart_serial_send_status(smartsensor_4, txn) !=
        UART_SERIAL_SEND_DONE) &&
        (uart_serial_send_status(smartsensor_4, txn) !=
          UART_SERIAL_SEND_ERROR)) {}
    uart_serial_send_finish(smartsensor_4, txn);

    buf2 = uart_serial_receive_packet(smartsensor_4, &len, 1);
    vPortFree(buf2);

    txn = uart_serial_send_data(radio_driver, "\x00\x0A""CDEFGHIJKL",
      12);
    while ((uart_serial_send_status(radio_driver, txn) !=
        UART_SERIAL_SEND_DONE) &&
        (uart_serial_send_status(radio_driver, txn) !=
          UART_SERIAL_SEND_ERROR)) {}
    uart_serial_send_finish(radio_driver, txn);

    buf2 = uart_serial_receive_packet(radio_driver, &len, 1);
    vPortFree(buf2);

    vTaskDelay(500);
    */

    buf2 = uart_serial_receive_packet(radio_driver, &len, 1);
    if (!xbee_verify_checksum(buf2)) {
      continue;
    }
    xbee_api_packet *packetIn = buf2;
    if (packetIn->xbee_api_type != XBEE_API_TYPE_RX64) {
      continue;
    }
    xbee_api_packet *packetOut = buf;
    packetOut->xbee_api_magic = XBEE_MAGIC;
    int lenThing = sizeof(xbee_tx64_header) + sizeof(pier_outgoingdata);
    packetOut->length = __REV16(lenThing);
    packetOut->tx64.xbee_api_type = XBEE_API_TYPE_TX64;
    packetOut->tx64.frameId = 0;
    packetOut->tx64.xbee_dest_addr = packetIn->rx64.xbee_src_addr;
    packetOut->tx64.options = 0;
    pier_incomingdata *inData = &(packetIn->rx64.data);
    pier_outgoingdata *outData = &(packetOut->tx64.data);

    if (inData->ident != PIER_INCOMINGDATA_IDENT) {
      continue;
    }

    outData->ident = PIER_OUTGOINGDATA_IDENT;
    outData->analog[0] = inData->analog[0] + inData->analog[1];
    outData->analog[1] = counter++;

    xbee_fill_checksum(packetOut);

    vPortFree(buf2);
    txn = uart_serial_send_data(radio_driver, buf, lenThing + 4);
    while ((uart_serial_send_status(radio_driver, txn) !=
        UART_SERIAL_SEND_DONE) &&
        (uart_serial_send_status(radio_driver, txn) !=
          UART_SERIAL_SEND_ERROR)) {}
    uart_serial_send_finish(radio_driver, txn);
  }
}

int main(int argc, char **argv) {
  led_driver_init();
  button_driver_init();
  // Enable GPIOD clock
  // RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
  // Enable GPIOB clock
  // RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  // TODO(rqou): Need some kind of io_init function

  // Set red and green LED GPIO output
  // CONFIGURE_IO(RED_LED);
  // CONFIGURE_IO(GREEN_LED);
  // Turn on red LED
  // GPIO_BANK(PINDEF_RED_LED)->ODR =
  //   (1 << GPIO_PIN(PINDEF_RED_LED));

  // debug_uart_setup();

  // Setup I2C
  i2c1_init();

  // Setup SmartSensors
  smartsensor1_init();
  smartsensor2_init();
  smartsensor3_init();
  smartsensor4_init();

  // Setup radio
  radio_driver_init();

  xTaskCreate(blinkTask, "Blink", 256, NULL, tskIDLE_PRIORITY, NULL);
  vTaskStartScheduler();
}
