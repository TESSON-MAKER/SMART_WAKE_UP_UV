#include "../Inc/ds3231.h"
#include "../Inc/tim.h"
#include "../Inc/usart.h"

/*******************************************************************
 * @name       :DS3231_GPIO_Config
 * @function   :Configure GPIO
 * @parameters :None
 * @retvalue   :None
 *******************************************************************/
static void DS3231_GPIO_Config(void)
{
    // Enable GPIOB clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    // Configure GPIOB Pin 8 as alternate function
    GPIOB->MODER |= GPIO_MODER_MODER8_1; // Set mode to alternate function
    GPIOB->MODER &= ~GPIO_MODER_MODER8_0; // Clear mode bits
    GPIOB->AFR[1] |= DS3231_I2C1_AF << GPIO_AFRH_AFRH0_Pos; //Alternante funtion for PB8
    GPIOB->OTYPER |= GPIO_OTYPER_OT8; // Set Pin 8 to open-drain

    // Configure GPIOB Pin 9 as alternate function
    GPIOB->MODER |= GPIO_MODER_MODER9_1; // Set mode to alternate function
    GPIOB->MODER &= ~GPIO_MODER_MODER9_0; // Clear mode bits
    GPIOB->AFR[1] |= DS3231_I2C1_AF << GPIO_AFRH_AFRH1_Pos; //Alternante funtion for PB9
    GPIOB->OTYPER |= GPIO_OTYPER_OT9; // Set Pin 9 to open-drain
}

/*******************************************************************
 * @name       :DS3231_I2C_Config
 * @function   :Configure I2C
 * @parameters :None
 * @retvalue   :None
 *******************************************************************/
static void DS3231_I2C_Config(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    I2C1->CR1 &= ~I2C_CR1_PE;
    I2C1->TIMINGR = 0x0000C1C1;
}

/*******************************************************************
 * @name       :DS3231_Init
 * @function   :DS3231 Initialization
 * @parameters :None
 * @retvalue   :None
 *******************************************************************/
void DS3231_Init(void)
{
    DS3231_GPIO_Config();
    DS3231_I2C_Config();
    TIM2_InitForGeneralPurpose();
}

/*******************************************************************
 * @name       :DS3231_BcdToBec
 * @function   :Convert BCD to decimal
 * @parameters :None
 * @retvalue   :Converted value
 *******************************************************************/
int DS3231_BcdToDec(unsigned char x)
{
    return x - 6 * (x >> 4);
}

/*******************************************************************
 * @name       :DS3231_DecToBcd
 * @function   :Convert decimal to BCD
 * @parameters :None
 * @retvalue   :Converted value
 *******************************************************************/
int DS3231_DecToBcd(unsigned char x)
{
    return x + 6 * (x / 10);
}

/*******************************************************************
 * @name       :DS3231_Read
 * @function   :Read data to DS3231 memory with timeout using TIM1
 * @parameters :None
 * @retvalue   :Converted value
 *******************************************************************/
int DS3231_Read(uint8_t memadd, uint8_t *data, uint8_t length, uint32_t timeout)
{
    // Enable I2C
    I2C1->CR1 |= I2C_CR1_PE;

    // Configure slave address for write mode
    I2C1->CR2 = (DS3231_I2C_ADRESS << 1); // Set slave address
    I2C1->CR2 &= ~I2C_CR2_ADD10; // 7-bit addressing
    I2C1->CR2 |= (1 << I2C_CR2_NBYTES_Pos); // 1 byte to write (memory address)
    I2C1->CR2 &= ~I2C_CR2_RD_WRN; // Write mode
    I2C1->CR2 |= I2C_CR2_START; // Generate start condition

    TIM2_ResetCounter();
    TIM2_StartTimer();

    // Wait for memory address transmission
    while (!(I2C1->ISR & I2C_ISR_TXIS))
    {
        if (TIM2_GetCounterValue() > timeout)
        {
            TIM2_StopTimer();
            return DS3231_TIMEOUT_ERROR;
        }
    }
    I2C1->TXDR = memadd;

    // Wait for transfer completion
    while (!(I2C1->ISR & I2C_ISR_TC))
    {
        if (TIM2_GetCounterValue() > timeout)
        {
            TIM2_StopTimer();
            return DS3231_TIMEOUT_ERROR;
        }
    }

    // Restart for read operation
    I2C1->CR2 = (DS3231_I2C_ADRESS << 1) | I2C_CR2_RD_WRN; // Address + Read mode
    I2C1->CR2 |= (length << I2C_CR2_NBYTES_Pos); // Number of bytes to read
    I2C1->CR2 |= I2C_CR2_AUTOEND; // Auto STOP
    I2C1->CR2 |= I2C_CR2_START; // Generate start condition

    for (uint8_t i = 0; i < length; i++)
    {
        while (!(I2C1->ISR & I2C_ISR_RXNE)) // Wait for data
        {
            if (TIM2_GetCounterValue() > timeout)
            {
                TIM2_StopTimer();
                return DS3231_TIMEOUT_ERROR;
            }
        }
        data[i] = I2C1->RXDR; // Read data
    }

    // Wait for STOP condition
    while (!(I2C1->ISR & I2C_ISR_STOPF))
    {
        if (TIM2_GetCounterValue() > timeout)
        {
            TIM2_StopTimer();
            return DS3231_TIMEOUT_ERROR;
        }
    }

    TIM2_StopTimer();
    I2C1->CR1 &= ~I2C_CR1_PE;
    return DS3231_SUCCESS;
}

/*******************************************************************
 * @name       :DS3231_Write
 * @function   :Writes data to DS3231 memory with a timeout
 * @parameters :memadd, data, length, timeout
 * @retvalue   :DS3231_SUCCESS or DS3231_TIMEOUT_ERROR
 *******************************************************************/
int DS3231_Write(uint8_t memadd, uint8_t *data, uint8_t length, uint32_t timeout)
{
    // Enable I2C
    I2C1->CR1 |= I2C_CR1_PE;

    // Configure slave address for write mode
    I2C1->CR2 = (DS3231_I2C_ADRESS << 1); // Set slave address
    I2C1->CR2 &= ~I2C_CR2_ADD10; // 7-bit addressing
    I2C1->CR2 |= ((length + 1) << I2C_CR2_NBYTES_Pos); // Memory address + data
    I2C1->CR2 &= ~I2C_CR2_RD_WRN; // Write mode
    I2C1->CR2 |= I2C_CR2_START; // Generate start condition

    TIM2_ResetCounter();
    TIM2_StartTimer();

    // Wait for memory address transmission
    while (!(I2C1->ISR & I2C_ISR_TXIS))
    {
        if (TIM2_GetCounterValue() > timeout)
        {
            TIM2_StopTimer();
            return DS3231_TIMEOUT_ERROR;
        }
    }
    I2C1->TXDR = memadd; // Send memory address

    // Transmit data
    for (uint8_t i = 0; i < length; i++)
    {
        while (!(I2C1->ISR & I2C_ISR_TXIS))
        {
            if (TIM2_GetCounterValue() > timeout)
            {
                TIM2_StopTimer();
                return DS3231_TIMEOUT_ERROR;
            }
        }
        I2C1->TXDR = data[i]; // Send data
    }

    // Wait for STOP condition
    while (!(I2C1->ISR & I2C_ISR_STOPF))
    {
        if (TIM2_GetCounterValue() > timeout)
        {
            TIM2_StopTimer();
            return DS3231_TIMEOUT_ERROR;
        }
    }

    TIM2_StopTimer();
    I2C1->CR1 &= ~I2C_CR1_PE;
    return DS3231_SUCCESS;
}