#ifndef RTOS_ARM_PORT_H
#define RTOS_ARM_PORT_H

#include "../../rtos_types.h"
#include <cstdint>

/**
 * ARM Cortex-M Architecture Port
 * 
 * This file provides the hardware-specific functions for ARM Cortex-M processors.
 * Supports Cortex-M3, M4, M7 with optional FPU.
 */

// Architecture configuration
#define ARCH_ARM_CORTEX_M

// FPU support (define in build system or BSP)
#ifndef ARM_FPU_PRESENT
    #define ARM_FPU_PRESENT 0
#endif

// Stack grows downward on ARM
#define STACK_GROWS_DOWN 1

// Minimum stack alignment (ARM requires 8-byte alignment)
#define STACK_ALIGNMENT 8

// Context switch trigger
#define PENDSV_PRIORITY 0xFF  // Lowest priority for PendSV

// Core register definitions
#define NVIC_INT_CTRL       0xE000ED04  // Interrupt control state register
#define NVIC_SYSPRI14       0xE000ED22  // System priority register (PendSV)
#define NVIC_PENDSV_PRI     0xFF        // PendSV priority value
#define NVIC_PENDSVSET      0x10000000  // Bit to trigger PendSV

// SysTick registers
#define SYSTICK_CTRL        0xE000E010
#define SYSTICK_LOAD        0xE000E014
#define SYSTICK_VAL         0xE000E018
#define SYSTICK_CALIB       0xE000E01C

// SysTick control bits
#define SYSTICK_ENABLE      (1 << 0)
#define SYSTICK_TICKINT     (1 << 1)
#define SYSTICK_CLKSOURCE   (1 << 2)

// Stack frame structure for Cortex-M (basic frame saved by hardware)
struct HardwareStackFrame {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t xpsr;
};

// Full context (hardware + software saved registers)
struct TaskContext {
    // Software-saved registers (pushed by context switch)
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    
#if (ARM_FPU_PRESENT == 1)
    uint32_t s16_s31[16];  // FPU registers (if present)
#endif
    
    // Hardware-saved registers (pushed by exception entry)
    HardwareStackFrame hw_frame;
};

// Critical section nesting counter
extern volatile uint32_t critical_nesting;

// Namespace for port-specific functions
namespace Port {
    /**
     * Initialize the hardware port
     * Sets up PendSV priority and other hardware-specific initialization
     */
    void initialize();
    
    /**
     * Initialize a new task's stack
     * Sets up the stack frame so the task can be switched in
     * 
     * @param stack_top Pointer to top of stack
     * @param task_entry Task function entry point
     * @param params Parameters to pass to task
     * @return Initialized stack pointer
     */
    uint32_t* initializeStack(uint32_t* stack_top, void (*task_entry)(void*), void* params);
    
    /**
     * Start the first task
     * This function never returns - it loads the context of the first task
     */
    void startFirstTask() __attribute__((noreturn));
    
    /**
     * Trigger a context switch
     * Sets the PendSV interrupt to switch to the next task
     */
    void triggerContextSwitch();
    
    /**
     * Disable interrupts (enter critical section)
     * @return Previous interrupt state
     */
    uint32_t disableInterrupts();
    
    /**
     * Enable interrupts (exit critical section)
     * @param state Previous interrupt state to restore
     */
    void enableInterrupts(uint32_t state);
    
    /**
     * Enter critical section (supports nesting)
     */
    void enterCritical();
    
    /**
     * Exit critical section (supports nesting)
     */
    void exitCritical();
    
    /**
     * Initialize SysTick timer
     * @param ticks_per_second Number of ticks per second (e.g., 1000 for 1ms)
     * @param cpu_freq_hz CPU frequency in Hz
     */
    void initSysTick(uint32_t ticks_per_second, uint32_t cpu_freq_hz);
    
    /**
     * Get current stack pointer
     */
    uint32_t* getCurrentSP();
    
    /**
     * Wait for interrupt (low power mode)
     */
    inline void waitForInterrupt() {
        __asm volatile ("wfi");
    }
    
    /**
     * Memory barrier
     */
    inline void memoryBarrier() {
        __asm volatile ("dmb" ::: "memory");
    }
    
    /**
     * Data synchronization barrier
     */
    inline void dataSyncBarrier() {
        __asm volatile ("dsb" ::: "memory");
    }
    
    /**
     * Instruction synchronization barrier
     */
    inline void instructionSyncBarrier() {
        __asm volatile ("isb" ::: "memory");
    }
}

// External assembly functions (defined in port.s)
extern "C" {
    /**
     * PendSV exception handler (context switch)
     * Saves current context and loads next context
     */
    void PendSV_Handler(void);
    
    /**
     * SysTick exception handler
     * Calls the scheduler's tick function
     */
    void SysTick_Handler(void);
    
    /**
     * Start the first task (assembly)
     * @param first_task_sp Stack pointer of first task
     */
    void port_start_first_task(uint32_t* first_task_sp);
    
    /**
     * Context switch function
     * @param current_sp Pointer to store current task's SP
     * @param next_sp Next task's SP
     */
    void port_context_switch(uint32_t** current_sp, uint32_t* next_sp);
}

// Critical section helper class (RAII)
class CriticalSection {
public:
    CriticalSection() {
        Port::enterCritical();
    }
    
    ~CriticalSection() {
        Port::exitCritical();
    }
    
    // Disable copy/move
    CriticalSection(const CriticalSection&) = delete;
    CriticalSection& operator=(const CriticalSection&) = delete;
};

#endif // RTOS_ARM_PORT_H
