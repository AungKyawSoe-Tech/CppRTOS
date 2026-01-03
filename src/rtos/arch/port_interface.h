#ifndef RTOS_PORT_INTERFACE_H
#define RTOS_PORT_INTERFACE_H

/**
 * RTOS Port Interface
 * 
 * This header provides a common interface for all architecture ports.
 * The scheduler and other kernel components use these functions for
 * hardware-specific operations.
 */

#include "../rtos_types.h"
#include <cstdint>

// Select the appropriate port based on build configuration
#if defined(ARCH_ARM_CORTEX_M)
    #include "arm/port.h"
#elif defined(ARCH_SIMULATION)
    // Simulation port (x86/x64)
    // For now, provide stub implementations
    namespace Port {
        inline void initialize() {}
        inline uint32_t* initializeStack(uint32_t* stack_top, void (*task_entry)(void*), void* params) {
            return stack_top;
        }
        inline void startFirstTask() { while(1); }
        inline void triggerContextSwitch() {}
        inline void enterCritical() {}
        inline void exitCritical() {}
        inline void initSysTick(uint32_t ticks_per_second, uint32_t cpu_freq_hz) {
            (void)ticks_per_second;
            (void)cpu_freq_hz;
        }
        inline void waitForInterrupt() {}
        inline void memoryBarrier() {}
    }
#else
    #error "No architecture port defined. Define ARCH_ARM_CORTEX_M or ARCH_SIMULATION"
#endif

#endif // RTOS_PORT_INTERFACE_H
