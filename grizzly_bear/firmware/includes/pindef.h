#ifndef PINDEF_H
#define PINDEF_H

#define REV_C

#if defined(REV_A) || defined(REV_B) || defined(REV_C)

// dip switch address select
#define PINDEF_DIP1             PF1
#define PINDEF_DIP2             PF4
#define PINDEF_DIP3             PF5
#define PINDEF_DIP4             PD3
#define PINDEF_DIP5             PF6
#define PINDEF_DIP6             PF7

// rotary encoder
#define PINDEF_ENCA             PB0
#define PINDEF_ENCB             PB4

// high side driver endable
#define PINDEF_HIGHSIDEENABLE   PE2

#endif

// TODO(rqou): Backport REV_A
#if defined(REV_B) || defined(REV_C)
#define PINDEF_LEDGRN           PD6
#define PINDEF_LEDRED           PD4

#define LED_PORT              PORTD
#define LED_PORT_MODE          DDRD
#endif

#endif
