#include "timer.h"
#include "rtos/hal/rtos_uart.h"
#include <cstring>

// ============================================================================
// Timer Manager Implementation
// ============================================================================

TimerManager::TimerManager()
    : timer_count(0)
    , next_handle(1)
    , initialized(false)
    , total_callbacks(0)
    , missed_callbacks(0)
{
    memset(timers, 0, sizeof(timers));
}

TimerManager::~TimerManager() {
}

TimerManager& TimerManager::getInstance() {
    static TimerManager instance;
    return instance;
}

RTOSResult TimerManager::init() {
    if (initialized) {
        return RTOS_OK;
    }
    
    timer_count = 0;
    next_handle = 1;
    total_callbacks = 0;
    missed_callbacks = 0;
    memset(timers, 0, sizeof(timers));
    
    initialized = true;
    rtos_printf("[Timer] Timer subsystem initialized\n");
    return RTOS_OK;
}

TimerHandle_t TimerManager::createTimer(
    const char* name,
    uint32_t period_ticks,
    TimerType type,
    TimerCallback_t callback,
    void* user_data)
{
    if (!initialized) {
        init();
    }
    
    if (timer_count >= MAX_TIMERS) {
        rtos_printf("[Timer] ERROR: Maximum timers reached\n");
        return 0;
    }
    
    if (period_ticks == 0 || callback == nullptr) {
        rtos_printf("[Timer] ERROR: Invalid parameters\n");
        return 0;
    }
    
    // Find free slot
    TimerCB* timer = nullptr;
    for (size_t i = 0; i < MAX_TIMERS; i++) {
        if (timers[i].handle == 0) {
            timer = &timers[i];
            break;
        }
    }
    
    if (!timer) {
        return 0;
    }
    
    // Initialize timer
    timer->handle = next_handle++;
    strncpy(timer->name, name ? name : "Timer", sizeof(timer->name) - 1);
    timer->name[sizeof(timer->name) - 1] = '\0';
    timer->type = type;
    timer->state = TIMER_STOPPED;
    timer->period_ticks = period_ticks;
    timer->remaining_ticks = period_ticks;
    timer->callback = callback;
    timer->user_data = user_data;
    timer->auto_reload = (type == TIMER_PERIODIC);
    timer->expiry_count = 0;
    
    timer_count++;
    
    rtos_printf("[Timer] Created timer '%s' (ID: %u, period: %u ticks, type: %s)\n",
                timer->name, timer->handle, period_ticks,
                type == TIMER_ONE_SHOT ? "one-shot" : "periodic");
    
    return timer->handle;
}

RTOSResult TimerManager::deleteTimer(TimerHandle_t handle) {
    TimerCB* timer = findTimer(handle);
    if (!timer) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    rtos_printf("[Timer] Deleted timer '%s' (ID: %u)\n", timer->name, timer->handle);
    
    memset(timer, 0, sizeof(TimerCB));
    timer_count--;
    
    return RTOS_OK;
}

RTOSResult TimerManager::startTimer(TimerHandle_t handle) {
    TimerCB* timer = findTimer(handle);
    if (!timer) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    if (timer->state == TIMER_RUNNING) {
        return RTOS_OK; // Already running
    }
    
    timer->state = TIMER_RUNNING;
    timer->remaining_ticks = timer->period_ticks;
    
    rtos_printf("[Timer] Started timer '%s' (ID: %u)\n", timer->name, timer->handle);
    
    return RTOS_OK;
}

RTOSResult TimerManager::stopTimer(TimerHandle_t handle) {
    TimerCB* timer = findTimer(handle);
    if (!timer) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    if (timer->state == TIMER_STOPPED) {
        return RTOS_OK; // Already stopped
    }
    
    timer->state = TIMER_STOPPED;
    
    rtos_printf("[Timer] Stopped timer '%s' (ID: %u)\n", timer->name, timer->handle);
    
    return RTOS_OK;
}

RTOSResult TimerManager::resetTimer(TimerHandle_t handle) {
    TimerCB* timer = findTimer(handle);
    if (!timer) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    timer->remaining_ticks = timer->period_ticks;
    timer->state = TIMER_RUNNING;
    
    rtos_printf("[Timer] Reset timer '%s' (ID: %u)\n", timer->name, timer->handle);
    
    return RTOS_OK;
}

