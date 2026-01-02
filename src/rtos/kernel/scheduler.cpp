#include "scheduler.h"
#include "../hal/rtos_uart.h"
#include <cstring>

// Initialize static instance
Scheduler* Scheduler::instance = nullptr;

// Idle task implementation
void Scheduler::idleTaskFunction(void* params) {
    (void)params; // Unused
    
    while (true) {
        // Idle loop - could add low power mode here
        // In real RTOS, this would use WFI (Wait For Interrupt)
        __asm__ volatile("nop");
    }
}

// Constructor
Scheduler::Scheduler()
    : current_task(nullptr), is_running(false), is_initialized(false),
      policy(SCHEDULER_ROUND_ROBIN), tick_count(0), time_slice_ticks(10),
      next_task_id(1) {
    // Clear idle stack
    memset(idle_stack, 0, sizeof(idle_stack));
}

// Get singleton instance
Scheduler* Scheduler::getInstance() {
    if (instance == nullptr) {
        instance = new Scheduler();
    }
    return instance;
}

// Initialize scheduler
RTOSResult Scheduler::initialize(SchedulerPolicy sched_policy) {
    if (is_initialized) {
        return RTOS_ERR_ALREADY_EXISTS;
    }
    
    policy = sched_policy;
    tick_count = 0;
    
    // Initialize idle task
    initializeIdleTask();
    
    is_initialized = true;
    rtos_printf("[Scheduler] Initialized with policy: %d\n", policy);
    
    return RTOS_OK;
}

// Initialize idle task
void Scheduler::initializeIdleTask() {
    idle_task_tcb.name = "IDLE";
    idle_task_tcb.task_id = 0;
    idle_task_tcb.state = TASK_READY;
    idle_task_tcb.priority = PRIORITY_IDLE;
    idle_task_tcb.task_function = idleTaskFunction;
    idle_task_tcb.task_params = nullptr;
    idle_task_tcb.stack_base = idle_stack;
    idle_task_tcb.stack_size = sizeof(idle_stack);
    idle_task_tcb.stack_pointer = idle_stack + (sizeof(idle_stack) / sizeof(uint32_t)) - 1;
    idle_task_tcb.next_task = nullptr;
    idle_task_tcb.time_slice = time_slice_ticks;
    
    rtos_printf("[Scheduler] Idle task initialized\n");
}

// Add task to scheduler
RTOSResult Scheduler::addTask(TaskControlBlock* task) {
    if (task == nullptr) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    if (task_list.size() >= MAX_TASKS) {
        return RTOS_ERR_FULL;
    }
    
    // Assign task ID
    task->task_id = next_task_id++;
    task->state = TASK_READY;
    task->time_slice = time_slice_ticks;
    
    // Add to task list
    task_list.push_back(task);
    
    rtos_printf("[Scheduler] Task '%s' (ID: %d) added\n", 
                task->name ? task->name : "unnamed", task->task_id);
    
    return RTOS_OK;
}

// Remove task from scheduler
RTOSResult Scheduler::removeTask(TaskControlBlock* task) {
    if (task == nullptr) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    // Find and remove task
    for (size_t i = 0; i < task_list.size(); i++) {
        if (task_list[i] == task) {
            // Mark as deleted
            task->state = TASK_DELETED;
            
            // Remove from list
            task_list.erase(i);
            
            rtos_printf("[Scheduler] Task '%s' (ID: %d) removed\n",
                        task->name ? task->name : "unnamed", task->task_id);
            
            return RTOS_OK;
        }
    }
    
    return RTOS_ERR_NOT_FOUND;
}

