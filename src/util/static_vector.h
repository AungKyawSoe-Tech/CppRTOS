#ifndef STATIC_VECTOR_H
#define STATIC_VECTOR_H

#include <cstddef>
#include <cstring>

// Fixed-capacity vector for embedded systems (no dynamic allocation)
template <typename T, size_t MAX_SIZE>
class StaticVector {
private:
    T data[MAX_SIZE];
    size_t count;
    
public:
    StaticVector() : count(0) {}
    
    // Capacity
    size_t size() const { return count; }
    size_t capacity() const { return MAX_SIZE; }
    bool empty() const { return count == 0; }
    bool full() const { return count >= MAX_SIZE; }
    
    // Element access
    T& operator[](size_t index) { return data[index]; }
    const T& operator[](size_t index) const { return data[index]; }
    
    T& at(size_t index) { return data[index]; }
    const T& at(size_t index) const { return data[index]; }
    
    T* begin() { return data; }
    const T* begin() const { return data; }
    
    T* end() { return data + count; }
    const T* end() const { return data + count; }
    
    // Modifiers
    bool push_back(const T& value) {
        if (count >= MAX_SIZE) {
            return false; // Full
        }
        data[count++] = value;
        return true;
    }
    
    void pop_back() {
        if (count > 0) {
            count--;
        }
    }
    
    void clear() {
        count = 0;
    }
    
    // For compatibility with std::vector
    T& back() { return data[count - 1]; }
    const T& back() const { return data[count - 1]; }
    
    T& front() { return data[0]; }
    const T& front() const { return data[0]; }
};

#endif // STATIC_VECTOR_H
