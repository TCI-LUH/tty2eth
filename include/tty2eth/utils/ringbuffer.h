#pragma once

#include <stddef.h>
#include <stdbool.h>
#include <string.h>

typedef struct
{
    char* buffer;
    size_t capacity;
    size_t start;
    size_t end;
} Ring;

typedef struct
{
    char* ptr[2];
    size_t size[2];
} RingSlice;


void ring_init(Ring* ring, char* buffer, size_t size);
size_t ring_size(Ring* ring);
bool ring_isfull(Ring* ring);
bool ring_isempty(Ring* ring);
bool ring_append(Ring* ring, char* data, size_t size);
RingSlice ring_peek(Ring* ring, size_t size);
RingSlice ring_consume(Ring* ring, size_t size);