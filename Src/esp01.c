#include "../Inc/esp01.h"
#include "../Inc/tim.h"

#include <string.h>
#include <stdio.h>

uint8_t ESP01_TXBuffer[ESP_BUF_SIZE] = {0};
uint8_t ESP01_RXBuffer[ESP_BUF_SIZE] = {0};
volatile uint8_t DataReady = 0; // Flag pour signaler que les données sont prêtes

/*******************************************************************
 * @name       :ESP01_GPIO_Config
 * @function   :Configure GPIO for UART7 (TX: PE8, RX: PE7)
 *******************************************************************/
static void ESP01_GPIO_Config(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN; // Activer l'horloge GPIOE

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
 * @function   :Configure UART7 for ESP01
 *******************************************************************/
static void ESP01_USART_Config(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_UART7EN; // Activer UART7
    UART7->BRR = SystemCoreClock / ESP01_BAUDRATE;
    UART7->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE; // Activer TX, RX et UART
    UART7->CR3 |= USART_CR3_DMAT | USART_CR3_DMAR; // Activer DMA pour TX et RX

    NVIC_EnableIRQ(UART7_IRQn); // Activer interruption UART7
}

/*******************************************************************
 * @name       :ESP01_DMA_Config
 * @function   :Configure DMA for UART7 TX/RX in circular mode
 *******************************************************************/
static void ESP01_DMA_Config(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN; // Activer DMA1

    // Configurer DMA1 Stream1 Channel5 pour UART7_TX
    DMA1_Stream1->CR &= ~DMA_SxCR_EN;
    DMA1_Stream1->PAR = (uint32_t)&UART7->TDR;
    DMA1_Stream1->M0AR = (uint32_t)ESP01_TXBuffer;
		DMA1_Stream1->NDTR = ESP_BUF_SIZE;
    DMA1_Stream1->CR = (5 << DMA_SxCR_CHSEL_Pos) | DMA_SxCR_MINC | DMA_SxCR_DIR_0 | DMA_SxCR_TCIE;

    // Configurer DMA1 Stream3 Channel5 pour UART7_RX en mode circulaire
    DMA1_Stream3->CR &= ~DMA_SxCR_EN;
    DMA1_Stream3->PAR = (uint32_t)&UART7->RDR;
    DMA1_Stream3->M0AR = (uint32_t)ESP01_RXBuffer;
    DMA1_Stream3->NDTR = ESP_BUF_SIZE;
    DMA1_Stream3->CR = (5 << DMA_SxCR_CHSEL_Pos) | DMA_SxCR_MINC | DMA_SxCR_TCIE | DMA_SxCR_CIRC; // Activer le mode circulaire

    NVIC_EnableIRQ(DMA1_Stream3_IRQn); // Activer interruption DMA RX

    // Démarrer la réception DMA immédiatement
    DMA1_Stream3->CR |= DMA_SxCR_EN;
}

/*******************************************************************
 * @name       :ESP01_Transmit_DMA
 * @function   :Transmit data via DMA
 *******************************************************************/
void ESP01_Transmit_DMA(const char *data)
{
    if (data == NULL) return; // Vérification de pointeur nul

    uint16_t size = strlen(data);
    if (size > ESP_BUF_SIZE) size = ESP_BUF_SIZE;

    memset(ESP01_TXBuffer, 0, ESP_BUF_SIZE);
    memcpy(ESP01_TXBuffer, data, size);

    // Désactiver DMA1 Stream1
    DMA1_Stream1->CR &= ~DMA_SxCR_EN;
    while (DMA1_Stream1->CR & DMA_SxCR_EN); // Attendre qu'il soit bien désactivé

    // Réinitialiser le FIFO et les flags d’erreur
    DMA1_Stream1->FCR &= ~DMA_SxFCR_DMDIS; // Activer le mode direct
    DMA1->HIFCR |= DMA_LIFCR_CTCIF1 | DMA_LIFCR_CHTIF1 | DMA_LIFCR_CTEIF1 | DMA_LIFCR_CDMEIF1 | DMA_LIFCR_CFEIF1; // Effacer les flags d’interruption

    // Reconfigurer le FIFO
    DMA1_Stream1->FCR |= DMA_SxFCR_FTH_0; // Régler le seuil FIFO à mi-rempli (1/2)

    // Réinitialiser le compteur de données
    DMA1_Stream1->NDTR = size;
    
    // S'assurer que l'UART est bien en mode DMA
    UART7->CR3 |= USART_CR3_DMAT; 

    // Réactiver le DMA
    DMA1_Stream1->CR |= DMA_SxCR_EN;
}



/*******************************************************************
 * @name       :DMA1_Stream3_IRQHandler
 * @function   :Handle DMA reception completion
 *******************************************************************/
void DMA1_Stream3_IRQHandler(void)
{
    if (DMA1->HISR & DMA_LISR_TCIF3) // Vérifier si le transfert est terminé
    {
        DMA1->HIFCR |= DMA_LIFCR_CTCIF3; // Effacer le flag d'interruption
        DataReady = 1; // Indiquer que des données sont prêtes
    }
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
        UART7->ICR |= (USART_ICR_FECF | USART_ICR_NCF | USART_ICR_ORECF | USART_ICR_PECF);
    }
}

/*******************************************************************
 * @name       :ESP01_GetReceivedData
 * @function   :Return received data
 *******************************************************************/
uint8_t ESP01_GetReceivedData(uint8_t *buffer, uint16_t maxSize)
{
    if (!DataReady) return 0; // Aucune donnée disponible

    uint16_t length = (maxSize < ESP_BUF_SIZE) ? maxSize : ESP_BUF_SIZE;
    memcpy(buffer, (uint8_t *)ESP01_RXBuffer, length); // Copier les données

    DataReady = 0; // Réinitialiser le flag
    return length; // Retourner la taille des données copiées
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
