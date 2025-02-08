#include "../Inc/esp01.h"
#include "../Inc/esp01_buffer.h"
#include "../Inc/tim.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#define ESP_BUF_SIZE  500
uint8_t ESP01_RX_FINISHED = 0;

uint8_t ESP_RX_BUF_ARRAY[ESP_BUF_SIZE];
uint8_t ESP_RX_CLIP_ARRAY[ESP_BUF_SIZE];

CircularBufferTypeDef ESP_RX_BUF;
ClipBufferTypeDef ESP_RX_CLIP;

uint8_t receive_complete = 0;

/*******************************************************************
 * @name       :ESP01_UART_ReceiveChar
 * @function   :Receive a single character
 *******************************************************************/
static uint8_t ESP01_UART_ReceiveChar(void)
{
    return (UART7->RDR & 0xFF);
}

static uint8_t ESP01_UART_GetITStatus(void)
{
	uint8_t verif = (UART7->ISR & USART_ISR_RXNE);
	return verif ? 1 : 0;
}

/*******************************************************************
 * @name       :UART7_IRQHandler
 * @function   :UART7 Interrupt Handler
 *******************************************************************/
void UART7_IRQHandler(void)
{
    if (ESP01_UART_GetITStatus())
    {
        uint8_t receivedChar = ESP01_UART_ReceiveChar();
        BUFFER_Write(&ESP_RX_BUF, receivedChar);
    }
}

/*******************************************************************
 * @name       :ESP01_GPIO_Config
 * @function   :Configure GPIO for UART
 *******************************************************************/
static void ESP01_GPIO_Config(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN; // Enable GPIOE clock

    // Configure PE8 as UART7 TX
    GPIOE->MODER &= ~GPIO_MODER_MODER8;
    GPIOE->MODER |= GPIO_MODER_MODER8_1;
    GPIOE->AFR[1] |= (UART7_AF8 << GPIO_AFRH_AFRH0_Pos);

    // Configure PE7 as UART7 RX
    GPIOE->MODER &= ~GPIO_MODER_MODER7;
    GPIOE->MODER |= GPIO_MODER_MODER7_1;
    GPIOE->AFR[0] |= (UART7_AF8 << GPIO_AFRL_AFRL7_Pos);
}

/*******************************************************************
 * @name       :ESP01_USART_Config
 * @function   :Configure UART for ESP01
 *******************************************************************/
static void ESP01_USART_Config(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_UART7EN;
    UART7->BRR = SystemCoreClock / ESP01_BAUDRATE;
    UART7->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE | USART_CR1_RXNEIE;
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
 * @function   :Initialize ESP01 module
 *******************************************************************/
void ESP01_Init(void)
{
    ESP01_GPIO_Config();
    ESP01_USART_Config();
    ESP01_BUFFER_Config();
}

/*******************************************************************
 * @name       :UART_SendByte
 * @function   :Send a byte via UART
 *******************************************************************/
void UART_SendByte(uint8_t data)
{
    while (!(UART7->ISR & USART_ISR_TXE));
    UART7->TDR = data;
}

/*******************************************************************
 * @name       :ESP01_UART_SendString
 * @function   :Send a string via UART
 *******************************************************************/
void ESP01_UART_SendString(const char* str)
{
    if (str == NULL) return;
    while (*str) UART_SendByte(*str++);
    while (!(UART7->ISR & USART_ISR_TC));
}

/*******************************************************************
 * @name       :ESP01_SendCommand
 * @function   :Send command and wait for expected response
 *******************************************************************/
// Correction dans ESP01_SendCommand pour mieux gérer le délai et la synchronisation
uint8_t ESP01_SendCommand(const char* cmd, const char* expected_response)
{
    BUFFER_ResetClip(&ESP_RX_CLIP);  // Réinitialiser le buffer
    ESP01_UART_SendString(cmd);      // Envoyer la commande
    ESP01_UART_SendString("\r\n");   // Ajouter un retour à la ligne
	
		TIM1_WaitMilliseconds(500);
		BUFFER_PopAllData(&ESP_RX_BUF, &ESP_RX_CLIP); // Extraire les données reçues
          
		if (strstr((char*)ESP_RX_CLIP_ARRAY, expected_response)) return 0;            // Retourner succès
		else return 1;
}
