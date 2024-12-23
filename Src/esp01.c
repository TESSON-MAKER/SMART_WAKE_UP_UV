#include "esp01.h"
#include <stdarg.h>
#include <stdio.h>

/*******************************************************************
 * @name       :ESP01_Usart_Init
 * @date       :2024-10-22
 * @function   :ESP01 USART Initialization
 * @parameters :None
 * @retvalue   :None
********************************************************************/
void ESP01_Usart_Init(void)
{
    // Enable clock for GPIOE (port used by UART7)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;

    // Enable clock for UART7
    RCC->APB1ENR |= RCC_APB1ENR_UART7EN;

    // Configure PE8 in alternate function mode (UART7 TX)
    GPIOE->MODER &= ~GPIO_MODER_MODER8;     // Clear the configuration bits
    GPIOE->MODER |= GPIO_MODER_MODER8_1;    // Set PE8 to alternate function mode

    // Configure PE7 in alternate function mode (UART7 RX)
    GPIOE->MODER &= ~GPIO_MODER_MODER7;     // Clear the configuration bits
    GPIOE->MODER |= GPIO_MODER_MODER7_1;    // Set PE7 to alternate function mode

    // Assign alternate function AF7 to PE8 (UART7 TX)
    GPIOE->AFR[1] &= ~(0xF << GPIO_AFRH_AFRH0_Pos); // Clear existing bits
    GPIOE->AFR[1] |= (UART7_AF8 << GPIO_AFRH_AFRH0_Pos);

    // Assign alternate function AF7 to PE7 (UART7 RX)
    GPIOE->AFR[0] &= ~(0xF << GPIO_AFRL_AFRL7_Pos); // Clear existing bits
    GPIOE->AFR[0] |= (UART7_AF8 << GPIO_AFRL_AFRL7_Pos);

    // Set the baud rate
    UART7->BRR = SystemCoreClock / ESP01_BAUDRATE;

    // Enable transmitter (TE) and receiver (RE)
    UART7->CR1 = USART_CR1_TE | USART_CR1_RE;

    // Enable UART7 peripheral
    UART7->CR1 |= USART_CR1_UE;
}

/*******************************************************************
 * @name       :ESP01_Send
 * @date       :2024-10-22
 * @function   :Send formatted data over UART
 * @parameters :const char *format - formatted string to send
 * @retvalue   :None
********************************************************************/
void ESP01_Send(const char *format, ...)
{
    // Check if the pointer is NULL
    if (format == NULL)
    {
        return; // Do nothing if the format is NULL
    }

    char buffer[128]; // Buffer to store the formatted string
    va_list args;

    // Format the string
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    // Send each character one by one
    for (int i = 0; buffer[i] != '\0'; i++)
    {
        // Wait until the TDR register is ready to transmit
        while (!(UART7->ISR & USART_ISR_TXE));

        // Write the character to the data register
        UART7->TDR = buffer[i];
    }

    // Wait until the transmission is complete
    while (!(UART7->ISR & USART_ISR_TC));
}
