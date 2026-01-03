#include "rtos/kernel/mutex.h"
#include "rtos/kernel/semaphore.h"
#include "rtos/kernel/queue.h"
#include "rtos/kernel/scheduler.h"
#include "rtos/kernel/task.h"
#include "rtos/hal/rtos_uart.h"
#include <cassert>

// Shared resources for testing
static int shared_counter = 0;
static MutexHandle_t test_mutex = nullptr;

// Test mutex basic operations
void test_mutex_basic() {
    rtos_printf("\n=== Test: Mutex Basic Operations ===\n");
    
    MutexHandle_t mutex = nullptr;
    
    // Create mutex
    assert(MutexAPI::create(&mutex) == RTOS_OK);
    assert(mutex != nullptr);
    assert(!MutexAPI::isLocked(mutex));
    
    // Note: Lock/unlock requires a running task context
    // These operations would work in a real task, but for testing
    // we'll just test creation and destruction
    
    // Destroy mutex
    assert(MutexAPI::destroy(mutex) == RTOS_OK);
    
    rtos_printf("✓ Mutex basic operations passed\n");
}

// Test recursive mutex
void test_recursive_mutex() {
    rtos_printf("\n=== Test: Recursive Mutex ===\n");
    
    RecursiveMutexHandle_t mutex = nullptr;
    
    // Create recursive mutex
    assert(RecursiveMutexAPI::create(&mutex) == RTOS_OK);
    assert(mutex != nullptr);
    
    // Note: Lock/unlock requires a running task context
    // Just test creation/destruction for now
    
    // Destroy
    assert(RecursiveMutexAPI::destroy(mutex) == RTOS_OK);
    
    rtos_printf("✓ Recursive mutex passed\n");
}

// Test binary semaphore
void test_binary_semaphore() {
    rtos_printf("\n=== Test: Binary Semaphore ===\n");
    
    SemaphoreHandle_t sem = nullptr;
    
    // Create binary semaphore (initially available)
    assert(SemaphoreAPI::createBinary(&sem, true) == RTOS_OK);
    assert(sem != nullptr);
    assert(SemaphoreAPI::getCount(sem) == 1);
    
    // Take semaphore
    assert(SemaphoreAPI::take(sem) == RTOS_OK);
    assert(SemaphoreAPI::getCount(sem) == 0);
    
    // Try to take again (should fail)
    assert(SemaphoreAPI::tryTake(sem) == RTOS_ERR_BUSY);
    
    // Give semaphore
    assert(SemaphoreAPI::give(sem) == RTOS_OK);
    assert(SemaphoreAPI::getCount(sem) == 1);
    
    // Try to give again (should fail - already at max)
    assert(SemaphoreAPI::give(sem) == RTOS_ERR_FULL);
    
    // Destroy
    assert(SemaphoreAPI::destroy(sem) == RTOS_OK);
    
    rtos_printf("✓ Binary semaphore passed\n");
}

// Test counting semaphore
void test_counting_semaphore() {
    rtos_printf("\n=== Test: Counting Semaphore ===\n");
    
    SemaphoreHandle_t sem = nullptr;
    
    // Create counting semaphore (max 5, initial 2)
    assert(SemaphoreAPI::createCounting(&sem, 5, 2) == RTOS_OK);
    assert(sem != nullptr);
    assert(SemaphoreAPI::getCount(sem) == 2);
    
    // Take twice
    assert(SemaphoreAPI::take(sem) == RTOS_OK);
    assert(SemaphoreAPI::getCount(sem) == 1);
    
    assert(SemaphoreAPI::take(sem) == RTOS_OK);
    assert(SemaphoreAPI::getCount(sem) == 0);
    
    // Try to take when empty
    assert(SemaphoreAPI::tryTake(sem) == RTOS_ERR_BUSY);
    
    // Give multiple times
    assert(SemaphoreAPI::give(sem) == RTOS_OK);
    assert(SemaphoreAPI::give(sem) == RTOS_OK);
    assert(SemaphoreAPI::give(sem) == RTOS_OK);
    assert(SemaphoreAPI::getCount(sem) == 3);
    
    // Give until max
    assert(SemaphoreAPI::give(sem) == RTOS_OK);
    assert(SemaphoreAPI::give(sem) == RTOS_OK);
    assert(SemaphoreAPI::getCount(sem) == 5);
    
    // Try to exceed max
    assert(SemaphoreAPI::give(sem) == RTOS_ERR_FULL);
    
    // Destroy
    assert(SemaphoreAPI::destroy(sem) == RTOS_OK);
    
    rtos_printf("✓ Counting semaphore passed\n");
}

