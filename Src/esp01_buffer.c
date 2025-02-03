#include <string.h>
#include "../Inc/esp01_buffer.h"

// Reset circular buffer (clear data, reset indices)
static void BUFFER_ResetCircular(CircularBufferTypeDef* buff)
{
    buff->readIndex = 0;
    buff->writeIndex = 0;
    memset(buff->data, 0, buff->capacity);
}

// Reset clip buffer (clear data)
static void BUFFER_ResetClip(ClipBufferTypeDef* clip)
{
    memset(clip->data, 0, clip->capacity);
}

// Initialize circular buffer with given data array and capacity
void BUFFER_CircularInit(CircularBufferTypeDef* buff, uint8_t* bufferArray, uint16_t capacity)
{
    buff->data = bufferArray;
    buff->capacity = capacity;
    BUFFER_ResetCircular(buff);
}

// Initialize clip buffer with given data array and capacity
void BUFFER_ClipInit(ClipBufferTypeDef* clip, uint8_t* clipArray, uint16_t capacity)
{
    clip->data = clipArray;
    clip->capacity = capacity;
    BUFFER_ResetClip(clip);
}

// Get number of elements in circular buffer
uint16_t BUFFER_GetLength(CircularBufferTypeDef* buff)
{
    return (buff->writeIndex >= buff->readIndex) ?
           (buff->writeIndex - buff->readIndex) :
           (buff->capacity - buff->readIndex + buff->writeIndex);
}

// Push data into circular buffer (returns 1 if full, 0 on success)
uint8_t BUFFER_Push(CircularBufferTypeDef* buff, uint8_t data)
{
    buff->data[buff->writeIndex] = data;
    buff->writeIndex = (buff->writeIndex + 1) % buff->capacity;
    return (buff->writeIndex == buff->readIndex) ? 1 : 0;
}

// Pop data from circular buffer (returns 1 if empty, 0 on success)
uint8_t BUFFER_PopData(CircularBufferTypeDef* buff, uint8_t* data)
{
    if (buff->readIndex == buff->writeIndex) return 1;
    *data = buff->data[buff->readIndex];
    buff->readIndex = (buff->readIndex + 1) % buff->capacity;
    return 0;
}

// Extract all data from circular buffer into clip buffer (returns 1 if empty, 0 otherwise)
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
