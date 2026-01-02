#include "context.h"
#include "../hal/rtos_uart.h"
#include <cstring>

namespace Context {

// Initialize stack frame for a new task
uint32_t* initializeStack(uint32_t* stack_top,
                          TaskFunction_t task_function,
                          void* task_params) {
    
    // Stack grows downward, so we start from the top
    uint32_t* sp = stack_top;
    
    // For simulation purposes, we create a simple stack frame
    // In a real RTOS, this would match the processor's exception frame
    
    // Simulate hardware-saved registers (as if from interrupt)
    *(--sp) = 0x01000000;  // xPSR (Thumb bit set)
    *(--sp) = reinterpret_cast<uintptr_t>(task_function); // PC
    *(--sp) = 0xFFFFFFFD;  // LR (EXC_RETURN)
    *(--sp) = 0x12121212;  // R12
    *(--sp) = 0x03030303;  // R3
    *(--sp) = 0x02020202;  // R2
    *(--sp) = 0x01010101;  // R1
    *(--sp) = reinterpret_cast<uintptr_t>(task_params); // R0 (first parameter)
    
    // Simulate software-saved registers
    *(--sp) = 0x11111111;  // R11
    *(--sp) = 0x10101010;  // R10
    *(--sp) = 0x09090909;  // R9
    *(--sp) = 0x08080808;  // R8
    *(--sp) = 0x07070707;  // R7
    *(--sp) = 0x06060606;  // R6
    *(--sp) = 0x05050505;  // R5
    *(--sp) = 0x04040404;  // R4
    
    return sp;
}

// Simplified context switch for simulation
void switchTo(TaskControlBlock* current, TaskControlBlock* next) {
    if (current == nullptr || next == nullptr) {
        return;
    }
    
    // In a real RTOS, this would:
    // 1. Save current task's context (registers, stack pointer)
    // 2. Load next task's context
    // 3. Jump to next task
    
    // For simulation, we just note the switch
    rtos_printf("[Context] Switch: '%s' -> '%s'\n",
                current->name ? current->name : "?",
                next->name ? next->name : "?");
    
    // In real implementation:
    // - Save registers to current->stack_pointer
    // - Load registers from next->stack_pointer
    // - Update stack pointer register
}

// Save context (simplified)
bool saveContext(TaskControlBlock* task) {
    if (task == nullptr) {
        return false;
    }
    
    // In real implementation, this would save CPU registers
    // For simulation, we just validate the task
    
    return true;
}

// Restore context (simplified)
void restoreContext(TaskControlBlock* task) {
    if (task == nullptr) {
        return;
    }
    
    // In real implementation, this would restore CPU registers
    // and jump to the task
    
    rtos_printf("[Context] Restore: '%s'\n",
                task->name ? task->name : "?");
}

// Get stack usage
size_t getStackUsage(TaskControlBlock* task) {
    if (task == nullptr || task->stack_base == nullptr) {
        return 0;
    }
    
    // Count bytes used from the pattern
    size_t used = 0;
    uint8_t* stack_bytes = reinterpret_cast<uint8_t*>(task->stack_base);
    size_t stack_size = task->stack_size;
    
    // Count from bottom until we find the pattern (0xA5)
    for (size_t i = 0; i < stack_size; i++) {
        if (stack_bytes[i] != 0xA5) {
            used = stack_size - i;
            break;
        }
    }
    
    return used;
}

// Check for stack overflow
bool checkStackOverflow(TaskControlBlock* task) {
    if (task == nullptr || task->stack_base == nullptr) {
        return false;
    }
    
    // Check if stack pointer is still within bounds
    uint32_t* stack_start = task->stack_base;
    uint32_t* stack_end = task->stack_base + (task->stack_size / sizeof(uint32_t));
    
    if (task->stack_pointer < stack_start || task->stack_pointer >= stack_end) {
        rtos_printf("[Context] STACK OVERFLOW detected in task '%s'!\n",
                    task->name ? task->name : "?");
        return true;
    }
    
    // Check if bottom of stack has been corrupted
    uint32_t* bottom = stack_start;
    if (*bottom != 0xA5A5A5A5) {
        // Bottom pattern corrupted
        rtos_printf("[Context] Stack corruption detected in task '%s'!\n",
                    task->name ? task->name : "?");
        return true;
    }
    
    return false;
}

} // namespace Context
