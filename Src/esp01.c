#include "../Inc/esp01.h"
#include "../Inc/esp01_buffer.h"
#include "../Inc/tim.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#define ESP_BUF_SIZE  128
uint8_t ESP01_RX_FINISHED=0;

CircularBufferTypeDef ESP_RX_BUF;
uint8_t ESP_RX_BUF_ARRAY[ESP_BUF_SIZE] = {0x00};

ClipBufferTypeDef ESP_RX_CLIP;
uint8_t ESP_RX_CLIP_ARRAY[ESP_BUF_SIZE] = {0x00};

/*******************************************************************
 * @name       :ESP01_GPIO_Config
 * @function   :Configure GPIO for UART
 * @parameters :None
 * @retvalue   :None
 *******************************************************************/
static void ESP01_GPIO_Config(void)
{
	// Enable clock for GPIOE (port used by UART7)
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
	
	// Configure PE8 in alternate function mode (UART7 TX)
	GPIOE->MODER &= ~GPIO_MODER_MODER8;
	GPIOE->MODER |= GPIO_MODER_MODER8_1;
	GPIOE->AFR[1] |= (UART7_AF8 << GPIO_AFRH_AFRH0_Pos);
	
	// Configure PE7 in alternate function mode (UART7 RX)
	GPIOE->MODER &= ~GPIO_MODER_MODER7;
	GPIOE->MODER |= GPIO_MODER_MODER7_1;
	GPIOE->AFR[0] |= (UART7_AF8 << GPIO_AFRL_AFRL7_Pos);
}

/*******************************************************************
 * @name       :ESP01_USART_Config
 * @function   :Configure UART for ESP01
 * @parameters :None
 * @retvalue   :None
 *******************************************************************/
static void ESP01_USART_Config(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_UART7EN;
	UART7->BRR = SystemCoreClock / ESP01_BAUDRATE;
	UART7->CR1 = USART_CR1_TE | USART_CR1_RE;
	UART7->CR1 |= USART_CR1_UE;
	UART7->CR1 |= USART_CR1_RXNEIE;
	NVIC_EnableIRQ(UART7_IRQn);
}

/*******************************************************************
 * @name       :ESP01_BUFFER_Config
 * @function   :Configure BUFFER for ESP01
 *******************************************************************/
static void ESP01_BUFFER_Config()
{
	BUFFER_CircularInit(&ESP_RX_BUF, ESP_RX_BUF_ARRAY, ESP_BUF_SIZE);
  BUFFER_ClipInit(&ESP_RX_CLIP, ESP_RX_CLIP_ARRAY, ESP_BUF_SIZE);
}

/*******************************************************************
 * @name       :ESP01_Init
 * @function   :Init ESP01
 *******************************************************************/
void ESP01_Init(void)
{
	ESP01_GPIO_Config();
	ESP01_USART_Config();
	ESP01_BUFFER_Config();
}

/*******************************************************************
 * @name       :UART_SendByte
 * @function   :Send a string
 * @parameters :None
 * @retvalue   :None
 *******************************************************************/
void UART_SendByte(uint8_t data) 
{
	while (!(UART7->ISR & USART_ISR_TXE));
	UART7->TDR = data;
}

/*******************************************************************
 * @name       :ESP01_UART_SendString
 * @function   :Send a string
 * @parameters :None
 * @retvalue   :None
 *******************************************************************/
void ESP01_UART_SendString(const char* str) 
{
	if (str == NULL) return; // Do nothing if the string is NULL
	while (*str) UART_SendByte(*str++);
	while (!(UART7->ISR & USART_ISR_TC));
}

/*******************************************************************
 * @name       :ESP01_UART_SendFormattedString
 * @function   :Send a formated string
 * @parameters :None
 * @retvalue   :None
 *******************************************************************/
void ESP01_UART_SendFormattedString(const char *format, ...)
{
	if (format == NULL) return; // Do nothing if the format is NULL

	va_list args;
	va_start(args, format);
	char buffer[128]; // Buffer to store the formatted string
	vsnprintf(buffer, sizeof(buffer), format, args); // Format the string
	va_end(args);

	ESP01_UART_SendString(buffer); // Send the formatted string
}

/*******************************************************************
 * @name       :ESP01_UART_ReceiveChar
 * @function   :Receive a uint8_t value
 * @parameters :None
 * @retvalue   :receivedValue
 *******************************************************************/
static uint8_t ESP01_UART_ReceiveChar(void) 
{
	uint8_t receivedValue = (UART7->RDR & 0xFF);
	return receivedValue;
}

/*******************************************************************
 * @name       :ESP01_UART_GetITStatus
 * @function   :Verification
 * @parameters :None
 * @retvalue   :receivedValue
 *******************************************************************/
static uint8_t ESP01_UART_GetITStatus(void)
{
	uint8_t verif = (UART7->ISR & USART_ISR_RXNE);
	return verif ? 1 : 0;
}

/*******************************************************************
 * @name       :USART_ClearITPendingBit
 * @function   :Clear the interrupt pending bit for USART errors
 * @parameters :None
 * @retvalue   :None
 *******************************************************************/
static void ESP01_USART_ClearITPendingBit(void) 
{
	UART7->ICR |= USART_ICR_ORECF;
	UART7->ICR |= USART_ICR_PECF;
}

/*******************************************************************
 * @name       :UART7_IRQHandler
 * @function   :UART7 Interrupt Handler for RX data reception
 * @parameters :None
 * @retvalue   :None
 *******************************************************************/
void UART7_IRQHandler(void)
{
	if (ESP01_UART_GetITStatus())
	{
		uint8_t receivedChar = ESP01_UART_ReceiveChar();
		BUFFER_Write(&ESP_RX_BUF, receivedChar);
		ESP01_USART_ClearITPendingBit();
	}
}