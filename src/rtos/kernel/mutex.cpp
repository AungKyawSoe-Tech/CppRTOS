#include "mutex.h"
#include "scheduler.h"
#include "../hal/rtos_uart.h"

namespace MutexAPI {

// Create a mutex
RTOSResult create(MutexHandle_t* handle) {
    if (handle == nullptr) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    Mutex* mutex = new Mutex();
    if (mutex == nullptr) {
        return RTOS_ERR_NOMEM;
    }
    
    *handle = mutex;
    rtos_printf("[Mutex] Created mutex\n");
    
    return RTOS_OK;
}

// Destroy a mutex
RTOSResult destroy(MutexHandle_t handle) {
    if (handle == nullptr) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    if (handle->is_locked) {
        rtos_printf("[Mutex] WARNING: Destroying locked mutex!\n");
    }
    
    delete handle;
    return RTOS_OK;
}

// Lock mutex (blocking)
RTOSResult lock(MutexHandle_t handle, TickType_t timeout) {
    if (handle == nullptr) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    Scheduler* sched = Scheduler::getInstance();
    TaskControlBlock* current = sched->getCurrentTask();
    
    if (current == nullptr) {
        return RTOS_ERR_NOT_READY;
    }
    
    // If mutex is not locked, acquire it
    if (!handle->is_locked) {
        handle->is_locked = true;
        handle->owner = current;
        handle->recursive_count = 1;
        return RTOS_OK;
    }
    
    // If current task already owns it, error (use recursive mutex instead)
    if (handle->owner == current) {
        rtos_printf("[Mutex] ERROR: Task already owns mutex (use recursive mutex)\n");
        return RTOS_ERR_BUSY;
    }
    
    // Mutex is locked by another task
    if (timeout == 0) {
        // Non-blocking, return immediately
        return RTOS_ERR_BUSY;
    }
    
    // Block current task and add to waiting list
    // For now, simplified implementation - just spin wait
    TickType_t start_tick = sched->getTickCount();
    
    while (handle->is_locked) {
        // Check timeout
        if (timeout != UINT32_MAX) {  // UINT32_MAX = infinite
            if ((sched->getTickCount() - start_tick) >= timeout) {
                return RTOS_ERR_TIMEOUT;
            }
        }
        
        // Yield to other tasks
        sched->yield();
    }
    
    // Acquired mutex
    handle->is_locked = true;
    handle->owner = current;
    handle->recursive_count = 1;
    
    return RTOS_OK;
}

// Try to lock mutex (non-blocking)
RTOSResult tryLock(MutexHandle_t handle) {
    return lock(handle, 0);
}

// Unlock mutex
RTOSResult unlock(MutexHandle_t handle) {
    if (handle == nullptr) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    if (!handle->is_locked) {
        rtos_printf("[Mutex] ERROR: Mutex not locked\n");
        return RTOS_ERR_INVALID_PARAM;
    }
    
    Scheduler* sched = Scheduler::getInstance();
    TaskControlBlock* current = sched->getCurrentTask();
    
    // Check if current task owns the mutex
    if (handle->owner != current) {
        rtos_printf("[Mutex] ERROR: Task doesn't own mutex\n");
        return RTOS_ERR_INVALID_PARAM;
    }
    
    // Release mutex
    handle->is_locked = false;
    handle->owner = nullptr;
    handle->recursive_count = 0;
    
    return RTOS_OK;
}

// Get mutex owner
TaskControlBlock* getOwner(MutexHandle_t handle) {
    if (handle == nullptr) {
        return nullptr;
    }
    return handle->owner;
}

// Check if mutex is locked
bool isLocked(MutexHandle_t handle) {
    if (handle == nullptr) {
        return false;
    }
    return handle->is_locked;
}

} // namespace MutexAPI

// Recursive Mutex Implementation
namespace RecursiveMutexAPI {

RTOSResult create(RecursiveMutexHandle_t* handle) {
    if (handle == nullptr) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    RecursiveMutex* mutex = new RecursiveMutex();
    if (mutex == nullptr) {
        return RTOS_ERR_NOMEM;
    }
    
    *handle = mutex;
    rtos_printf("[RecursiveMutex] Created recursive mutex\n");
    
    return RTOS_OK;
}

RTOSResult destroy(RecursiveMutexHandle_t handle) {
    if (handle == nullptr) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    delete handle;
    return RTOS_OK;
}

RTOSResult lock(RecursiveMutexHandle_t handle, TickType_t timeout) {
    if (handle == nullptr) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    Scheduler* sched = Scheduler::getInstance();
    TaskControlBlock* current = sched->getCurrentTask();
    
    if (current == nullptr) {
        return RTOS_ERR_NOT_READY;
    }
    
    // If not locked, acquire it
    if (!handle->base_mutex.is_locked) {
        handle->base_mutex.is_locked = true;
        handle->base_mutex.owner = current;
        handle->base_mutex.recursive_count = 1;
        return RTOS_OK;
    }
    
    // If current task owns it, increment count
    if (handle->base_mutex.owner == current) {
        handle->base_mutex.recursive_count++;
        return RTOS_OK;
    }
    
    // Locked by another task - wait
    TickType_t start_tick = sched->getTickCount();
    
    while (handle->base_mutex.is_locked) {
        if (timeout != UINT32_MAX) {
            if ((sched->getTickCount() - start_tick) >= timeout) {
                return RTOS_ERR_TIMEOUT;
            }
        }
        sched->yield();
    }
    
    // Acquired
    handle->base_mutex.is_locked = true;
    handle->base_mutex.owner = current;
    handle->base_mutex.recursive_count = 1;
    
    return RTOS_OK;
}

RTOSResult unlock(RecursiveMutexHandle_t handle) {
    if (handle == nullptr) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    if (!handle->base_mutex.is_locked) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    Scheduler* sched = Scheduler::getInstance();
    TaskControlBlock* current = sched->getCurrentTask();
    
    if (handle->base_mutex.owner != current) {
        rtos_printf("[RecursiveMutex] ERROR: Task doesn't own mutex\n");
        return RTOS_ERR_INVALID_PARAM;
    }
    
    // Decrement recursive count
    handle->base_mutex.recursive_count--;
    
    // Only release when count reaches zero
    if (handle->base_mutex.recursive_count == 0) {
        handle->base_mutex.is_locked = false;
        handle->base_mutex.owner = nullptr;
    }
    
    return RTOS_OK;
}

} // namespace RecursiveMutexAPI
