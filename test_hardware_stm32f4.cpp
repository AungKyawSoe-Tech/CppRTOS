/**
 * STM32F4 Hardware Test Application
 * 
 * This application tests the RTOS running on real STM32F4 hardware.
 * It creates multiple tasks and verifies context switching, scheduling,
 * and basic RTOS functionality.
 */

#include "src/bsp/stm32f4/bsp.h"
#include "src/rtos/arch/arm/port.h"
#include "src/rtos/kernel/scheduler.h"
#include "src/rtos/kernel/task.h"
#include "src/rtos/kernel/mutex.h"
#include "src/rtos/kernel/semaphore.h"
#include "src/rtos/hal/rtos_uart.h"

// Task stacks
#define TASK_STACK_SIZE 256
uint32_t task1_stack[TASK_STACK_SIZE];
uint32_t task2_stack[TASK_STACK_SIZE];
uint32_t task3_stack[TASK_STACK_SIZE];

// Task handles
TaskHandle_t task1_handle = nullptr;
TaskHandle_t task2_handle = nullptr;
TaskHandle_t task3_handle = nullptr;

// Shared mutex
MutexHandle_t print_mutex = nullptr;

// Safe printf with mutex
void safe_printf(const char* format, ...) {
    if (print_mutex) {
        Mutex::lock(print_mutex, RTOS_MAX_DELAY);
    }
    
    // Simple string output (format not fully supported)
    const char* p = format;
    while (*p) {
        BSP::putChar(*p++);
    }
    
    if (print_mutex) {
        Mutex::unlock(print_mutex);
    }
}

/**
 * Task 1: Blink green LED
 */
void task1_function(void* params) {
    (void)params;
    uint32_t count = 0;
    
    while (true) {
        safe_printf("Task1: Running (count: ");
        // Print count (simplified)
        BSP::putChar('0' + (count % 10));
        safe_printf(")\n");
        
        BSP::setLED(0, true);   // Green LED on
        Scheduler::getInstance()->delay(100);  // 100ms
        
        BSP::setLED(0, false);  // Green LED off
        Scheduler::getInstance()->delay(400);  // 400ms
        
        count++;
    }
}

/**
 * Task 2: Blink orange LED
 */
void task2_function(void* params) {
    (void)params;
    uint32_t count = 0;
    
    while (true) {
        safe_printf("Task2: Running (count: ");
        BSP::putChar('0' + (count % 10));
        safe_printf(")\n");
        
        BSP::setLED(1, true);   // Orange LED on
        Scheduler::getInstance()->delay(150);  // 150ms
        
        BSP::setLED(1, false);  // Orange LED off
        Scheduler::getInstance()->delay(350);  // 350ms
        
        count++;
    }
}

/**
 * Task 3: Blink blue LED
 */
void task3_function(void* params) {
    (void)params;
    uint32_t count = 0;
    
    while (true) {
        safe_printf("Task3: Running (count: ");
        BSP::putChar('0' + (count % 10));
        safe_printf(")\n");
        
        BSP::setLED(3, true);   // Blue LED on
        Scheduler::getInstance()->delay(200);  // 200ms
        
        BSP::setLED(3, false);  // Blue LED off
        Scheduler::getInstance()->delay(300);  // 300ms
        
        count++;
    }
}

/**
 * Override rtos_printf to use BSP UART
 */
void rtos_printf(const char* format, ...) {
    const char* p = format;
    while (*p) {
        if (*p == '\n') {
            BSP::putChar('\r');
        }
        BSP::putChar(*p++);
    }
}

/**
 * Main function
 */
int main(void) {
    // Initialize BSP
    BSP::initialize();
    BSP::putChar('\n');
    BSP::putChar('-');
    BSP::putChar('-');
    BSP::putChar('-');
    BSP::putChar('\n');
    rtos_printf("STM32F4 RTOS Hardware Test\n");
    rtos_printf("==========================\n\n");
    
    // Initialize ARM port
    Port::initialize();
    
    // Initialize SysTick (1 kHz = 1ms tick)
    Port::initSysTick(SYSTICK_FREQUENCY_HZ, CPU_FREQUENCY_HZ);
    rtos_printf("SysTick initialized\n\n");
    
    // Initialize scheduler
    Scheduler* scheduler = Scheduler::getInstance();
    if (scheduler->initialize() != RTOS_OK) {
        rtos_printf("ERROR: Scheduler initialization failed!\n");
        while (1);
    }
    
    // Create mutex
    print_mutex = Mutex::create();
    if (!print_mutex) {
        rtos_printf("ERROR: Mutex creation failed!\n");
        while (1);
    }
    rtos_printf("Print mutex created\n");
    
    // Create tasks
    TaskCreateParams task1_params;
    task1_params.name = "Task1-Green";
    task1_params.function = task1_function;
    task1_params.params = nullptr;
    task1_params.stack_size = sizeof(task1_stack);
    task1_params.priority = PRIORITY_NORMAL;
    
    if (Task::create(&task1_handle, task1_params) != RTOS_OK) {
        rtos_printf("ERROR: Task1 creation failed!\n");
        while (1);
    }
    rtos_printf("Task1 created\n");
    
    TaskCreateParams task2_params;
    task2_params.name = "Task2-Orange";
    task2_params.function = task2_function;
    task2_params.params = nullptr;
    task2_params.stack_size = sizeof(task2_stack);
    task2_params.priority = PRIORITY_NORMAL;
    
    if (Task::create(&task2_handle, task2_params) != RTOS_OK) {
        rtos_printf("ERROR: Task2 creation failed!\n");
        while (1);
    }
    rtos_printf("Task2 created\n");
    
    TaskCreateParams task3_params;
    task3_params.name = "Task3-Blue";
    task3_params.function = task3_function;
    task3_params.params = nullptr;
    task3_params.stack_size = sizeof(task3_stack);
    task3_params.priority = PRIORITY_NORMAL;
    
    if (Task::create(&task3_handle, task3_params) != RTOS_OK) {
        rtos_printf("ERROR: Task3 creation failed!\n");
        while (1);
    }
    rtos_printf("Task3 created\n\n");
    
    // Start scheduler (never returns)
    rtos_printf("Starting scheduler...\n");
    rtos_printf("====================\n\n");
    
    scheduler->start();
    
    // Should never reach here
    while (1) {
        BSP::setLED(2, true);  // Red LED = error
    }
    
    return 0;
}

/**
 * SysTick handler override
 */
extern "C" void SysTick_Handler(void) {
    // Call scheduler tick
    Scheduler* scheduler = Scheduler::getInstance();
    if (scheduler && scheduler->isRunning()) {
        scheduler->tick();
    }
}
