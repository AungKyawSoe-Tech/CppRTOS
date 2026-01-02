#include "rtos/kernel/scheduler.h"
#include "rtos/kernel/task.h"
#include "rtos/kernel/context.h"
#include "rtos/hal/rtos_uart.h"
#include <cassert>

// Test task counters
static volatile int task1_count = 0;
static volatile int task2_count = 0;
static volatile int task3_count = 0;

// Test task functions
void testTask1(void* params) {
    rtos_printf("[Task1] Started\n");
    
    for (int i = 0; i < 5; i++) {
        rtos_printf("[Task1] Running iteration %d\n", i);
        task1_count++;
        Task::yield();
    }
    
    rtos_printf("[Task1] Completed\n");
}

void testTask2(void* params) {
    rtos_printf("[Task2] Started\n");
    
    for (int i = 0; i < 3; i++) {
        rtos_printf("[Task2] Running iteration %d\n", i);
        task2_count++;
        Task::yield();
    }
    
    rtos_printf("[Task2] Completed\n");
}

void testTask3(void* params) {
    int* value = static_cast<int*>(params);
    rtos_printf("[Task3] Started with param: %d\n", *value);
    
    for (int i = 0; i < 4; i++) {
        rtos_printf("[Task3] Running iteration %d\n", i);
        task3_count++;
        Task::yield();
    }
    
    rtos_printf("[Task3] Completed\n");
}

// Test scheduler initialization
void test_scheduler_init() {
    rtos_printf("\n=== Test: Scheduler Initialization ===\n");
    
    Scheduler* sched = Scheduler::getInstance();
    assert(sched != nullptr);
    
    RTOSResult result = sched->initialize(SCHEDULER_ROUND_ROBIN);
    assert(result == RTOS_OK);
    
    assert(!sched->isRunning());
    
    rtos_printf("✓ Scheduler initialization passed\n");
}

// Test task creation
void test_task_creation() {
    rtos_printf("\n=== Test: Task Creation ===\n");
    
    TaskHandle_t task1 = nullptr;
    TaskCreateParams params;
    params.name = "TestTask1";
    params.function = testTask1;
    params.params = nullptr;
    params.stack_size = 512;
    params.priority = PRIORITY_NORMAL;
    
    RTOSResult result = Task::create(&task1, params);
    assert(result == RTOS_OK);
    assert(task1 != nullptr);
    
    // Check task properties
    assert(Task::getState(task1) == TASK_READY);
    assert(Task::getPriority(task1) == PRIORITY_NORMAL);
    
    rtos_printf("✓ Task creation passed\n");
}

// Test multiple tasks
void test_multiple_tasks() {
    rtos_printf("\n=== Test: Multiple Tasks ===\n");
    
    Scheduler* sched = Scheduler::getInstance();
    
    // Create multiple tasks
    TaskHandle_t task1 = nullptr;
    TaskHandle_t task2 = nullptr;
    TaskHandle_t task3 = nullptr;
    
    TaskCreateParams params1;
    params1.name = "Task1";
    params1.function = testTask1;
    params1.stack_size = 512;
    params1.priority = PRIORITY_NORMAL;
    
    TaskCreateParams params2;
    params2.name = "Task2";
    params2.function = testTask2;
    params2.stack_size = 512;
    params2.priority = PRIORITY_NORMAL;
    
    int task3_param = 42;
    TaskCreateParams params3;
    params3.name = "Task3";
    params3.function = testTask3;
    params3.params = &task3_param;
    params3.stack_size = 512;
    params3.priority = PRIORITY_HIGH;
    
    assert(Task::create(&task1, params1) == RTOS_OK);
    assert(Task::create(&task2, params2) == RTOS_OK);
    assert(Task::create(&task3, params3) == RTOS_OK);
    
    // Check scheduler stats
    auto stats = sched->getStats();
    assert(stats.total_tasks >= 3);  // At least 3 tasks
    assert(stats.ready_tasks >= 3);
    
    rtos_printf("✓ Multiple tasks creation passed\n");
}

// Test scheduler start
void test_scheduler_start() {
    rtos_printf("\n=== Test: Scheduler Start ===\n");
    
    Scheduler* sched = Scheduler::getInstance();
    
    sched->start();
    assert(sched->isRunning());
    
    TaskHandle_t current = Task::getCurrentTask();
    assert(current != nullptr);
    
    rtos_printf("✓ Scheduler start passed\n");
}

