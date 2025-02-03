#ifndef ESP01_H
#define ESP01_H

#include <stm32f7xx.h>

#define ESP01_BAUDRATE 115200
#define UART7_AF8 0x08

#define TIM3_PRESCALER_VALUE (SystemCoreClock / 1000000) - 1;
#define TIM3_PUSH_DELAY_VALUE 50000 - 1

void ESP01_Init(void);
void ESP01_UART_SendString(const char *str);
void ESP01_UART_SendFormattedString(const char *format, ...);
uint8_t ESP8266_Send_Cmd(char *cmd, char *ack, uint16_t timeout);

#endif /* ESP01_H */
