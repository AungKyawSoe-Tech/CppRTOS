#include "rtos_heap.h"
#include "../rtos/hal/rtos_uart.h"
#include <cstring>

// Constructor
RTOSHeap::RTOSHeap(void* buffer, size_t size) 
    : heap_start(static_cast<uint8_t*>(buffer))
    , heap_end(static_cast<uint8_t*>(buffer) + size)
    , heap_size(size)
    , lock_flag(false) {
    
    // Initialize first block (entire heap is one free block)
    first_block = reinterpret_cast<HeapBlock*>(heap_start);
    first_block->size = size - sizeof(HeapBlock);
    first_block->is_free = true;
    first_block->next = nullptr;
    first_block->prev = nullptr;
    first_block->magic = HEAP_MAGIC;
    
    last_block = first_block;
    
    // Initialize stats
    stats.total_size = size;
    stats.free_size = first_block->size;
    stats.allocated_size = 0;
    stats.peak_allocated = 0;
    stats.num_allocations = 0;
    stats.num_frees = 0;
    stats.num_blocks = 1;
    stats.largest_free_block = first_block->size;
}

// Simple spin lock
void RTOSHeap::lock() {
    while (lock_flag) {
        // Spin wait
    }
    lock_flag = true;
}

void RTOSHeap::unlock() {
    lock_flag = false;
}

