#include "rtos/kernel/timer.h"
#include "rtos/kernel/scheduler.h"
#include "rtos/hal/rtos_uart.h"
#include <cassert>

// Test callback counters
static int one_shot_count = 0;
static int periodic_count = 0;
static int user_data_value = 0;

// Callback functions
void oneShotCallback(TimerHandle_t timer, void* user_data) {
    (void)timer;
    (void)user_data;
    one_shot_count++;
    rtos_printf("[Callback] One-shot timer fired (count: %d)\n", one_shot_count);
}

void periodicCallback(TimerHandle_t timer, void* user_data) {
    (void)timer;
    (void)user_data;
    periodic_count++;
    rtos_printf("[Callback] Periodic timer fired (count: %d)\n", periodic_count);
}

void userDataCallback(TimerHandle_t timer, void* user_data) {
    (void)timer;
    if (user_data) {
        int* value = static_cast<int*>(user_data);
        user_data_value = *value;
        rtos_printf("[Callback] User data callback: value = %d\n", user_data_value);
    }
}

// Test timer initialization
void test_timer_init() {
    rtos_printf("\n=== Test: Timer Initialization ===\n");
    
    RTOSResult result = TimerAPI::init();
    assert(result == RTOS_OK);
    
    TimerStats stats = TimerAPI::getStats();
    assert(stats.total_timers == 0);
    
    rtos_printf("✓ Timer initialization passed\n");
}

// Test timer creation
void test_timer_creation() {
    rtos_printf("\n=== Test: Timer Creation ===\n");
    
    // Create one-shot timer
    TimerHandle_t timer1 = TimerAPI::create(
        "OneShotTimer",
        100,
        TIMER_ONE_SHOT,
        oneShotCallback
    );
    assert(timer1 != 0);
    rtos_printf("Created one-shot timer: %u\n", timer1);
    
    // Create periodic timer
    TimerHandle_t timer2 = TimerAPI::create(
        "PeriodicTimer",
        50,
        TIMER_PERIODIC,
        periodicCallback
    );
    assert(timer2 != 0);
    rtos_printf("Created periodic timer: %u\n", timer2);
    
    TimerStats stats = TimerAPI::getStats();
    assert(stats.total_timers == 2);
    
    // Clean up
    TimerAPI::destroy(timer1);
    TimerAPI::destroy(timer2);
    
    rtos_printf("✓ Timer creation passed\n");
}

// Test one-shot timer
void test_one_shot_timer() {
    rtos_printf("\n=== Test: One-Shot Timer ===\n");
    
    one_shot_count = 0;
    
    // Create and start one-shot timer (10 ticks)
    TimerHandle_t timer = TimerAPI::create(
        "OneShotTest",
        10,
        TIMER_ONE_SHOT,
        oneShotCallback
    );
    assert(timer != 0);
    
    RTOSResult result = TimerAPI::start(timer);
    assert(result == RTOS_OK);
    assert(TimerAPI::isActive(timer));
    
    // Simulate 15 ticks
    for (int i = 0; i < 15; i++) {
        TimerAPI::processTick();
        rtos_printf("Tick %d: remaining = %u, count = %d\n",
                    i + 1, TimerAPI::getRemainingTime(timer), one_shot_count);
    }
    
    // Timer should have fired once and stopped
    assert(one_shot_count == 1);
    assert(!TimerAPI::isActive(timer));
    
    TimerAPI::destroy(timer);
    
    rtos_printf("✓ One-shot timer passed\n");
}

// Test periodic timer
void test_periodic_timer() {
    rtos_printf("\n=== Test: Periodic Timer ===\n");
    
    periodic_count = 0;
    
    // Create and start periodic timer (5 ticks)
    TimerHandle_t timer = TimerAPI::create(
        "PeriodicTest",
        5,
        TIMER_PERIODIC,
        periodicCallback
    );
    assert(timer != 0);
    
    RTOSResult result = TimerAPI::start(timer);
    assert(result == RTOS_OK);
    
    // Simulate 20 ticks (should fire 4 times)
    for (int i = 0; i < 20; i++) {
        TimerAPI::processTick();
    }
    
    // Timer should have fired 4 times and still be running
    assert(periodic_count == 4);
    assert(TimerAPI::isActive(timer));
    
    TimerAPI::stop(timer);
    assert(!TimerAPI::isActive(timer));
    
    TimerAPI::destroy(timer);
    
    rtos_printf("✓ Periodic timer passed (fired %d times)\n", periodic_count);
}

