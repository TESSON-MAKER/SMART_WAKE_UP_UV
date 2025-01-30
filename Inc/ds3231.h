#ifndef DS3231_H_
#define DS3231_H_

#include <stm32f7xx.h>

#define DS3231_I2C1_AF 0x04
#define DS3231_I2C_ADRESS 0x68

#define DS3231_TIMEOUT_ERROR 1
#define DS3231_SUCCESS 0

void DS3231_Init(void);
int DS3231_BcdToDec(unsigned char x);
int DS3231_DecToBcd(unsigned char x);
int DS3231_Read(uint8_t memadd, uint8_t *data, uint8_t length, uint32_t timeout);
int DS3231_Write(uint8_t memadd, uint8_t *data, uint8_t length, uint32_t timeout);


#endif /* DS3231_H_ */
