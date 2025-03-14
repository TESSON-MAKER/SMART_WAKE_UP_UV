#include "../Inc/sh1106.h"
#include "../Inc/tim.h"
#include "../Inc/buttons.h"
#include "../Inc/ds3231.h"
#include "../Inc/gpio.h"
#include "../Inc/urm37.h"
#include "../Inc/usart.h"
#include "../Inc/esp01.h"

const char *days[] = {"NA", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"}; 
const char *months[] = {"NA", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

static int8_t DS3231_Second = 0;
static int8_t DS3231_Minute = 0;
static int8_t DS3231_Hour = 0;
static int8_t DS3231_DayWeek = 1;
static int8_t DS3231_DayMonth = 1;
static int8_t DS3231_Month = 1;
static int8_t DS3231_Year = 0;
static int8_t DS3231_Century = 0;

static uint8_t UpdateToDisplay = 0;
static uint8_t UpdateToSetting = 0;

int move = 0;
static uint8_t state = 0;

static void MAIN_DisplayDate(void);
static void MAIN_Settings(void);

int main(void) 
{
	TIM1_InitForDelay();
	TIM2_Init();
	SH1106_Init();
	SH1106_ClearBuffer();
	USART_Serial_Begin(9600); 
	BUTTONS_Init();
	DS3231_Init();
	URM37_Init();
	ESP01_Init();	
	
	ESP01_Transmit_DMA("AT+CWMODE?\r\n");
	TIM1_WaitMilliseconds(2000);
	ESP01_Transmit_DMA("AT+CWMODE=1\r\n");
	TIM1_WaitMilliseconds(1000);
	
	GPIO_PinMode(GPIOB, 7, OUTPUT);
	GPIO_PinMode(GPIOB, 14, OUTPUT);
	
	uint8_t dataI[7] = {
			DS3231_DecToBcd(0),
			DS3231_DecToBcd(55),
			DS3231_DecToBcd(21),
			DS3231_DecToBcd(6),
			DS3231_DecToBcd(10),
			DS3231_DecToBcd(3),
			DS3231_DecToBcd(1)};

	if (DS3231_Write(0x00, dataI, 7, 40000))
	{
		while(1);
	}

	while (1) 
	{
		SH1106_ClearBuffer();
		BUTTONS_KeyState();
		GPIO_DigitalWrite(GPIOB, 7, state);	
		GPIO_DigitalWrite(GPIOB, 14, !state);	
		TIM1_WaitMilliseconds(50);
		
		switch (BUTTON_Switch)
		{
			case 0:
				MAIN_DisplayDate();
				break;
			case 1:
				MAIN_Settings();
				break;
		}
		state ^= 1;

		SH1106_SendBuffer();
	}
}

static void MAIN_DisplayDate(void)
{
	UpdateToSetting = 1;
	
	if (UpdateToDisplay)
	{
		uint8_t dataS[7] = {DS3231_DecToBcd(DS3231_Second), DS3231_DecToBcd(DS3231_Minute), DS3231_DecToBcd(DS3231_Hour), DS3231_DecToBcd(DS3231_DayWeek), DS3231_DecToBcd(DS3231_DayMonth), DS3231_DecToBcd(DS3231_Month), DS3231_DecToBcd(DS3231_Year)};
		
		if (DS3231_Write(0x00, dataS, 7, 40000))
		{
			while(1);
		}
		
		BUTTON_TopState = 0;
		BUTTON_BottomState = 0;
		BUTTON_RightState = 0;
		BUTTON_LeftState = 0;
		
		move = 0;
		UpdateToDisplay = 0;
	}
	
	uint8_t data[7] = {0};
	uint8_t data_temp[2] = {0};
	if (DS3231_Read(0x0,data,7, 3000)
	 || DS3231_Read(0x11, data_temp, 2, 1500))
	{
		SH1106_FontPrint(1, 7, 13, &Arial12x12, "E:DS3231");
	}
	else 
	{
		DS3231_Second = DS3231_BcdToDec(data[0] & 0x7F);
		DS3231_Minute = DS3231_BcdToDec(data[1]);
		DS3231_Hour = DS3231_BcdToDec(data[2] & 0x3F);
		DS3231_DayWeek = DS3231_BcdToDec(data[3]);
		DS3231_DayMonth = DS3231_BcdToDec(data[4]);
		DS3231_Month = DS3231_BcdToDec(data[5]);
		DS3231_Year = DS3231_BcdToDec(data[6]);
		DS3231_Century = DS3231_BcdToDec(data[5] & 0x80);
		
		uint8_t temp_msb = data_temp[0]; // MSB à l'adresse 0x11
		uint8_t temp_lsb = data_temp[1]; // LSB à l'adresse 0x12

		// Calcul de la température
		int16_t value = (temp_msb << 8) | temp_lsb;
		value = (value >> 6);

		float temperature = value / 4.0f;
		
		SH1106_FontPrint(1, 0, 0, &Arial12x12, "Temp: %.2f", temperature);
		SH1106_FontPrint(1, 7, 13, &Arial28x28, "%02d:%02d:%02d", DS3231_Hour, DS3231_Minute, DS3231_Second);
		//USART_Serial_Print("%02d:%02d:%02d\r\n", DS3231_Hour, DS3231_Minute, DS3231_Second);
		SH1106_FontPrint(1, 0, 39, &Arial12x12, "%s,", days[DS3231_DayWeek]);
		SH1106_FontPrint(1, 0, 52, &Arial12x12, "%s %d, 2%d%02d", months[DS3231_Month], DS3231_DayMonth, DS3231_Century, DS3231_Year);
		SH1106_DrawLine(1, 0, 37, 131, 37);
		SH1106_DrawLine(1, 0, 12, 131, 12);
	}
}

static void handling(int8_t* data, const char* title, int max, int min)
{
	if (BUTTON_TopState) 
	{
		(*data)++;
		BUTTON_TopState = 0;
	}
	if (BUTTON_BottomState) 
	{
		(*data)--;
		BUTTON_BottomState = 0;
	}

	if (*data > max) *data = min;
	if (*data < min) *data = max;

	SH1106_FontPrint(1, 0, 13, &Arial12x12, "Setting %s : %d", title, *data);
}

static void handlingDay()
{
	uint8_t isLeapYear = (DS3231_Year %4 == 0 && DS3231_Year %100 != 0) || (DS3231_Year %400 == 0);

	if (BUTTON_TopState) 
	{
		DS3231_DayMonth++;
		BUTTON_TopState = 0;
	}
	if (BUTTON_BottomState) 
	{
		DS3231_DayMonth--;
		BUTTON_BottomState = 0;
	}

	if ((DS3231_Month == 4 || DS3231_Month == 6 || DS3231_Month == 9 || DS3231_Month == 11) && (DS3231_DayMonth > 30)) DS3231_DayMonth=0;
	if ((DS3231_Month == 4 || DS3231_Month == 6 || DS3231_Month == 9 || DS3231_Month == 11) && (DS3231_DayMonth < 0)) DS3231_DayMonth=30;
	if ((DS3231_Month == 1 || DS3231_Month == 3 || DS3231_Month == 5 || DS3231_Month == 7 || DS3231_Month == 9 || DS3231_Month == 11) && (DS3231_DayMonth > 31)) DS3231_DayMonth=0;
	if ((DS3231_Month == 1 || DS3231_Month == 3 || DS3231_Month == 5 || DS3231_Month == 7 || DS3231_Month == 9 || DS3231_Month == 11) && (DS3231_DayMonth < 0)) DS3231_DayMonth=31;
	if ((DS3231_Month == 2) && (DS3231_DayMonth > 28)) DS3231_DayMonth=0;
	if ((DS3231_Month == 2) && (DS3231_DayMonth < 0)) DS3231_DayMonth=28;
	if (DS3231_Month == 2 && isLeapYear && DS3231_DayMonth > 29) DS3231_DayMonth = 0;
	if (DS3231_Month == 2 && !isLeapYear && DS3231_DayMonth > 28) DS3231_DayMonth = 0;
	if (DS3231_Month == 2 && isLeapYear && DS3231_DayMonth < 0) DS3231_DayMonth = 29;
	if (DS3231_Month == 2 && !isLeapYear && DS3231_DayMonth < 0) DS3231_DayMonth = 28;

	SH1106_FontPrint(1, 0, 13, &Arial12x12, "Setting day : %d", DS3231_DayMonth);
}

static void handlingMonth()
{
	uint8_t isLeapYear = (DS3231_Year %4 == 0 && DS3231_Year %100 != 0) || (DS3231_Year %400 == 0);

	if (BUTTON_TopState) 
	{
		DS3231_Month++;
		BUTTON_TopState = 0;
	}
	if (BUTTON_BottomState) 
	{
		DS3231_Month--;
		BUTTON_BottomState = 0;
	}

	if (DS3231_Month>12) DS3231_Month=1;
	if (DS3231_Month<1) DS3231_Month=12;

	if ((DS3231_Month == 4 || DS3231_Month == 6 || DS3231_Month == 9 || DS3231_Month == 11) && (DS3231_DayMonth > 30)) DS3231_DayMonth=30;  //Cas ou dans 1 mois, il n'y a que 30 jours

	if (DS3231_Month == 2 && isLeapYear && DS3231_DayMonth > 29) DS3231_DayMonth = 29;                                 //Cas de Fevrier dans les annees bissextiles (29 jours)
	if (DS3231_Month == 2 && !isLeapYear && DS3231_DayMonth > 28) DS3231_DayMonth = 28;                                //Cas de Fevrier hors annees bissextiles (28 jours)

	SH1106_FontPrint(1, 0, 13, &Arial12x12, "Setting month : %d", DS3231_Month);
}

static void handlingYear()
{
	uint8_t isLeapYear = (DS3231_Year %4 == 0 && DS3231_Year %100 != 0) || (DS3231_Year %400 == 0);

	if (BUTTON_TopState) 
	{
		DS3231_Year++;
		BUTTON_TopState = 0;
	}
	if (BUTTON_BottomState) 
	{
		DS3231_Year--;
		BUTTON_BottomState = 0;
	}

	if (DS3231_Year>99) DS3231_Year=0;
	if (DS3231_Year<0) DS3231_Year=99;

	if (DS3231_Month == 2 && isLeapYear && DS3231_DayMonth > 29) DS3231_DayMonth = 29;           // Cas de Fevrier dans les annees bissextiles (29 jours)
	if (DS3231_Month == 2 && !isLeapYear && DS3231_DayMonth > 28) DS3231_DayMonth = 28;          // Cas de Fevrier hors annees bissextiles (28 jours)

	SH1106_FontPrint(1, 0, 13, &Arial12x12, "Setting year : %d", DS3231_Year);
}

static void MAIN_Settings(void)
{
	//keyboard();
	UpdateToDisplay = 1;
	
	if (UpdateToSetting)
	{
		BUTTON_TopState = 0;
		BUTTON_BottomState = 0;
		BUTTON_RightState = 0;
		BUTTON_LeftState = 0;
		
		UpdateToSetting = 0;
	}

	if (BUTTON_RightState) 
	{
		move++;
		BUTTON_RightState = 0;
	}
	if (BUTTON_LeftState) 
	{
		move--;
		BUTTON_LeftState = 0;
	}
  
	if (move > 6) move = 0;
	if (move < 0) move = 6;
  
	switch (move)
	{
		case 0:
			handling(&DS3231_Second, "sec", 59, 0);
			break;
		case 1:
			handling(&DS3231_Minute, "min", 59, 0);
			break;
		case 2:
			handling(&DS3231_Hour, "hour", 23, 0);
			break;
		case 3:
			handling(&DS3231_DayWeek, "dayW", 7, 1);
			break;
		case 4:
			handlingDay();
			break;
		case 5:
			handlingMonth();
			break;
		case 6:
			handlingYear();
			break;
	}
}
