#include "port.h"
#include "../../hal/rtos_uart.h"
#include <cstring>

// Critical section nesting counter
volatile uint32_t critical_nesting = 0;

// Store current and next task stack pointers (used by assembly)
uint32_t* volatile current_task_sp = nullptr;
uint32_t* volatile next_task_sp = nullptr;

namespace Port {

void initialize() {
    // Set PendSV to lowest priority (0xFF)
    volatile uint8_t* pendsv_priority = (volatile uint8_t*)NVIC_SYSPRI14;
    *pendsv_priority = NVIC_PENDSV_PRI;
    
    // Initialize critical section counter
    critical_nesting = 0;
    
    rtos_printf("[Port] ARM Cortex-M port initialized\n");
    rtos_printf("[Port] PendSV priority set to lowest (0xFF)\n");
#if (ARM_FPU_PRESENT == 1)
    rtos_printf("[Port] FPU support enabled\n");
#endif
}

uint32_t* initializeStack(uint32_t* stack_top, void (*task_entry)(void*), void* params) {
    // ARM stack is full-descending, so we work backwards from stack_top
    uint32_t* sp = stack_top;
    
    // Align stack pointer to 8 bytes (ARM EABI requirement)
    sp = (uint32_t*)((uint32_t)sp & ~0x7UL);
    
    // Hardware-saved registers (pushed by exception entry)
    // These are pushed automatically when an interrupt occurs
    *(--sp) = 0x01000000UL;              // xPSR (Thumb bit set)
    *(--sp) = (uint32_t)task_entry;      // PC (task entry point)
    *(--sp) = 0xFFFFFFFDUL;              // LR (EXC_RETURN - return to thread mode with PSP)
    *(--sp) = 0x12121212UL;              // R12
    *(--sp) = 0x03030303UL;              // R3
    *(--sp) = 0x02020202UL;              // R2
    *(--sp) = 0x01010101UL;              // R1
    *(--sp) = (uint32_t)params;          // R0 (task parameter)
    
#if (ARM_FPU_PRESENT == 1)
    // FPU registers S0-S15 (saved by hardware if FPU used)
    // S16-S31 need to be saved by software
    for (int i = 0; i < 16; i++) {
        *(--sp) = 0;  // S16-S31
    }
#endif
    
    // Software-saved registers (we push these in context switch)
    *(--sp) = 0x11111111UL;  // R11
    *(--sp) = 0x10101010UL;  // R10
    *(--sp) = 0x09090909UL;  // R9
    *(--sp) = 0x08080808UL;  // R8
    *(--sp) = 0x07070707UL;  // R7
    *(--sp) = 0x06060606UL;  // R6
    *(--sp) = 0x05050505UL;  // R5
    *(--sp) = 0x04040404UL;  // R4
    
    return sp;
}

void startFirstTask() {
    rtos_printf("[Port] Starting first task...\n");
    
    // This function should never return
    // It will be called from scheduler.start()
    // The actual implementation is in assembly (port.s)
    
    // For now, just loop forever if called
    while(1) {
        __asm volatile ("wfi");
    }
}

void triggerContextSwitch() {
    // Set PendSV interrupt pending
    volatile uint32_t* icsr = (volatile uint32_t*)NVIC_INT_CTRL;
    *icsr = NVIC_PENDSVSET;
    
    // Data synchronization barrier
    dataSyncBarrier();
    instructionSyncBarrier();
}

uint32_t disableInterrupts() {
    uint32_t primask;
    __asm volatile (
        "mrs %0, primask    \n"  // Read current PRIMASK
        "cpsid i            \n"  // Disable interrupts
        : "=r" (primask)
        :
        : "memory"
    );
    return primask;
}

void enableInterrupts(uint32_t state) {
    __asm volatile (
        "msr primask, %0    \n"  // Restore PRIMASK
        :
        : "r" (state)
        : "memory"
    );
}

void enterCritical() {
    disableInterrupts();
    critical_nesting++;
}

void exitCritical() {
    critical_nesting--;
    if (critical_nesting == 0) {
        enableInterrupts(0);  // Re-enable interrupts
    }
}

void initSysTick(uint32_t ticks_per_second, uint32_t cpu_freq_hz) {
    uint32_t reload_value = (cpu_freq_hz / ticks_per_second) - 1;
    
    volatile uint32_t* systick_ctrl = (volatile uint32_t*)SYSTICK_CTRL;
    volatile uint32_t* systick_load = (volatile uint32_t*)SYSTICK_LOAD;
    volatile uint32_t* systick_val = (volatile uint32_t*)SYSTICK_VAL;
    
    // Disable SysTick during configuration
    *systick_ctrl = 0;
    
    // Set reload value
    *systick_load = reload_value;
    
    // Clear current value
    *systick_val = 0;
    
    // Enable SysTick with interrupt and processor clock
    *systick_ctrl = SYSTICK_ENABLE | SYSTICK_TICKINT | SYSTICK_CLKSOURCE;
    
    rtos_printf("[Port] SysTick configured: %u Hz (%u ticks)\n", 
                ticks_per_second, reload_value);
}

uint32_t* getCurrentSP() {
    uint32_t* sp;
    __asm volatile (
        "mrs %0, psp    \n"  // Read Process Stack Pointer
        : "=r" (sp)
    );
    return sp;
}

} // namespace Port

// C linkage for exception handlers

extern "C" {

// These will be called from assembly
void port_systick_handler() {
    // This will be implemented to call Scheduler::tick()
    // For now, just a placeholder
    static uint32_t tick_count = 0;
    tick_count++;
}

void port_pendsv_handler() {
    // This is the actual context switch
    // Will be implemented in assembly for efficiency
}

} // extern "C"