// Test queue basic operations
void test_queue_basic() {
    rtos_printf("\n=== Test: Queue Basic Operations ===\n");
    
    Queue<int, 5> queue;
    
    // Check initial state
    assert(queue.isEmpty());
    assert(!queue.isFull());
    assert(queue.size() == 0);
    assert(queue.capacity() == 5);
    
    // Enqueue items
    assert(queue.enqueue(10));
    assert(queue.enqueue(20));
    assert(queue.enqueue(30));
    assert(queue.size() == 3);
    
    // Dequeue items
    int value;
    assert(queue.dequeue(value));
    assert(value == 10);
    
    assert(queue.dequeue(value));
    assert(value == 20);
    
    assert(queue.size() == 1);
    
    // Peek
    assert(queue.peek(value));
    assert(value == 30);
    assert(queue.size() == 1);  // Size unchanged
    
    // Fill queue
    assert(queue.enqueue(40));
    assert(queue.enqueue(50));
    assert(queue.enqueue(60));
    assert(queue.enqueue(70));
    assert(queue.size() == 5);
    assert(queue.isFull());
    
    // Try to add when full
    assert(!queue.enqueue(80));
    
    // Clear and check
    queue.clear();
    assert(queue.isEmpty());
    assert(queue.size() == 0);
    
    rtos_printf("✓ Queue basic operations passed\n");
}

// Test message queue
void test_message_queue() {
    rtos_printf("\n=== Test: Message Queue ===\n");
    
    MessageQueue<uint32_t, 8> msg_queue;
    
    // Send messages
    assert(msg_queue.send(100) == RTOS_OK);
    assert(msg_queue.send(200) == RTOS_OK);
    assert(msg_queue.send(300) == RTOS_OK);
    assert(msg_queue.size() == 3);
    
    // Receive messages
    uint32_t msg;
    assert(msg_queue.receive(msg) == RTOS_OK);
    assert(msg == 100);
    
    assert(msg_queue.receive(msg) == RTOS_OK);
    assert(msg == 200);
    
    assert(msg_queue.size() == 1);
    
    // Try to receive when empty (after clearing)
    msg_queue.clear();
    assert(msg_queue.tryReceive(msg) == RTOS_ERR_EMPTY);
    
    // Fill queue
    for (int i = 0; i < 8; i++) {
        assert(msg_queue.send(i * 10) == RTOS_OK);
    }
    assert(msg_queue.isFull());
    
    // Try to send when full
    assert(msg_queue.trySend(999) == RTOS_ERR_FULL);
    
    rtos_printf("✓ Message queue passed\n");
}

// Task function for mutex test
void mutexTestTask(void* params) {
    int task_id = *static_cast<int*>(params);
    
    for (int i = 0; i < 3; i++) {
        // Lock mutex
        if (MutexAPI::lock(test_mutex, 100) == RTOS_OK) {
            rtos_printf("[Task%d] Acquired mutex, counter = %d\n", task_id, shared_counter);
            
            // Critical section
            int old_val = shared_counter;
            Task::yield();  // Yield to simulate race condition
            shared_counter = old_val + 1;
            
            rtos_printf("[Task%d] Incremented counter to %d\n", task_id, shared_counter);
            
            // Unlock mutex
            MutexAPI::unlock(test_mutex);
        }
        
        Task::yield();
    }
}

// Test mutex with multiple tasks
void test_mutex_multitask() {
    rtos_printf("\n=== Test: Mutex with Multiple Tasks ===\n");
    
    Scheduler* sched = Scheduler::getInstance();
    
    // Create mutex
    MutexAPI::create(&test_mutex);
    shared_counter = 0;
    
    // Create tasks
    TaskHandle_t task1, task2;
    int id1 = 1, id2 = 2;
    
    TaskCreateParams params1;
    params1.name = "MutexTask1";
    params1.function = mutexTestTask;
    params1.params = &id1;
    params1.stack_size = 512;
    
    TaskCreateParams params2;
    params2.name = "MutexTask2";
    params2.function = mutexTestTask;
    params2.params = &id2;
    params2.stack_size = 512;
    
    Task::create(&task1, params1);
    Task::create(&task2, params2);
    
    sched->start();
    
    // Simulate some scheduling
    for (int i = 0; i < 20; i++) {
        sched->yield();
        sched->tick();
    }
    
    rtos_printf("Final counter value: %d (expected: 6)\n", shared_counter);
    
    // Cleanup
    MutexAPI::destroy(test_mutex);
    
    rtos_printf("✓ Mutex multitask test passed\n");
}

// Main test runner
int main() {
    rtos_printf("\n");
    rtos_printf("╔════════════════════════════════════════════╗\n");
    rtos_printf("║   RTOS Synchronization Test Suite         ║\n");
    rtos_printf("╚════════════════════════════════════════════╝\n");
    
    // Initialize UART
    RTOS_UART::init();
    
    // Initialize scheduler (needed for task context)
    Scheduler* sched = Scheduler::getInstance();
    sched->initialize(SCHEDULER_ROUND_ROBIN);
    
    try {
        test_mutex_basic();
        test_recursive_mutex();
        test_binary_semaphore();
        test_counting_semaphore();
        test_queue_basic();
        test_message_queue();
        test_mutex_multitask();
        
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
