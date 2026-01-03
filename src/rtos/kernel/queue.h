#ifndef RTOS_QUEUE_H
#define RTOS_QUEUE_H

#include "../rtos_types.h"
#include "task.h"
#include <cstring>

// Queue structure (circular buffer implementation)
template <typename T, size_t MAX_ITEMS>
class Queue {
private:
    T buffer[MAX_ITEMS];
    size_t head;
    size_t tail;
    size_t count;
    
public:
    Queue() : head(0), tail(0), count(0) {}
    
    // Check if queue is empty
    bool isEmpty() const {
        return count == 0;
    }
    
    // Check if queue is full
    bool isFull() const {
        return count >= MAX_ITEMS;
    }
    
    // Get current item count
    size_t size() const {
        return count;
    }
    
    // Get capacity
    size_t capacity() const {
        return MAX_ITEMS;
    }
    
    // Enqueue (add to queue)
    bool enqueue(const T& item) {
        if (isFull()) {
            return false;
        }
        
        buffer[tail] = item;
        tail = (tail + 1) % MAX_ITEMS;
        count++;
        
        return true;
    }
    
    // Dequeue (remove from queue)
    bool dequeue(T& item) {
        if (isEmpty()) {
            return false;
        }
        
        item = buffer[head];
        head = (head + 1) % MAX_ITEMS;
        count--;
        
        return true;
    }
    
    // Peek at front item without removing
    bool peek(T& item) const {
        if (isEmpty()) {
            return false;
        }
        
        item = buffer[head];
        return true;
    }
    
    // Clear queue
    void clear() {
        head = 0;
        tail = 0;
        count = 0;
    }
};

// Message Queue (for inter-task communication)
template <typename T, size_t MAX_ITEMS>
class MessageQueue {
private:
    Queue<T, MAX_ITEMS> queue;
    TaskControlBlock* waiting_receivers;
    TaskControlBlock* waiting_senders;
    
public:
    MessageQueue() : waiting_receivers(nullptr), waiting_senders(nullptr) {}
    
    // Send message (with optional timeout)
    RTOSResult send(const T& item, TickType_t timeout = 0);
    
    // Receive message (with optional timeout)
    RTOSResult receive(T& item, TickType_t timeout = 0);
    
    // Try to send without blocking
    RTOSResult trySend(const T& item) {
        return send(item, 0);
    }
    
    // Try to receive without blocking
    RTOSResult tryReceive(T& item) {
        return receive(item, 0);
    }
    
    // Get queue statistics
    size_t size() const { return queue.size(); }
    size_t capacity() const { return queue.capacity(); }
    bool isEmpty() const { return queue.isEmpty(); }
    bool isFull() const { return queue.isFull(); }
    
    // Clear queue
    void clear() { queue.clear(); }
};

// Message Queue handle
template <typename T, size_t MAX_ITEMS>
using MessageQueueHandle_t = MessageQueue<T, MAX_ITEMS>*;

// Queue API
namespace QueueAPI {
    // Create a message queue
    template <typename T, size_t MAX_ITEMS>
    RTOSResult create(MessageQueueHandle_t<T, MAX_ITEMS>* handle);
    
    // Destroy a message queue
    template <typename T, size_t MAX_ITEMS>
    RTOSResult destroy(MessageQueueHandle_t<T, MAX_ITEMS> handle);
}

#endif // RTOS_QUEUE_H
