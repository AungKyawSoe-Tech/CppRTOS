#include "task.h"
#include "scheduler.h"
#include "../hal/rtos_uart.h"
#include <cstring>
#include <cstdlib>

namespace Task {

// Create a new task
RTOSResult create(TaskHandle_t* handle, const TaskCreateParams& params) {
    if (handle == nullptr || params.function == nullptr) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    // Validate stack size
    if (params.stack_size < TASK_STACK_SIZE_MIN) {
        rtos_printf("[Task] ERROR: Stack size too small (%d < %d)\n",
                    params.stack_size, TASK_STACK_SIZE_MIN);
        return RTOS_ERR_INVALID_PARAM;
    }
    
    if (params.stack_size > TASK_STACK_SIZE_MAX) {
        rtos_printf("[Task] ERROR: Stack size too large (%d > %d)\n",
                    params.stack_size, TASK_STACK_SIZE_MAX);
        return RTOS_ERR_INVALID_PARAM;
    }
    
    // Allocate TCB
    TaskControlBlock* tcb = new TaskControlBlock();
    if (tcb == nullptr) {
        return RTOS_ERR_NOMEM;
    }
    
    // Allocate stack
    uint32_t* stack = new uint32_t[params.stack_size / sizeof(uint32_t)];
    if (stack == nullptr) {
        delete tcb;
        return RTOS_ERR_NOMEM;
    }
    
    // Initialize TCB
    tcb->name = params.name;
    tcb->state = TASK_READY;
    tcb->priority = params.priority;
    tcb->task_function = params.function;
    tcb->task_params = params.params;
    tcb->stack_base = stack;
    tcb->stack_size = params.stack_size;
    
    // Initialize stack pointer to top of stack
    // Stack grows downward on most architectures
    tcb->stack_pointer = stack + (params.stack_size / sizeof(uint32_t)) - 1;
    
    // Initialize stack with pattern for debugging
    memset(stack, 0xA5, params.stack_size);
    
    // Add task to scheduler
    Scheduler* scheduler = Scheduler::getInstance();
    RTOSResult result = scheduler->addTask(tcb);
    
    if (result != RTOS_OK) {
        delete[] stack;
        delete tcb;
        return result;
    }
    
    *handle = tcb;
    
    rtos_printf("[Task] Created task '%s' (stack: %d bytes)\n",
                params.name ? params.name : "unnamed", params.stack_size);
    
    return RTOS_OK;
}

// Delete a task
RTOSResult destroy(TaskHandle_t handle) {
    if (handle == nullptr) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    TaskControlBlock* tcb = static_cast<TaskControlBlock*>(handle);
    
    // Remove from scheduler
    Scheduler* scheduler = Scheduler::getInstance();
    RTOSResult result = scheduler->removeTask(tcb);
    
    if (result != RTOS_OK) {
        return result;
    }
    
    // Free resources
    if (tcb->stack_base != nullptr) {
        delete[] tcb->stack_base;
    }
    
    delete tcb;
    
    rtos_printf("[Task] Destroyed task\n");
    
    return RTOS_OK;
}

// Suspend a task
RTOSResult suspend(TaskHandle_t handle) {
    if (handle == nullptr) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    TaskControlBlock* tcb = static_cast<TaskControlBlock*>(handle);
    
    if (tcb->state == TASK_DELETED) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    tcb->state = TASK_SUSPENDED;
    
    // If suspending current task, yield
    Scheduler* scheduler = Scheduler::getInstance();
    if (tcb == scheduler->getCurrentTask()) {
        scheduler->yield();
    }
    
    return RTOS_OK;
}

// Resume a task
RTOSResult resume(TaskHandle_t handle) {
    if (handle == nullptr) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    TaskControlBlock* tcb = static_cast<TaskControlBlock*>(handle);
    
    if (tcb->state != TASK_SUSPENDED) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    tcb->state = TASK_READY;
    
    return RTOS_OK;
}

// Get current task handle
TaskHandle_t getCurrentTask() {
    Scheduler* scheduler = Scheduler::getInstance();
    return scheduler->getCurrentTask();
}

// Yield execution to next task
void yield() {
    Scheduler* scheduler = Scheduler::getInstance();
    scheduler->yield();
}

// Delay for specified ticks
void delay(TickType_t ticks) {
    Scheduler* scheduler = Scheduler::getInstance();
    scheduler->delay(ticks);
}

// Get task state
TaskState getState(TaskHandle_t handle) {
    if (handle == nullptr) {
        return TASK_DELETED;
    }
    
    TaskControlBlock* tcb = static_cast<TaskControlBlock*>(handle);
    return tcb->state;
}

// Get task priority
TaskPriority getPriority(TaskHandle_t handle) {
    if (handle == nullptr) {
        return PRIORITY_IDLE;
    }
    
    TaskControlBlock* tcb = static_cast<TaskControlBlock*>(handle);
    return tcb->priority;
}

// Set task priority
RTOSResult setPriority(TaskHandle_t handle, TaskPriority priority) {
    if (handle == nullptr) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    TaskControlBlock* tcb = static_cast<TaskControlBlock*>(handle);
    tcb->priority = priority;
    
    // If scheduler is priority-based, may need to reschedule
    // For now, just update the priority
    
    return RTOS_OK;
}

} // namespace Task
