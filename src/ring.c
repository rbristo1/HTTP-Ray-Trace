#include "ring.h"
#include <string.h>
// Write your ring buffer functions here
struct RingBuffer
{
    size_t at;
    size_t size;
    size_t capacity;
    char *buffer;
};

RingBuffer *rb_new(size_t capacity) {
    RingBuffer * rb = malloc(sizeof(RingBuffer));
    rb->buffer = calloc(capacity, sizeof(char));
    rb->at = 0;
    rb->size = 0;
    rb->capacity = capacity;
    return rb;
}
void rb_free(RingBuffer *rb) {
    free(rb->buffer);
    free(rb);
}

// Ring buffer components
size_t  rb_at(const RingBuffer *rb) {
    return rb->at;
}
size_t  rb_size(const RingBuffer *rb) {
    return rb->size;
}
size_t  rb_capacity(const RingBuffer *rb) {
    return rb->capacity;
}
bool    rb_push(RingBuffer *rb, char data) {
    if (rb->size == rb->capacity) {
        return false;
    }
    else {
        rb->buffer[(rb->at + rb->size)%rb->capacity] = data;
        rb->size += 1;
        return true;
    }
}
bool    rb_pop(RingBuffer *rb, char *data) {
    if (rb->size == 0) {
        return false;
    }
    else if (data != NULL) {
        memcpy(data, &rb->buffer[rb->at], 1);
        /*for (size_t i = rb->at; i < rb->at+rb->size; i++) {
            rb->buffer[i] = rb->buffer[i+1];
        }*/
        //data = &rb->buffer[rb->at];
        rb->at += 1;
        rb->size -= 1;
        rb->at %= rb->capacity;
    }
    else {
        /*for (size_t i = rb->at; i < rb->at+rb->size; i++) {
            rb->buffer[i] = rb->buffer[i+1];
        }*/
        rb->at += 1;
        rb->at %= rb->capacity;
        rb->size -= 1;
    }
    return true;
    
}
char    rb_peek(const RingBuffer *rb) {
    return rb->buffer[rb->at];   
}
void    rb_ignore(RingBuffer *rb, size_t num) {
    size_t num2;
    if (num < rb->size) {
        num2 = num;
    }
    else {
        num2 = rb->size;
    }
    rb->at += num2;
    rb->at %= rb->capacity;
    rb->size -= num2;
}
void    rb_clear(RingBuffer *rb) {
    rb->at = 0;
    rb->size = 0;
}

size_t  rb_read(RingBuffer *rb, char *buf, size_t max_bytes) {
    if (rb->size < max_bytes) {
        max_bytes = rb->size;
    }

    size_t first, second;

    first = rb->capacity-rb->at;
    if (max_bytes < first) {
        first = max_bytes;
    }
    second = max_bytes-first;
    
    memcpy(buf, &rb->buffer[rb->at], first);
    if (second > 0) {
        memcpy(buf + first, &rb->buffer[0], second);
    }
    rb->at = (rb->at + max_bytes) % rb->capacity;
    rb->size = rb->size - max_bytes;
    return max_bytes;
}
size_t  rb_write(RingBuffer *rb, const char *buf, size_t bytes) {
    if (rb->capacity-rb->size < bytes) {
        bytes = rb->capacity - rb->size;
    }
    size_t first = rb->capacity-((rb->at+rb->size)%rb->capacity);
    if (bytes < first) {
        first = bytes;
    }
    size_t second = bytes-first;
    memcpy(&rb->buffer[(rb->at+rb->size)%rb->capacity], buf, first);
    if (second > 0) {
        memcpy(&rb->buffer[(rb->at+rb->size+first) % rb->capacity], buf+first, second);
    }
    //rb->at += bytes;
    rb->size += bytes;
    //rb->at %= rb->capacity;
    
    return bytes;
}