// Select next task (round-robin)
TaskControlBlock* Scheduler::selectNextTask() {
    // For round-robin scheduling
    if (policy == SCHEDULER_ROUND_ROBIN) {
        // Find next ready task
        bool found_current = false;
        
        // First, try to find next task after current
        for (size_t i = 0; i < task_list.size(); i++) {
            if (task_list[i] == current_task) {
                found_current = true;
                continue;
            }
            
            if (found_current && task_list[i]->state == TASK_READY) {
                return task_list[i];
            }
        }
        
        // Wrap around and check from beginning
        for (size_t i = 0; i < task_list.size(); i++) {
            if (task_list[i]->state == TASK_READY) {
                return task_list[i];
            }
            if (task_list[i] == current_task) {
                break;
            }
        }
    }
    
    // Priority scheduling (simplified)
    else if (policy == SCHEDULER_PRIORITY) {
        TaskControlBlock* highest_priority_task = nullptr;
        TaskPriority highest_priority = PRIORITY_IDLE;
        
        for (size_t i = 0; i < task_list.size(); i++) {
            if (task_list[i]->state == TASK_READY) {
                if (task_list[i]->priority > highest_priority) {
                    highest_priority = task_list[i]->priority;
                    highest_priority_task = task_list[i];
                }
            }
        }
        
        if (highest_priority_task != nullptr) {
            return highest_priority_task;
        }
    }
    
    // No ready task found, return idle task
    return &idle_task_tcb;
}

// Context switching (simplified for now)
void Scheduler::switchContext(TaskControlBlock* next_task) {
    if (next_task == nullptr) {
        return;
    }
    
    TaskControlBlock* prev_task = current_task;
    
    // Update task states
    if (prev_task != nullptr && prev_task->state == TASK_RUNNING) {
        prev_task->state = TASK_READY;
    }
    
    next_task->state = TASK_RUNNING;
    next_task->run_count++;
    next_task->time_slice = time_slice_ticks;
    
    current_task = next_task;
    
    // TODO: Actual context switch (save/restore registers, stack pointer)
    // For now, this is a cooperative scheduler simulation
}

// Start scheduler
void Scheduler::start() {
    if (!is_initialized) {
        rtos_printf("[Scheduler] ERROR: Not initialized!\n");
        return;
    }
    
    if (task_list.size() == 0) {
        rtos_printf("[Scheduler] WARNING: No tasks to schedule!\n");
    }
    
    rtos_printf("[Scheduler] Starting with %d tasks...\n", task_list.size());
    
    is_running = true;
    
    // Select first task
    current_task = selectNextTask();
    
    if (current_task != nullptr) {
        current_task->state = TASK_RUNNING;
        rtos_printf("[Scheduler] First task: '%s'\n", 
                    current_task->name ? current_task->name : "unnamed");
    }
    
    // In a real RTOS, this would jump to first task and never return
    // For simulation, we'll return and let the test harness drive execution
}

// Yield to next task
void Scheduler::yield() {
    if (!is_running) {
        return;
    }
    
    // Select next task
    TaskControlBlock* next_task = selectNextTask();
    
    // Switch to next task
    if (next_task != current_task) {
        switchContext(next_task);
    }
}

// System tick handler
void Scheduler::tick() {
    tick_count++;
    
    // Update current task time slice
    if (current_task != nullptr && current_task->time_slice > 0) {
        current_task->time_slice--;
        
        // Time slice expired, yield
        if (current_task->time_slice == 0) {
            yield();
        }
    }
    
    // Check for blocked tasks that should wake up
    for (size_t i = 0; i < task_list.size(); i++) {
        if (task_list[i]->state == TASK_BLOCKED) {
            if (tick_count >= task_list[i]->blocked_until) {
                task_list[i]->state = TASK_READY;
            }
        }
    }
}

// Delay current task
void Scheduler::delay(TickType_t ticks) {
    if (current_task == nullptr || !is_running) {
        return;
    }
    
    if (ticks == 0) {
        yield();
        return;
    }
    
    // Block current task until tick_count + ticks
    current_task->state = TASK_BLOCKED;
    current_task->blocked_until = tick_count + ticks;
    
    // Yield to next task
    yield();
}

// Get scheduler statistics
Scheduler::SchedulerStats Scheduler::getStats() const {
    SchedulerStats stats;
    stats.total_tasks = task_list.size();
    stats.ready_tasks = 0;
    stats.blocked_tasks = 0;
    stats.suspended_tasks = 0;
    stats.uptime_ticks = tick_count;
    
    for (size_t i = 0; i < task_list.size(); i++) {
        switch (task_list[i]->state) {
            case TASK_READY:
                stats.ready_tasks++;
                break;
            case TASK_BLOCKED:
                stats.blocked_tasks++;
                break;
            case TASK_SUSPENDED:
                stats.suspended_tasks++;
                break;
            default:
                break;
        }
    }
    
    return stats;
}
