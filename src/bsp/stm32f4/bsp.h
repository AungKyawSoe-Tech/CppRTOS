#ifndef BSP_STM32F4_H
#define BSP_STM32F4_H

/**
 * Board Support Package for STM32F4 Discovery Board
 * 
 * Target: STM32F407VGT6
 * - ARM Cortex-M4F @ 168 MHz
 * - 192 KB SRAM
 * - 1 MB Flash
 * - FPU (single precision)
 * - Integrated peripherals: UART, SPI, I2C, USB, etc.
 */

#include <cstdint>

// System clock configuration
#define CPU_FREQUENCY_HZ        168000000UL  // 168 MHz
#define SYSTICK_FREQUENCY_HZ    1000UL       // 1 kHz (1ms tick)

// Memory map
#define FLASH_BASE              0x08000000UL
#define SRAM_BASE               0x20000000UL
#define PERIPH_BASE             0x40000000UL

// Flash and SRAM sizes
#define FLASH_SIZE              (1024 * 1024)  // 1 MB
#define SRAM_SIZE               (192 * 1024)   // 192 KB

// Stack configuration
#define MAIN_STACK_SIZE         4096
#define PROCESS_STACK_SIZE      1024

// UART configuration (for debug output)
#define DEBUG_UART_NUM          2              // USART2
#define DEBUG_UART_BAUDRATE     115200
#define DEBUG_UART_TX_PIN       2              // PA2
#define DEBUG_UART_RX_PIN       3              // PA3

// LED configuration (for testing)
#define LED_GPIO_PORT           3              // GPIOD
#define LED_GREEN_PIN           12             // PD12
#define LED_ORANGE_PIN          13             // PD13
#define LED_RED_PIN             14             // PD14
#define LED_BLUE_PIN            15             // PD15

// Peripheral register definitions
struct GPIO_Registers {
    volatile uint32_t MODER;      // Mode register
    volatile uint32_t OTYPER;     // Output type register
    volatile uint32_t OSPEEDR;    // Output speed register
    volatile uint32_t PUPDR;      // Pull-up/pull-down register
    volatile uint32_t IDR;        // Input data register
    volatile uint32_t ODR;        // Output data register
    volatile uint32_t BSRR;       // Bit set/reset register
    volatile uint32_t LCKR;       // Lock register
    volatile uint32_t AFR[2];     // Alternate function registers
};

struct RCC_Registers {
    volatile uint32_t CR;         // Clock control register
    volatile uint32_t PLLCFGR;    // PLL configuration register
    volatile uint32_t CFGR;       // Clock configuration register
    volatile uint32_t CIR;        // Clock interrupt register
    volatile uint32_t AHB1RSTR;   // AHB1 peripheral reset register
    volatile uint32_t AHB2RSTR;   // AHB2 peripheral reset register
    volatile uint32_t AHB3RSTR;   // AHB3 peripheral reset register
    volatile uint32_t RESERVED0;
    volatile uint32_t APB1RSTR;   // APB1 peripheral reset register
    volatile uint32_t APB2RSTR;   // APB2 peripheral reset register
    volatile uint32_t RESERVED1[2];
    volatile uint32_t AHB1ENR;    // AHB1 peripheral clock enable
    volatile uint32_t AHB2ENR;    // AHB2 peripheral clock enable
    volatile uint32_t AHB3ENR;    // AHB3 peripheral clock enable
    volatile uint32_t RESERVED2;
    volatile uint32_t APB1ENR;    // APB1 peripheral clock enable
    volatile uint32_t APB2ENR;    // APB2 peripheral clock enable
};

struct USART_Registers {
    volatile uint32_t SR;         // Status register
    volatile uint32_t DR;         // Data register
    volatile uint32_t BRR;        // Baud rate register
    volatile uint32_t CR1;        // Control register 1
    volatile uint32_t CR2;        // Control register 2
    volatile uint32_t CR3;        // Control register 3
    volatile uint32_t GTPR;       // Guard time and prescaler
};

// Peripheral base addresses
#define GPIOA               ((GPIO_Registers*)(PERIPH_BASE + 0x00020000))
#define GPIOB               ((GPIO_Registers*)(PERIPH_BASE + 0x00020400))
#define GPIOC               ((GPIO_Registers*)(PERIPH_BASE + 0x00020800))
#define GPIOD               ((GPIO_Registers*)(PERIPH_BASE + 0x00020C00))
#define GPIOE               ((GPIO_Registers*)(PERIPH_BASE + 0x00021000))
#define RCC                 ((RCC_Registers*)(PERIPH_BASE + 0x00023800))
#define USART1              ((USART_Registers*)(PERIPH_BASE + 0x00011000))
#define USART2              ((USART_Registers*)(PERIPH_BASE + 0x00004400))
#define USART3              ((USART_Registers*)(PERIPH_BASE + 0x00004800))

// RCC register bits
#define RCC_AHB1ENR_GPIOAEN (1 << 0)
#define RCC_AHB1ENR_GPIOBEN (1 << 1)
#define RCC_AHB1ENR_GPIOCEN (1 << 2)
#define RCC_AHB1ENR_GPIODEN (1 << 3)
#define RCC_AHB1ENR_GPIOEEN (1 << 4)
#define RCC_APB1ENR_USART2EN (1 << 17)

// USART register bits
#define USART_SR_TXE        (1 << 7)          // Transmit data register empty
#define USART_SR_RXNE       (1 << 5)          // Read data register not empty
#define USART_CR1_UE        (1 << 13)         // USART enable
#define USART_CR1_TE        (1 << 3)          // Transmitter enable
#define USART_CR1_RE        (1 << 2)          // Receiver enable

namespace BSP {
    /**
     * Initialize the BSP
     * - Configure system clocks
     * - Initialize debug UART
     * - Setup LEDs
     */
    void initialize();
    
    /**
     * Configure system clocks
     * - Setup PLL for 168 MHz
     * - Configure flash latency
     * - Enable FPU
     */
    void configureClocks();
    
    /**
     * Initialize debug UART
     */
    void initUART();
    
    /**
     * Initialize LEDs
     */
    void initLEDs();
    
    /**
     * Write a character to debug UART
     */
    void putChar(char c);
    
    /**
     * Read a character from debug UART (blocking)
     */
    char getChar();
    
    /**
     * Control LED
     */
    void setLED(uint8_t led_num, bool on);
    
    /**
     * Toggle LED
     */
    void toggleLED(uint8_t led_num);
    
    /**
     * Delay in milliseconds (busy wait - not for use in RTOS tasks!)
     */
    void delayMs(uint32_t ms);
}

#endif // BSP_STM32F4_H
