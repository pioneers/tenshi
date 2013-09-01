// Configure the processor into a "normal" state. This mostly consists of
// initializing the clocks. Mostly derived from system_stm32f4xx.c.

#include "stm32f4xx.h"

// This function is called from the crt0 before main or even libc_init_array.
// However, a stack has been set up and data has been relocated to the correct
// addresses. Things like global initializers will not work.
void __ll_init(void) {
  // set CP10 and CP11 Full Access (enable access to FPU)
  SCB->CPACR |= ((0b11 << 10*2)|(0b11 << 11*2));
}