// Test timer start/stop
void test_timer_start_stop() {
    rtos_printf("\n=== Test: Timer Start/Stop ===\n");
    
    periodic_count = 0;
    
    TimerHandle_t timer = TimerAPI::create(
        "StartStopTest",
        10,
        TIMER_PERIODIC,
        periodicCallback
    );
    assert(timer != 0);
    
    // Start timer
    TimerAPI::start(timer);
    assert(TimerAPI::isActive(timer));
    
    // Run 5 ticks
    for (int i = 0; i < 5; i++) {
        TimerAPI::processTick();
    }
    
    uint32_t remaining = TimerAPI::getRemainingTime(timer);
    rtos_printf("After 5 ticks, remaining: %u\n", remaining);
    assert(remaining == 5);
    
    // Stop timer
    TimerAPI::stop(timer);
    assert(!TimerAPI::isActive(timer));
    
    // Run more ticks - timer should not fire
    for (int i = 0; i < 10; i++) {
        TimerAPI::processTick();
    }
    
    assert(periodic_count == 0);
    
    TimerAPI::destroy(timer);
    
    rtos_printf("✓ Timer start/stop passed\n");
}

// Test timer reset
void test_timer_reset() {
    rtos_printf("\n=== Test: Timer Reset ===\n");
    
    one_shot_count = 0;
    
    TimerHandle_t timer = TimerAPI::create(
        "ResetTest",
        10,
        TIMER_ONE_SHOT,
        oneShotCallback
    );
    assert(timer != 0);
    
    TimerAPI::start(timer);
    
    // Run 5 ticks
    for (int i = 0; i < 5; i++) {
        TimerAPI::processTick();
    }
    
    assert(TimerAPI::getRemainingTime(timer) == 5);
    
    // Reset timer
    TimerAPI::reset(timer);
    assert(TimerAPI::getRemainingTime(timer) == 10);
    assert(TimerAPI::isActive(timer));
    
    // Run 15 ticks
    for (int i = 0; i < 15; i++) {
        TimerAPI::processTick();
    }
    
    assert(one_shot_count == 1);
    
    TimerAPI::destroy(timer);
    
    rtos_printf("✓ Timer reset passed\n");
}

// Test change period
void test_timer_change_period() {
    rtos_printf("\n=== Test: Timer Change Period ===\n");
    
    periodic_count = 0;
    
    TimerHandle_t timer = TimerAPI::create(
        "ChangePeriodTest",
        10,
        TIMER_PERIODIC,
        periodicCallback
    );
    assert(timer != 0);
    
    TimerAPI::start(timer);
    
    // Run 15 ticks (should fire once at 10 ticks)
    for (int i = 0; i < 15; i++) {
        TimerAPI::processTick();
    }
    
    int count_before = periodic_count;
    rtos_printf("Fired %d times with 10-tick period\n", count_before);
    
    // Change period to 5 ticks
    TimerAPI::changePeriod(timer, 5);
    
    // Run 20 more ticks (should fire 4 times)
    for (int i = 0; i < 20; i++) {
        TimerAPI::processTick();
    }
    
    int count_after = periodic_count - count_before;
    rtos_printf("Fired %d times with 5-tick period\n", count_after);
    assert(count_after == 4);
    
    TimerAPI::destroy(timer);
    
    rtos_printf("✓ Timer change period passed\n");
}

// Test user data
void test_timer_user_data() {
    rtos_printf("\n=== Test: Timer User Data ===\n");
    
    user_data_value = 0;
    int test_value = 42;
    
    TimerHandle_t timer = TimerAPI::create(
        "UserDataTest",
        10,
        TIMER_ONE_SHOT,
        userDataCallback,
        &test_value
    );
    assert(timer != 0);
    
    TimerAPI::start(timer);
    
    // Run until timer fires
    for (int i = 0; i < 15; i++) {
        TimerAPI::processTick();
    }
    
    assert(user_data_value == 42);
    rtos_printf("User data passed correctly: %d\n", user_data_value);
    
    TimerAPI::destroy(timer);
    
    rtos_printf("✓ Timer user data passed\n");
}

