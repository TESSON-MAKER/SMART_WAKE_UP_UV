#include "../Inc/sh1106.h"
#include "../Inc/tim.h"

#include <stdarg.h>
#include <stdio.h>

static uint8_t SH1106_Buffer[(SH1106_WIDTH*SH1106_HEIGHT)/SH1106_DATA_SIZE];

static void SH1106_GPIO_Init(void);
static void SH1106_SPI1_Init(void);
static void SH1106_Screen_Init(void);
static void SH1106_SpiTransmit(uint8_t msg);
static void SH1106_SendCmd(uint8_t cmd);
static void SH1106_SendDoubleCmd(uint8_t cmd1, uint8_t cmd2);
static void SH1106_SendData(uint8_t data);
static void SH1106_Reset(void);

/*******************************************************************
 * @name       : SH1106_GPIO_Init
 * @brief      : Initializes GPIO for SH1106 display
 * @details    : Configures GPIO pins for DC, CS, RST, SCK, and MOSI
 * @parameters : None
 * @return     : None
 *******************************************************************/
static void SH1106_GPIO_Init(void)
{
	// Enable clock for GPIOA and GPIOC
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;  // Enable clock for GPIOA
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;  // Enable clock for GPIOC

	// PA0-DC (GPIOA pin 0)
	GPIOA->MODER |= GPIO_MODER_MODER0_0;  // Configure as output mode
	GPIOA->MODER &= ~GPIO_MODER_MODER0_1; // Reset bits to configure as output

	// PC0-CS (GPIOC pin 0)
	GPIOC->MODER |= GPIO_MODER_MODER0_0;  // Configure as output mode
	GPIOC->MODER &= ~GPIO_MODER_MODER0_1; // Reset bits to configure as output

	// PC1-RST (GPIOC pin 1)
	GPIOC->MODER |= GPIO_MODER_MODER1_0;  // Configure as output mode
	GPIOC->MODER &= ~GPIO_MODER_MODER1_1; // Reset bits to configure as output

	// PA5-SCK (GPIOA pin 5)
	GPIOA->MODER |= GPIO_MODER_MODER5_1;  // Configure as alternate function (SCK)
	GPIOA->MODER &= ~GPIO_MODER_MODER5_0; // Reset bits to configure as alternate function
	GPIOA->AFR[0] |= SH1106_SPI1_AF << GPIO_AFRL_AFRL5_Pos; // Configure PA5 for SPI1

	// PA7-MOSI (GPIOA pin 7)
	GPIOA->MODER |= GPIO_MODER_MODER7_1;  // Configure as alternate function (MOSI)
	GPIOA->MODER &= ~GPIO_MODER_MODER7_0; // Reset bits to configure as alternate function
	GPIOA->AFR[0] |= SH1106_SPI1_AF << GPIO_AFRL_AFRL7_Pos; // Configure PA7 for SPI1
}

/*******************************************************************
 * @name       : SH1106_SPI1_Init
 * @brief      : Initializes SPI1 for SH1106 display
 * @details    : Configures SPI1 in master mode with specific settings
 * @parameters : None
 * @return     : None
 *******************************************************************/
static void SH1106_SPI1_Init(void)
{
	// Enable clock for the SPI1 module
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

	// SPI1 configuration
	// Set MSB first
	SPI1->CR1 &= ~SPI_CR1_LSBFIRST;

	// Configure SPI as MASTER
	SPI1->CR1 |= SPI_CR1_MSTR;

	// Select software slave management (SSM=1, SSI=1)
	SPI1->CR1 |= SPI_CR1_SSM;
	SPI1->CR1 |= SPI_CR1_SSI;

	// Configure SPI mode (MODE1: CPHA=0, CPOL=0)
	SPI1->CR1 &= ~SPI_CR1_CPHA;
	SPI1->CR1 &= ~SPI_CR1_CPOL;

	// Set SPI frequency to 500 kHz
	SPI1->CR1 |= SPI_CR1_BR_2;

	// Enable the SPI module
	SPI1->CR1 |= SPI_CR1_SPE;
}

/*******************************************************************
 * @name       : SH1106_Screen_Init
 * @brief      : Initializes the SH1106 display screen
 * @details    : Sends a series of commands to configure the display settings
 * @parameters : None
 * @return     : None
 *******************************************************************/
