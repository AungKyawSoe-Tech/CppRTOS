#include "semaphore.h"
#include "scheduler.h"
#include "../hal/rtos_uart.h"

namespace SemaphoreAPI {

// Create a binary semaphore
RTOSResult createBinary(SemaphoreHandle_t* handle, bool initial_available) {
    if (handle == nullptr) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    Semaphore* sem = new Semaphore(SEMAPHORE_BINARY, initial_available ? 1 : 0, 1);
    if (sem == nullptr) {
        return RTOS_ERR_NOMEM;
    }
    
    *handle = sem;
    rtos_printf("[Semaphore] Created binary semaphore (initial: %d)\n", initial_available);
    
    return RTOS_OK;
}

// Create a counting semaphore
RTOSResult createCounting(SemaphoreHandle_t* handle, uint32_t max_count, uint32_t initial_count) {
    if (handle == nullptr || max_count == 0 || initial_count > max_count) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    Semaphore* sem = new Semaphore(SEMAPHORE_COUNTING, initial_count, max_count);
    if (sem == nullptr) {
        return RTOS_ERR_NOMEM;
    }
    
    *handle = sem;
    rtos_printf("[Semaphore] Created counting semaphore (max: %d, initial: %d)\n", 
                max_count, initial_count);
    
    return RTOS_OK;
}

// Destroy a semaphore
RTOSResult destroy(SemaphoreHandle_t handle) {
    if (handle == nullptr) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    if (handle->waiting_list_head != nullptr) {
        rtos_printf("[Semaphore] WARNING: Destroying semaphore with waiting tasks!\n");
    }
    
    delete handle;
    return RTOS_OK;
}

// Take (wait/acquire) semaphore
RTOSResult take(SemaphoreHandle_t handle, TickType_t timeout) {
    if (handle == nullptr) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    Scheduler* sched = Scheduler::getInstance();
    
    // If semaphore is available, take it
    if (handle->count > 0) {
        handle->count--;
        return RTOS_OK;
    }
    
    // Semaphore not available
    if (timeout == 0) {
        return RTOS_ERR_BUSY;
    }
    
    // Wait for semaphore
    TickType_t start_tick = sched->getTickCount();
    
    while (handle->count == 0) {
        // Check timeout
        if (timeout != UINT32_MAX) {
            if ((sched->getTickCount() - start_tick) >= timeout) {
                return RTOS_ERR_TIMEOUT;
            }
        }
        
        // Yield to other tasks
        sched->yield();
    }
    
    // Acquired semaphore
    handle->count--;
    return RTOS_OK;
}

// Give (signal/release) semaphore
RTOSResult give(SemaphoreHandle_t handle) {
    if (handle == nullptr) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    // Check if we can increment
    if (handle->count >= handle->max_count) {
        rtos_printf("[Semaphore] WARNING: Semaphore already at max count\n");
        return RTOS_ERR_FULL;
    }
    
    // Increment count
    handle->count++;
    
    return RTOS_OK;
}

// Get current count
uint32_t getCount(SemaphoreHandle_t handle) {
    if (handle == nullptr) {
        return 0;
    }
    return handle->count;
}

// Try to take without blocking
RTOSResult tryTake(SemaphoreHandle_t handle) {
    return take(handle, 0);
}

} // namespace SemaphoreAPI
