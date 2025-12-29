#ifndef SINGLY_LINKED_LIST_H
#define SINGLY_LINKED_LIST_H

#include <iostream>
#include <stdexcept>

// Node structure
template <typename T>
struct Node {
    T data;
    Node<T>* next;
    
    // Constructor
    Node(T value) : data(value), next(nullptr) {}
};

// SinglyLinkedList class template
template <typename T>
class SinglyLinkedList {
private:
    Node<T>* head;
    Node<T>* tail;
    int size;
    
public:
    // Constructor
    SinglyLinkedList() : head(nullptr), tail(nullptr), size(0) {}
    
    // Destructor
    ~SinglyLinkedList() {
        clear();
    }

    bool isEmpty() const;
    int getSize() const;
    void insertAtBeginning(T value);
    void insertAtEnd(T value);
    void insertAtPosition(T value, int position);
    void deleteFromBeginning();
    void deleteFromEnd();
    void deleteFromPosition(int position);
    bool search(T value) const;
    T get(int position) const;
    T& getRef(int position);  // ADD THIS: returns reference
    const T& getConstRef(int position) const;  // ADD THIS: returns const reference
    void update(int position, T newValue);
    void reverse();
    void clear();
    void display() const;
    void displaySize() const;
};

// Include the implementation
#include "singly_linked_list.cpp"

#endif // SINGLY_LINKED_LIST_H