static void SH1106_Screen_Init(void)
{
	// Reset
	SH1106_Reset();
	// Display OFF
	SH1106_SendCmd(SH1106_CMD_DISP_OFF);
	// Set multiplex ratio (visible lines)
	SH1106_SendDoubleCmd(SH1106_CMD_SETMUX, 0x3F);
	// Set display offset (offset of first line from the top of display)
	SH1106_SendDoubleCmd(SH1106_CMD_SETOFFS, 0x00);
	// Set display start line (first line displayed)
	SH1106_SendCmd(SH1106_CMD_STARTLINE);
	// Set segment re-map (X coordinate)
	SH1106_SendCmd(SH1106_CMD_SEG_INV);
	// Set COM output scan direction (Y coordinate)
	SH1106_SendCmd(SH1106_CMD_COM_INV);
	// Set COM pins hardware configuration
	SH1106_SendDoubleCmd(SH1106_CMD_COM_HW, 0x12);
	// Set contrast control
	SH1106_SendDoubleCmd(SH1106_CMD_CONTRAST, 0xFF); // Contrast: middle level
	// Disable entire display ON
	SH1106_SendCmd(SH1106_CMD_EDOFF);
	// Disable display inversion
	SH1106_SendCmd(SH1106_CMD_INV_OFF);
	// Set clock divide ratio and oscillator frequency
	SH1106_SendDoubleCmd(SH1106_CMD_CLOCKDIV, 0x80);
	// Display ON
	SH1106_SendCmd(SH1106_CMD_DISP_ON);
}

/*******************************************************************
 * @name       : SH1106_Init
 * @brief      : Initializes the SH1106 display
 * @details    : Calls GPIO, SPI, and screen initialization functions
 * @parameters : None
 * @return     : None
 *******************************************************************/
void SH1106_Init(void)
{
	SH1106_GPIO_Init();
	SH1106_SPI1_Init();
	TIM1_WaitMilliseconds(200);
	SH1106_Screen_Init();
}

/*******************************************************************
 * @name       : SH1106_SpiTransmit
 * @brief      : Transmits data via SPI
 * @details    : Sends a byte of data through the SPI interface
 * @parameters : msg - Data byte to transmit
 * @return     : None
 *******************************************************************/
static void SH1106_SpiTransmit(uint8_t msg)
{
	while (!(SPI1->SR & SPI_SR_TXE));    // Wait until TXE is set or timeout occurs
	*(volatile uint8_t*) & SPI1->DR = msg; // Write the data to the data register
	while (!(SPI1->SR & SPI_SR_TXE));    // Wait until TXE is set or timeout occurs
	while ((SPI1->SR & SPI_SR_BSY));     // Wait for BUSY flag to reset or timeout occurs

	// Clear OVR flag (Overrun flag) by reading DR and SR registers
	(void)SPI1->DR;
	(void)SPI1->SR;
}

/*******************************************************************
 * @name       : SH1106_SendCmd
 * @brief      : Sends a command to the SH1106 display
 * @details    : Transmits a command byte to the display
 * @parameters : cmd - Command byte to send
 * @return     : None
 *******************************************************************/
static void SH1106_SendCmd(uint8_t cmd)
{
	SH1106_DC_LOW; // Command mode
	SH1106_CS_LOW;
	SH1106_SpiTransmit(cmd);
	SH1106_CS_HIGH;
}

/*******************************************************************
 * @name       : SH1106_SendDoubleCmd
 * @brief      : Sends two commands to the SH1106 display
 * @details    : Transmits two command bytes to the display
 * @parameters : cmd1 - First command byte
 *               cmd2 - Second command byte
 * @return     : None
 *******************************************************************/
static void SH1106_SendDoubleCmd(uint8_t cmd1, uint8_t cmd2)
{
	SH1106_SendCmd(cmd1);
	SH1106_SendCmd(cmd2);
}

/*******************************************************************
 * @name       : SH1106_SendData
 * @brief      : Sends data to the SH1106 display
 * @details    : Transmits a data byte to the display
 * @parameters : data - Data byte to send
 * @return     : None
 *******************************************************************/
