#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>
#include <stddef.h>

// Circular buffer structure
typedef struct {
    uint8_t* data;        // Pointer to buffer memory (array of bytes)
    uint16_t capacity;    // Total buffer size (maximum number of elements)
    uint16_t size;        // Current number of elements in the buffer
    uint16_t readIndex;   // Index of the element to be read
    uint16_t writeIndex;  // Index of the element to be written
} CircularBufferTypeDef;

// Structure to store extracted data from the buffer
typedef struct {
    uint8_t* data;  // Pointer to the extracted data (array of bytes)
    uint8_t size;   // Maximum capacity of the buffer (number of elements it can hold)
    uint8_t length; // Current number of elements stored
} BufferClip;

// Function prototypes
void BUFFER_Reset(CircularBufferTypeDef* buff);
uint16_t BUFFER_Length(CircularBufferTypeDef* buff);
uint8_t BUFFER_Push(CircularBufferTypeDef* buff, uint8_t data);
uint8_t BUFFER_Pop(CircularBufferTypeDef* buff, uint8_t* data);
uint8_t BUFFER_PopAllData(CircularBufferTypeDef* buff, BufferClip* clip);

#endif
