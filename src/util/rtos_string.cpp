#include "rtos_string.h"

// Default constructor
RTOSString::RTOSString() : len(0) {
    buffer[0] = '\0';
}

// Constructor from C string
RTOSString::RTOSString(const char* str) : len(0) {
    if (str) {
        size_t str_len = strlen(str);
        if (str_len >= MAX_LENGTH) {
            str_len = MAX_LENGTH - 1;
        }
        memcpy(buffer, str, str_len);
        buffer[str_len] = '\0';
        len = str_len;
    } else {
        buffer[0] = '\0';
    }
}

// Copy constructor
RTOSString::RTOSString(const RTOSString& other) : len(other.len) {
    memcpy(buffer, other.buffer, len + 1);
}

// Constructor from std::string
RTOSString::RTOSString(const std::string& str) : len(0) {
    size_t str_len = str.length();
    if (str_len >= MAX_LENGTH) {
        str_len = MAX_LENGTH - 1;
    }
    memcpy(buffer, str.c_str(), str_len);
    buffer[str_len] = '\0';
    len = str_len;
}

// Assignment from C string
RTOSString& RTOSString::operator=(const char* str) {
    if (str) {
        size_t str_len = strlen(str);
        if (str_len >= MAX_LENGTH) {
            str_len = MAX_LENGTH - 1;
        }
        memcpy(buffer, str, str_len);
        buffer[str_len] = '\0';
        len = str_len;
    } else {
        buffer[0] = '\0';
        len = 0;
    }
    return *this;
}

// Assignment from RTOSString
RTOSString& RTOSString::operator=(const RTOSString& other) {
    if (this != &other) {
        len = other.len;
        memcpy(buffer, other.buffer, len + 1);
    }
    return *this;
}

// Assignment from std::string
RTOSString& RTOSString::operator=(const std::string& str) {
    size_t str_len = str.length();
    if (str_len >= MAX_LENGTH) {
        str_len = MAX_LENGTH - 1;
    }
    memcpy(buffer, str.c_str(), str_len);
    buffer[str_len] = '\0';
    len = str_len;
    return *this;
}

// Equality comparison
bool RTOSString::operator==(const RTOSString& other) const {
    return strcmp(buffer, other.buffer) == 0;
}

bool RTOSString::operator==(const char* str) const {
    return str ? strcmp(buffer, str) == 0 : (len == 0);
}

// Clear string
void RTOSString::clear() {
    buffer[0] = '\0';
    len = 0;
}

// Append C string
void RTOSString::append(const char* str) {
    if (str) {
        size_t str_len = strlen(str);
        size_t remaining = MAX_LENGTH - len - 1;
        if (str_len > remaining) {
            str_len = remaining;
        }
        memcpy(buffer + len, str, str_len);
        len += str_len;
        buffer[len] = '\0';
    }
}

// Append RTOSString
void RTOSString::append(const RTOSString& other) {
    append(other.buffer);
}

// Compare strings
int RTOSString::compare(const RTOSString& other) const {
    return strcmp(buffer, other.buffer);
}

int RTOSString::compare(const char* str) const {
    return str ? strcmp(buffer, str) : 1;
}

// Find character
size_t RTOSString::find(char c) const {
    for (size_t i = 0; i < len; i++) {
        if (buffer[i] == c) {
            return i;
        }
    }
    return npos;
}

// Find substring
size_t RTOSString::find(const char* substr) const {
    if (!substr) return npos;
    
    const char* pos = strstr(buffer, substr);
    if (pos) {
        return pos - buffer;
    }
    return npos;
}

// Find last occurrence of any character in chars
size_t RTOSString::find_last_of(const char* chars) const {
    if (!chars) return npos;
    
    for (int i = len - 1; i >= 0; i--) {
        for (const char* c = chars; *c; c++) {
            if (buffer[i] == *c) {
                return i;
            }
        }
    }
    return npos;
}

// Substring
RTOSString RTOSString::substr(size_t pos, size_t length) const {
    RTOSString result;
    if (pos < len) {
        size_t actual_len = len - pos;
        if (length < actual_len) {
            actual_len = length;
        }
        if (actual_len >= MAX_LENGTH) {
            actual_len = MAX_LENGTH - 1;
        }
        memcpy(result.buffer, buffer + pos, actual_len);
        result.buffer[actual_len] = '\0';
        result.len = actual_len;
    }
    return result;
}
