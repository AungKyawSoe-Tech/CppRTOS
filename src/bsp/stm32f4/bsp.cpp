#include "bsp.h"

namespace BSP {

void initialize() {
    // Configure system clocks
    configureClocks();
    
    // Initialize UART for debug output
    initUART();
    
    // Initialize LEDs
    initLEDs();
    
    putChar('\n');
    putChar('B');
    putChar('S');
    putChar('P');
    putChar(' ');
    putChar('O');
    putChar('K');
    putChar('\n');
}

void configureClocks() {
    // Enable HSI (High-Speed Internal oscillator)
    RCC->CR |= (1 << 0);  // HSION
    while (!(RCC->CR & (1 << 1)));  // Wait for HSIRDY
    
    // Configure PLL (simplified - use HSI for now)
    // For production, configure PLL to 168 MHz from HSE
    
    // Enable peripheral clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;  // GPIOA
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;  // GPIOD (LEDs)
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN; // USART2
    
    // Enable FPU (Cortex-M4F)
    // CPACR: Coprocessor Access Control Register
    volatile uint32_t* cpacr = (volatile uint32_t*)0xE000ED88;
    *cpacr |= (0xF << 20);  // Enable CP10 and CP11 (FPU)
}

void initUART() {
    // Configure PA2 (TX) and PA3 (RX) for USART2
    // Set alternate function mode (AF7 for USART2)
    
    // PA2 and PA3 to alternate function mode
    GPIOA->MODER &= ~((3 << (2*2)) | (3 << (3*2)));
    GPIOA->MODER |= (2 << (2*2)) | (2 << (3*2));
    
    // Set alternate function 7 (USART2) for PA2 and PA3
    GPIOA->AFR[0] &= ~((0xF << (2*4)) | (0xF << (3*4)));
    GPIOA->AFR[0] |= (7 << (2*4)) | (7 << (3*4));
    
    // Configure USART2
    // Baud rate = fCK / (8 * (2 - OVER8) * USARTDIV)
    // For 16 MHz clock and 115200 baud: USARTDIV = 16000000 / (16 * 115200) = 8.68
    // BRR = 8 (mantissa) and 0.68 * 16 = 11 (fraction)
    USART2->BRR = (8 << 4) | 11;  // Simplified calculation
    
    // Enable USART, transmitter, and receiver
    USART2->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

void initLEDs() {
    // Configure PD12-PD15 as output (LEDs on Discovery board)
    GPIOD->MODER &= ~((3 << (12*2)) | (3 << (13*2)) | (3 << (14*2)) | (3 << (15*2)));
    GPIOD->MODER |= (1 << (12*2)) | (1 << (13*2)) | (1 << (14*2)) | (1 << (15*2));
    
    // Set to push-pull output
    GPIOD->OTYPER &= ~((1 << 12) | (1 << 13) | (1 << 14) | (1 << 15));
    
    // Set to low speed
    GPIOD->OSPEEDR &= ~((3 << (12*2)) | (3 << (13*2)) | (3 << (14*2)) | (3 << (15*2)));
    
    // No pull-up/pull-down
    GPIOD->PUPDR &= ~((3 << (12*2)) | (3 << (13*2)) | (3 << (14*2)) | (3 << (15*2)));
    
    // Turn off all LEDs initially
    GPIOD->BSRR = (1 << (12+16)) | (1 << (13+16)) | (1 << (14+16)) | (1 << (15+16));
}

void putChar(char c) {
    // Wait for transmit data register empty
    while (!(USART2->SR & USART_SR_TXE));
    
    // Write character
    USART2->DR = c;
}

char getChar() {
    // Wait for data available
    while (!(USART2->SR & USART_SR_RXNE));
    
    // Read character
    return (char)USART2->DR;
}

void setLED(uint8_t led_num, bool on) {
    uint8_t pin = 12 + led_num;  // LEDs on PD12-PD15
    if (pin > 15) return;
    
    if (on) {
        GPIOD->BSRR = (1 << pin);  // Set bit
    } else {
        GPIOD->BSRR = (1 << (pin + 16));  // Reset bit
    }
}

void toggleLED(uint8_t led_num) {
    uint8_t pin = 12 + led_num;  // LEDs on PD12-PD15
    if (pin > 15) return;
    
    GPIOD->ODR ^= (1 << pin);
}

void delayMs(uint32_t ms) {
    // Very rough delay - DO NOT use in RTOS tasks!
    // Use for initialization only
    for (uint32_t i = 0; i < ms; i++) {
        for (volatile uint32_t j = 0; j < 1000; j++);
    }
}

} // namespace BSP
