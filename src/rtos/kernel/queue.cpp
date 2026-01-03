#include "queue.h"
#include "scheduler.h"
#include "../hal/rtos_uart.h"

// MessageQueue implementation (template methods must be in header or explicitly instantiated)

template <typename T, size_t MAX_ITEMS>
RTOSResult MessageQueue<T, MAX_ITEMS>::send(const T& item, TickType_t timeout) {
    Scheduler* sched = Scheduler::getInstance();
    
    // Try to enqueue
    if (queue.enqueue(item)) {
        return RTOS_OK;
    }
    
    // Queue is full
    if (timeout == 0) {
        return RTOS_ERR_FULL;
    }
    
    // Wait for space
    TickType_t start_tick = sched->getTickCount();
    
    while (queue.isFull()) {
        // Check timeout
        if (timeout != UINT32_MAX) {
            if ((sched->getTickCount() - start_tick) >= timeout) {
                return RTOS_ERR_TIMEOUT;
            }
        }
        
        // Yield to other tasks
        sched->yield();
    }
    
    // Try again
    if (queue.enqueue(item)) {
        return RTOS_OK;
    }
    
    return RTOS_ERR_FULL;
}

template <typename T, size_t MAX_ITEMS>
RTOSResult MessageQueue<T, MAX_ITEMS>::receive(T& item, TickType_t timeout) {
    Scheduler* sched = Scheduler::getInstance();
    
    // Try to dequeue
    if (queue.dequeue(item)) {
        return RTOS_OK;
    }
    
    // Queue is empty
    if (timeout == 0) {
        return RTOS_ERR_EMPTY;
    }
    
    // Wait for data
    TickType_t start_tick = sched->getTickCount();
    
    while (queue.isEmpty()) {
        // Check timeout
        if (timeout != UINT32_MAX) {
            if ((sched->getTickCount() - start_tick) >= timeout) {
                return RTOS_ERR_TIMEOUT;
            }
        }
        
        // Yield to other tasks
        sched->yield();
    }
    
    // Try again
    if (queue.dequeue(item)) {
        return RTOS_OK;
    }
    
    return RTOS_ERR_EMPTY;
}

// Queue API implementation
namespace QueueAPI {

template <typename T, size_t MAX_ITEMS>
RTOSResult create(MessageQueueHandle_t<T, MAX_ITEMS>* handle) {
    if (handle == nullptr) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    MessageQueue<T, MAX_ITEMS>* queue = new MessageQueue<T, MAX_ITEMS>();
    if (queue == nullptr) {
        return RTOS_ERR_NOMEM;
    }
    
    *handle = queue;
    rtos_printf("[Queue] Created message queue (capacity: %d)\n", MAX_ITEMS);
    
    return RTOS_OK;
}

template <typename T, size_t MAX_ITEMS>
RTOSResult destroy(MessageQueueHandle_t<T, MAX_ITEMS> handle) {
    if (handle == nullptr) {
        return RTOS_ERR_INVALID_PARAM;
    }
    
    delete handle;
    return RTOS_OK;
}

// Explicit template instantiations for common types
template RTOSResult create<uint32_t, 16>(MessageQueueHandle_t<uint32_t, 16>*);
template RTOSResult destroy<uint32_t, 16>(MessageQueueHandle_t<uint32_t, 16>);

template RTOSResult create<int, 16>(MessageQueueHandle_t<int, 16>*);
template RTOSResult destroy<int, 16>(MessageQueueHandle_t<int, 16>);

template RTOSResult create<uint32_t, 8>(MessageQueueHandle_t<uint32_t, 8>*);
template RTOSResult destroy<uint32_t, 8>(MessageQueueHandle_t<uint32_t, 8>);

} // namespace QueueAPI

// Explicit template instantiations (outside namespace)
template class MessageQueue<uint32_t, 16>;
template class MessageQueue<int, 16>;
template class MessageQueue<uint32_t, 8>;
