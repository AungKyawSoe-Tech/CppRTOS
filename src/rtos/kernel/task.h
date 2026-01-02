#ifndef RTOS_TASK_H
#define RTOS_TASK_H

#include "../rtos_types.h"
#include <cstddef>
#include <cstdint>

// Forward declarations
class Scheduler;

// Task function pointer type
typedef void (*TaskFunction_t)(void* params);

// Stack size configuration
#define TASK_STACK_SIZE_MIN     256
#define TASK_STACK_SIZE_DEFAULT 512
#define TASK_STACK_SIZE_MAX     2048

// Task Control Block (TCB)
struct TaskControlBlock {
    // Task identification
    const char* name;
    uint32_t task_id;
    
    // Task state
    TaskState state;
    TaskPriority priority;
    
    // Stack information
    uint32_t* stack_pointer;     // Current stack pointer
    uint32_t* stack_base;        // Base of stack (bottom)
    size_t stack_size;           // Size of stack in bytes
    
    // Task function and parameters
    TaskFunction_t task_function;
    void* task_params;
    
    // Scheduling information
    TickType_t time_slice;       // Remaining time slice
    TickType_t blocked_until;    // Time to wake up if blocked
    
    // Linked list for scheduler
    TaskControlBlock* next_task;
    
    // Statistics (optional)
    uint32_t run_count;
    TickType_t total_runtime;
    
    // Constructor
    TaskControlBlock() {}
};

// Note: TaskHandle_t is already defined in rtos_types.h as void*
// We'll use TaskControlBlock* directly in implementation

// Task creation parameters
struct TaskCreateParams {
    const char* name;
    TaskFunction_t function;
    void* params;
    size_t stack_size;
    TaskPriority priority;
    
    TaskCreateParams()
        : name("task"), function(nullptr), params(nullptr),
          stack_size(TASK_STACK_SIZE_DEFAULT), priority(PRIORITY_NORMAL) {}
};

// Task API
namespace Task {
    // Create a new task
    RTOSResult create(TaskHandle_t* handle, const TaskCreateParams& params);
    
    // Delete a task
    RTOSResult destroy(TaskHandle_t handle);
    
    // Suspend a task
    RTOSResult suspend(TaskHandle_t handle);
    
    // Resume a task
    RTOSResult resume(TaskHandle_t handle);
    
    // Get current task handle
    TaskHandle_t getCurrentTask();
    
    // Yield execution to next task
    void yield();
    
    // Delay for specified ticks
    void delay(TickType_t ticks);
    
    // Get task state
    TaskState getState(TaskHandle_t handle);
    
    // Get task priority
    TaskPriority getPriority(TaskHandle_t handle);
    
    // Set task priority
    RTOSResult setPriority(TaskHandle_t handle, TaskPriority priority);
}

#endif // RTOS_TASK_H