static void SH1106_SendData(uint8_t data)
{
	SH1106_DC_HIGH; // Data mode
	SH1106_CS_LOW;
	SH1106_SpiTransmit(data);
	SH1106_CS_HIGH;
}

/*******************************************************************
 * @name       : SH1106_SendBuffer
 * @brief      : Sends the display buffer to the SH1106 display
 * @details    : Transmits the entire buffer to the display
 * @parameters : None
 * @return     : None
 *******************************************************************/
void SH1106_SendBuffer(void)
{
	for(int i=0; i<SH1106_DATA_SIZE; i++)
	{
		SH1106_SendCmd(YLevel+i);
		SH1106_SendCmd(XLevelL);
		SH1106_SendCmd(XLevelH);
		for(int n=0; n<SH1106_WIDTH; n++)
		{
			SH1106_SendData(SH1106_Buffer[i*SH1106_WIDTH+n]);
		}
	}
}

/*******************************************************************
 * @name       : SH1106_Reset
 * @brief      : Resets the SH1106 display
 * @details    : Performs a hardware reset of the display
 * @parameters : None
 * @return     : None
 *******************************************************************/
static void SH1106_Reset(void)
{
	SH1106_RST_HIGH;
	TIM1_WaitMilliseconds(100);
	SH1106_RST_LOW;
	TIM1_WaitMilliseconds(100);
	SH1106_RST_HIGH;
}

/*******************************************************************
 * @name       : SH1106_SetPixel
 * @brief      : Sets a pixel in the display buffer
 * @details    : Sets or clears a pixel at the specified coordinates
 * @parameters : color - Pixel color (1 = on, 0 = off)
 *               x - Horizontal position (in pixels)
 *               y - Vertical position (in pixels)
 * @return     : None
 *******************************************************************/
void SH1106_SetPixel(uint8_t color, int16_t x, int16_t y)
{
	if (x >= SH1106_WIDTH || y >= SH1106_HEIGHT || x < 0 || y < 0) return;

	uint16_t index = (y / SH1106_DATA_SIZE) * SH1106_WIDTH + x;
	uint8_t bitOffset = y % SH1106_DATA_SIZE;

	if (color) SH1106_Buffer[index] |= (1 << bitOffset);
	else SH1106_Buffer[index] &= ~(1 << bitOffset);
}

/*******************************************************************
 * @name       : SH1106_DrawCharacter
 * @brief      : Draws a character in the buffer
 * @details    : Draws a character at the specified coordinates using the specified font
 * @parameters : color - Text color (1 = on, 0 = off)
 *               x - Starting horizontal position (in pixels)
 *               y - Starting vertical position (in pixels)
 *               font - Font used to draw the text
 *               letterNumberAscii - ASCII value of the character to draw
 * @return     : None
 *******************************************************************/
void SH1106_DrawCharacter(uint8_t color, int16_t x, int16_t y, const Font *font, uint8_t letterNumberAscii)
{
	if (letterNumberAscii < font->asciiBegin || letterNumberAscii > font->asciiEnd) return;

	uint8_t letterNumber = letterNumberAscii - font->asciiOffset;
	uint16_t index_letterSize = letterNumber * font->datasize;
	uint8_t letterSize = font->data[index_letterSize];

	for (int column = 0; column < letterSize; column++)
	{
		for (int byteColumn = 0; byteColumn < font->bytesPerColums; byteColumn++)
		{
			uint16_t index_Buffer = 1 + letterNumber * font->datasize + byteColumn + font->bytesPerColums * column;
			uint8_t data = font->data[index_Buffer];
			for (int bit = 0; bit < 8; bit++)
			{
				uint8_t pixel = (data >> bit) & 1;
				int16_t a = x + column;
				int16_t b = y + (bit + 8 * byteColumn);
				if (pixel) SH1106_SetPixel(color, a, b);
			}
		}
	}
}

/*******************************************************************
 * @name       : SH1106_DrawStr
 * @brief      : Displays a string on the OLED screen
 * @details    : Draws a string starting from the specified 
 *               coordinates using the specified font
 * @parameters : color - Text color (1 = on, 0 = off)
 *               x - Starting horizontal position (in pixels)
 *               y - Starting vertical position (in pixels)
 *               font - Font used to draw the text
 *               format - String to be displayed
 * @return     : None
 *******************************************************************/
