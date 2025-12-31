#include "util/rtos_string.h"
#include "rtos/hal/rtos_uart.h"
#include "util/memory_pool.h"
#include <cassert>

// Test RTOSString
void test_rtos_string() {
    rtos_printf("\n=== Testing RTOSString ===\n");
    
    // Constructor tests
    RTOSString s1;
    assert(s1.empty());
    assert(s1.size() == 0);
    
    RTOSString s2("Hello");
    assert(s2.size() == 5);
    assert(s2 == "Hello");
    
    // Copy constructor
    RTOSString s3(s2);
    assert(s3 == s2);
    
    // Assignment
    s1 = "World";
    assert(s1 == "World");
    
    // Append
    s1.append(" Test");
    assert(s1 == "World Test");
    
    // Find
    assert(s1.find('W') == 0);
    assert(s1.find('T') == 6);
    
    // Substring
    RTOSString s4 = s1.substr(0, 5);
    assert(s4 == "World");
    
    rtos_printf("✓ RTOSString tests passed\n");
}

// Test Memory Pool
void test_memory_pool() {
    rtos_printf("\n=== Testing Memory Pool ===\n");
    
    struct TestData {
        int value;
        TestData() : value(0) {}
    };
    
    MemoryPool<TestData, 10> pool;
    
    assert(pool.get_free_count() == 10);
    assert(pool.is_empty());
    
    // Allocate some objects
    TestData* obj1 = pool.allocate();
    TestData* obj2 = pool.allocate();
    TestData* obj3 = pool.allocate();
    
    assert(obj1 != nullptr);
    assert(obj2 != nullptr);
    assert(obj3 != nullptr);
    assert(pool.get_allocated_count() == 3);
    assert(pool.get_free_count() == 7);
    
    // Use objects
    obj1->value = 100;
    obj2->value = 200;
    obj3->value = 300;
    
    // Deallocate
    pool.deallocate(obj2);
    assert(pool.get_allocated_count() == 2);
    assert(pool.get_free_count() == 8);
    
    // Allocate again (should reuse freed memory)
    TestData* obj4 = pool.allocate();
    assert(obj4 != nullptr);
    assert(pool.get_allocated_count() == 3);
    
    // Clean up
    pool.deallocate(obj1);
    pool.deallocate(obj3);
    pool.deallocate(obj4);
    
    assert(pool.is_empty());
    assert(pool.get_free_count() == 10);
    
    rtos_printf("✓ Memory Pool tests passed\n");
}

// Test UART output
void test_uart_output() {
    rtos_printf("\n=== Testing UART Output ===\n");
    
    RTOS_UART::init();
    
    rtos_puts("Simple string output\n");
    rtos_printf("Formatted output: %d %s %c\n", 42, "test", 'X');
    
    rtos_printf("✓ UART output tests passed\n");
}

int main() {
    rtos_printf("========================================\n");
    rtos_printf("RTOS Utility Test Suite\n");
    rtos_printf("========================================\n");
    
    try {
        test_uart_output();
        test_rtos_string();
        test_memory_pool();
        
        rtos_printf("\n========================================\n");
        rtos_printf("✓ ALL TESTS PASSED!\n");
        rtos_printf("========================================\n");
        
        return 0;
    } catch (...) {
        rtos_printf("\n✗ TEST FAILED!\n");
        return 1;
    }
}
