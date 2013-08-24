.section .vectors, "ax", %progbits
.syntax unified
.thumb

.global _start
.align 4
_start:

@ Vector table
.macro vector name
.weak           \name
.thumb_set      \name, __unhandled_isr
.word           \name
.endm

    .word __sp_main
    .word _reset+1
    vector NMI_Handler
    vector HardFault_Handler
    vector MemManage_Handler
    vector BusFault_Handler
    vector UsageFault_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    vector SVC_Handler
    vector DebugMon_Handler
    .word 0
    vector PendSV_Handler
    vector SysTick_Handler

    /* External Interrupts */
    vector WWDG_IRQHandler                   /* Window WatchDog              */                                        
    vector PVD_IRQHandler                    /* PVD through EXTI Line detection */                        
    vector TAMP_STAMP_IRQHandler             /* Tamper and TimeStamps through the EXTI line */            
    vector RTC_WKUP_IRQHandler               /* RTC Wakeup through the EXTI line */                      
    vector FLASH_IRQHandler                  /* FLASH                        */                                          
    vector RCC_IRQHandler                    /* RCC                          */                                            
    vector EXTI0_IRQHandler                  /* EXTI Line0                   */                        
    vector EXTI1_IRQHandler                  /* EXTI Line1                   */                          
    vector EXTI2_IRQHandler                  /* EXTI Line2                   */                          
    vector EXTI3_IRQHandler                  /* EXTI Line3                   */                          
    vector EXTI4_IRQHandler                  /* EXTI Line4                   */                          
    vector DMA1_Stream0_IRQHandler           /* DMA1 Stream 0                */                  
    vector DMA1_Stream1_IRQHandler           /* DMA1 Stream 1                */                   
    vector DMA1_Stream2_IRQHandler           /* DMA1 Stream 2                */                   
    vector DMA1_Stream3_IRQHandler           /* DMA1 Stream 3                */                   
    vector DMA1_Stream4_IRQHandler           /* DMA1 Stream 4                */                   
    vector DMA1_Stream5_IRQHandler           /* DMA1 Stream 5                */                   
    vector DMA1_Stream6_IRQHandler           /* DMA1 Stream 6                */                   
    vector ADC_IRQHandler                    /* ADC1, ADC2 and ADC3s         */                   
    vector CAN1_TX_IRQHandler                /* CAN1 TX                      */                         
    vector CAN1_RX0_IRQHandler               /* CAN1 RX0                     */                          
    vector CAN1_RX1_IRQHandler               /* CAN1 RX1                     */                          
    vector CAN1_SCE_IRQHandler               /* CAN1 SCE                     */                          
    vector EXTI9_5_IRQHandler                /* External Line[9:5]s          */                          
    vector TIM1_BRK_TIM9_IRQHandler          /* TIM1 Break and TIM9          */         
    vector TIM1_UP_TIM10_IRQHandler          /* TIM1 Update and TIM10        */         
    vector TIM1_TRG_COM_TIM11_IRQHandler     /* TIM1 Trigger and Commutation and TIM11 */
    vector TIM1_CC_IRQHandler                /* TIM1 Capture Compare         */                          
    vector TIM2_IRQHandler                   /* TIM2                         */                   
    vector TIM3_IRQHandler                   /* TIM3                         */                   
    vector TIM4_IRQHandler                   /* TIM4                         */                   
    vector I2C1_EV_IRQHandler                /* I2C1 Event                   */                          
    vector I2C1_ER_IRQHandler                /* I2C1 Error                   */                          
    vector I2C2_EV_IRQHandler                /* I2C2 Event                   */                          
    vector I2C2_ER_IRQHandler                /* I2C2 Error                   */                            
    vector SPI1_IRQHandler                   /* SPI1                         */                   
    vector SPI2_IRQHandler                   /* SPI2                         */                   
    vector USART1_IRQHandler                 /* USART1                       */                   
    vector USART2_IRQHandler                 /* USART2                       */                   
    vector USART3_IRQHandler                 /* USART3                       */                   
    vector EXTI15_10_IRQHandler              /* External Line[15:10]s        */                          
    vector RTC_Alarm_IRQHandler              /* RTC Alarm (A and B) through EXTI Line */                 
    vector OTG_FS_WKUP_IRQHandler            /* USB OTG FS Wakeup through EXTI line */                       
    vector TIM8_BRK_TIM12_IRQHandler         /* TIM8 Break and TIM12         */         
    vector TIM8_UP_TIM13_IRQHandler          /* TIM8 Update and TIM13        */         
    vector TIM8_TRG_COM_TIM14_IRQHandler     /* TIM8 Trigger and Commutation and TIM14 */
    vector TIM8_CC_IRQHandler                /* TIM8 Capture Compare         */                          
    vector DMA1_Stream7_IRQHandler           /* DMA1 Stream7                 */                          
    vector FSMC_IRQHandler                   /* FSMC                         */                   
    vector SDIO_IRQHandler                   /* SDIO                         */                   
    vector TIM5_IRQHandler                   /* TIM5                         */                   
    vector SPI3_IRQHandler                   /* SPI3                         */                   
    vector UART4_IRQHandler                  /* UART4                        */                   
    vector UART5_IRQHandler                  /* UART5                        */                   
    vector TIM6_DAC_IRQHandler               /* TIM6 and DAC1&2 underrun errors */                   
    vector TIM7_IRQHandler                   /* TIM7                         */
    vector DMA2_Stream0_IRQHandler           /* DMA2 Stream 0                */                   
    vector DMA2_Stream1_IRQHandler           /* DMA2 Stream 1                */                   
    vector DMA2_Stream2_IRQHandler           /* DMA2 Stream 2                */                   
    vector DMA2_Stream3_IRQHandler           /* DMA2 Stream 3                */                   
    vector DMA2_Stream4_IRQHandler           /* DMA2 Stream 4                */                   
    vector ETH_IRQHandler                    /* Ethernet                     */                   
    vector ETH_WKUP_IRQHandler               /* Ethernet Wakeup through EXTI line */                     
    vector CAN2_TX_IRQHandler                /* CAN2 TX                      */                          
    vector CAN2_RX0_IRQHandler               /* CAN2 RX0                     */                          
    vector CAN2_RX1_IRQHandler               /* CAN2 RX1                     */                          
    vector CAN2_SCE_IRQHandler               /* CAN2 SCE                     */                          
    vector OTG_FS_IRQHandler                 /* USB OTG FS                   */                   
    vector DMA2_Stream5_IRQHandler           /* DMA2 Stream 5                */                   
    vector DMA2_Stream6_IRQHandler           /* DMA2 Stream 6                */                   
    vector DMA2_Stream7_IRQHandler           /* DMA2 Stream 7                */                   
    vector USART6_IRQHandler                 /* USART6                       */                    
    vector I2C3_EV_IRQHandler                /* I2C3 event                   */                          
    vector I2C3_ER_IRQHandler                /* I2C3 error                   */                          
    vector OTG_HS_EP1_OUT_IRQHandler         /* USB OTG HS End Point 1 Out   */                   
    vector OTG_HS_EP1_IN_IRQHandler          /* USB OTG HS End Point 1 In    */                   
    vector OTG_HS_WKUP_IRQHandler            /* USB OTG HS Wakeup through EXTI */                         
    vector OTG_HS_IRQHandler                 /* USB OTG HS                   */                   
    vector DCMI_IRQHandler                   /* DCMI                         */                   
    vector CRYP_IRQHandler                   /* CRYP crypto                  */                   
    vector HASH_RNG_IRQHandler               /* Hash and Rng                 */
    vector FPU_IRQHandler                    /* FPU                          */

