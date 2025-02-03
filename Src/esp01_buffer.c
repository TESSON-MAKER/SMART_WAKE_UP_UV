#include <string.h>
#include "../Inc/esp01_buffer.h"

/*******************************************************************
 * @name       : BUFFER_ResetCircular
 * @function   : Resets the circular buffer by setting indices to 
								 0 and clearing content.
 * @param      : CircularBufferTypeDef* buff - Pointer to buffer.
 * @return     : None
 *******************************************************************/
static void BUFFER_ResetCircular(CircularBufferTypeDef* buff)
{
	buff->readIndex = 0;
	buff->writeIndex = 0;
	memset(buff->data, 0, buff->capacity);
}

/*******************************************************************
 * @name       : BUFFER_ResetClip
 * @function   : Clears a clip buffer.
 * @param      : ClipBufferTypeDef* clip - Pointer to buffer.
 * @return     : None
 *******************************************************************/
static void BUFFER_ResetClip(ClipBufferTypeDef* clip)
{
	memset(clip->data, 0, clip->capacity);
}

/*******************************************************************
 * @name       : BUFFER_CircularInit
 * @function   : Initializes a circular buffer.
 * @param      : CircularBufferTypeDef* buff - Pointer to buffer.
 *               uint8_t* bufferArray - Data array.
 *               uint16_t capacity - Buffer size.
 * @return     : None
 *******************************************************************/
void BUFFER_CircularInit(CircularBufferTypeDef* buff, uint8_t* bufferArray, uint16_t capacity)
{
	buff->data = bufferArray;
	buff->capacity = capacity;
	BUFFER_ResetCircular(buff);
}

/*******************************************************************
 * @name       : BUFFER_ClipInit
 * @function   : Initializes a clip buffer.
 * @param      : ClipBufferTypeDef* clip - Pointer to buffer.
 *               uint8_t* clipArray - Data array.
 *               uint16_t capacity - Buffer size.
 * @return     : None
 *******************************************************************/
void BUFFER_ClipInit(ClipBufferTypeDef* clip, uint8_t* clipArray, uint16_t capacity)
{
	clip->data = clipArray;
	clip->capacity = capacity;
	BUFFER_ResetClip(clip);
}

/*******************************************************************
 * @name       : BUFFER_GetLength
 * @function   : Returns the number of elements in the buffer.
 * @param      : CircularBufferTypeDef* buff - Pointer to buffer.
 * @return     : uint16_t - Elements count.
 *******************************************************************/
uint16_t BUFFER_GetLength(CircularBufferTypeDef* buff)
{
	if (buff->writeIndex >= buff->readIndex) 
	{
		return buff->writeIndex - buff->readIndex;
	} 
	return (buff->capacity - buff->readIndex) + buff->writeIndex; 
}

/*******************************************************************
 * @name       : BUFFER_Push
 * @function   : Adds an element to the buffer.
 * @param      : CircularBufferTypeDef* buff - Pointer to buffer.
 *               uint8_t data - Byte to push.
 * @return     : uint8_t - 1 if full, 0 if success.
 *******************************************************************/
uint8_t BUFFER_Push(CircularBufferTypeDef* buff, uint8_t data)
{
	buff->data[buff->writeIndex] = data;
	buff->writeIndex = (buff->writeIndex + 1) % buff->capacity;
	return (buff->writeIndex == buff->readIndex) ? 1 : 0;
}

/*******************************************************************
 * @name       : BUFFER_PopData
 * @function   : Retrieves and removes an element.
 * @param      : CircularBufferTypeDef* buff - Pointer to buffer.
 *               uint8_t* data - Storage for retrieved byte.
 * @return     : uint8_t - 0 if success, 1 if empty.
 *******************************************************************/
uint8_t BUFFER_PopData(CircularBufferTypeDef* buff, uint8_t* data)
{
	if (buff->readIndex == buff->writeIndex) return 1; 
	*data = buff->data[buff->readIndex];
	buff->readIndex = (buff->readIndex + 1) % buff->capacity;
	return 0;
}

/*******************************************************************
 * @name       : BUFFER_PopAllData
 * @function   : Extracts all data into a clip.
 * @param      : CircularBufferTypeDef* buff - Pointer to buffer.
 *               ClipBufferTypeDef* clip - Storage for data.
 * @return     : uint8_t - 0 if success, 1 if empty.
 *******************************************************************/
uint8_t BUFFER_PopAllData(CircularBufferTypeDef* buff, ClipBufferTypeDef* clip)
{
	BUFFER_ResetClip(clip);
	if (buff->readIndex == buff->writeIndex) return 1; 

	int i = 0;
	while (buff->readIndex != buff->writeIndex && i < clip->capacity) 
	{
		clip->data[i++] = buff->data[buff->readIndex];
		buff->readIndex = (buff->readIndex + 1) % buff->capacity;
	}
	return (i > 0) ? 0 : 1;
}
