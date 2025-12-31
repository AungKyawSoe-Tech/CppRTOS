#ifndef STATIC_MAP_H
#define STATIC_MAP_H

#include <cstddef>

// Fixed-capacity map for embedded systems (no dynamic allocation)
// Uses linear search - suitable for small number of entries
template <typename K, typename V, size_t MAX_SIZE>
class StaticMap {
private:
    struct Entry {
        K key;
        V value;
        bool occupied;
        
        Entry() : occupied(false) {}
    };
    
    Entry entries[MAX_SIZE];
    size_t count;
    
public:
    StaticMap() : count(0) {}
    
    // Capacity
    size_t size() const { return count; }
    size_t capacity() const { return MAX_SIZE; }
    bool empty() const { return count == 0; }
    bool full() const { return count >= MAX_SIZE; }
    
    // Element access
    V& operator[](const K& key) {
        // Find existing entry
        for (size_t i = 0; i < MAX_SIZE; i++) {
            if (entries[i].occupied && entries[i].key == key) {
                return entries[i].value;
            }
        }
        
        // Create new entry
        for (size_t i = 0; i < MAX_SIZE; i++) {
            if (!entries[i].occupied) {
                entries[i].key = key;
                entries[i].occupied = true;
                count++;
                return entries[i].value;
            }
        }
        
        // Should never reach here if used properly
        return entries[0].value; // Fallback
    }
    
    // Find element
    V* find(const K& key) {
        for (size_t i = 0; i < MAX_SIZE; i++) {
            if (entries[i].occupied && entries[i].key == key) {
                return &entries[i].value;
            }
        }
        return nullptr;
    }
    
    const V* find(const K& key) const {
        for (size_t i = 0; i < MAX_SIZE; i++) {
            if (entries[i].occupied && entries[i].key == key) {
                return &entries[i].value;
            }
        }
        return nullptr;
    }
    
    // Insert or update
    bool insert(const K& key, const V& value) {
        // Check if already exists
        for (size_t i = 0; i < MAX_SIZE; i++) {
            if (entries[i].occupied && entries[i].key == key) {
                entries[i].value = value;
                return true;
            }
        }
        
        // Insert new
        if (count >= MAX_SIZE) {
            return false; // Full
        }
        
        for (size_t i = 0; i < MAX_SIZE; i++) {
            if (!entries[i].occupied) {
                entries[i].key = key;
                entries[i].value = value;
                entries[i].occupied = true;
                count++;
                return true;
            }
        }
        return false;
    }
    
    // Remove element
    bool erase(const K& key) {
        for (size_t i = 0; i < MAX_SIZE; i++) {
            if (entries[i].occupied && entries[i].key == key) {
                entries[i].occupied = false;
                count--;
                return true;
            }
        }
        return false;
    }
    
    // Clear all entries
    void clear() {
        for (size_t i = 0; i < MAX_SIZE; i++) {
            entries[i].occupied = false;
        }
        count = 0;
    }
    
    // Iterator support (simplified)
    class Iterator {
    private:
        StaticMap* map;
        size_t index;
        
        void advance() {
            while (index < MAX_SIZE && !map->entries[index].occupied) {
                index++;
            }
        }
        
    public:
        Iterator(StaticMap* m, size_t idx) : map(m), index(idx) {
            advance();
        }
        
        Entry& operator*() { return map->entries[index]; }
        Entry* operator->() { return &map->entries[index]; }
        
        Iterator& operator++() {
            index++;
            advance();
            return *this;
        }
        
        bool operator!=(const Iterator& other) const {
            return index != other.index;
        }
    };
    
    Iterator begin() { return Iterator(this, 0); }
    Iterator end() { return Iterator(this, MAX_SIZE); }
};

#endif // STATIC_MAP_H