@ Unhandled Exception
.weak    __unhandled_isr
.global    __unhandled_isr
.func    __unhandled_isr
__unhandled_isr:
    b __unhandled_isr
.endfunc

@ Low level init
.weak    __ll_init
.global    __ll_init
.func    __ll_init
__ll_init:
    bx lr
.endfunc

_reset:
#ifdef DEBUG
    @ Debug, clear memory with known pattern
    ldr r4, =0xAAAAAAAA
    movs r5, r4
    movs r6, r4
    movs r7, r4
    ldr r0, =0x20000000
    ldr r1, =0x00008000
    bl ClearData
#endif

    @ Stack is already set up
    @ Copy initialized data
    ldr r0, =__data_lma
    ldr r1, =__data_start
    ldr r2, =__data_end
    subs r2, r2, r1
    bl CopyData

    @ Copy CCM data
    ldr r0, =__ccram_lma
    ldr r1, =__ccram_start
    ldr r2, =__ccram_end
    subs r2, r2, r1
    bl CopyData

    @ Clear BSS and SBSS
    movs r4, #0
    movs r5, #0
    movs r6, #0
    movs r7, #0
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    subs r1, r1, r0
    bl ClearData
    ldr r0, =__sbss_start
    ldr r1, =__sbss_end
    subs r1, r1, r0
    bl ClearData

    @ init C stuff
    bl __ll_init
    bl __libc_init_array

    @ Main
    movs r0, #0
    movs r1, #0
    bl main

    @exit
    b _exit

.pool

@ r0 = src
@ r1 = len (must be multiple of 4)
@ r4-r7 = must be 0
ClearData:
    movs r3, r1, lsr #4
    beq clearremainder
clear16loop:
    stmia r0!, {r4-r7}
    subs r3, r3, #1
    bne clear16loop
clearremainder:
    ands r3, r1, #0x0F
    it eq
    bxeq lr
clearremainderloop:
    stmia r0!, {r4}
    subs r3, r3, #4
    bne clearremainderloop
    bx lr

@ r0 = src
@ r1 = dst
@ r2 = len (must be multiple of 4)
CopyData:
    movs r3, r2, lsr #4
    beq copyremainder
copy16loop:
    ldmia r0!, {r4-r7}
    stmia r1!, {r4-r7}
    subs r3, r3, #1
    bne copy16loop
copyremainder:
    ands r3, r2, #0x0F
    it eq
    bxeq lr
copyremainderloop:
    ldmia r0!, {r4}
    stmia r1!, {r4}
    subs r3, r3, #4
    bne copyremainderloop
    bx lr
