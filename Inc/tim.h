#ifndef TIM_H
#define TIM_H

#include <stm32f7xx.h>

// Prescalers based on SystemCoreClock
#define TIM_PSC_MICROSECONDS ((SystemCoreClock / 1000000) - 1) // Prescaler for microseconds (1 us per tick)
#define TIM_PSC_MILLISECONDS ((SystemCoreClock / 1000) - 1)    // Prescaler for milliseconds (1 ms per tick)

// Timer 1 and Timer 2 function prototypes
void TIM1_InitForDelay(void);           // Initialize Timer 1 for delay purposes
void TIM1_WaitMicroseconds(unsigned int us);  // Wait for a specific number of microseconds using Timer 1
void TIM1_WaitMilliseconds(unsigned int ms);  // Wait for a specific number of milliseconds using Timer 1

void TIM2_Init(void);  // Initialize Timer 2 for general purposes
void TIM2_SetTimer(uint32_t timeout);
void TIM2_StartTimer(void);
void TIM2_StopTimer(void);
void TIM2_ResetCounter(void);
uint32_t TIM2_GetCounterValue(void);
uint32_t TIM2_TimeoutFlag(void);
void TIM2_ResetTimeoutFlag(void);
	

#endif // TIM_H
