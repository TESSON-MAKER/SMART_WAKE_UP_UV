#include "../Inc/esp01_buffer.h"
#include <string.h>

/***********************************************************************************************
 * @name       : BUFFER_ResetCircular
 * @function   : Clears all data in the circular buffer.
 * @parameters : CircularBufferTypeDef* buff - Pointer to the circular buffer structure.
 * @retvalue   : None
 ***********************************************************************************************/
static void BUFFER_ResetCircular(CircularBufferTypeDef* buff) 
{
    buff->readIndex = 0;
    buff->writeIndex = 0;
    buff->size = 0;
    memset(buff->data, 0, buff->capacity);
}

/***********************************************************************************************
 * @name       : BUFFER_ResetClip
 * @function   : Clears all data in a clip buffer.
 * @parameters : ClipBufferTypeDef* clip - Pointer to the clip buffer structure.
 * @retvalue   : None
 ***********************************************************************************************/
void BUFFER_ResetClip(ClipBufferTypeDef* clip) 
{
    clip->size = 0;
    memset(clip->data, 0, clip->capacity);
}

/***********************************************************************************************
 * @name       : BUFFER_CircularInit
 * @function   : Initializes the circular buffer with a data array and a given capacity.
 * @parameters : CircularBufferTypeDef* buff - Pointer to the circular buffer structure.
 *               uint8_t* bufferArray - Pointer to the data array used by the buffer.
 *               uint16_t capacity - Maximum capacity of the buffer.
 * @retvalue   : None
 ***********************************************************************************************/
void BUFFER_CircularInit(CircularBufferTypeDef* buff, uint8_t* bufferArray, uint16_t capacity) 
{
    buff->data = bufferArray;
    buff->capacity = capacity;
    BUFFER_ResetCircular(buff); // Resets the indices and clears the buffer
}

/***********************************************************************************************
 * @name       : BUFFER_ClipInit
 * @function   : Initializes the clip buffer with a data array and a given capacity.
 * @parameters : ClipBufferTypeDef* clip - Pointer to the clip buffer structure.
 *               uint8_t* clipArray - Pointer to the data array used by the buffer.
 *               uint16_t capacity - Maximum capacity of the buffer.
 * @retvalue   : None
 ***********************************************************************************************/
void BUFFER_ClipInit(ClipBufferTypeDef* clip, uint8_t* clipArray, uint16_t capacity) 
{
    clip->data = clipArray;
    clip->capacity = capacity;
    BUFFER_ResetClip(clip);
}

/***********************************************************************************************
 * @name       : BUFFER_Length
 * @function   : Returns the current size of the circular buffer.
 * @parameters : CircularBufferTypeDef* buff - Pointer to the circular buffer structure.
 * @retvalue   : uint16_t - Current size of the buffer.
 ***********************************************************************************************/
uint16_t BUFFER_Length(CircularBufferTypeDef* buff) 
{
    return buff->size;
}

/***********************************************************************************************
 * @name       : BUFFER_Write
 * @function   : Writes a byte of data into the circular buffer with overwrite capability.
 * @parameters : CircularBufferTypeDef* buff - Pointer to the circular buffer structure.
 *               uint8_t data - Data byte to be written.
 * @retvalue   : uint8_t - Always returns 1 as data will be written even if buffer is full.
 ***********************************************************************************************/
uint8_t BUFFER_Write(CircularBufferTypeDef* buff, uint8_t data)
{
    if (buff->size >= buff->capacity) 
    {
        // Overwrite: Move readIndex forward to drop the oldest data
        buff->readIndex = (buff->readIndex + 1) % buff->capacity;
        buff->size--; // Maintain correct size tracking
    }
    
    buff->data[buff->writeIndex] = data;
    buff->writeIndex = (buff->writeIndex + 1) % buff->capacity;
    buff->size++;
    return 0;
}

/***********************************************************************************************
 * @name       : BUFFER_PopData
 * @function   : Pops a single byte of data from the circular buffer.
 * @parameters : CircularBufferTypeDef* buff - Pointer to the circular buffer structure.
 *               uint8_t* data - Pointer to store the popped data.
 * @retvalue   : uint8_t - 1 if successful, 0 if buffer is empty.
 ***********************************************************************************************/
uint8_t BUFFER_PopData(CircularBufferTypeDef* buff, uint8_t* data) 
{
    if (buff->size == 0) return 1; // Buffer empty
    *data = buff->data[buff->readIndex];
    buff->readIndex = (buff->readIndex + 1) % buff->capacity;
    buff->size--;
    return 0;
}

/***********************************************************************************************
 * @name       : BUFFER_PopAllData
 * @function   : Pops all data from the circular buffer into a clip buffer.
 * @parameters : CircularBufferTypeDef* buff - Pointer to the circular buffer structure.
 *               ClipBufferTypeDef* clip - Pointer to the clip buffer to store the extracted data.
 * @retvalue   : uint8_t - 0 if successful, 1 if buffer is empty.
 ***********************************************************************************************/
uint8_t BUFFER_PopAllData(CircularBufferTypeDef* buff, ClipBufferTypeDef* clip) 
{
    if (buff->size == 0) 
    {
        clip->size = 0;
        return 1; // Buffer empty
    }

    uint16_t count;
    if (buff->size > clip->capacity) count = clip->capacity;
    else count = buff->size;
    
    uint16_t i = 0;
    while (i < count) 
    {
        clip->data[i++] = buff->data[buff->readIndex];
        buff->readIndex = (buff->readIndex + 1) % buff->capacity;
    }

    buff->size -= count;
    clip->size = count;
    return 0;
}