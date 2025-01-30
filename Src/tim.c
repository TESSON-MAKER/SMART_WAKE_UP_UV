#include "../Inc/tim.h"

/*******************************************************************
 * @name       : TIM1_InitForDelay
 * @function   : Initialize Timer 1 for delay purposes
 * @parameters : None
 * @retvalue   : None
 *******************************************************************/
void TIM1_InitForDelay(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;   // Enable Timer 1 clock
	while (!(RCC->APB2ENR & RCC_APB2ENR_TIM1EN)); // Wait for Timer 1 clock to be enabled

	TIM1->CR1 |= TIM_CR1_OPM;             // One-Pulse Mode (stops after expiration)
}

/*******************************************************************
 * @name       : TIM2_InitForGeneralPurpose
 * @function   : Initialize Timer 2 for general purposes
 * @parameters : None
 * @retvalue   : None
 *******************************************************************/
void TIM2_InitForGeneralPurpose(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;   // Enable Timer 2 clock
	while (!(RCC->APB1ENR & RCC_APB1ENR_TIM2EN)); // Wait for Timer 2 clock to be enabled

	TIM2->PSC = TIM_PSC_MICROSECONDS;     // Prescaler for milliseconds
	TIM2->ARR = 0xFFFF;                   // Set max count value
	//TIM2->CR1 |= TIM_CR1_OPM;           // One-Pulse Mode (stops after expiration)
}

/*******************************************************************
 * @name       : TIM1_WaitMicroseconds
 * @function   : Wait for a given number of microseconds using Timer 1
 * @parameters : us (microseconds to wait)
 * @retvalue   : None
 *******************************************************************/
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

/*******************************************************************
 * @name       : TIM1_WaitMilliseconds
 * @function   : Wait for a given number of milliseconds using Timer 1
 * @parameters : ms (milliseconds to wait)
 * @retvalue   : None
 *******************************************************************/
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

/*******************************************************************
 * @name       : TIM2_StartTimer
 * @function   : Start Timer 2
 * @parameters : None
 * @retvalue   : None
 *******************************************************************/
void TIM2_StartTimer(void)
{
	TIM2->CR1 |= TIM_CR1_CEN;  // Start Timer 2
}

/*******************************************************************
 * @name       : TIM2_StopTimer
 * @function   : Stop Timer 2
 * @parameters : None
 * @retvalue   : None
 *******************************************************************/
void TIM2_StopTimer(void)
{
	TIM2->CR1 &= ~TIM_CR1_CEN; // Stop Timer 2
}

/*******************************************************************
 * @name       : TIM2_ResetCounter
 * @function   : Reset Timer 2 counter
 * @parameters : None
 * @retvalue   : None
 *******************************************************************/
void TIM2_ResetCounter(void)
{
	TIM2->CNT = 0;  // Reset Timer 2 counter
}

/*******************************************************************
 * @name       : TIM2_GetCounterValue
 * @function   : Get the current value of Timer 2 counter
 * @parameters : None
 * @retvalue   : Current counter value
 *******************************************************************/
uint32_t TIM2_GetCounterValue(void)
{
	return TIM2->CNT;  // Return the current value of Timer 2 counter
}
