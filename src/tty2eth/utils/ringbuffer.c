#include "utils/ringbuffer.h"

#include <sys/param.h>


void ring_init(Ring* ring, char* buffer, size_t size)
{
    *ring = (Ring){
        .buffer = buffer,
        .capacity = size,
        .start = 0,
        .end = 0,
    };
}

size_t ring_size(Ring* ring)
{
    return (ring->end - ring->start + ring->capacity) % ring->capacity;
}

bool ring_isfull(Ring* ring)
{
    return ring_size(ring) == ring->capacity - 1;
}

bool ring_isempty(Ring* ring)
{
    return ring->end == ring->start;
}

bool ring_append(Ring* ring, char* data, size_t size)
{
    size_t free = ring->capacity - ring_size(ring);
    if(free == 1 || free < size)
        return false;
    
    size_t n = MIN(size, ring->capacity - ring->end);
    memcpy(&ring->buffer[ring->end], data, n);
    if(n < size)
        memcpy(&ring->buffer[0], &data[n], size - n);
    ring->end = (ring->end + size) % ring->capacity;
    return true;
}

RingSlice ring_peek(Ring* ring, size_t size)
{
    RingSlice result = {0};
    size_t len = ring_size(ring);
    len = MIN(len, size);
    
    result.ptr[0] = &ring->buffer[ring->start];
    result.size[0] = MIN(len, ring->capacity - ring->start);
    if(result.size[0] < len)
    {
        result.ptr[1] = &ring->buffer[0];
        result.size[1] = len - result.size[0];
    }
    return result;
}

RingSlice ring_consume(Ring* ring, size_t size)
{
    RingSlice result = ring_peek(ring, size);
    ring->start = (ring->start + result.size[0] + result.size[1]) % ring->capacity;
}