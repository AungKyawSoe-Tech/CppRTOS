#ifndef RTOS_UART_H
#define RTOS_UART_H

#include <cstddef>
#include <cstdarg>

// UART output interface (replaces std::cout)
class RTOS_UART {
public:
    // Initialize UART (platform-specific)
    static void init();
    
    // Output functions
    static void putc(char c);
    static void puts(const char* str);
    static void write(const char* data, size_t length);
    
    // Printf-style formatting
    static void printf(const char* format, ...);
    static void vprintf(const char* format, va_list args);
    
private:
    static bool initialized;
};

// Convenience macros
#define rtos_printf RTOS_UART::printf
#define rtos_puts RTOS_UART::puts
#define rtos_putc RTOS_UART::putc

#endif // RTOS_UART_H
