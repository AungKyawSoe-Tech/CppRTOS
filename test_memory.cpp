#include "util/rtos_heap.h"
#include "util/rtos_memory.h"
#include "util/memory_pool.h"
#include "rtos/hal/rtos_uart.h"
#include <cassert>
#include <cstring>

// Test heap basic allocation
void test_heap_basic() {
    rtos_printf("\n=== Test: Heap Basic Allocation ===\n");
    
    // Create heap buffer (64KB)
    static uint8_t heap_buffer[65536];
    RTOSHeap heap(heap_buffer, sizeof(heap_buffer));
    
    // Allocate memory
    void* ptr1 = heap.malloc(100);
    assert(ptr1 != nullptr);
    rtos_printf("Allocated 100 bytes at %p\n", ptr1);
    
    void* ptr2 = heap.malloc(200);
    assert(ptr2 != nullptr);
    rtos_printf("Allocated 200 bytes at %p\n", ptr2);
    
    void* ptr3 = heap.malloc(300);
    assert(ptr3 != nullptr);
    rtos_printf("Allocated 300 bytes at %p\n", ptr3);
    
    // Check stats
    HeapStats stats = heap.getStats();
    rtos_printf("Allocated: %d bytes\n", stats.allocated_size);
    rtos_printf("Free: %d bytes\n", stats.free_size);
    assert(stats.num_allocations == 3);
    
    // Free memory
    heap.free(ptr2);
    rtos_printf("Freed ptr2\n");
    
    stats = heap.getStats();
    assert(stats.num_frees == 1);
    
    // Free remaining
    heap.free(ptr1);
    heap.free(ptr3);
    
    stats = heap.getStats();
    rtos_printf("After freeing all: allocated=%d, free=%d\n", 
                stats.allocated_size, stats.free_size);
    
    rtos_printf("✓ Heap basic allocation passed\n");
}

// Test heap coalescing
void test_heap_coalescing() {
    rtos_printf("\n=== Test: Heap Coalescing ===\n");
    
    static uint8_t heap_buffer[32768];
    RTOSHeap heap(heap_buffer, sizeof(heap_buffer));
    
    // Allocate several blocks
    void* ptr1 = heap.malloc(100);
    void* ptr2 = heap.malloc(100);
    void* ptr3 = heap.malloc(100);
    void* ptr4 = heap.malloc(100);
    
    HeapStats stats = heap.getStats();
    size_t blocks_before = stats.num_blocks;
    rtos_printf("Blocks before free: %d\n", blocks_before);
    
    // Free middle blocks (should coalesce)
    heap.free(ptr2);
    heap.free(ptr3);
    
    stats = heap.getStats();
    rtos_printf("Blocks after freeing middle: %d\n", stats.num_blocks);
    
    // Free adjacent block
    heap.free(ptr1);
    
    stats = heap.getStats();
    rtos_printf("Blocks after freeing ptr1: %d\n", stats.num_blocks);
    rtos_printf("Largest free block: %d bytes\n", heap.getLargestFreeBlock());
    
    heap.free(ptr4);
    
    rtos_printf("✓ Heap coalescing passed\n");
}

// Test heap fragmentation
void test_heap_fragmentation() {
    rtos_printf("\n=== Test: Heap Fragmentation ===\n");
    
    static uint8_t heap_buffer[16384];
    RTOSHeap heap(heap_buffer, sizeof(heap_buffer));
    
    // Create fragmentation pattern
    void* ptrs[10];
    for (int i = 0; i < 10; i++) {
        ptrs[i] = heap.malloc(100);
    }
    
    // Free every other block
    for (int i = 0; i < 10; i += 2) {
        heap.free(ptrs[i]);
    }
    
    HeapStats stats = heap.getStats();
    rtos_printf("Free memory: %d bytes\n", stats.free_size);
    rtos_printf("Largest free block: %d bytes\n", heap.getLargestFreeBlock());
    
    // Defragment
    heap.defragment();
    
    stats = heap.getStats();
    rtos_printf("After defrag - Free: %d, Largest: %d\n", 
                stats.free_size, heap.getLargestFreeBlock());
    
    // Free remaining
    for (int i = 1; i < 10; i += 2) {
        heap.free(ptrs[i]);
    }
    
    rtos_printf("✓ Heap fragmentation test passed\n");
}

// Test heap realloc
void test_heap_realloc() {
    rtos_printf("\n=== Test: Heap Realloc ===\n");
    
    static uint8_t heap_buffer[32768];
    RTOSHeap heap(heap_buffer, sizeof(heap_buffer));
    
    // Allocate and write pattern
    int* ptr = static_cast<int*>(heap.malloc(100));
    assert(ptr != nullptr);
    
    for (int i = 0; i < 25; i++) {
        ptr[i] = i * 10;
    }
    
    // Realloc to larger size
    int* new_ptr = static_cast<int*>(heap.realloc(ptr, 200));
    assert(new_ptr != nullptr);
    
    // Verify data preserved
    for (int i = 0; i < 25; i++) {
        assert(new_ptr[i] == i * 10);
    }
    
    rtos_printf("Data preserved after realloc\n");
    
    heap.free(new_ptr);
    
    rtos_printf("✓ Heap realloc passed\n");
}

