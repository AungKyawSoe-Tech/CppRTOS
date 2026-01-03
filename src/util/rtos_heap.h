#ifndef RTOS_HEAP_H
#define RTOS_HEAP_H

#include <cstddef>
#include <cstdint>
#include "rtos/rtos_types.h"

// Heap block header
struct HeapBlock {
    size_t size;              // Size of usable memory (not including header)
    bool is_free;             // Free/allocated flag
    HeapBlock* next;          // Next block in list
    HeapBlock* prev;          // Previous block in list
    uint32_t magic;           // Magic number for validation
};

// Heap statistics
struct HeapStats {
    size_t total_size;        // Total heap size
    size_t free_size;         // Current free memory
    size_t allocated_size;    // Current allocated memory
    size_t peak_allocated;    // Peak allocation
    size_t num_allocations;   // Total allocations
    size_t num_frees;         // Total frees
    size_t num_blocks;        // Number of blocks
    size_t largest_free_block; // Largest contiguous free block
};

// RTOS Heap Allocator
// Simple first-fit allocator with coalescing
class RTOSHeap {
private:
    static const uint32_t HEAP_MAGIC = 0xDEADBEEF;
    static const size_t MIN_BLOCK_SIZE = 16;
    
    uint8_t* heap_start;
    uint8_t* heap_end;
    size_t heap_size;
    
    HeapBlock* first_block;
    HeapBlock* last_block;
    
    // Statistics
    HeapStats stats;
    
    // Thread safety (simple flag for now)
    volatile bool lock_flag;
    
    // Helper methods
    void lock();
    void unlock();
    HeapBlock* findFreeBlock(size_t size);
    void splitBlock(HeapBlock* block, size_t size);
    void coalesceBlocks(HeapBlock* block);
    bool validateBlock(HeapBlock* block) const;
    void updateStats();
    
public:
    // Constructor - initialize heap with static buffer
    RTOSHeap(void* buffer, size_t size);
    
    // Allocate memory
    void* malloc(size_t size);
    
    // Free memory
    void free(void* ptr);
    
    // Reallocate memory
    void* realloc(void* ptr, size_t new_size);
    
    // Allocate and zero memory
    void* calloc(size_t num, size_t size);
    
    // Get heap statistics
    HeapStats getStats() const;
    
    // Check heap integrity
    bool checkIntegrity() const;
    
    // Defragment heap (compact free blocks)
    void defragment();
    
    // Get largest available block
    size_t getLargestFreeBlock() const;
};

#endif // RTOS_HEAP_H
