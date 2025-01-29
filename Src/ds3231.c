#include "ds3231.h"

/*******************************************************************
 * @name       :DS3231_Init
 * @function   :DS3231 Initialization
 * @parameters :None
 * @retvalue   :None
********************************************************************/
void DS3231_Init(void)
{
    // Enable GPIOB clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    // Configure GPIOB Pin 8 as alternate function
    GPIOB->MODER |= GPIO_MODER_MODER8_1; // Set mode to alternate function
    GPIOB->MODER &= ~GPIO_MODER_MODER8_0; // Clear mode bits

    // Configure GPIOB Pin 9 as alternate function
    GPIOB->MODER |= GPIO_MODER_MODER9_1; // Set mode to alternate function
    GPIOB->MODER &= ~GPIO_MODER_MODER9_0; // Clear mode bits

    // Set GPIOB Pin 8 and Pin 9 to open-drain mode
    GPIOB->OTYPER |= GPIO_OTYPER_OT8; // Set Pin 8 to open-drain
    GPIOB->OTYPER |= GPIO_OTYPER_OT9; // Set Pin 9 to open-drain

    // Set alternate function for GPIOB Pin 8 and Pin 9 to I2C1
    GPIOB->AFR[1] |= DS3231_I2C1_AF << GPIO_AFRH_AFRH0_Pos;
    GPIOB->AFR[1] |= DS3231_I2C1_AF << GPIO_AFRH_AFRH1_Pos;

    // Enable I2C1 clock
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    // Disable I2C1 before configuring it
    I2C1->CR1 &= ~I2C_CR1_PE;

    // Set I2C timing register (standard or fast mode)
    I2C1->TIMINGR = 0x0000C1C1; // Configure timing for I2C
}

/*******************************************************************
 * @name       :DS3231_BCD_DEC
 * @function   :Convert BCD to decimal
 * @parameters :None
 * @retvalue   :Converted value
********************************************************************/
int DS3231_BCD_DEC(unsigned char x)
{
    return x - 6 * (x >> 4);
}

/*******************************************************************
 * @name       :DS3231_DEC_BCD
 * @function   :Convert decimal to BCD
 * @parameters :None
 * @retvalue   :Converted value
 *******************************************************************/
int DS3231_DEC_BCD(unsigned char x)
{
    return x + 6 * (x / 10);
}

/*******************************************************************
 * @name       :DS3231_Read
 * @function   :Read data to DS3231 memory with timeout using TIM1
 * @parameters :None
 * @retvalue   :Converted value
 *******************************************************************/
void DS3231_Read(uint8_t memadd, uint8_t *data, uint8_t length)
{
    // Enable I2C
    I2C1->CR1 |= I2C_CR1_PE;

    // Set slave address for write operation
    I2C1->CR2 = (DS3231_I2C_ADRESS << 1); // Set slave address
    I2C1->CR2 &= ~I2C_CR2_ADD10; // 7-bit addressing
    I2C1->CR2 |= (1 << I2C_CR2_NBYTES_Pos); // Set number to transfer to 1 for write operation
    I2C1->CR2 &= ~I2C_CR2_RD_WRN; // Set the mode to write mode
    I2C1->CR2 &= ~I2C_CR2_AUTOEND; // Software end
    I2C1->CR2 |= I2C_CR2_START; // Generate start

    while (!(I2C1->ISR & I2C_ISR_TC)) // Wait until transfer complete
    {

        // If TX buffer is empty, send the memory address
        if (I2C1->ISR & I2C_ISR_TXE)
        {
            I2C1->TXDR = memadd;
        }
    }

    // Reset I2C and enable for read operation
    I2C1->CR1 &= ~I2C_CR1_PE; // Reset I2C
    I2C1->CR1 |= I2C_CR1_PE; // Enable I2C
    I2C1->CR2 = (DS3231_I2C_ADRESS << 1); // Set slave address
    I2C1->CR2 |= I2C_CR2_RD_WRN; // Set mode to read operation
    I2C1->CR2 |= (length << I2C_CR2_NBYTES_Pos); // Set length to the required length
    I2C1->CR2 |= I2C_CR2_AUTOEND; // Auto-generate stop after transfer is completed
    I2C1->CR2 |= I2C_CR2_START; // Generate start

    while (!(I2C1->ISR & I2C_ISR_STOPF))
    {
        // If RX buffer is not empty
        if (I2C1->ISR & I2C_ISR_RXNE)
        {
            *data++ = I2C1->RXDR; // Read the data and increment the pointer
        }
    }

    // Disable the peripheral
    I2C1->CR1 &= ~I2C_CR1_PE;
}

/*******************************************************************
 * @name       :DS3231_Write
 * @function   :Write data to DS3231 memory with timeout using TIM1
 * @parameters :memadd, data, length, timeout
 * @retvalue   :None
 *******************************************************************/
void DS3231_Write(uint8_t memadd, uint8_t *data, uint8_t length)
{
    // Enable I2C1 peripheral
    I2C1->CR1 |= I2C_CR1_PE;

    // Configure I2C for writing data
    I2C1->CR2 = 0;  // Reset control register
    I2C1->CR2 = (DS3231_I2C_ADRESS << 1);  // Set slave address (shifted left)
    I2C1->CR2 &= ~I2C_CR2_ADD10;  // 7-bit addressing mode
    I2C1->CR2 |= ((length + 1) << I2C_CR2_NBYTES_Pos);  // Set number of bytes to write (include memory address)
    I2C1->CR2 &= ~I2C_CR2_RD_WRN;  // Set to write mode
    I2C1->CR2 |= I2C_CR2_AUTOEND;  // Enable auto-stop
    I2C1->CR2 |= I2C_CR2_START;  // Generate start condition

    // Send memory address and data
    int i = 0;  // Initialize index for data

    // Wait for stop condition or timeout
    while (!(I2C1->ISR & I2C_ISR_STOPF))
    {
        // If transmit buffer is empty, send memory address or data
        if (I2C1->ISR & I2C_ISR_TXE)
        {
            I2C1->TXDR = (i == 0) ? memadd : data[i - 1]; // Send memory address first, then data
            i++;  // Increment index
        }
    }

    // Disable I2C1 after transmission
    I2C1->CR1 &= ~I2C_CR1_PE;
}