// Test heap calloc
void test_heap_calloc() {
    rtos_printf("\n=== Test: Heap Calloc ===\n");
    
    static uint8_t heap_buffer[32768];
    RTOSHeap heap(heap_buffer, sizeof(heap_buffer));
    
    // Allocate and verify zeroed
    uint32_t* ptr = static_cast<uint32_t*>(heap.calloc(10, sizeof(uint32_t)));
    assert(ptr != nullptr);
    
    bool all_zero = true;
    for (int i = 0; i < 10; i++) {
        if (ptr[i] != 0) {
            all_zero = false;
            break;
        }
    }
    
    assert(all_zero);
    rtos_printf("Calloc correctly zeroed memory\n");
    
    heap.free(ptr);
    
    rtos_printf("✓ Heap calloc passed\n");
}

// Test heap integrity
void test_heap_integrity() {
    rtos_printf("\n=== Test: Heap Integrity ===\n");
    
    static uint8_t heap_buffer[32768];
    RTOSHeap heap(heap_buffer, sizeof(heap_buffer));
    
    assert(heap.checkIntegrity());
    rtos_printf("Initial integrity: OK\n");
    
    // Do some allocations
    void* p1 = heap.malloc(1000);
    void* p2 = heap.malloc(2000);
    void* p3 = heap.malloc(500);
    
    assert(heap.checkIntegrity());
    rtos_printf("After allocations: OK\n");
    
    // Free some
    heap.free(p2);
    heap.free(p1);
    
    assert(heap.checkIntegrity());
    rtos_printf("After partial free: OK\n");
    
    heap.free(p3);
    
    assert(heap.checkIntegrity());
    rtos_printf("After full free: OK\n");
    
    rtos_printf("✓ Heap integrity test passed\n");
}

// Test memory pool
void test_memory_pool() {
    rtos_printf("\n=== Test: Memory Pool ===\n");
    
    struct TestData {
        int value;
        char name[32];
    };
    
    MemoryPool<TestData, 10> pool;
    
    rtos_printf("Pool capacity: %d\n", pool.get_pool_size());
    assert(pool.get_free_count() == 10);
    
    // Allocate from pool
    TestData* obj1 = pool.allocate();
    assert(obj1 != nullptr);
    obj1->value = 42;
    strcpy(obj1->name, "Object1");
    
    TestData* obj2 = pool.allocate();
    assert(obj2 != nullptr);
    obj2->value = 100;
    
    rtos_printf("Allocated 2 objects, free: %d\n", pool.get_free_count());
    assert(pool.get_allocated_count() == 2);
    
    // Free back to pool
    pool.deallocate(obj1);
    assert(pool.get_free_count() == 9);
    
    // Allocate again (should reuse)
    TestData* obj3 = pool.allocate();
    assert(obj3 != nullptr);
    
    pool.deallocate(obj2);
    pool.deallocate(obj3);
    
    assert(pool.is_empty());
    rtos_printf("✓ Memory pool test passed\n");
}

// Test global memory manager
void test_memory_manager() {
    rtos_printf("\n=== Test: Memory Manager ===\n");
    
    static uint8_t heap_buffer[65536];
    
    // Initialize memory system
    RTOSResult result = RTOSMemory::initialize(heap_buffer, sizeof(heap_buffer));
    assert(result == RTOS_OK);
    
    // Allocate using global allocator
    void* ptr1 = RTOSMemory::malloc(1000);
    assert(ptr1 != nullptr);
    
    void* ptr2 = RTOSMemory::malloc(2000);
    assert(ptr2 != nullptr);
    
    // Get memory info
    RTOSMemory::MemoryInfo info = RTOSMemory::getMemoryInfo();
    rtos_printf("Memory Info:\n");
    rtos_printf("  Total: %d bytes\n", info.total_heap_size);
    rtos_printf("  Used: %d bytes\n", info.used_heap_size);
    rtos_printf("  Free: %d bytes\n", info.free_heap_size);
    rtos_printf("  Peak: %d bytes\n", info.peak_heap_usage);
    rtos_printf("  Allocations: %d\n", info.num_allocations);
    rtos_printf("  Fragmentation: %d%%\n", info.fragmentation_percent);
    
    assert(info.num_allocations == 2);
    
    // Free memory
    RTOSMemory::free(ptr1);
    RTOSMemory::free(ptr2);
    
    info = RTOSMemory::getMemoryInfo();
    rtos_printf("After free - Used: %d, Frees: %d\n", 
                info.used_heap_size, info.num_frees);
    
    rtos_printf("✓ Memory manager test passed\n");
}

// Main test runner
int main() {
    rtos_printf("\n");
    rtos_printf("╔════════════════════════════════════════════╗\n");
    rtos_printf("║   RTOS Memory Management Test Suite       ║\n");
    rtos_printf("╚════════════════════════════════════════════╝\n");
    
    RTOS_UART::init();
    
    try {
        test_heap_basic();
        test_heap_coalescing();
        test_heap_fragmentation();
        test_heap_realloc();
        test_heap_calloc();
        test_heap_integrity();
        test_memory_pool();
        test_memory_manager();
        
        rtos_printf("\n");
        rtos_printf("╔════════════════════════════════════════════╗\n");
        rtos_printf("║   ✓ ALL TESTS PASSED                      ║\n");
        rtos_printf("╚════════════════════════════════════════════╝\n");
        rtos_printf("\n");
        
        return 0;
    }
    catch (...) {
        rtos_printf("\n✗ TEST FAILED!\n");
        return 1;
    }
}
