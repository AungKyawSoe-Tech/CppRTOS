#ifndef RTOS_TYPES_H
#define RTOS_TYPES_H

#include <cstdint>

// RTOS result codes (replacing exceptions)
enum RTOSResult {
    RTOS_OK = 0,
    RTOS_ERR_NOMEM,          // Out of memory
    RTOS_ERR_INVALID_PARAM,  // Invalid parameter
    RTOS_ERR_TIMEOUT,        // Operation timed out
    RTOS_ERR_NOT_FOUND,      // Resource not found
    RTOS_ERR_ALREADY_EXISTS, // Resource already exists
    RTOS_ERR_BUSY,           // Resource busy
    RTOS_ERR_NOT_READY,      // Not ready
    RTOS_ERR_IO,             // I/O error
    RTOS_ERR_FULL,           // Container full
    RTOS_ERR_EMPTY,          // Container empty
    RTOS_ERR_GENERIC         // Generic error
};

// Task states
enum TaskState {
    TASK_READY = 0,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_SUSPENDED,
    TASK_DELETED
};

// Task priority levels
enum TaskPriority {
    PRIORITY_IDLE = 0,
    PRIORITY_LOW = 1,
    PRIORITY_NORMAL = 2,
    PRIORITY_HIGH = 3,
    PRIORITY_REALTIME = 4
};

// Type aliases
using TaskHandle_t = void*;
using TickType_t = uint32_t;

#endif // RTOS_TYPES_H
