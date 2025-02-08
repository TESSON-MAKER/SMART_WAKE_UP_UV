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
    GPIOB->AFR[1] |= DS3231_I2C1_AF << GPIO_AFRH_AFRH0_Pos; // Alternate function for PB8
    GPIOB->OTYPER |= GPIO_OTYPER_OT8; // Set Pin 8 to open-drain

    // Configure GPIOB Pin 9 as alternate function
    GPIOB->MODER |= GPIO_MODER_MODER9_1; // Set mode to alternate function
    GPIOB->MODER &= ~GPIO_MODER_MODER9_0; // Clear mode bits
    GPIOB->AFR[1] |= DS3231_I2C1_AF << GPIO_AFRH_AFRH1_Pos; // Alternate function for PB9
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
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;  // Enable I2C1 clock
    I2C1->CR1 &= ~I2C_CR1_PE;             // Disable I2C before configuration
    I2C1->TIMINGR = 0x0000C1C1;           // Set timing register (standard speed)
    I2C1->CR1 |= I2C_CR1_PE;              // Enable I2C after configuration
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
    //TIM2_InitForGeneralPurpose(); // Uncomment if necessary
}

/*******************************************************************
 * @name       :DS3231_BcdToDec
 * @function   :Convert BCD to decimal
 * @parameters :BCD value
 * @retvalue   :Decimal value
 *******************************************************************/
int DS3231_BcdToDec(unsigned char x)
{
    return x - 6 * (x >> 4);
}

/*******************************************************************
 * @name       :DS3231_DecToBcd
 * @function   :Convert decimal to BCD
 * @parameters :Decimal value
 * @retvalue   :BCD value
 *******************************************************************/
int DS3231_DecToBcd(unsigned char x)
{
    return x + 6 * (x / 10);
}

/*******************************************************************
 * @name       :DS3231_Read
 * @function   :Read data from DS3231 memory with timeout using TIM2
 * @parameters :memadd, data, length, timeout
 * @retvalue   :Status of the operation
 *******************************************************************/
int DS3231_Read(uint8_t memadd, uint8_t *data, uint8_t length, uint32_t timeout)
{
    // Enable I2C
    I2C1->CR1 |= I2C_CR1_PE;

    // Set slave address for write operation
    I2C1->CR2 = (DS3231_I2C_ADDRESS << 1); // Set slave address
    I2C1->CR2 &= ~I2C_CR2_ADD10;            // 7-bit addressing
    I2C1->CR2 |= (1 << I2C_CR2_NBYTES_Pos); // Set the number of bytes to transfer to 1
    I2C1->CR2 &= ~I2C_CR2_RD_WRN;           // Set to write mode
    I2C1->CR2 &= ~I2C_CR2_AUTOEND;
    I2C1->CR2 |= I2C_CR2_START;             // Generate start condition

    TIM2_StopTimer();
    TIM2_ResetCounter();
    TIM2_SetTimer(timeout);
    TIM2_StartTimer();

    while (!(I2C1->ISR & I2C_ISR_TC)) // Wait for transfer completion
    {
        if (TIM2_TimeoutFlag()) // Timeout check
        {
            TIM2_ResetTimeoutFlag();
            TIM2_StopTimer();
            return DS3231_TIMEOUT_ERROR;
        }

        if (I2C1->ISR & I2C_ISR_TXE) // If TX buffer is empty, send memory address
        {
            I2C1->TXDR = memadd;
        }
    }

    // Reset I2C and enable for read operation
    I2C1->CR1 &= ~I2C_CR1_PE;
    I2C1->CR1 |= I2C_CR1_PE;
    I2C1->CR2 = (DS3231_I2C_ADDRESS << 1); // Set slave address for read operation
    I2C1->CR2 |= I2C_CR2_RD_WRN;           // Set to read mode
    I2C1->CR2 |= (length << I2C_CR2_NBYTES_Pos); // Set number of bytes to read
    I2C1->CR2 |= I2C_CR2_AUTOEND;          // Auto-generate stop after transfer
    I2C1->CR2 |= I2C_CR2_START;            // Generate start condition

    while (!(I2C1->ISR & I2C_ISR_STOPF)) // Wait for stop condition
    {
        if (TIM2_TimeoutFlag()) // Timeout check
        {
            TIM2_ResetTimeoutFlag();
            TIM2_StopTimer();
            return DS3231_TIMEOUT_ERROR;
        }

        if (I2C1->ISR & I2C_ISR_RXNE) // If RX buffer is not empty, read data
        {
            *data++ = I2C1->RXDR;
        }
    }

    TIM2_StopTimer();
    I2C1->CR1 &= ~I2C_CR1_PE;
    return DS3231_SUCCESS;
}

/*******************************************************************
 * @name       :DS3231_Write
 * @function   :Write data to DS3231 memory with timeout using TIM2
 * @parameters :memadd, data, length, timeout
 * @retvalue   :Status of the operation
 *******************************************************************/
int DS3231_Write(uint8_t memadd, uint8_t *data, uint8_t length, uint32_t timeout)
{
    // Enable I2C1 peripheral
    I2C1->CR1 |= I2C_CR1_PE;

    // Configure I2C for writing data
    I2C1->CR2 = 0;  // Reset control register
    I2C1->CR2 = (DS3231_I2C_ADDRESS << 1);  // Set slave address (shifted left)
    I2C1->CR2 &= ~I2C_CR2_ADD10;  // 7-bit addressing mode
    I2C1->CR2 |= ((length + 1) << I2C_CR2_NBYTES_Pos);  // Set number of bytes to write (include memory address)
    I2C1->CR2 &= ~I2C_CR2_RD_WRN;  // Set to write mode
    I2C1->CR2 |= I2C_CR2_AUTOEND;  // Enable auto-stop
    I2C1->CR2 |= I2C_CR2_START;    // Generate start condition

    // Send memory address and data
    int i = 0;  // Initialize index for data

    TIM2_StopTimer();
    TIM2_ResetCounter();
    TIM2_SetTimer(timeout);
    TIM2_StartTimer();

    while (!(I2C1->ISR & I2C_ISR_STOPF)) // Wait for stop condition
    {
        if (TIM2_TimeoutFlag()) // Timeout check
        {
            TIM2_ResetTimeoutFlag();
            return DS3231_TIMEOUT_ERROR;
        }

        if (I2C1->ISR & I2C_ISR_TXE) // If transmit buffer is empty, send data
        {
            I2C1->TXDR = (i == 0) ? memadd : data[i - 1]; // Send memory address first, then data
            i++;  // Increment index
        }
    }

    // Disable I2C1 after transmission
    TIM2_StopTimer();
    I2C1->CR1 &= ~I2C_CR1_PE;
    return DS3231_SUCCESS;
}
