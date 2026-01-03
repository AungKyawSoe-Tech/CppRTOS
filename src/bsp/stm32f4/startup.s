/**
 * STM32F4 Startup Code
 * 
 * This file contains the reset handler and vector table for STM32F4.
 * It initializes the C/C++ runtime environment and calls main().
 */

    .syntax unified
    .cpu cortex-m4
    .fpu softvfp
    .thumb

/* Start address for the initialization values of the .data section */
.word   _sidata
/* Start address for the .data section */
.word   _sdata
/* End address for the .data section */
.word   _edata
/* Start address for the .bss section */
.word   _sbss
/* End address for the .bss section */
.word   _ebss

/**
 * Reset_Handler - Entry point after reset
 * 
 * This is the first code that runs after the processor resets.
 * It initializes memory and calls main().
 */
    .section .text.Reset_Handler
    .weak Reset_Handler
    .type Reset_Handler, %function
Reset_Handler:
    /* Set stack pointer */
    ldr   sp, =_estack

    /* Copy .data section from Flash to RAM */
    movs  r1, #0
    b     LoopCopyDataInit

CopyDataInit:
    ldr   r3, =_sidata
    ldr   r3, [r3, r1]
    str   r3, [r0, r1]
    adds  r1, r1, #4

LoopCopyDataInit:
    ldr   r0, =_sdata
    ldr   r3, =_edata
    adds  r2, r0, r1
    cmp   r2, r3
    bcc   CopyDataInit
    
    /* Zero fill the .bss section */
    ldr   r2, =_sbss
    ldr   r4, =_ebss
    movs  r3, #0
    b     LoopFillZerobss

FillZerobss:
    str   r3, [r2]
    adds  r2, r2, #4

LoopFillZerobss:
    cmp   r2, r4
    bcc   FillZerobss

    /* Call static constructors */
    bl    __libc_init_array

    /* Call main() */
    bl    main

    /* If main returns, loop forever */
InfiniteLoop:
    b     InfiniteLoop

    .size Reset_Handler, .-Reset_Handler

/**
 * Default_Handler - Default handler for unhandled exceptions
 */
    .section .text.Default_Handler,"ax",%progbits
Default_Handler:
    b     Default_Handler
    .size Default_Handler, .-Default_Handler

/**
 * Vector Table
 * 
 * The vector table contains the initial stack pointer and
 * exception/interrupt handler addresses.
 */
    .section .isr_vector,"a",%progbits
    .type g_pfnVectors, %object
    .size g_pfnVectors, .-g_pfnVectors

g_pfnVectors:
    /* Core exceptions */
    .word _estack                    /* Initial stack pointer */
    .word Reset_Handler              /* Reset */
    .word NMI_Handler                /* NMI */
    .word HardFault_Handler          /* Hard Fault */
    .word MemManage_Handler          /* MPU Fault */
    .word BusFault_Handler           /* Bus Fault */
    .word UsageFault_Handler         /* Usage Fault */
    .word 0                          /* Reserved */
    .word 0                          /* Reserved */
    .word 0                          /* Reserved */
    .word 0                          /* Reserved */
    .word SVC_Handler                /* SVCall */
    .word DebugMon_Handler           /* Debug Monitor */
    .word 0                          /* Reserved */
    .word PendSV_Handler             /* PendSV (context switch) */
    .word SysTick_Handler            /* SysTick (system tick) */
    
    /* External interrupts (STM32F4 specific) */
    .word WWDG_IRQHandler            /* Window Watchdog */
    .word PVD_IRQHandler             /* PVD through EXTI Line detect */
    .word TAMP_STAMP_IRQHandler      /* Tamper and TimeStamp */
    .word RTC_WKUP_IRQHandler        /* RTC Wakeup */
    .word FLASH_IRQHandler           /* Flash global */
    .word RCC_IRQHandler             /* RCC global */
    .word EXTI0_IRQHandler           /* EXTI Line 0 */
    .word EXTI1_IRQHandler           /* EXTI Line 1 */
    .word EXTI2_IRQHandler           /* EXTI Line 2 */
    .word EXTI3_IRQHandler           /* EXTI Line 3 */
    .word EXTI4_IRQHandler           /* EXTI Line 4 */
    /* ... more interrupt vectors would go here ... */
    
    /* For brevity, not including all 82 STM32F4 interrupt vectors */
    /* In production, all vectors should be defined */

/**
 * Weak definitions for exception handlers
 * These can be overridden by user code
 */
    .weak NMI_Handler
    .thumb_set NMI_Handler, Default_Handler
    
    .weak HardFault_Handler
    .thumb_set HardFault_Handler, Default_Handler
    
    .weak MemManage_Handler
    .thumb_set MemManage_Handler, Default_Handler
    
    .weak BusFault_Handler
    .thumb_set BusFault_Handler, Default_Handler
    
    .weak UsageFault_Handler
    .thumb_set UsageFault_Handler, Default_Handler
    
    .weak SVC_Handler
    .thumb_set SVC_Handler, Default_Handler
    
    .weak DebugMon_Handler
    .thumb_set DebugMon_Handler, Default_Handler
    
    /* PendSV and SysTick are defined in port.s */
    .weak PendSV_Handler
    .weak SysTick_Handler
    
    /* External interrupt handlers */
    .weak WWDG_IRQHandler
    .thumb_set WWDG_IRQHandler, Default_Handler
    
    .weak PVD_IRQHandler
    .thumb_set PVD_IRQHandler, Default_Handler
    
    .weak TAMP_STAMP_IRQHandler
    .thumb_set TAMP_STAMP_IRQHandler, Default_Handler
    
    .weak RTC_WKUP_IRQHandler
    .thumb_set RTC_WKUP_IRQHandler, Default_Handler
    
    .weak FLASH_IRQHandler
    .thumb_set FLASH_IRQHandler, Default_Handler
    
    .weak RCC_IRQHandler
    .thumb_set RCC_IRQHandler, Default_Handler
    
    .weak EXTI0_IRQHandler
    .thumb_set EXTI0_IRQHandler, Default_Handler
    
    .weak EXTI1_IRQHandler
    .thumb_set EXTI1_IRQHandler, Default_Handler
    
    .weak EXTI2_IRQHandler
    .thumb_set EXTI2_IRQHandler, Default_Handler
    
    .weak EXTI3_IRQHandler
    .thumb_set EXTI3_IRQHandler, Default_Handler
    
    .weak EXTI4_IRQHandler
    .thumb_set EXTI4_IRQHandler, Default_Handler

    .end
