#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <cstddef>
#include <cstdint>
#include <new>
#include "../rtos/rtos_types.h"

// Fixed-size memory pool allocator (no dynamic allocation)
template <typename T, size_t POOL_SIZE>
class MemoryPool {
private:
    struct Block {
        alignas(T) uint8_t storage[sizeof(T)];
        Block* next_free;
        bool allocated;
    };
    
    Block pool[POOL_SIZE];
    Block* free_list;
    size_t allocated_count;
    
public:
    MemoryPool();
    
    // Allocate object from pool
    T* allocate();
    
    // Deallocate object back to pool
    void deallocate(T* ptr);
    
    // Get statistics
    size_t get_allocated_count() const { return allocated_count; }
    size_t get_free_count() const { return POOL_SIZE - allocated_count; }
    size_t get_pool_size() const { return POOL_SIZE; }
    bool is_full() const { return allocated_count >= POOL_SIZE; }
    bool is_empty() const { return allocated_count == 0; }
};

// Implementation
template <typename T, size_t POOL_SIZE>
MemoryPool<T, POOL_SIZE>::MemoryPool() : free_list(nullptr), allocated_count(0) {
    // Initialize free list
    for (size_t i = 0; i < POOL_SIZE; i++) {
        pool[i].allocated = false;
        pool[i].next_free = (i < POOL_SIZE - 1) ? &pool[i + 1] : nullptr;
    }
    free_list = &pool[0];
}

template <typename T, size_t POOL_SIZE>
T* MemoryPool<T, POOL_SIZE>::allocate() {
    if (!free_list) {
        return nullptr; // Pool exhausted
    }
    
    Block* block = free_list;
    free_list = block->next_free;
    block->allocated = true;
    allocated_count++;
    
    // Get pointer to storage and use placement new
    T* ptr = reinterpret_cast<T*>(block->storage);
    return new(ptr) T();
}

template <typename T, size_t POOL_SIZE>
void MemoryPool<T, POOL_SIZE>::deallocate(T* ptr) {
    if (!ptr) return;
    
    // Find the block containing this pointer
    Block* block = nullptr;
    for (size_t i = 0; i < POOL_SIZE; i++) {
        if (reinterpret_cast<T*>(pool[i].storage) == ptr) {
            block = &pool[i];
            break;
        }
    }
    
    if (!block || !block->allocated) {
        return; // Invalid pointer or double free
    }
    
    // Call destructor
    ptr->~T();
    
    // Return to free list
    block->allocated = false;
    block->next_free = free_list;
    free_list = block;
    allocated_count--;
}

#endif // MEMORY_POOL_H
