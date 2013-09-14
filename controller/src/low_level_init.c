// Configure the processor into a "normal" state. This mostly consists of
// initializing the clocks. Mostly derived from system_stm32f4xx.c.

#include "inc/stm32f4xx.h"

/************************* PLL Parameters *************************************/
// The following values are set such that an 8 MHz crystal gives an output of
// P = 168 MHz and Q = 48 MHz.
// PLL_VCO = (HSE_VALUE or HSI_VALUE / PLL_M) * PLL_N
#define PLL_M      8
#define PLL_N      336
// SYSCLK = PLL_VCO / PLL_P
#define PLL_P      2
// USB OTG FS, SDIO and RNG Clock =  PLL_VCO / PLLQ
#define PLL_Q      7

// This function is called from the crt0 before main or even libc_init_array.
// However, a stack has been set up and data has been relocated to the correct
// addresses. Things like global initializers will not work.
void __ll_init(void) {
  // set CP10 and CP11 Full Access (enable access to FPU)
  SCB->CPACR |= ((0b11 << 10*2) | (0b11 << 11*2));

  // Reset the RCC clock configuration to the default reset state. The clock
  // configuration will be set to its final value later.
  // Set HSION bit
  RCC->CR |= 0x00000001;
  // Reset CFGR register
  RCC->CFGR = 0x00000000;
  // Reset HSEON, CSSON and PLLON bits
  RCC->CR &= 0xFEF6FFFF;
  // Reset PLLCFGR register
  RCC->PLLCFGR = 0x24003010;
  // Reset HSEBYP bit
  RCC->CR &= 0xFFFBFFFF;
  // Disable all interrupts
  RCC->CIR = 0x00000000;

  // Set up clocks and PLL.
  uint32_t StartUpCounter = 1;
  // Enable HSE
  RCC->CR |= RCC_CR_HSEON;
  // Wait till HSE is ready and if Time out is reached exit
  while (!(RCC->CR & RCC_CR_HSERDY) &&
        (StartUpCounter != HSE_STARTUP_TIMEOUT)) {
    StartUpCounter++;
  }

  // Did the HSE clock enable successfully?
  if ((RCC->CR & RCC_CR_HSERDY) != RESET) {
    // Select regulator voltage output Scale 1 mode (max performance)
    // System frequency up to 168 MHz
    RCC->APB1ENR |= RCC_APB1ENR_PWREN;
    PWR->CR |= PWR_CR_VOS;

    // HCLK = SYSCLK / 1
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
    // PCLK2 = HCLK / 2
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;
    // PCLK1 = HCLK / 4
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;
    // Configure the main PLL
    RCC->PLLCFGR = PLL_M | (PLL_N << 6) | (((PLL_P >> 1) -1) << 16) |
        (RCC_PLLCFGR_PLLSRC_HSE) | (PLL_Q << 24);
    // Enable the main PLL
    RCC->CR |= RCC_CR_PLLON;
    // Wait till the main PLL is ready
    while ((RCC->CR & RCC_CR_PLLRDY) == 0) {}

    // Configure Flash prefetch, Instruction cache, Data cache and wait state
    FLASH->ACR =
        FLASH_ACR_PRFTEN |
        FLASH_ACR_ICEN |
        FLASH_ACR_DCEN |
        FLASH_ACR_LATENCY_5WS;

    // Select the main PLL as system clock source
    RCC->CFGR &= ~(RCC_CFGR_SW);
    RCC->CFGR |= RCC_CFGR_SW_PLL;

    // Wait till the main PLL is used as system clock source
    while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {}
  } else {
    // If HSE fails to start-up, the application will have wrong clock
    // configuration. User can add here some code to deal with this error
  }

  // Vector Table Relocation in Internal FLASH
  SCB->VTOR = FLASH_BASE;
}