void SH1106_DrawStr(uint8_t color, int16_t x, int16_t y, const Font *font, const char *format)
{
	while (*format && x < SH1106_WIDTH && y < SH1106_HEIGHT)
	{
		uint8_t currentChar = *format;

		SH1106_DrawCharacter(color, x, y, font, currentChar);

		// Create a space between the letters
		uint8_t letterNumber = currentChar - font->asciiOffset;
		uint16_t index_letterSize = letterNumber * font->datasize;
		uint8_t letterSize = font->data[index_letterSize];
		x += letterSize + (font->length / 10);

		// Go to next letter
		format++;
	}
}

/*******************************************************************
 * @name       : SH1106_FontPrint
 * @brief      : Prints a formatted string on the OLED screen
 * @details    : Draws a formatted string starting from the specified 
 *               coordinates using the specified font
 * @parameters : color - Text color (1 = on, 0 = off)
 *               x - Starting horizontal position (in pixels)
 *               y - Starting vertical position (in pixels)
 *               font - Font used to draw the text
 *               format - Formatted string to be displayed
 * @return     : None
 *******************************************************************/
void SH1106_FontPrint(uint8_t color, int16_t x, int16_t y, const Font *font, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	char formatted_string[50];
	vsprintf(formatted_string, format, args);
	va_end(args);

	SH1106_DrawStr(color, x, y, font, formatted_string);
}

/*******************************************************************
 * @name       : SH1106_DrawLine
 * @brief      : Draws a line on the OLED screen
 * @details    : Draws a line from (x0, y0) to (x1, y1)
 * @parameters : color - Line color (1 = on, 0 = off)
 *               x0 - Starting horizontal position (in pixels)
 *               y0 - Starting vertical position (in pixels)
 *               x1 - Ending horizontal position (in pixels)
 *               y1 - Ending vertical position (in pixels)
 * @return     : None
 *******************************************************************/
void SH1106_DrawLine(uint8_t color, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
	int dx = (x1 >= x0) ? x1 - x0 : x0 - x1;
	int dy = (y1 >= y0) ? y1 - y0 : y0 - y1;
	int sx = (x0 < x1) ? 1 : -1;
	int sy = (y0 < y1) ? 1 : -1;
	int err = dx - dy;

	while(1)
	{
		SH1106_SetPixel(color, x0, y0);
		if (x0 == x1 && y0 == y1) break;
		int e2 = err + err;
		if (e2 > -dy)
		{
			err -= dy;
			x0 += sx;
		}
		if (e2 < dx)
		{
			err += dx;
			y0 += sy;
		}
	}
}

/*******************************************************************
 * @name       : SH1106_DrawRectangle
 * @brief      : Draws a rectangle on the OLED screen
 * @details    : Draws a rectangle with the specified width and 
 *               height starting from the specified coordinates
 * @parameters : color - Rectangle color (1 = on, 0 = off)
 *               x - Starting horizontal position (in pixels)
 *               y - Starting vertical position (in pixels)
 *               w - Width of the rectangle (in pixels)
 *               h - Height of the rectangle (in pixels)
 * @return     : None
 *******************************************************************/
void SH1106_DrawRectangle(uint8_t color, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	// Check input parameters
	if (x >= SH1106_WIDTH || y >= SH1106_HEIGHT) return;

	// Check width and height
	if ((x + w) >= SH1106_WIDTH) w = SH1106_WIDTH - x;
	if ((y + h) >= SH1106_HEIGHT) h = SH1106_HEIGHT - y;

	// Draw 4 lines
	SH1106_DrawLine(color, x, y, x + w, y);         // Top line
	SH1106_DrawLine(color, x, y + h, x + w, y + h); // Bottom line
	SH1106_DrawLine(color, x, y, x, y + h);         // Left line
	SH1106_DrawLine(color, x + w, y, x + w, y + h); // Right line
}

/*******************************************************************
 * @name       : SH1106_DrawFilledRectangle
 * @brief      : Draws a filled rectangle on the OLED screen
 * @details    : Draws a filled rectangle with the specified 
 *               width and height starting from the specified 
 *               coordinates
 * @parameters : color - Rectangle color (1 = on, 0 = off)
 *               x - Starting horizontal position (in pixels)
 *               y - Starting vertical position (in pixels)
 *               w - Width of the rectangle (in pixels)
 *               h - Height of the rectangle (in pixels)
 * @return     : None
 *******************************************************************/