// Find first free block that fits
HeapBlock* RTOSHeap::findFreeBlock(size_t size) {
    HeapBlock* current = first_block;
    
    while (current != nullptr) {
        if (current->is_free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    
    return nullptr;
}

// Split block if it's large enough
void RTOSHeap::splitBlock(HeapBlock* block, size_t size) {
    size_t remaining = block->size - size - sizeof(HeapBlock);
    
    // Only split if remainder is useful
    if (remaining >= MIN_BLOCK_SIZE) {
        // Create new block in remaining space
        HeapBlock* new_block = reinterpret_cast<HeapBlock*>(
            reinterpret_cast<uint8_t*>(block) + sizeof(HeapBlock) + size
        );
        
        new_block->size = remaining;
        new_block->is_free = true;
        new_block->next = block->next;
        new_block->prev = block;
        new_block->magic = HEAP_MAGIC;
        
        if (block->next != nullptr) {
            block->next->prev = new_block;
        }
        
        block->next = new_block;
        block->size = size;
        
        if (block == last_block) {
            last_block = new_block;
        }
        
        stats.num_blocks++;
    }
}

// Coalesce adjacent free blocks
void RTOSHeap::coalesceBlocks(HeapBlock* block) {
    if (block == nullptr || !block->is_free) {
        return;
    }
    
    // Coalesce with next block
    while (block->next != nullptr && block->next->is_free) {
        HeapBlock* next = block->next;
        
        block->size += sizeof(HeapBlock) + next->size;
        block->next = next->next;
        
        if (next->next != nullptr) {
            next->next->prev = block;
        }
        
        if (next == last_block) {
            last_block = block;
        }
        
        stats.num_blocks--;
    }
    
    // Coalesce with previous block
    if (block->prev != nullptr && block->prev->is_free) {
        coalesceBlocks(block->prev);
    }
}

// Validate block
bool RTOSHeap::validateBlock(HeapBlock* block) const {
    if (block == nullptr) {
        return false;
    }
    
    // Check magic number
    if (block->magic != HEAP_MAGIC) {
        return false;
    }
    
    // Check bounds
    uint8_t* block_addr = reinterpret_cast<uint8_t*>(block);
    if (block_addr < heap_start || block_addr >= heap_end) {
        return false;
    }
    
    return true;
}

// Update statistics
void RTOSHeap::updateStats() {
    stats.free_size = 0;
    stats.allocated_size = 0;
    stats.largest_free_block = 0;
    
    HeapBlock* current = first_block;
    while (current != nullptr) {
        if (current->is_free) {
            stats.free_size += current->size;
            if (current->size > stats.largest_free_block) {
                stats.largest_free_block = current->size;
            }
        } else {
            stats.allocated_size += current->size;
        }
        current = current->next;
    }
}

// Allocate memory
void* RTOSHeap::malloc(size_t size) {
    if (size == 0) {
        return nullptr;
    }
    
    // Align size to 8 bytes
    size = (size + 7) & ~7;
    
    lock();
    
    // Find free block
    HeapBlock* block = findFreeBlock(size);
    if (block == nullptr) {
        unlock();
        return nullptr;  // Out of memory
    }
    
    // Split if possible
    splitBlock(block, size);
    
    // Mark as allocated
    block->is_free = false;
    
    // Update stats
    stats.num_allocations++;
    stats.allocated_size += block->size;
    stats.free_size -= block->size;
    
    if (stats.allocated_size > stats.peak_allocated) {
        stats.peak_allocated = stats.allocated_size;
    }
    
    unlock();
    
    // Return pointer to usable memory (after header)
    return reinterpret_cast<void*>(
        reinterpret_cast<uint8_t*>(block) + sizeof(HeapBlock)
    );
}

// Free memory
void RTOSHeap::free(void* ptr) {
    if (ptr == nullptr) {
        return;
    }
    
    lock();
    
    // Get block header
    HeapBlock* block = reinterpret_cast<HeapBlock*>(
        static_cast<uint8_t*>(ptr) - sizeof(HeapBlock)
    );
    
    // Validate block
    if (!validateBlock(block)) {
        rtos_printf("[Heap] ERROR: Invalid block in free()\n");
        unlock();
        return;
    }
    
    if (block->is_free) {
        rtos_printf("[Heap] WARNING: Double free detected\n");
        unlock();
        return;
    }
    
    // Mark as free
    block->is_free = true;
    
    // Update stats
    stats.num_frees++;
    stats.allocated_size -= block->size;
    stats.free_size += block->size;
    
    // Coalesce adjacent free blocks
    coalesceBlocks(block);
    
    unlock();
}

// Reallocate memory
void* RTOSHeap::realloc(void* ptr, size_t new_size) {
    if (ptr == nullptr) {
        return malloc(new_size);
    }
    
    if (new_size == 0) {
        free(ptr);
        return nullptr;
    }
    
    // Get current block
    HeapBlock* block = reinterpret_cast<HeapBlock*>(
        static_cast<uint8_t*>(ptr) - sizeof(HeapBlock)
    );
    
    if (!validateBlock(block)) {
        return nullptr;
    }
    
    // If new size fits in current block, just return
    if (new_size <= block->size) {
        return ptr;
    }
    
    // Allocate new block
    void* new_ptr = malloc(new_size);
    if (new_ptr == nullptr) {
        return nullptr;
    }
    
    // Copy old data
    memcpy(new_ptr, ptr, block->size);
    
    // Free old block
    free(ptr);
    
    return new_ptr;
}

// Allocate and zero memory
void* RTOSHeap::calloc(size_t num, size_t size) {
    size_t total_size = num * size;
    void* ptr = malloc(total_size);
    
    if (ptr != nullptr) {
        memset(ptr, 0, total_size);
    }
    
    return ptr;
}

// Get statistics
HeapStats RTOSHeap::getStats() const {
    return stats;
}

// Check heap integrity
bool RTOSHeap::checkIntegrity() const {
    HeapBlock* current = first_block;
    
    while (current != nullptr) {
        if (!validateBlock(current)) {
            return false;
        }
        
        // Check that next's prev points back to current
        if (current->next != nullptr && current->next->prev != current) {
            return false;
        }
        
        current = current->next;
    }
    
    return true;
}

// Defragment (coalesce all free blocks)
void RTOSHeap::defragment() {
    lock();
    
    HeapBlock* current = first_block;
    while (current != nullptr) {
        if (current->is_free) {
            coalesceBlocks(current);
        }
        current = current->next;
    }
    
    updateStats();
    
    unlock();
}

// Get largest free block
size_t RTOSHeap::getLargestFreeBlock() const {
    size_t largest = 0;
    HeapBlock* current = first_block;
    
    while (current != nullptr) {
        if (current->is_free && current->size > largest) {
            largest = current->size;
        }
        current = current->next;
    }
    
    return largest;
}
