#include "../Inc/esp01.h"
#include "../Inc/tim.h"

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#define ESP_BUF_SIZE  50

static uint8_t ESP01_TXBuffer[ESP_BUF_SIZE];
static uint8_t ESP01_RXBuffer[ESP_BUF_SIZE];

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
	UART7->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
	UART7->CR3 |= USART_CR3_DMAT | USART_CR3_DMAR; // Enable DMA TX and RX

	NVIC_EnableIRQ(UART7_IRQn);
}

/*******************************************************************
 * @name       :ESP01_DMA_Config
 * @function   :Configure DMA for ESP01
 *******************************************************************/
static void ESP01_DMA_Config(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN; // Enable DMA1 clock

	// Configure DMA1 Stream1 Channel5 for UART7_TX
	DMA1_Stream1->CR &= ~DMA_SxCR_EN;
	DMA1_Stream1->PAR = (uint32_t)&UART7->TDR;
	DMA1_Stream1->M0AR = (uint32_t)ESP01_TXBuffer;
	DMA1_Stream1->CR = (5 << DMA_SxCR_CHSEL_Pos) | DMA_SxCR_MINC | DMA_SxCR_DIR_0 | DMA_SxCR_TCIE;

	// Configure DMA1 Stream3 Channel5 for UART7_RX
	DMA1_Stream3->CR &= ~DMA_SxCR_EN;
	DMA1_Stream3->PAR = (uint32_t)&UART7->RDR;
	DMA1_Stream3->M0AR = (uint32_t)ESP01_RXBuffer;
	DMA1_Stream3->NDTR = ESP_BUF_SIZE;
	DMA1_Stream3->CR = (5 << DMA_SxCR_CHSEL_Pos) | DMA_SxCR_MINC | DMA_SxCR_CIRC | DMA_SxCR_TCIE;

	NVIC_EnableIRQ(DMA1_Stream3_IRQn); // Enable DMA RX interrupt
}

/*******************************************************************
 * @name       :ESP01_Transmit_DMA
 * @function   :Transmit data via DMA
 *******************************************************************/
void ESP01_Transmit_DMA(uint8_t *data)
{
	uint16_t size = strlen((char *)data);
	if (size > ESP_BUF_SIZE) size = ESP_BUF_SIZE;

	memset(ESP01_TXBuffer, 0, ESP_BUF_SIZE);
	memcpy(ESP01_TXBuffer, data, size);
	DMA1_Stream1->NDTR = size;
	DMA1_Stream1->CR |= DMA_SxCR_EN;
}

/*******************************************************************
 * @name       :UART7_IRQHandler
 * @function   :Handle UART7 interrupts
 *******************************************************************/
void UART7_IRQHandler(void)
{
	if (UART7->ISR & USART_ISR_RXNE)
	{
		uint8_t received = UART7->RDR; // Read received byte
		// Process received data if needed
	}
}

/*******************************************************************
 * @name       :DMA1_Stream3_IRQHandler
 * @function   :Handle DMA reception completion
 *******************************************************************/
void DMA1_Stream3_IRQHandler(void)
{
	if (DMA1->HISR & DMA_LISR_TCIF3)
	{
		DMA1->HIFCR |= DMA_LIFCR_CTCIF3; // Clear transfer complete flag
		// Process received data
	}
}
/*******************************************************************
 * @name       :ESP01_Init
 * @function   :Initialize ESP01 module
 *******************************************************************/
void ESP01_Init(void)
{
    ESP01_GPIO_Config();
    ESP01_USART_Config();
    ESP01_DMA_Config();
}