void SH1106_DrawFilledRectangle(uint8_t color, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	// Check input parameters
	if (x >= SH1106_WIDTH || y >= SH1106_HEIGHT) return;

	// Check width and height
	if ((x + w) >= SH1106_WIDTH) w = SH1106_WIDTH - x;
	if ((y + h) >= SH1106_HEIGHT) h = SH1106_HEIGHT - y;

	// Draw lines
	for (int i = 0; i <= h; i++)
		SH1106_DrawLine(color, x, y + i, x + w, y + i);
}

/*******************************************************************
 * @name       : SH1106_DrawCircle
 * @brief      : Draws a circle on the OLED screen
 * @details    : Draws a circle with the specified radius centered 
 *               at (x0, y0)
 * @parameters : color - Circle color (1 = on, 0 = off)
 *               x0 - Horizontal center position (in pixels)
 *               y0 - Vertical center position (in pixels)
 *               radius - Radius of the circle (in pixels)
 * @return     : None
 *******************************************************************/
void SH1106_DrawCircle(uint8_t color, uint8_t x0, uint8_t y0, uint8_t radius)
{
    int x = radius;
    int y = 0;
    int err = 0;

    SH1106_SetPixel(color, x0, y0 + radius);
    SH1106_SetPixel(color, x0, y0 - radius);
    SH1106_SetPixel(color, x0 + radius, y0);
    SH1106_SetPixel(color, x0 - radius, y0);

    while (x >= y)
    {
        SH1106_SetPixel(color, x0 + x, y0 + y);
        SH1106_SetPixel(color, x0 - x, y0 + y);
        SH1106_SetPixel(color, x0 + x, y0 - y);
        SH1106_SetPixel(color, x0 - x, y0 - y);
        SH1106_SetPixel(color, x0 + y, y0 + x);
        SH1106_SetPixel(color, x0 - y, y0 + x);
        SH1106_SetPixel(color, x0 + y, y0 - x);
        SH1106_SetPixel(color, x0 - y, y0 - x);

        y++;
        err += 1 + 2*y;

        if (2*(err - x) + 1 > 0)
        {
            x--;
            err += 1 - 2*x;
        }
    }
}

/*******************************************************************
 * @name       : SH1106_DrawFilledCircle
 * @brief      : Draws a filled circle on the OLED screen
 * @details    : Draws a filled circle with the specified radius 
 *               centered at (x0, y0)
 * @parameters : color - Circle color (1 = on, 0 = off)
 *               x0 - Horizontal center position (in pixels)
 *               y0 - Vertical center position (in pixels)
 *               r - Radius of the circle (in pixels)
 * @return     : None
 *******************************************************************/
void SH1106_DrawFilledCircle(uint8_t color, int16_t x0, int16_t y0, int16_t r)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	SH1106_SetPixel(color, x0, y0 + r);
	SH1106_SetPixel(color, x0, y0 - r);
	SH1106_SetPixel(color, x0 + r, y0);
	SH1106_SetPixel(color, x0 - r, y0);
	SH1106_DrawLine(color, x0 - r, y0, x0 + r, y0);

	while (x < y)
	{
		if (f >= 0)
		{
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		SH1106_DrawLine(color, x0 - x, y0 + y, x0 + x, y0 + y);
		SH1106_DrawLine(color, x0 - x, y0 - y, x0 + x, y0 - y);

		SH1106_DrawLine(color, x0 - y, y0 + x, x0 + y, y0 + x);
		SH1106_DrawLine(color, x0 - y, y0 - x, x0 + y, y0 - x);
	}
}

/*******************************************************************
 * @name       : SH1106_ClearBuffer
 * @brief      : Clears the display buffer
 * @details    : Sets all pixels in the buffer to off
 * @parameters : None
 * @return     : None
 *******************************************************************/
void SH1106_ClearBuffer(void)
{
	uint16_t bufferSize = (SH1106_WIDTH*SH1106_HEIGHT)/SH1106_DATA_SIZE;
	for (int i=0; i<bufferSize; i++)
		SH1106_Buffer[i] = 0;
}
