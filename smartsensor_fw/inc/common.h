
#ifndef INC_COMMON_H_
#define INC_COMMON_H_

#include <stdlib.h>
#include <stdint.h>
#include "inc/pindef.h"
#include "inc/id.h"
#include "inc/config.h"


#define F_CPU 8000000  // AVR clock frequency in Hz, used by util/delay.h
#define SMART_BAUD 500000  // Smartsensor baud rate
#define SMART_ID_LEN 8   // Length of smartsensor personal ID

// May change
#define ACTIVE_PACKET_MAX_LEN 10  // Not including prefix or extra COBS byte



// ****Sensor Personal Data*** // to be a struct later.
extern uint8_t smartID[SMART_ID_LEN];

// Only uses last three bits
// TODO(tobinsarah): allow for multiple frames
extern uint8_t my_frame;
extern uint32_t sample_rate;  // hardcoded for now;


#endif  // INC_COMMON_H_