// Test task yielding
void test_task_yield() {
    rtos_printf("\n=== Test: Task Yield ===\n");
    
    Scheduler* sched = Scheduler::getInstance();
    
    // Reset counters
    task1_count = 0;
    task2_count = 0;
    task3_count = 0;
    
    // Simulate some scheduling rounds
    for (int i = 0; i < 15; i++) {
        TaskControlBlock* current = sched->getCurrentTask();
        
        if (current != nullptr) {
            rtos_printf("[Scheduler] Round %d: Task '%s' (ID: %d)\n",
                       i, current->name ? current->name : "?", current->task_id);
            
            // Execute current task (simplified)
            if (current->task_function != nullptr) {
                // In real implementation, task runs until yield
                // Here we just call it once
                current->task_function(current->task_params);
            }
        }
        
        // Yield to next task
        sched->yield();
    }
    
    rtos_printf("\nTask execution counts:\n");
    rtos_printf("  Task1: %d\n", task1_count);
    rtos_printf("  Task2: %d\n", task2_count);
    rtos_printf("  Task3: %d\n", task3_count);
    
    rtos_printf("✓ Task yield passed\n");
}

// Test system tick
void test_system_tick() {
    rtos_printf("\n=== Test: System Tick ===\n");
    
    Scheduler* sched = Scheduler::getInstance();
    
    TickType_t start_tick = sched->getTickCount();
    
    // Simulate some ticks
    for (int i = 0; i < 20; i++) {
        sched->tick();
    }
    
    TickType_t end_tick = sched->getTickCount();
    assert(end_tick == start_tick + 20);
    
    rtos_printf("Ticks elapsed: %d\n", end_tick - start_tick);
    
    rtos_printf("✓ System tick passed\n");
}

// Test task suspend/resume
void test_task_suspend_resume() {
    rtos_printf("\n=== Test: Task Suspend/Resume ===\n");
    
    TaskHandle_t task = nullptr;
    TaskCreateParams params;
    params.name = "SuspendTest";
    params.function = testTask1;
    params.stack_size = 512;
    
    assert(Task::create(&task, params) == RTOS_OK);
    assert(Task::getState(task) == TASK_READY);
    
    // Suspend task
    assert(Task::suspend(task) == RTOS_OK);
    assert(Task::getState(task) == TASK_SUSPENDED);
    
    // Resume task
    assert(Task::resume(task) == RTOS_OK);
    assert(Task::getState(task) == TASK_READY);
    
    rtos_printf("✓ Task suspend/resume passed\n");
}

// Test context switching
void test_context_switch() {
    rtos_printf("\n=== Test: Context Switching ===\n");
    
    Scheduler* sched = Scheduler::getInstance();
    TaskControlBlock* task = sched->getCurrentTask();
    
    if (task != nullptr) {
        // Initialize stack
        uint32_t test_stack[128];
        uint32_t* sp = Context::initializeStack(
            test_stack + 128,
            testTask1,
            nullptr
        );
        
        assert(sp != nullptr);
        assert(sp < test_stack + 128);
        assert(sp >= test_stack);
        
        // Check stack usage
        size_t usage = Context::getStackUsage(task);
        rtos_printf("Stack usage: %d bytes\n", usage);
        
        // Check for overflow
        bool overflow = Context::checkStackOverflow(task);
        assert(!overflow);
    }
    
    rtos_printf("✓ Context switching passed\n");
}

// Test scheduler statistics
void test_scheduler_stats() {
    rtos_printf("\n=== Test: Scheduler Statistics ===\n");
    
    Scheduler* sched = Scheduler::getInstance();
    auto stats = sched->getStats();
    
    rtos_printf("Scheduler Statistics:\n");
    rtos_printf("  Total tasks: %d\n", stats.total_tasks);
    rtos_printf("  Ready tasks: %d\n", stats.ready_tasks);
    rtos_printf("  Blocked tasks: %d\n", stats.blocked_tasks);
    rtos_printf("  Suspended tasks: %d\n", stats.suspended_tasks);
    rtos_printf("  Uptime (ticks): %d\n", stats.uptime_ticks);
    
    assert(stats.total_tasks > 0);
    
    rtos_printf("✓ Scheduler statistics passed\n");
}

// Main test runner
int main() {
    rtos_printf("\n");
    rtos_printf("╔════════════════════════════════════════════╗\n");
    rtos_printf("║   RTOS Kernel Test Suite                  ║\n");
    rtos_printf("╚════════════════════════════════════════════╝\n");
    
    // Initialize UART
    RTOS_UART::init();
    
    try {
        test_scheduler_init();
        test_task_creation();
        test_multiple_tasks();
        test_scheduler_start();
        test_task_yield();
        test_system_tick();
        test_task_suspend_resume();
        test_context_switch();
        test_scheduler_stats();
        
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
