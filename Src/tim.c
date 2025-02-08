#include "../Inc/tim.h"

///////////////////////////////////////TIM1////////////////////////////////////////////////
void TIM1_InitForDelay(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;   // Enable Timer 1 clock
}

void TIM1_WaitMicroseconds(uint32_t us)
{
	TIM1->CR1 &= ~TIM_CR1_CEN;            // Disable timer before configuration
	TIM1->CNT = 0;                        // Reset counter
	TIM1->PSC = TIM_PSC_MICROSECONDS;     // Prescaler for microseconds
	TIM1->ARR = us;                       // Set delay
	TIM1->SR &= ~TIM_SR_UIF;              // Clear interrupt flag
	TIM1->CR1 |= TIM_CR1_CEN;             // Start timer

	while (!(TIM1->SR & TIM_SR_UIF));     // Wait for update flag

	TIM1->SR &= ~TIM_SR_UIF;              // Clear flag
	TIM1->CR1 &= ~TIM_CR1_CEN;            // Stop timer
}

void TIM1_WaitMilliseconds(uint32_t ms)
{
	TIM1->CR1 &= ~TIM_CR1_CEN;            // Disable timer before configuration
	TIM1->CNT = 0;                        // Reset counter
	TIM1->PSC = TIM_PSC_MILLISECONDS;     // Prescaler for milliseconds
	TIM1->ARR = ms;                       // Set delay
	TIM1->SR &= ~TIM_SR_UIF;              // Clear interrupt flag
	TIM1->CR1 |= TIM_CR1_CEN;             // Start timer

	while (!(TIM1->SR & TIM_SR_UIF));     // Wait for update flag

	TIM1->SR &= ~TIM_SR_UIF;              // Clear flag
	TIM1->CR1 &= ~TIM_CR1_CEN;            // Stop timer
}

//////////////////////////TIM2//////////////////////////////////////////////
void TIM2_Init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	TIM2->CR1 &= ~TIM_CR1_CEN;
	TIM2->PSC = TIM_PSC_MICROSECONDS;
	TIM2->DIER |= TIM_DIER_UIE;
	TIM2->SR &= ~TIM_SR_UIF;
}

void TIM2_SetTimer(uint32_t timeout)
{
	TIM2->ARR = timeout;
}

void TIM2_StartTimer(void)
{
	TIM2->CR1 |= TIM_CR1_CEN;  // Start Timer 2
}

void TIM2_StopTimer(void)
{
	TIM2->CR1 &= ~TIM_CR1_CEN; // Stop Timer 2
}

void TIM2_ResetCounter(void)
{
	TIM2->CNT = 0;  // Reset Timer 2 counter
}

uint32_t TIM2_GetCounterValue(void)
{
	return TIM2->CNT;  // Return the current value of Timer 2 counter
}

uint32_t TIM2_TimeoutFlag(void)
{
	return (TIM2->SR & TIM_SR_UIF);
}

void TIM2_ResetTimeoutFlag(void)
{
	TIM2->SR &= ~TIM_SR_UIF;
}
	
