#include "singly_linked_list.h"

using namespace std;

int main() {
    // Create a linked list of integers
    SinglyLinkedList<int> list;
    
    // Insert elements
    cout << "Inserting elements:" << endl;
    list.insertAtEnd(10);
    list.insertAtEnd(20);
    list.insertAtEnd(30);
    list.insertAtBeginning(5);
    list.insertAtPosition(15, 2);
    
    // Display list
    list.display();
    list.displaySize();
    
    // Search for an element
    cout << "\nSearching for 20: " << (list.search(20) ? "Found" : "Not Found") << endl;
    cout << "Searching for 25: " << (list.search(25) ? "Found" : "Not Found") << endl;
    
    // Get element at position
    cout << "\nElement at position 2: " << list.get(2) << endl;
    
    // Update element
    cout << "\nUpdating position 3 to 25:" << endl;
    list.update(3, 25);
    list.display();
    
    // Delete elements
    cout << "\nDeleting from beginning:" << endl;
    list.deleteFromBeginning();
    list.display();
    
    cout << "\nDeleting from end:" << endl;
    list.deleteFromEnd();
    list.display();
    
    cout << "\nDeleting from position 1:" << endl;
    list.deleteFromPosition(1);
    list.display();
    
    // Reverse list
    cout << "\nReversing the list:" << endl;
    list.reverse();
    list.display();
    
    // Clear list
    cout << "\nClearing the list:" << endl;
    list.clear();
    list.display();
    
    return 0;
}