// Test multiple timers
void test_multiple_timers() {
    rtos_printf("\n=== Test: Multiple Timers ===\n");
    
    one_shot_count = 0;
    periodic_count = 0;
    
    // Create multiple timers with different periods
    TimerHandle_t timer1 = TimerAPI::create("Timer1", 10, TIMER_ONE_SHOT, oneShotCallback);
    TimerHandle_t timer2 = TimerAPI::create("Timer2", 5, TIMER_PERIODIC, periodicCallback);
    TimerHandle_t timer3 = TimerAPI::create("Timer3", 20, TIMER_ONE_SHOT, oneShotCallback);
    
    assert(timer1 != 0 && timer2 != 0 && timer3 != 0);
    
    // Start all timers
    TimerAPI::start(timer1);
    TimerAPI::start(timer2);
    TimerAPI::start(timer3);
    
    TimerStats stats = TimerAPI::getStats();
    rtos_printf("Active timers: %u\n", stats.active_timers);
    assert(stats.active_timers == 3);
    
    // Run 25 ticks
    for (int i = 0; i < 25; i++) {
        TimerAPI::processTick();
    }
    
    rtos_printf("One-shot fired: %d times\n", one_shot_count);
    rtos_printf("Periodic fired: %d times\n", periodic_count);
    
    // One-shot should fire 2 times (at 10 and 20 ticks)
    // Periodic should fire 5 times (every 5 ticks)
    assert(one_shot_count == 2);
    assert(periodic_count == 5);
    
    TimerAPI::destroy(timer1);
    TimerAPI::destroy(timer2);
    TimerAPI::destroy(timer3);
    
    rtos_printf("✓ Multiple timers passed\n");
}

// Test timer statistics
void test_timer_statistics() {
    rtos_printf("\n=== Test: Timer Statistics ===\n");
    
    // Create several timers
    TimerHandle_t t1 = TimerAPI::create("Stat1", 10, TIMER_PERIODIC, periodicCallback);
    TimerHandle_t t2 = TimerAPI::create("Stat2", 20, TIMER_ONE_SHOT, oneShotCallback);
    TimerHandle_t t3 = TimerAPI::create("Stat3", 15, TIMER_PERIODIC, periodicCallback);
    
    TimerAPI::start(t1);
    TimerAPI::start(t3);
    // t2 remains stopped
    
    TimerStats stats = TimerAPI::getStats();
    rtos_printf("Timer Statistics:\n");
    rtos_printf("  Total timers: %u\n", stats.total_timers);
    rtos_printf("  Active timers: %u\n", stats.active_timers);
    rtos_printf("  Stopped timers: %u\n", stats.stopped_timers);
    
    assert(stats.total_timers == 3);
    assert(stats.active_timers == 2);
    assert(stats.stopped_timers == 1);
    
    TimerAPI::destroy(t1);
    TimerAPI::destroy(t2);
    TimerAPI::destroy(t3);
    
    rtos_printf("✓ Timer statistics passed\n");
}

// Main test runner
int main() {
    rtos_printf("\n");
    rtos_printf("╔════════════════════════════════════════════╗\n");
    rtos_printf("║   RTOS Timer Test Suite                   ║\n");
    rtos_printf("╚════════════════════════════════════════════╝\n");
    
    RTOS_UART::init();
    
    // Initialize scheduler (needed for tick processing)
    Scheduler::getInstance()->initialize(SCHEDULER_ROUND_ROBIN);
    
    try {
        test_timer_init();
        test_timer_creation();
        test_one_shot_timer();
        test_periodic_timer();
        test_timer_start_stop();
        test_timer_reset();
        test_timer_change_period();
        test_timer_user_data();
        test_multiple_timers();
        test_timer_statistics();
        
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
