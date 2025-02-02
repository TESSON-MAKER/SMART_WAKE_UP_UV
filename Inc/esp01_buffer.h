#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>

typedef struct
{
  uint8_t* data;        // Pointer to the buffer memory (array of bytes)
  uint16_t capacity;    // Total size of the buffer (maximum number of elements)
  uint16_t readIndex;   // Index of the element to be read (read position)
  uint16_t writeIndex;  // Index of the element to be written (write position)
} CircularBufferTypeDef;

typedef struct
{
  uint8_t* data;  // Pointer to the buffer's data (array of bytes)
  uint8_t size;   // Maximum capacity of the buffer (number of elements it can hold)
  uint8_t length; // Current number of elements in the buffer
} BufferClip;

void Buffer_Reset(CircularBufferTypeDef* buff);
uint16_t Buffer_Length(CircularBufferTypeDef* buff);
uint8_t BUFFER_Push(CircularBufferTypeDef* buff, uint8_t data);
uint8_t Buffer_Pop(CircularBufferTypeDef* buff, uint8_t* data);
uint8_t Buffer_Pop_All(CircularBufferTypeDef* buff, BufferClip* clip);


#endif