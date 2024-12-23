#ifndef ESP01_H
#define ESP01_H

#include <stm32f7xx.h>

#define ESP01_BAUDRATE 115200
#define UART7_AF8 0x08

void ESP01_Usart_Init(void);
void ESP01_Send(const char *format, ...);

#endif /* ESP01_H */
