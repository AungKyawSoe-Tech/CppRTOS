#include "rtos_memory.h"
#include "../rtos/hal/rtos_uart.h"

// Static members
RTOSHeap* RTOSMemory::system_heap = nullptr;
bool RTOSMemory::initialized = false;

// Initialize memory system
RTOSResult RTOSMemory::initialize(void* heap_buffer, size_t heap_size) {
    if (initialized) {
        return RTOS_ERR_ALREADY_EXISTS;
    }
    
    if (heap_buffer == nullptr || heap_size == 0) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    // Create heap in provided buffer
    system_heap = new (heap_buffer) RTOSHeap(
        static_cast<uint8_t*>(heap_buffer) + sizeof(RTOSHeap),
        heap_size - sizeof(RTOSHeap)
    );
    
    initialized = true;
    
    rtos_printf("[Memory] Initialized with %d bytes heap\n", heap_size);
    
    return RTOS_OK;
}

// Allocate memory
void* RTOSMemory::malloc(size_t size) {
    if (!initialized || system_heap == nullptr) {
        return nullptr;
    }
    
    return system_heap->malloc(size);
}

// Free memory
void RTOSMemory::free(void* ptr) {
    if (!initialized || system_heap == nullptr) {
        return;
    }
    
    system_heap->free(ptr);
}

// Reallocate memory
void* RTOSMemory::realloc(void* ptr, size_t new_size) {
    if (!initialized || system_heap == nullptr) {
        return nullptr;
    }
    
    return system_heap->realloc(ptr, new_size);
}

// Allocate and zero memory
void* RTOSMemory::calloc(size_t num, size_t size) {
    if (!initialized || system_heap == nullptr) {
        return nullptr;
    }
    
    return system_heap->calloc(num, size);
}

// Get heap statistics
HeapStats RTOSMemory::getHeapStats() {
    if (!initialized || system_heap == nullptr) {
        HeapStats empty = {0};
        return empty;
    }
    
    return system_heap->getStats();
}

// Check heap integrity
bool RTOSMemory::checkHeapIntegrity() {
    if (!initialized || system_heap == nullptr) {
        return false;
    }
    
    return system_heap->checkIntegrity();
}

// Defragment heap
void RTOSMemory::defragmentHeap() {
    if (!initialized || system_heap == nullptr) {
        return;
    }
    
    system_heap->defragment();
}

// Get memory info
RTOSMemory::MemoryInfo RTOSMemory::getMemoryInfo() {
    MemoryInfo info = {0};
    
    if (!initialized || system_heap == nullptr) {
        return info;
    }
    
    HeapStats stats = system_heap->getStats();
    
    info.total_heap_size = stats.total_size;
    info.free_heap_size = stats.free_size;
    info.used_heap_size = stats.allocated_size;
    info.peak_heap_usage = stats.peak_allocated;
    info.num_allocations = stats.num_allocations;
    info.num_frees = stats.num_frees;
    
    // Calculate fragmentation
    if (stats.free_size > 0) {
        size_t largest_free = system_heap->getLargestFreeBlock();
        info.fragmentation_percent = 100 - (largest_free * 100 / stats.free_size);
    } else {
        info.fragmentation_percent = 0;
    }
    
    return info;
}
