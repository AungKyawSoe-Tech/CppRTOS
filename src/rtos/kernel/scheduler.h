#ifndef RTOS_SCHEDULER_H
#define RTOS_SCHEDULER_H

#include "task.h"
#include "../rtos_types.h"
#include "../../util/static_vector.h"

// Maximum number of tasks
#define MAX_TASKS 16

// Scheduler policies
enum SchedulerPolicy {
    SCHEDULER_ROUND_ROBIN,
    SCHEDULER_PRIORITY,
    SCHEDULER_COOPERATIVE
};

// Scheduler class
class Scheduler {
private:
    // Task list (using static vector for embedded)
    StaticVector<TaskControlBlock*, MAX_TASKS> task_list;
    
    // Current running task
    TaskControlBlock* current_task;
    
    // Idle task
    TaskControlBlock idle_task_tcb;
    uint32_t idle_stack[TASK_STACK_SIZE_MIN / sizeof(uint32_t)];
    
    // Scheduler state
    bool is_running;
    bool is_initialized;
    SchedulerPolicy policy;
    
    // System tick counter
    volatile TickType_t tick_count;
    
    // Time slice for round-robin (in ticks)
    TickType_t time_slice_ticks;
    
    // Next task ID
    uint32_t next_task_id;
    
    // Private helper methods
    TaskControlBlock* selectNextTask();
    void switchContext(TaskControlBlock* next_task);
    void initializeIdleTask();
    
    // Idle task function
    static void idleTaskFunction(void* params);
    
    // Singleton instance
    static Scheduler* instance;
    
    // Private constructor for singleton
    Scheduler();
    
public:
    // Get singleton instance
    static Scheduler* getInstance();
    
    // Initialize scheduler
    RTOSResult initialize(SchedulerPolicy sched_policy = SCHEDULER_ROUND_ROBIN);
    
    // Start scheduler (never returns)
    void start();
    
    // Add task to scheduler
    RTOSResult addTask(TaskControlBlock* task);
    
    // Remove task from scheduler
    RTOSResult removeTask(TaskControlBlock* task);
    
    // Get current task
    TaskControlBlock* getCurrentTask() const { return current_task; }
    
    // Yield to next task
    void yield();
    
    // System tick handler (called from timer interrupt)
    void tick();
    
    // Get system tick count
    TickType_t getTickCount() const { return tick_count; }
    
    // Delay current task
    void delay(TickType_t ticks);
    
    // Check if scheduler is running
    bool isRunning() const { return is_running; }
    
    // Get scheduler statistics
    struct SchedulerStats {
        uint32_t total_tasks;
        uint32_t ready_tasks;
        uint32_t blocked_tasks;
        uint32_t suspended_tasks;
        TickType_t uptime_ticks;
    };
    
    SchedulerStats getStats() const;
};

#endif // RTOS_SCHEDULER_H
