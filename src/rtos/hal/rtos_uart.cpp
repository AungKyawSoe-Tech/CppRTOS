#include "rtos_uart.h"
#include <cstdio>
#include <iostream>

bool RTOS_UART::initialized = false;

void RTOS_UART::init() {
    // For now, use stdio (will be replaced with actual UART for embedded)
    initialized = true;
}

void RTOS_UART::putc(char c) {
    // Host implementation (will be replaced with UART register writes)
    std::putchar(c);
}

void RTOS_UART::puts(const char* str) {
    if (str) {
        while (*str) {
            putc(*str++);
        }
    }
}

void RTOS_UART::write(const char* data, size_t length) {
    if (data) {
        for (size_t i = 0; i < length; i++) {
            putc(data[i]);
        }
    }
}

void RTOS_UART::printf(const char* format, ...) {
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    puts(buffer);
}

void RTOS_UART::vprintf(const char* format, va_list args) {
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    puts(buffer);
}
