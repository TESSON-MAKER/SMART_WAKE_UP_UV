#ifndef ESP01_H
#define ESP01_H

#include <stm32f7xx.h>

#define ESP_BUF_SIZE  128

#define ESP01_BAUDRATE 115200
#define UART7_AF8 0x08

extern uint8_t ESP01_TXBuffer[ESP_BUF_SIZE];
extern uint8_t ESP01_RXBuffer[ESP_BUF_SIZE];

void ESP01_Init(void);
void ESP01_UART_SendString(const char *str);
void ESP01_UART_SendFormattedString(const char *format, ...);
uint8_t ESP01_SendCommand(const char* cmd, const char* expected_response);

void ESP01_Transmit_DMA(const char *data);
void UART7_IRQHandler(void);
void DMA1_Stream3_IRQHandler(void);

#endif /* ESP01_H */
