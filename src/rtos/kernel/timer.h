#ifndef RTOS_TIMER_H
#define RTOS_TIMER_H

#include "rtos/rtos_types.h"
#include <cstdint>
#include <cstddef>

// Timer callback function type
typedef void (*TimerCallback_t)(TimerHandle_t timer, void* user_data);

// Timer types
enum TimerType {
    TIMER_ONE_SHOT,   // Fires once then stops
    TIMER_PERIODIC    // Fires repeatedly
};

// Timer states
enum TimerState {
    TIMER_STOPPED,
    TIMER_RUNNING,
    TIMER_EXPIRED
};

// Timer Control Block
struct TimerCB {
    TimerHandle_t handle;
    char name[32];
    TimerType type;
    TimerState state;
    uint32_t period_ticks;      // Timer period in ticks
    uint32_t remaining_ticks;   // Ticks remaining until expiry
    TimerCallback_t callback;   // Function to call on expiry
    void* user_data;            // User data passed to callback
    bool auto_reload;           // Auto-reload for periodic timers
    uint32_t expiry_count;      // Number of times timer has expired
};

// Timer statistics
struct TimerStats {
    uint32_t total_timers;
    uint32_t active_timers;
    uint32_t stopped_timers;
    uint32_t total_callbacks;
    uint32_t missed_callbacks;
};

// Timer API namespace
namespace TimerAPI {

// Initialize timer subsystem
RTOSResult init();

// Create a new timer
TimerHandle_t create(
    const char* name,
    uint32_t period_ticks,
    TimerType type,
    TimerCallback_t callback,
    void* user_data = nullptr
);

// Delete a timer
RTOSResult destroy(TimerHandle_t timer);

// Start a timer
RTOSResult start(TimerHandle_t timer);

// Stop a timer
RTOSResult stop(TimerHandle_t timer);

// Reset a timer (restart from beginning)
RTOSResult reset(TimerHandle_t timer);

// Change timer period
RTOSResult changePeriod(TimerHandle_t timer, uint32_t new_period_ticks);

// Check if timer is active
bool isActive(TimerHandle_t timer);

// Get timer remaining time
uint32_t getRemainingTime(TimerHandle_t timer);

// Get timer name
const char* getName(TimerHandle_t timer);

// Get timer statistics
TimerStats getStats();

// Process timers (called by scheduler on each tick)
void processTick();

} // namespace TimerAPI

// Timer Manager class (singleton)
class TimerManager {
public:
    static TimerManager& getInstance();
    
    RTOSResult init();
    TimerHandle_t createTimer(const char* name, uint32_t period_ticks, 
                              TimerType type, TimerCallback_t callback,
                              void* user_data);
    RTOSResult deleteTimer(TimerHandle_t timer);
    RTOSResult startTimer(TimerHandle_t timer);
    RTOSResult stopTimer(TimerHandle_t timer);
    RTOSResult resetTimer(TimerHandle_t timer);
    RTOSResult changeTimerPeriod(TimerHandle_t timer, uint32_t new_period);
    bool isTimerActive(TimerHandle_t timer);
    uint32_t getTimerRemaining(TimerHandle_t timer);
    const char* getTimerName(TimerHandle_t timer);
    TimerStats getStatistics();
    void tick();
    
private:
    TimerManager();
    ~TimerManager();
    TimerManager(const TimerManager&) = delete;
    TimerManager& operator=(const TimerManager&) = delete;
    
    TimerCB* findTimer(TimerHandle_t handle);
    void executeCallback(TimerCB* timer);
    
    static constexpr size_t MAX_TIMERS = 32;
    TimerCB timers[MAX_TIMERS];
    uint32_t timer_count;
    uint32_t next_handle;
    bool initialized;
    
    // Statistics
    uint32_t total_callbacks;
    uint32_t missed_callbacks;
};

#endif // RTOS_TIMER_H
