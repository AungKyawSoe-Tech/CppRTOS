/**
 * ARM Cortex-M Context Switch and Exception Handlers
 * 
 * This file contains the low-level assembly code for context switching
 * on ARM Cortex-M processors (M3, M4, M7).
 * 
 * Context Switch Strategy:
 * - Use PendSV exception for context switching (lowest priority)
 * - Use SysTick for system tick generation
 * - Hardware automatically saves R0-R3, R12, LR, PC, xPSR
 * - Software must save/restore R4-R11 (and FPU regs if present)
 */

    .syntax unified
    .cpu cortex-m4
    .fpu softvfp
    .thumb

/* External symbols */
    .extern current_task_sp
    .extern next_task_sp

/* Export symbols */
    .global PendSV_Handler
    .global SysTick_Handler
    .global port_start_first_task
    .global port_context_switch

/* Constants */
    .equ ICSR,              0xE000ED04      /* Interrupt Control State Register */
    .equ PENDSVSET,         0x10000000      /* Bit to trigger PendSV */

/**
 * PendSV_Handler - Context Switch Handler
 * 
 * This is the main context switch routine. It's triggered by setting
 * the PendSV pending bit. Since PendSV has the lowest priority,
 * it only runs when no other exceptions are active.
 * 
 * Process:
 * 1. Save current task context (R4-R11)
 * 2. Save current stack pointer
 * 3. Load next task's stack pointer
 * 4. Restore next task context (R4-R11)
 * 5. Return (hardware restores R0-R3, R12, LR, PC, xPSR)
 */
    .type PendSV_Handler, %function
PendSV_Handler:
    /* Disable interrupts */
    cpsid   i
    
    /* Check if this is the first context switch */
    ldr     r0, =current_task_sp
    ldr     r1, [r0]
    cbz     r1, restore_context     /* If current_task_sp is NULL, just restore */
    
    /* Save current context */
save_context:
    /* Get current PSP (Process Stack Pointer) */
    mrs     r0, psp
    
    /* Is the task using FPU? Check bit 4 of LR (EXC_RETURN) */
    tst     lr, #0x10
    it      eq
    vstmdbeq r0!, {s16-s31}         /* Save FPU registers if FPU was used */
    
    /* Save R4-R11 onto the task's stack */
    stmdb   r0!, {r4-r11}
    
    /* Save the new stack pointer to current_task_sp */
    ldr     r1, =current_task_sp
    ldr     r2, [r1]
    str     r0, [r2]                /* *current_task_sp = new SP */
    
restore_context:
    /* Load next task's stack pointer */
    ldr     r0, =next_task_sp
    ldr     r1, [r0]
    ldr     r0, [r1]                /* r0 = *next_task_sp */
    
    /* Restore R4-R11 from next task's stack */
    ldmia   r0!, {r4-r11}
    
    /* Is the task using FPU? Check bit 4 of LR */
    tst     lr, #0x10
    it      eq
    vldmiaeq r0!, {s16-s31}         /* Restore FPU registers if FPU was used */
    
    /* Update PSP to point to next task's stack */
    msr     psp, r0
    
    /* Ensure changes take effect before returning */
    isb
    
    /* Enable interrupts */
    cpsie   i
    
    /* Return from exception (hardware will restore R0-R3, R12, LR, PC, xPSR) */
    bx      lr

/**
 * SysTick_Handler - System Tick Interrupt
 * 
 * Called every system tick (typically 1ms).
 * Calls the C function to handle tick processing.
 */
    .type SysTick_Handler, %function
SysTick_Handler:
    /* Save context */
    push    {lr}
    
    /* Call C function to handle tick */
    bl      port_systick_handler
    
    /* Restore context and return */
    pop     {lr}
    bx      lr

/**
 * port_start_first_task - Start the first task
 * 
 * Called once to start the scheduler.
 * Sets up PSP and jumps to the first task.
 * 
 * @param r0: Stack pointer of first task
 */
    .type port_start_first_task, %function
port_start_first_task:
    /* Disable interrupts */
    cpsid   i
    
    /* Load first task's stack pointer from argument */
    ldr     r0, =next_task_sp
    ldr     r1, [r0]
    ldr     r0, [r1]                /* r0 = *next_task_sp */
    
    /* Restore R4-R11 */
    ldmia   r0!, {r4-r11}
    
    /* Skip hardware-saved registers to get to PSP value */
    /* R4-R11 already popped, now skip R0-R3, R12, LR, PC, xPSR */
    
    /* Set PSP to this value */
    msr     psp, r0
    
    /* Use PSP for stack, not MSP */
    movs    r0, #2
    msr     control, r0
    isb
    
    /* Enable interrupts */
    cpsie   i
    
    /* Manually pop hardware frame to start first task */
    /* Actually, we need to simulate an exception return */
    /* Load dummy values into R0-R3, R12 */
    mov     r0, #0
    mov     r1, #0
    mov     r2, #0
    mov     r3, #0
    mov     r12, #0
    
    /* Load EXC_RETURN value to trigger return to thread mode with PSP */
    ldr     lr, =0xFFFFFFFD
    
    /* Exception return */
    bx      lr

/**
 * port_context_switch - Trigger a context switch
 * 
 * Sets the PendSV interrupt pending to trigger a context switch.
 */
    .type port_context_switch, %function
port_context_switch:
    /* Load ICSR address */
    ldr     r0, =ICSR
    
    /* Set PENDSVSET bit */
    ldr     r1, =PENDSVSET
    str     r1, [r0]
    
    /* Memory barriers */
    dsb
    isb
    
    bx      lr

    .end
