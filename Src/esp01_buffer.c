#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Inc/esp01_buffer.h"

/************************************************************************************
 * @name       : Buffer_Reset
 * @function   : Resets the circular buffer by setting the read and write indices to 0.
 * @parameters : CircularBufferTypeDef* buff - Pointer to the buffer structure.
 * @retvalue   : None
 ************************************************************************************/
void Buffer_Reset(CircularBufferTypeDef* buff)
{
  // Reset both read and write indices to the start of the buffer
  buff->readIndex = 0;
  buff->writeIndex = 0;
}

/************************************************************************************
 * @name       : Buffer_Length
 * @function   : Calculates the current length of data in the circular buffer.
 * @parameters : CircularBufferTypeDef* buff - Pointer to the buffer structure.
 * @retvalue   : uint16_t - The current number of elements in the buffer.
 ************************************************************************************/
uint16_t Buffer_Length(CircularBufferTypeDef* buff)
{
  // If the buffer hasn't wrapped around, length is simply the difference
  if (buff->writeIndex >= buff->readIndex) 
  {
    uint16_t normalCase = buff->writeIndex - buff->readIndex;
    return normalCase;  // Normal case
  }

  // If the buffer has wrapped around, account for the wrap
  uint16_t wrapAroundCase = (buff->capacity - buff->readIndex) + buff->writeIndex;
  return wrapAroundCase; // Wrap-around case
}

/************************************************************************************
 * @name       : Buffer_Push
 * @function   : Adds a new data element to the circular buffer.
 * @parameters : CircularBufferTypeDef* buff - Pointer to the buffer structure.
 *               uint8_t data - The data byte to push into the buffer.
 * @retvalue   : uint8_t - Returns 1 if the push was successful, 0 if the data was overwritten.
 ************************************************************************************/
uint8_t BUFFER_Push(CircularBufferTypeDef* buff, uint8_t data)
{
  // Write the new data at the current write position
  buff->data[buff->writeIndex] = data;

  // Move the write index to the next position
  buff->writeIndex++;

  // Wrap around the write index if it exceeds the buffer capacity
  if (buff->writeIndex >= buff->capacity)
  {
    buff->writeIndex = 0;
  }

  // If the buffer is full (read index equals write index), overwrite the oldest data
  if (buff->readIndex == buff->writeIndex)
  {
    // Move the read index forward to discard the oldest data
    buff->readIndex = (buff->readIndex + 1) % buff->capacity;
    return 0;  // Indicate that data was overwritten
  } 
  
  return 1;  // Indicate successful push
}

/************************************************************************************
 * @name       : Buffer_Pop
 * @function   : Removes a data element from the readIndex of the buffer.
 * @parameters : CircularBufferTypeDef* buff - Pointer to the buffer structure.
 *               uint8_t* data - Pointer to the data byte where the popped value will be stored.
 * @retvalue   : uint8_t - Returns 1 if data was successfully popped, 0 if the buffer is empty.
 ************************************************************************************/
uint8_t Buffer_Pop(CircularBufferTypeDef* buff, uint8_t* data)
{
  // If the buffer is empty (readIndex == writeIndex), no data can be popped
  if (buff->readIndex == buff->writeIndex) return 0;

  // Retrieve the data at the readIndex of the buffer
  *data = buff->data[buff->readIndex];

  // Move the readIndex index forward to discard the popped data
  buff->readIndex = (buff->readIndex + 1) % buff->capacity;

  return 1;  // Indicate successful pop
}

/************************************************************************************
 * @name       : Buffer_Pop_All
 * @function   : Removes all data elements from the buffer and stores them in a clip.
 * @parameters : CircularBufferTypeDef* buff - Pointer to the buffer structure.
 *               BufferClip* clip - Pointer to the clip structure to store the popped data.
 * @retvalue   : uint8_t - Returns 1 if all data was successfully popped, 0 if the buffer is empty.
 ************************************************************************************/
uint8_t Buffer_Pop_All(CircularBufferTypeDef* buff, BufferClip* clip)
{
  // If the buffer is empty (readIndex == writeIndex), no data can be popped
  if (buff->readIndex == buff->writeIndex) return 0;
  
  // Clear the clip data and reset its length
  memset(clip->data, 0x00, clip->size * sizeof(uint8_t));
  clip->length = 0;

  // If the readIndex index is greater than the writeIndex (buffer wrapped around)
  if (buff->readIndex > buff->writeIndex) 
  {
    // Copy data from the readIndex to the end of the buffer
    while (buff->readIndex < buff->capacity && clip->length < clip->size) 
    {
      clip->data[clip->length++] = buff->data[buff->readIndex++];
    }

    // Reset the readIndex index if it reached the buffer end
    if (buff->readIndex == buff->capacity) 
    {
      buff->readIndex = 0;
    }
  }

  // Copy the remaining data from the readIndex to the writeIndex
  while (buff->readIndex < buff->writeIndex && clip->length < clip->size) 
  {
    clip->data[clip->length++] = buff->data[buff->readIndex++];
  }

  return 1;  // Indicate successful pop
}


