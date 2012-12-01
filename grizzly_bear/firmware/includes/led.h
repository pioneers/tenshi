#ifndef LED_H_
#define LED_H_

// Initializes the led's.
void init_led();

// Turn the green led on or off.
void set_green_led(unsigned char on);

// Turn the red led on or off.
void set_red_led(unsigned char on);

// Turn the red and green led's on or off.
// The amber led cannot be controlled by software.
void set_all_leds(unsigned char on);

#define LED_ON 1
#define LED_OFF 0


#endif /* LED_H_ */
