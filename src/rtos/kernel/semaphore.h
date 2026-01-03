#ifndef RTOS_SEMAPHORE_H
#define RTOS_SEMAPHORE_H

#include "../rtos_types.h"
#include "task.h"

// Semaphore types
enum SemaphoreType {
    SEMAPHORE_BINARY,    // Binary semaphore (0 or 1)
    SEMAPHORE_COUNTING   // Counting semaphore (0 to max_count)
};

// Semaphore structure
struct Semaphore {
    SemaphoreType type;
    uint32_t count;
    uint32_t max_count;
    TaskControlBlock* waiting_list_head;
    
    Semaphore(SemaphoreType t, uint32_t initial, uint32_t max)
        : type(t), count(initial), max_count(max), waiting_list_head(nullptr) {}
};

// Semaphore handle
typedef Semaphore* SemaphoreHandle_t;

// Semaphore API
namespace SemaphoreAPI {
    // Create a binary semaphore
    RTOSResult createBinary(SemaphoreHandle_t* handle, bool initial_available = true);
    
    // Create a counting semaphore
    RTOSResult createCounting(SemaphoreHandle_t* handle, uint32_t max_count, uint32_t initial_count = 0);
    
    // Destroy a semaphore
    RTOSResult destroy(SemaphoreHandle_t handle);
    
    // Take (wait/acquire) semaphore
    RTOSResult take(SemaphoreHandle_t handle, TickType_t timeout = 0);
    
    // Give (signal/release) semaphore
    RTOSResult give(SemaphoreHandle_t handle);
    
    // Get current count
    uint32_t getCount(SemaphoreHandle_t handle);
    
    // Try to take without blocking
    RTOSResult tryTake(SemaphoreHandle_t handle);
}

#endif // RTOS_SEMAPHORE_H