RTOSResult TimerManager::changeTimerPeriod(TimerHandle_t handle, uint32_t new_period) {
    TimerCB* timer = findTimer(handle);
    if (!timer) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    if (new_period == 0) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    timer->period_ticks = new_period;
    
    // If timer is running, update remaining ticks
    if (timer->state == TIMER_RUNNING) {
        timer->remaining_ticks = new_period;
    }
    
    rtos_printf("[Timer] Changed period of timer '%s' to %u ticks\n",
                timer->name, new_period);
    
    return RTOS_OK;
}

bool TimerManager::isTimerActive(TimerHandle_t handle) {
    TimerCB* timer = findTimer(handle);
    if (!timer) {
        return false;
    }
    
    return timer->state == TIMER_RUNNING;
}

uint32_t TimerManager::getTimerRemaining(TimerHandle_t handle) {
    TimerCB* timer = findTimer(handle);
    if (!timer) {
        return 0;
    }
    
    return timer->remaining_ticks;
}

const char* TimerManager::getTimerName(TimerHandle_t handle) {
    TimerCB* timer = findTimer(handle);
    if (!timer) {
        return nullptr;
    }
    
    return timer->name;
}

TimerStats TimerManager::getStatistics() {
    TimerStats stats;
    stats.total_timers = timer_count;
    stats.active_timers = 0;
    stats.stopped_timers = 0;
    stats.total_callbacks = total_callbacks;
    stats.missed_callbacks = missed_callbacks;
    
    for (size_t i = 0; i < MAX_TIMERS; i++) {
        if (timers[i].handle != 0) {
            if (timers[i].state == TIMER_RUNNING) {
                stats.active_timers++;
            } else if (timers[i].state == TIMER_STOPPED) {
                stats.stopped_timers++;
            }
        }
    }
    
    return stats;
}

void TimerManager::tick() {
    // Process all active timers
    for (size_t i = 0; i < MAX_TIMERS; i++) {
        TimerCB* timer = &timers[i];
        
        if (timer->handle == 0 || timer->state != TIMER_RUNNING) {
            continue;
        }
        
        // Decrement remaining ticks
        if (timer->remaining_ticks > 0) {
            timer->remaining_ticks--;
        }
        
        // Check if timer expired
        if (timer->remaining_ticks == 0) {
            timer->expiry_count++;
            
            // Execute callback
            executeCallback(timer);
            
            // Handle one-shot vs periodic
            if (timer->auto_reload) {
                // Periodic timer - reload
                timer->remaining_ticks = timer->period_ticks;
            } else {
                // One-shot timer - stop
                timer->state = TIMER_STOPPED;
            }
        }
    }
}

TimerCB* TimerManager::findTimer(TimerHandle_t handle) {
    for (size_t i = 0; i < MAX_TIMERS; i++) {
        if (timers[i].handle == handle) {
            return &timers[i];
        }
    }
    return nullptr;
}

void TimerManager::executeCallback(TimerCB* timer) {
    if (timer->callback) {
        total_callbacks++;
        timer->callback(timer->handle, timer->user_data);
    } else {
        missed_callbacks++;
    }
}

// ============================================================================
// Timer API Implementation
// ============================================================================

namespace TimerAPI {

RTOSResult init() {
    return TimerManager::getInstance().init();
}

TimerHandle_t create(
    const char* name,
    uint32_t period_ticks,
    TimerType type,
    TimerCallback_t callback,
    void* user_data)
{
    return TimerManager::getInstance().createTimer(
        name, period_ticks, type, callback, user_data);
}

RTOSResult destroy(TimerHandle_t timer) {
    return TimerManager::getInstance().deleteTimer(timer);
}

RTOSResult start(TimerHandle_t timer) {
    return TimerManager::getInstance().startTimer(timer);
}

RTOSResult stop(TimerHandle_t timer) {
    return TimerManager::getInstance().stopTimer(timer);
}

RTOSResult reset(TimerHandle_t timer) {
    return TimerManager::getInstance().resetTimer(timer);
}

RTOSResult changePeriod(TimerHandle_t timer, uint32_t new_period_ticks) {
    return TimerManager::getInstance().changeTimerPeriod(timer, new_period_ticks);
}

bool isActive(TimerHandle_t timer) {
    return TimerManager::getInstance().isTimerActive(timer);
}

uint32_t getRemainingTime(TimerHandle_t timer) {
    return TimerManager::getInstance().getTimerRemaining(timer);
}

const char* getName(TimerHandle_t timer) {
    return TimerManager::getInstance().getTimerName(timer);
}

TimerStats getStats() {
    return TimerManager::getInstance().getStatistics();
}

void processTick() {
    TimerManager::getInstance().tick();
}

} // namespace TimerAPI
