#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Inc/esp01_buffer.h"

/************************************************************************************
 * @name       : BUFFER_Reset
 * @function   : Resets the circular buffer by setting the read and write indices to 0.
 * @parameters : CircularBufferTypeDef* buff - Pointer to the buffer structure.
 * @retvalue   : None
 ************************************************************************************/
void BUFFER_Reset(CircularBufferTypeDef* buff)
{
	// Reset indices and buffer size
	buff->readIndex = 0;
	buff->writeIndex = 0;
	buff->size = 0;
}

/************************************************************************************
 * @name       : BUFFER_GetLength
 * @function   : Calculates the current length of data in the circular buffer.
 * @parameters : CircularBufferTypeDef* buff - Pointer to the buffer structure.
 * @retvalue   : uint16_t - The current number of elements in the buffer.
 ************************************************************************************/
uint16_t BUFFER_GetLength(CircularBufferTypeDef* buff)
{
	return buff->size;
}

/************************************************************************************
 * @name       : BUFFER_Push
 * @function   : Adds a new data element to the circular buffer.
 * @parameters : CircularBufferTypeDef* buff - Pointer to the buffer structure.
 *               uint8_t data - The data byte to push into the buffer.
 * @retvalue   : uint8_t - Returns 1 if the push was successful, 0 if the buffer is full.
 ************************************************************************************/
uint8_t BUFFER_Push(CircularBufferTypeDef* buff, uint8_t data)
{
	// Check if buffer is full
	if (buff->size >= buff->capacity)
	{
			return 0;  // Buffer is full, cannot push data
	}

	// Write data to the buffer
	buff->data[buff->writeIndex] = data;

	// Move the write index forward and wrap around if necessary
	buff->writeIndex = (buff->writeIndex + 1) % buff->capacity;

	// Increase buffer size
	buff->size++;

	return 1;  // Push successful
}

/************************************************************************************
 * @name       : BUFFER_PopData
 * @function   : Removes a data element from the buffer and stores it in *data.
 * @parameters : CircularBufferTypeDef* buff - Pointer to the buffer structure.
 *               uint8_t* data - Pointer where the popped value will be stored.
 * @retvalue   : uint8_t - Returns 1 if data was successfully popped, 0 if the buffer is empty.
 ************************************************************************************/
uint8_t BUFFER_PopData(CircularBufferTypeDef* buff, uint8_t* data)
{
	// Check if buffer is empty
	if (buff->size == 0) 
	{
			return 0;  // No data to pop
	}

	// Retrieve data from the buffer
	*data = buff->data[buff->readIndex];

	// Move the read index forward and wrap around if necessary
	buff->readIndex = (buff->readIndex + 1) % buff->capacity;

	// Decrease buffer size
	buff->size--;

	return 1;  // Pop successful
}

/************************************************************************************
 * @name       : BUFFER_PopAllData
 * @function   : Removes all data elements from the buffer and stores them in a clip.
 * @parameters : CircularBufferTypeDef* buff - Pointer to the buffer structure.
 *               BufferClip* clip - Pointer to the clip structure to store the popped data.
 * @retvalue   : uint8_t - Returns 1 if at least one element was extracted, 0 if the buffer is empty.
 ************************************************************************************/
uint8_t BUFFER_PopAllData(CircularBufferTypeDef* buff, BufferClip* clip)
{
	// Check if buffer is empty
	if (buff->size == 0) 
	{
		return 0;  // No data to pop
	}

	// Clear clip memory (optional)
	memset(clip->data, 0x00, clip->size);

	// Extract as many elements as possible
	clip->length = 0;
	while (buff->size > 0 && clip->length < clip->size)
	{
		clip->data[clip->length++] = buff->data[buff->readIndex];
		buff->readIndex = (buff->readIndex + 1) % buff->capacity;
		buff->size--;
	}

	return 1;  // Pop successful
}