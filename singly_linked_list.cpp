#ifndef SINGLY_LINKED_LIST_CPP
#define SINGLY_LINKED_LIST_CPP

#include <iostream>
#include "singly_linked_list.h"

using namespace std;

// Check if list is empty
template <typename T>
bool SinglyLinkedList<T>::isEmpty() const {
    return head == nullptr;
}
    
// Get size of list
template <typename T>
int SinglyLinkedList<T>::getSize() const {
    return size;
}
    
// Insert at beginning
template <typename T>
void SinglyLinkedList<T>::insertAtBeginning(T value) {
    Node<T>* newNode = new Node<T>(value);
        
    if (isEmpty()) {
        head = tail = newNode;
    } else {
        newNode->next = head;
        head = newNode;
    }
    size++;
}
    
// Insert at end
template <typename T>
void SinglyLinkedList<T>::insertAtEnd(T value) {
    Node<T>* newNode = new Node<T>(value);
        
    if (isEmpty()) {
        head = tail = newNode;
    } else {
        tail->next = newNode;
        tail = newNode;
    }
    size++;
}
    
// Insert at specific position (0-based index)
template <typename T>
void SinglyLinkedList<T>::insertAtPosition(T value, int position) {
    if (position < 0 || position > size) {
        std::cout << "Invalid position!" << std::endl;
        return;
    }
        
    if (position == 0) {
        insertAtBeginning(value);
    } else if (position == size) {
        insertAtEnd(value);
    } else {
        Node<T>* newNode = new Node<T>(value);
        Node<T>* current = head;
            
        // Traverse to position-1
        for (int i = 0; i < position - 1; i++) {
            current = current->next;
        }
            
        newNode->next = current->next;
        current->next = newNode;
        size++;
    }
}
    
// Delete from beginning
template <typename T>
void SinglyLinkedList<T>::deleteFromBeginning() {
    if (isEmpty()) {
        std::cout << "List is empty!" << std::endl;
        return;
    }
        
    Node<T>* temp = head;
    head = head->next;
        
    if (head == nullptr) { // If list becomes empty
        tail = nullptr;
    }
        
    delete temp;
    size--;
}
    
// Delete from end
template <typename T>
void SinglyLinkedList<T>::deleteFromEnd() {
    if (isEmpty()) {
        std::cout << "List is empty!" << std::endl;
        return;
    }
        
    if (head == tail) { // Only one element
        delete head;
        head = tail = nullptr;
    } else {
        Node<T>* current = head;
            
        // Traverse to second last node
        while (current->next != tail) {
            current = current->next;
        }
            
        delete tail;
        tail = current;
        tail->next = nullptr;
    }
    size--;
}
    
// Delete from specific position
template <typename T>
void SinglyLinkedList<T>::deleteFromPosition(int position) {
    if (position < 0 || position >= size) {
        std::cout << "Invalid position!" << std::endl;
        return;
    }
        
    if (position == 0) {
        deleteFromBeginning();
    } else if (position == size - 1) {
        deleteFromEnd();
    } else {
        Node<T>* current = head;
        Node<T>* previous = nullptr;
            
        // Traverse to the position
        for (int i = 0; i < position; i++) {
            previous = current;
            current = current->next;
        }
            
        previous->next = current->next;
        delete current;
        size--;
    }
}
    
// Search for a value
template <typename T>
bool SinglyLinkedList<T>::search(T value) const {
    Node<T>* current = head;
        
    while (current != nullptr) {
        if (current->data == value) {
            return true;
        }
        current = current->next;
    }
    return false;
}
    
// Get value at position (returns copy)
template <typename T>
T SinglyLinkedList<T>::get(int position) const {
    if (position < 0 || position >= size) {
        throw out_of_range("Position out of range");
    }
        
    Node<T>* current = head;
    for (int i = 0; i < position; i++) {
        current = current->next;
    }
        
    return current->data;
}

// Get reference at position (returns reference)
template <typename T>
T& SinglyLinkedList<T>::getRef(int position) {
    if (position < 0 || position >= size) {
        throw out_of_range("Position out of range");
    }
        
    Node<T>* current = head;
    for (int i = 0; i < position; i++) {
        current = current->next;
    }
        
    return current->data;
}

// Get const reference at position
template <typename T>
const T& SinglyLinkedList<T>::getConstRef(int position) const {
    if (position < 0 || position >= size) {
        throw out_of_range("Position out of range");
    }
        
    Node<T>* current = head;
    for (int i = 0; i < position; i++) {
        current = current->next;
    }
        
    return current->data;
}
    
// Update value at position
template <typename T>
void SinglyLinkedList<T>::update(int position, T newValue) {
    if (position < 0 || position >= size) {
        std::cout << "Invalid position!" << std::endl;
        return;
    }
        
    Node<T>* current = head;
    for (int i = 0; i < position; i++) {
        current = current->next;
    }
        
    current->data = newValue;
}
    
// Reverse the linked list
template <typename T>
void SinglyLinkedList<T>::reverse() {
    if (isEmpty() || head == tail) {
        return; // Empty or single element list
    }
        
    Node<T>* previous = nullptr;
    Node<T>* current = head;
    Node<T>* next = nullptr;
        
    tail = head; // Head becomes tail
        
    while (current != nullptr) {
        next = current->next;
        current->next = previous;
        previous = current;
        current = next;
    }
       
    head = previous; // Last node becomes head
}
    
// Clear the entire list
template <typename T>
void SinglyLinkedList<T>::clear() {
    while (!isEmpty()) {
        deleteFromBeginning();
    }
}
    
// Display the list
template <typename T>
void SinglyLinkedList<T>::display() const {
    if (isEmpty()) {
        std::cout << "List is empty!" << std::endl;
        return;
    }
        
    Node<T>* current = head;
    std::cout << "List: ";
    while (current != nullptr) {
        std::cout << current->data;
        if (current->next != nullptr) {
            std::cout << " -> ";
        }
        current = current->next;
    }
    std::cout << std::endl;
}
    
// Display list size
template <typename T>
void SinglyLinkedList<T>::displaySize() const {
    std::cout << "Size: " << size << std::endl;
}

#endif // SINGLY_LINKED_LIST_CPP