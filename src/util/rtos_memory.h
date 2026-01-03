#ifndef RTOS_MEMORY_H
#define RTOS_MEMORY_H

#include "rtos_heap.h"
#include "memory_pool.h"
#include "../rtos/rtos_types.h"

// Global memory manager for RTOS
class RTOSMemory {
private:
    static RTOSHeap* system_heap;
    static bool initialized;
    
public:
    // Initialize memory system with static heap
    static RTOSResult initialize(void* heap_buffer, size_t heap_size);
    
    // Allocate memory from system heap
    static void* malloc(size_t size);
    
    // Free memory to system heap
    static void free(void* ptr);
    
    // Reallocate memory
    static void* realloc(void* ptr, size_t new_size);
    
    // Allocate and zero memory
    static void* calloc(size_t num, size_t size);
    
    // Get heap statistics
    static HeapStats getHeapStats();
    
    // Check heap integrity
    static bool checkHeapIntegrity();
    
    // Defragment heap
    static void defragmentHeap();
    
    // Get memory info
    struct MemoryInfo {
        size_t total_heap_size;
        size_t free_heap_size;
        size_t used_heap_size;
        size_t peak_heap_usage;
        size_t num_allocations;
        size_t num_frees;
        size_t fragmentation_percent;  // Percentage of fragmentation
    };
    
    static MemoryInfo getMemoryInfo();
};

#endif // RTOS_MEMORY_H
