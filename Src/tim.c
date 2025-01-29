#include "../Inc/tim.h"

/*******************************************************************
 * @name       :TIM1_InitForDelay
 * @function   :Initialize Timer 1 for delay purposes
 * @parameters :None
 * @retvalue   :None
 *******************************************************************/
void TIM1_InitForDelay(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;   // Enable Timer 1 clock
	TIM1->CR1 |= TIM_CR1_OPM;             // One-Pulse Mode (stops after expiration)
}

/*******************************************************************
 * @name       :TIM2_InitForGeneralPurpose
 * @function   :Initialize Timer 2 for general purposes
 * @parameters :None
 * @retvalue   :None
 *******************************************************************/
void TIM2_InitForGeneralPurpose(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;   // Enable Timer 2 clock
	TIM2->PSC = TIM_PSC_MILLISECONDS;     // Prescaler for 1 MHz (1 us per tick with a 16 MHz clock)
	TIM2->CR1 |= TIM_CR1_OPM;             // One-Pulse Mode (stops after expiration)
}

/*******************************************************************
 * @name       :TIM1_WaitMicroseconds
 * @function   :Wait for a given number of microseconds using Timer 1
 * @parameters :us (microseconds to wait)
 * @retvalue   :None
 *******************************************************************/
void TIM1_WaitMicroseconds(unsigned int us)
{
	TIM1->CR1 &= ~TIM_CR1_CEN;            // Disable the timer before configuration
	TIM1->CNT = 0;                        // Reset the counter
	TIM1->PSC = TIM_PSC_MICROSECONDS;     // Prescaler for 1 MHz
	TIM1->ARR = us;                       // Set the wait time
	TIM1->SR &= ~TIM_SR_UIF;              // Clear the interrupt flag
	TIM1->CR1 |= TIM_CR1_CEN;             // Start the timer

	while (!(TIM1->SR & TIM_SR_UIF));     // Wait for the delay to complete

	TIM1->SR &= ~TIM_SR_UIF;              // Clear the interrupt flag
	TIM1->CR1 &= ~TIM_CR1_CEN;            // Stop the timer
}

/*******************************************************************
 * @name       :TIM1_WaitMilliseconds
 * @function   :Wait for a given number of milliseconds using Timer 1
 * @parameters :ms (milliseconds to wait)
 * @retvalue   :None
 *******************************************************************/
void TIM1_WaitMilliseconds(unsigned int ms)
{
	TIM1->CR1 &= ~TIM_CR1_CEN;            // Disable the timer before configuration
	TIM1->CNT = 0;                        // Reset the counter
	TIM1->PSC = TIM_PSC_MILLISECONDS;     // Prescaler for 1 kHz
	TIM1->ARR = ms;                       // Set the wait time
	TIM1->SR &= ~TIM_SR_UIF;              // Clear the interrupt flag
	TIM1->CR1 |= TIM_CR1_CEN;             // Start the timer

	while (!(TIM1->SR & TIM_SR_UIF));     // Wait for the delay to complete

	TIM1->SR &= ~TIM_SR_UIF;              // Clear the interrupt flag
	TIM1->CR1 &= ~TIM_CR1_CEN;            // Stop the timer
}

/*******************************************************************
 * @name       :TIM2_StartTimer
 * @function   :Start Timer 2
 * @parameters :None
 * @retvalue   :None
 *******************************************************************/
void TIM2_StartTimer(void)
{
	TIM2->CR1 |= TIM_CR1_CEN;  // Start Timer 2
}

/*******************************************************************
 * @name       :TIM2_StopTimer
 * @function   :Stop Timer 2
 * @parameters :None
 * @retvalue   :None
 *******************************************************************/
void TIM2_StopTimer(void)
{
	TIM2->CR1 &= ~TIM_CR1_CEN; // Stop Timer 2
}

/*******************************************************************
 * @name       :TIM2_ResetCounter
 * @function   :Reset Timer 2 counter
 * @parameters :None
 * @retvalue   :None
 *******************************************************************/
void TIM2_ResetCounter(void)
{
	TIM2->CNT = 0;  // Reset Timer 2 counter
}

/*******************************************************************
 * @name       :TIM2_GetCounterValue
 * @function   :Get the current value of Timer 2 counter
 * @parameters :None
 * @retvalue   :Current counter value
 *******************************************************************/
unsigned int TIM2_GetCounterValue(void)
{
	return TIM2->CNT;  // Return the current value of Timer 2 counter
}
