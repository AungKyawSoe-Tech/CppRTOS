#ifndef RTOS_CONTEXT_H
#define RTOS_CONTEXT_H

#include <cstdint>
#include "task.h"

// Context switching interface
// This provides a platform-independent API for context switching
// Platform-specific implementations would go in architecture-specific files

namespace Context {

// Initialize stack frame for a new task
// Returns the new stack pointer value
uint32_t* initializeStack(uint32_t* stack_top, 
                          TaskFunction_t task_function,
                          void* task_params);

// Perform context switch (platform-specific)
// This would typically be implemented in assembly
// For simulation/testing, we use a simplified C++ version
void switchTo(TaskControlBlock* current, TaskControlBlock* next);

// Save current context (platform-specific)
// Returns true if this is the first call (save path)
// Returns false if returning from restore (resume path)
bool saveContext(TaskControlBlock* task);

// Restore context (platform-specific)
void restoreContext(TaskControlBlock* task);

// Get stack usage for a task
size_t getStackUsage(TaskControlBlock* task);

// Check for stack overflow
bool checkStackOverflow(TaskControlBlock* task);

} // namespace Context

// Platform-specific stack frame structure (example for ARM Cortex-M)
// In real implementation, this would vary by architecture
struct StackFrame {
    // Registers saved by software
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    
    // Registers saved by hardware (on exception entry)
    uint32_t r0;   // Arguments/return value
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;   // Link register
    uint32_t pc;   // Program counter
    uint32_t psr;  // Program status register
};

#endif // RTOS_CONTEXT_H
