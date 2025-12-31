#ifndef RTOS_STRING_H
#define RTOS_STRING_H

#include <cstddef>
#include <cstring>
#include <string>

// Fixed-size string for embedded systems (no dynamic allocation)
class RTOSString {
public:
    static constexpr size_t MAX_LENGTH = 256;
    static constexpr size_t npos = static_cast<size_t>(-1);
    
private:
    char buffer[MAX_LENGTH];
    size_t len;
    
public:
    // Constructors
    RTOSString();
    RTOSString(const char* str);
    RTOSString(const RTOSString& other);
    RTOSString(const std::string& str); // Conversion from std::string
    
    // Assignment
    RTOSString& operator=(const char* str);
    RTOSString& operator=(const RTOSString& other);
    RTOSString& operator=(const std::string& str); // Assignment from std::string
    
    // Accessors
    const char* c_str() const { return buffer; }
    size_t size() const { return len; }
    size_t length() const { return len; }
    bool empty() const { return len == 0; }
    
    // Comparison
    bool operator==(const RTOSString& other) const;
    bool operator==(const char* str) const;
    bool operator!=(const RTOSString& other) const { return !(*this == other); }
    bool operator!=(const char* str) const { return !(*this == str); }
    
    // String operations
    void clear();
    void append(const char* str);
    void append(const RTOSString& other);
    int compare(const RTOSString& other) const;
    int compare(const char* str) const;
    
    // Find operations
    size_t find(char c) const;
    size_t find(const char* substr) const;
    size_t find_last_of(const char* chars) const;
    
    // Substring
    RTOSString substr(size_t pos, size_t len = MAX_LENGTH) const;
    
    // Character access
    char operator[](size_t index) const { return buffer[index]; }
    char& operator[](size_t index) { return buffer[index]; }
};

// Stream output operator for std::ostream compatibility
#include <ostream>
inline std::ostream& operator<<(std::ostream& os, const RTOSString& str) {
    return os << str.c_str();
}

#endif // RTOS_STRING_H
