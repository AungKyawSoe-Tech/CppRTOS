#ifndef RTOS_MUTEX_H
#define RTOS_MUTEX_H

#include "../rtos_types.h"
#include "task.h"

// Mutex structure
struct Mutex {
    bool is_locked;
    TaskControlBlock* owner;
    TaskControlBlock* waiting_list_head;  // Tasks waiting for this mutex
    uint32_t recursive_count;              // For recursive mutexes
    
    Mutex() 
        : is_locked(false), owner(nullptr), 
          waiting_list_head(nullptr), recursive_count(0) {}
};

// Mutex handle
typedef Mutex* MutexHandle_t;

// Mutex API
namespace MutexAPI {
    // Create a mutex
    RTOSResult create(MutexHandle_t* handle);
    
    // Destroy a mutex
    RTOSResult destroy(MutexHandle_t handle);
    
    // Lock mutex (blocking)
    RTOSResult lock(MutexHandle_t handle, TickType_t timeout = 0);
    
    // Try to lock mutex (non-blocking)
    RTOSResult tryLock(MutexHandle_t handle);
    
    // Unlock mutex
    RTOSResult unlock(MutexHandle_t handle);
    
    // Get mutex owner
    TaskControlBlock* getOwner(MutexHandle_t handle);
    
    // Check if mutex is locked
    bool isLocked(MutexHandle_t handle);
}

// Recursive Mutex (allows same task to lock multiple times)
struct RecursiveMutex {
    Mutex base_mutex;
    
    RecursiveMutex() {}
};

typedef RecursiveMutex* RecursiveMutexHandle_t;

namespace RecursiveMutexAPI {
    RTOSResult create(RecursiveMutexHandle_t* handle);
    RTOSResult destroy(RecursiveMutexHandle_t handle);
    RTOSResult lock(RecursiveMutexHandle_t handle, TickType_t timeout = 0);
    RTOSResult unlock(RecursiveMutexHandle_t handle);
}

#endif // RTOS_MUTEX_H
